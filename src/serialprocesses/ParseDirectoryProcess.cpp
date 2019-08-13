/*
 * Copyright (C) 2019 ram
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   ParseDirectoryProcess.cpp
 * Author: ram
 * 
 * Created on August 8, 2019, 7:57 PM
 */

#include "ParseDirectoryProcess.h"
#include "Database.h"
#include "ParseDirectory.h"
#include "ParseDirectoryGame.h"
#include "Game.h"
#include "Utils.h"
#include "Platform.h"
#include "thegamesdb.h"
#include "SerialProcessExecutor.h"
#include "Developer.h"
#include "GameDeveloper.h"
#include "Genre.h"
#include "GameGenre.h"
#include "Publisher.h"
#include "GamePublisher.h"
#include "GameImage.h"

//ln -s /usr/include/libxml2/libxml/ /usr/include/libxml
#include <libxml/parser.h>
#include <dirent.h>
#include <cstring>
#include <iostream>

const string ParseDirectoryProcess::TYPE = "ParseDirectoryProcess";


ParseDirectoryProcess::ParseDirectoryProcess(void *requester, void (*statusCallback)(void *, void*)) : SerialProcess(TYPE, requester, statusCallback)
{
    parseDirectory = NULL;
    parseDirectoryGames = NULL;
}

ParseDirectoryProcess::~ParseDirectoryProcess()
{
    if(parseDirectory)
    {
        delete parseDirectory;
    }
    
    if(parseDirectoryGames)
    {
        ParseDirectoryGame::releaseItems(parseDirectoryGames);
    }
}

int ParseDirectoryProcess::execute()
{
    status = STATUS_RUNNING;
        
    sqlite3 *sqlite = Database::getInstance()->acquire();
    parseDirectory = ParseDirectory::getPengingItem(sqlite);
    Database::getInstance()->release();
    
    if(parseDirectory)
    {
        postStatus(parseDirectory->getDirectory());
        
        parseDirectory->setStart(Utils::getInstance()->nowIsoDateTime());        
        sqlite = Database::getInstance()->acquire();
        parseDirectory->save(sqlite);
        Database::getInstance()->release();
    
        parseGameFiles(Utils::getInstance()->strSplitByWhiteSpace(parseDirectory->getFileExtensions()), parseDirectory->getDirectory());
        
        sqlite = Database::getInstance()->acquire();
        parseDirectoryGames = ParseDirectoryGame::getPendingItems(sqlite, parseDirectory->getId());
        Database::getInstance()->release();
        
        parseDirectoryGamesIndex = 0;        
        fetchGameInformation();
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    
    return status;
}

ParseDirectory* ParseDirectoryProcess::getParseDirectory()
{
    return parseDirectory;
}


void ParseDirectoryProcess::parseGameFiles(list<string> extensions, string directory)
{
    DIR *dir;
    struct dirent *entry;
    
    if(directory.rfind("/") == string::npos || directory.rfind("/") != directory.length() - 1)
    {
        directory += "/";
    }
    
    if((dir = opendir(directory.c_str())))
    {
        while ((entry = readdir(dir)))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            
            if(entry->d_type == DT_REG)
            {
                string fileName = string(entry->d_name);                
                
                for(list<string>::iterator extension = extensions.begin(); extension != extensions.end(); extension++)
                {
                    if(Utils::getInstance()->strToLowerCase(fileName).find(*extension) != string::npos)
                    {
                        sqlite3 *sqlite = Database::getInstance()->acquire();
                        Game *game = Game::getGameWithFileName(sqlite, parseDirectory->getPlatformId(), directory + fileName);
                        Database::getInstance()->release();
                        
                        if(game)
                        {
                            delete game;
                            break;
                        }
                        
                        ParseDirectoryGame *parseDirectoryGame = ParseDirectoryGame::getItem(sqlite, parseDirectory->getId(), directory + fileName);
                        if(!parseDirectoryGame)
                        {
                            parseDirectoryGame = new ParseDirectoryGame((int64_t)0);
                            parseDirectoryGame->setParseDirectoryId(parseDirectory->getId());
                            parseDirectoryGame->setFileName(directory + fileName);
                            parseDirectoryGame->setProcessed(0);
                            parseDirectoryGame->setTimestamp(Utils::getInstance()->nowIsoDateTime());
                            parseDirectoryGame->setName(cleanName(fileName, *extension));

                            if(parseDirectory->getUseMame())
                            {
                                parseDirectoryGame->setMameName(cleanName(queryMame(parseDirectoryGame->getName()), ""));
                            }
                            else
                            {
                                parseDirectoryGame->setMameName("");
                            }

                            sqlite = Database::getInstance()->acquire();
                            parseDirectoryGame->save(sqlite);
                            Database::getInstance()->release();
                        }
                        
                        postStatus(parseDirectoryGame->getFileName());
                        
                        delete parseDirectoryGame;
                        break;
                    }
                }
            }
            else if(entry->d_type == DT_DIR)
            {
                parseGameFiles(extensions, directory + string(entry->d_name));
            }
        }        
        closedir(dir);
    }
}

string ParseDirectoryProcess::cleanName(string rawName, string extension)
{
    if(extension.length() > 0)
    {
        rawName.replace(rawName.length() - extension.length(), extension.length(), "");
    }
    
    string name = "";
    int open = 0;
    for(unsigned int c = 0; c < rawName.length(); c++)
    {
        char letter = rawName.c_str()[c];
        
        if(letter == '(' || letter == '[' || letter == '{')
        {
            open++;
        }
        else if(open > 0 && (letter == ')' || letter == ']' || letter == '}'))
        {
            open--;
        }                
        else if(!open)
        {
            name += letter;
        }
    }
    
    name = Utils::getInstance()->strReplace(name, "!", "");
    name = Utils::getInstance()->strReplace(name, "?", "");

    return Utils::getInstance()->trim(name);
}

string ParseDirectoryProcess::queryMame(string name)
{    
    string mameName = "";
    string mameOutput = "";
    string command = parseDirectory->getMame() + " -lx " + name;
    cout << "ParseDirectoryProcess::" << __FUNCTION__ << " command: " << command << endl;
    
    if(!Utils::getInstance()->executeApplication(command, &mameOutput))
    {
        //cout << "ParseDirectoryProcess::" << __FUNCTION__ << " mameOutput: " << mameOutput << endl;
        
        xmlDoc *xmlDocument = xmlParseMemory(mameOutput.c_str(), mameOutput.length());
        if(xmlDocument)
        {
            xmlNode *rootNode = xmlDocGetRootElement(xmlDocument);
            for (xmlNode *node = rootNode->children; node; node = node->next)
            {
                cout << "ParseDirectoryProcess::" << __FUNCTION__ << " node->name: " << node->name << endl;
                if (node->type == XML_ELEMENT_NODE && strcmp((const char *)node->name, "machine") == 0)
                {
                    char *propertyName = (char *)xmlGetProp(node, (const xmlChar *)"name");
                    cout << "ParseDirectoryProcess::" << __FUNCTION__ << " propertyName: " << propertyName << endl;
                    
                    if(name.compare(string(propertyName)) == 0)
                    {
                        for (xmlNode *gameNode = node->children; gameNode; gameNode = gameNode->next)
                        {
                            if (gameNode->type == XML_ELEMENT_NODE && strcmp((const char *)gameNode->name, "description") == 0)
                            {
                                char *content = (char *)xmlNodeGetContent(gameNode);
                                mameName = string(content);
                                xmlFree(content);
                                
                                break;
                            }
                        }
                        
                        break;
                    }                    
                }
            }
            
            xmlFreeDoc(xmlDocument);
        }
    }
    
    return mameName;
}

list<string> ParseDirectoryProcess::tokenizeName(string name)
{
    string purgedName = Utils::getInstance()->strToLowerCase(name);
    
    purgedName = Utils::getInstance()->strReplace(purgedName, "-", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ":", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "_", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ",", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ";", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ".", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "~", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "'", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "\"", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "!", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "?", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "*", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "+", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "=", "");
    purgedName = Utils::getInstance()->strReplace(purgedName, "(", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ")", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "/", " ");
    
    return Utils::getInstance()->strSplitByWhiteSpace(purgedName);
}

void ParseDirectoryProcess::getGameImagesFromDirectory(Game *game, string name, string directory, int64_t gameImageType)
{
    DIR *dir;
    struct dirent *entry;
    
    if(directory.rfind("/") == string::npos || directory.rfind("/") != directory.length() - 1)
    {
        directory += "/";
    }
    
    if((dir = opendir(directory.c_str())))
    {
        while ((entry = readdir(dir)))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            
            if(entry->d_type == DT_REG)
            {
                string fileName = string(entry->d_name);                
                
                if(fileName.find(name + ".", 0) != string::npos)
                {
                    GameImage *gameImage = new GameImage((int64_t)0);
                    gameImage->setApiId(0);
                    gameImage->setApiItemId(0);
                    gameImage->setGameId(game->getId());
                    gameImage->setType(gameImageType);
                    gameImage->setExternal(0);
                    gameImage->setDownloaded(1);
                    gameImage->setUrl("");
                    
                    sqlite3 *sqlite = Database::getInstance()->acquire();
                    gameImage->save(sqlite);
                    Database::getInstance()->release();
                    
                    string imageFileName = game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId());
                    if(!Utils::getInstance()->copyFile(directory + fileName, imageFileName))
                    {
                        gameImage->setFileName(imageFileName);

                        Utils::getInstance()->scaleImage(gameImage->getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, gameImage->getThumbnailFileName());                        
                    }
                    else
                    {
                        gameImage->setExternal(1);
                        gameImage->setFileName(directory + fileName);
                    }
                    
                    sqlite = Database::getInstance()->acquire();
                    gameImage->save(sqlite);
                    Database::getInstance()->release();
                    
                    delete gameImage;
                }
            }
            else if(entry->d_type == DT_DIR)
            {
                getGameImagesFromDirectory(game, name, directory + string(entry->d_name), gameImageType);
            }
        }
        closedir(dir);
    }
}

void ParseDirectoryProcess::fetchGameInformation()
{
    if(parseDirectoryGamesIndex < parseDirectoryGames->size())
    {
        status = STATUS_RUNNING;
        
        Platform *platform = new Platform(parseDirectory->getPlatformId());
        sqlite3 *sqlite = Database::getInstance()->acquire();
        platform->load(sqlite);
        Database::getInstance()->release();
        
        ParseDirectoryGame *parseDirectoryGame = ParseDirectoryGame::getItem(parseDirectoryGames, parseDirectoryGamesIndex);
    
        string name = parseDirectoryGame->getName();
        if(parseDirectory->getUseMame())
        {
            name = parseDirectoryGame->getMameName();
        }
        TheGamesDB::Elasticsearch::getInstance()->getGames(platform->getApiItemId(), name, this, callbackElasticsearchGames);
        
        int progress = ((double)parseDirectoryGamesIndex / (double)parseDirectoryGames->size()) * 100.0;
        postStatus(platform->getName(), parseDirectoryGame->getFileName(), progress);
        
        delete platform;
    }
    else
    {
        parseDirectory->setEnd(Utils::getInstance()->nowIsoDateTime());
        sqlite3 *sqlite = Database::getInstance()->acquire();
        parseDirectory->save(sqlite);
        Database::getInstance()->release();
        
        status = STATUS_SUCCESS;
        SerialProcessExecutor::getInstance()->finish(this);
    }
}


void ParseDirectoryProcess::callbackElasticsearchGames(void *pParseDirectoryProcess, void *pResult)
{
    ParseDirectoryProcess *parseDirectoryProcess = (ParseDirectoryProcess *)pParseDirectoryProcess;
    TheGamesDB::Elasticsearch::Result_t *result = (TheGamesDB::Elasticsearch::Result_t *)pResult;
    ParseDirectoryGame *parseDirectoryGame = ParseDirectoryGame::getItem(parseDirectoryProcess->parseDirectoryGames, parseDirectoryProcess->parseDirectoryGamesIndex);
    string gameName = parseDirectoryProcess->parseDirectory->getUseMame() ? parseDirectoryGame->getMameName() : parseDirectoryGame->getName();
    list<string> gameNameTokens = parseDirectoryProcess->tokenizeName(gameName);
    Game *game = NULL;
    
    //cout << "ParseDirectoryProcess::" << __FUNCTION__ << " gameName: " << gameName << endl;
    
    typedef struct{
        TheGamesDB::Game *apiGame;
        list<string> nameTokens;
        unsigned int coincidences;
        unsigned int notCoincidences;
    }GameItem_t;
    
    list<TheGamesDB::Game *> *apiGames = (list<TheGamesDB::Game *> *)result->data;
    GameItem_t gameItems[apiGames->size()];
    unsigned int maxCoincidences = 0;
    for(unsigned int index = 0; index < apiGames->size(); index++)
    {
        TheGamesDB::Game *apiGame = TheGamesDB::Game::getItem(apiGames, index);
    
        //cout << "ParseDirectoryProcess::" << __FUNCTION__ << " apiGame: " << apiGame->getName() << endl;
        
        gameItems[index].apiGame = apiGame;
        gameItems[index].nameTokens = parseDirectoryProcess->tokenizeName(apiGame->getName());
        gameItems[index].coincidences = 0;
        gameItems[index].notCoincidences = 0;
        
        
        // Checks the number of words that are in both, the game file name and in the api game name
        if(gameNameTokens.size() > gameItems[index].nameTokens.size())
        {
            for(list<string>::iterator gameNameToken = gameNameTokens.begin(); gameNameToken != gameNameTokens.end(); gameNameToken++)
            {
                int is = 0;
                for(list<string>::iterator gameItemNameToken = gameItems[index].nameTokens.begin(); gameItemNameToken != gameItems[index].nameTokens.end(); gameItemNameToken++)
                {
                    //cout << "ParseDirectoryProcess::" << __FUNCTION__ << " apiGame: " << apiGame->getName() << "    gameNameToken: " << *gameNameToken << "    gameItemNameToken: " << *gameItemNameToken << endl;

                    if((*gameNameToken).compare((*gameItemNameToken)) == 0)
                    {
                        is = 1;
                        break;
                    }
                }

                if(is)
                {
                    gameItems[index].coincidences++;
                    if(gameItems[index].coincidences > maxCoincidences)
                    {
                        maxCoincidences = gameItems[index].coincidences;
                    }
                }
                else
                {
                    gameItems[index].notCoincidences++;
                }
            }            
        }
        else
        {
            for(list<string>::iterator gameItemNameToken = gameItems[index].nameTokens.begin(); gameItemNameToken != gameItems[index].nameTokens.end(); gameItemNameToken++)            
            {
                int is = 0;
                for(list<string>::iterator gameNameToken = gameNameTokens.begin(); gameNameToken != gameNameTokens.end(); gameNameToken++)
                {
                    //cout << "ParseDirectoryProcess::" << __FUNCTION__ << " apiGame: " << apiGame->getName() << "    gameNameToken: " << *gameNameToken << "    gameItemNameToken: " << *gameItemNameToken << endl;

                    if((*gameNameToken).compare((*gameItemNameToken)) == 0)
                    {
                        is = 1;
                        break;
                    }
                }

                if(is)
                {
                    gameItems[index].coincidences++;
                    if(gameItems[index].coincidences > maxCoincidences)
                    {
                        maxCoincidences = gameItems[index].coincidences;
                    }
                }
                else
                {
                    gameItems[index].notCoincidences++;
                }
            }
        }
    }

    if(maxCoincidences > 0)
    {
        GameItem_t *matchGameItem = NULL;
        unsigned int minNotCoincidences = UINT_MAX;
        for(unsigned int index = 0; index < apiGames->size(); index++)
        {
            if(gameItems[index].coincidences == maxCoincidences)
            {
                if(gameItems[index].notCoincidences < minNotCoincidences)
                {
                    minNotCoincidences = gameItems[index].notCoincidences;
                    matchGameItem = &gameItems[index];
                }
            }
        }
        
        if(matchGameItem)
        {
            int matches = 0;
            
            // @TODO Find a more elegant way to validate. Right now it is required 70% of tokens to match
            if(matchGameItem->coincidences > matchGameItem->nameTokens.size())
            {
                if((double)matchGameItem->nameTokens.size() / (double)matchGameItem->coincidences > 0.7)
                {
                    matches = 1;
                }
            }
            else
            {
                if((double)matchGameItem->coincidences / (double)matchGameItem->nameTokens.size() > 0.7)
                {
                    matches = 1;
                }
            }
            
            if(matches)
            {
                cout << "ParseDirectoryProcess::" << __FUNCTION__ << " gameName: " << gameName << "     matchGameItem->apiGame->getName(): " << matchGameItem->apiGame->getName()<< endl;
                
                game = new Game((int64_t)0);
                game->setApiId(TheGamesDB::API_ID);
                game->setApiItemId(matchGameItem->apiGame->getId());
                game->setPlatformId(parseDirectoryProcess->parseDirectory->getPlatformId());
                game->setTimestamp(Utils::getInstance()->nowIsoDateTime());
                game->setName(matchGameItem->apiGame->getName());
                game->setFileName(parseDirectoryGame->getFileName());
                game->setDescription(matchGameItem->apiGame->getDescription());
                game->setEsrbRatingId(matchGameItem->apiGame->getEsrbRatingId());
                game->setReleaseDate(matchGameItem->apiGame->getReleaseDate());
                
                sqlite3 *sqlite = Database::getInstance()->acquire();
                game->save(sqlite);
                Database::getInstance()->release();
                
                list<TheGamesDB::GameGenre *> *apiGameGenres = matchGameItem->apiGame->getGameGenres();
                for(unsigned int index = 0; index < apiGameGenres->size(); index++)
                {
                    TheGamesDB::GameGenre *apiGameGenre = TheGamesDB::GameGenre::getItem(apiGameGenres, index);
                    
                    sqlite = Database::getInstance()->acquire();
                    Genre *genre = Genre::getGenre(sqlite, TheGamesDB::API_ID, apiGameGenre->getGenreId());
                    Database::getInstance()->release();
                    
                    if(genre)
                    {
                        GameGenre *gameGenre = new GameGenre(game->getId(), genre->getId());
                        
                        sqlite = Database::getInstance()->acquire();
                        gameGenre->save(sqlite);
                        Database::getInstance()->release();
                        
                        delete gameGenre;
                        delete genre;
                    }
                }
                
                list<TheGamesDB::GamePublisher *> *apiGamePublishers = matchGameItem->apiGame->getGamePublishers();
                for(unsigned int index = 0; index < apiGamePublishers->size(); index++)
                {
                    TheGamesDB::GamePublisher *apiGamePublisher = TheGamesDB::GamePublisher::getItem(apiGamePublishers, index);
                    
                    sqlite = Database::getInstance()->acquire();
                    Publisher *publisher = Publisher::getPublisher(sqlite, TheGamesDB::API_ID, apiGamePublisher->getPublisherId());
                    Database::getInstance()->release();
                    
                    if(publisher)
                    {
                        GamePublisher *gamePublisher = new GamePublisher(game->getId(), publisher->getId());
                        
                        sqlite = Database::getInstance()->acquire();
                        gamePublisher->save(sqlite);
                        Database::getInstance()->release();
                        
                        delete gamePublisher;
                        delete publisher;
                    }
                }                
                
                list<TheGamesDB::GameDeveloper *> *apiGameDevelopers = matchGameItem->apiGame->getGameDevelopers();
                for(unsigned int index = 0; index < apiGameDevelopers->size(); index++)
                {
                    TheGamesDB::GameDeveloper *apiGameDeveloper = TheGamesDB::GameDeveloper::getItem(apiGameDevelopers, index);
                    
                    sqlite = Database::getInstance()->acquire();
                    Developer *developer = Developer::getDeveloper(sqlite, TheGamesDB::API_ID, apiGameDeveloper->getDeveloperId());
                    Database::getInstance()->release();
                    
                    if(developer)
                    {
                        GameDeveloper *gameDeveloper = new GameDeveloper(game->getId(), developer->getId());
                        
                        sqlite = Database::getInstance()->acquire();
                        gameDeveloper->save(sqlite);
                        Database::getInstance()->release();
                        
                        delete gameDeveloper;
                        delete developer;
                    }
                }
                
                
                int areBoxFrontImages = 0;                
                if(parseDirectoryProcess->parseDirectory->getBoxFrontImagesDirectory().length() > 0)
                {
                    parseDirectoryProcess->getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectoryProcess->parseDirectory->getBoxFrontImagesDirectory(), GameImage::TYPE_BOX_FRONT);
                    
                    sqlite = Database::getInstance()->acquire();
                    list<GameImage *> *boxFrontGameImages = GameImage::getItems(sqlite, game->getId(), GameImage::TYPE_BOX_FRONT);                
                    Database::getInstance()->release();
                    
                    areBoxFrontImages = boxFrontGameImages->size() > 0;
                    GameImage::releaseItems(boxFrontGameImages);
                }
                
                int areBoxBackImages = 0;
                if(parseDirectoryProcess->parseDirectory->getBoxBackImagesDirectory().length() > 0)
                {
                    parseDirectoryProcess->getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectoryProcess->parseDirectory->getBoxBackImagesDirectory(), GameImage::TYPE_BOX_BACK);
                    
                    sqlite = Database::getInstance()->acquire();
                    list<GameImage *> *boxFrontGameImages = GameImage::getItems(sqlite, game->getId(), GameImage::TYPE_BOX_BACK);                
                    Database::getInstance()->release();
                    
                    areBoxBackImages = boxFrontGameImages->size() > 0;
                    GameImage::releaseItems(boxFrontGameImages);
                }
                if(parseDirectoryProcess->parseDirectory->getScreenshotImagesDirectory().length() > 0)
                {
                    parseDirectoryProcess->getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectoryProcess->parseDirectory->getScreenshotImagesDirectory(), GameImage::TYPE_SCREENSHOT);
                }
                if(parseDirectoryProcess->parseDirectory->getLogoImagesDirectory().length() > 0)
                {
                    parseDirectoryProcess->getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectoryProcess->parseDirectory->getLogoImagesDirectory(), GameImage::TYPE_CLEAR_LOGO);
                }
                if(parseDirectoryProcess->parseDirectory->getBannerImagesDirectory().length() > 0)
                {
                    parseDirectoryProcess->getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectoryProcess->parseDirectory->getBannerImagesDirectory(), GameImage::TYPE_BANNER);
                }
                
                
                list<TheGamesDB::GameImage *> *apiGameImages = matchGameItem->apiGame->getGameImages();
                for(unsigned int index = 0; index < apiGameImages->size(); index++)
                {
                    TheGamesDB::GameImage *apiGameImage = TheGamesDB::GameImage::getItem(apiGameImages, index);
                    
                    if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_BOXART) == 0)
                    {
                        if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_FRONT) == 0)
                        {
                            if(!areBoxFrontImages)
                            {
                                GameImage *gameImage = new GameImage((int64_t)0);
                                gameImage->setApiId(TheGamesDB::API_ID);
                                gameImage->setApiItemId(apiGameImage->getId());
                                gameImage->setGameId(game->getId());
                                gameImage->setType(GameImage::TYPE_BOX_FRONT);
                                gameImage->setExternal(0);
                                gameImage->setDownloaded(0);
                                gameImage->setUrl(apiGameImage->getOriginal());

                                // Saves before to generate the id
                                sqlite = Database::getInstance()->acquire();
                                gameImage->save(sqlite);
                                Database::getInstance()->release();
                                
                                gameImage->setFileName(game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId()));
                                
                                sqlite = Database::getInstance()->acquire();
                                gameImage->save(sqlite);
                                Database::getInstance()->release();
                                
                                delete gameImage;
                            }
                        }
                        else if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_BACK) == 0)
                        {
                            if(!areBoxBackImages)
                            {
                                GameImage *gameImage = new GameImage((int64_t)0);
                                gameImage->setApiId(TheGamesDB::API_ID);
                                gameImage->setApiItemId(apiGameImage->getId());
                                gameImage->setGameId(game->getId());
                                gameImage->setType(GameImage::TYPE_BOX_BACK);
                                gameImage->setExternal(0);
                                gameImage->setDownloaded(0);
                                gameImage->setUrl(apiGameImage->getOriginal());
                                
                                // Saves before to generate the id
                                sqlite = Database::getInstance()->acquire();
                                gameImage->save(sqlite);
                                Database::getInstance()->release();
                                
                                gameImage->setFileName(game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId()));
                                
                                sqlite = Database::getInstance()->acquire();
                                gameImage->save(sqlite);
                                Database::getInstance()->release();
                                
                                delete gameImage;
                            }
                        }
                    }
                }                                
            }
        }
    }
    
    
    // If no game data is found, saves it with the basics
    if(!game)
    {
        game = new Game((int64_t)0);
        game->setApiId(0);
        game->setApiItemId(0);
        game->setPlatformId(parseDirectoryProcess->parseDirectory->getPlatformId());
        game->setTimestamp(Utils::getInstance()->nowIsoDateTime());
        game->setName(gameName);
        game->setFileName(parseDirectoryGame->getFileName());

        sqlite3 *sqlite = Database::getInstance()->acquire();
        game->save(sqlite);
        Database::getInstance()->release();
    }
    
    delete game;
    
    
    TheGamesDB::Game::releaseItems(apiGames);
            
    parseDirectoryProcess->parseDirectoryGamesIndex++;
    parseDirectoryProcess->fetchGameInformation();
}
