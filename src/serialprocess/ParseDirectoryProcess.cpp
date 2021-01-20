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
#include "ParseDirectory.h"
#include "ParseDirectoryGame.h"
#include "Game.h"
#include "Utils.h"
#include "Platform.h"
#include "SerialProcessExecutor.h"
#include "Developer.h"
#include "GameDeveloper.h"
#include "Genre.h"
#include "GameGenre.h"
#include "Publisher.h"
#include "GamePublisher.h"
#include "GameImage.h"
#include "Notifications.h"
#include "NotificationManager.h"
#include "Logger.h"
#include "EsrbRating.h"

#include <libxml/parser.h> //For Ubuntu: ln -s /usr/include/libxml2/libxml/ /usr/include/libxml
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <regex>

const string ParseDirectoryProcess::TYPE = "ParseDirectoryProcess";


ParseDirectoryProcess::ParseDirectoryProcess() : SerialProcess(TYPE)
{
    parseDirectory = NULL;
}

ParseDirectoryProcess::~ParseDirectoryProcess()
{    
    if(parseDirectory)
    {
        delete parseDirectory;
    }
}

void ParseDirectoryProcess::execute()
{
    status = STATUS_RUNNING;
        
    parseDirectory = ParseDirectory::getPengingItem();    
    if(parseDirectory)
    {                
        NotificationManager::getInstance()->notify(TYPE, parseDirectory->getDirectory(), status);        
        
        parseDirectory->setStart(Utils::getInstance()->nowIsoDateTime());
        parseDirectory->save();
    
        parseGameFiles(Utils::getInstance()->strSplitByWhiteSpace(parseDirectory->getFileExtensions()), parseDirectory->getDirectory());
        
        list<ParseDirectoryGame *> *parseDirectoryGames = ParseDirectoryGame::getPendingItems(parseDirectory->getId());
        for(unsigned int index = 0; index < parseDirectoryGames->size(); index++)
        {
            ParseDirectoryGame *parseDirectoryGame = ParseDirectoryGame::getItem(parseDirectoryGames, index);
            
            int progress = ((double)index / (double)parseDirectoryGames->size()) * 100.0;
            NotificationManager::getInstance()->notify(TYPE, parseDirectoryGame->getFileName(), status, 0, NULL, progress);
                
            fetch(parseDirectoryGame);
        }
        ParseDirectoryGame::releaseItems(parseDirectoryGames);
                
        parseDirectory->setEnd(Utils::getInstance()->nowIsoDateTime());
        parseDirectory->save();
        
        Platform *platform = new Platform(parseDirectory->getPlatformId());
        platform->load();        
        NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0, 0, platform);
    }
    
    status = STATUS_SUCCESS;
    NotificationManager::getInstance()->notify(TYPE, "", status);
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
                        Game *game = Game::getGameWithFileName(parseDirectory->getPlatformId(), directory + fileName);
                        
                        if(game)
                        {
                            delete game;
                            break;
                        }
                        
                        ParseDirectoryGame *parseDirectoryGame = ParseDirectoryGame::getItem(parseDirectory->getId(), directory + fileName);
                        if(!parseDirectoryGame)
                        {
                            string fileNameWithoutExtension = Utils::getInstance()->getFileNameWithoutExtension(fileName);
                            
                            parseDirectoryGame = new ParseDirectoryGame((int64_t)0);
                            parseDirectoryGame->setParseDirectoryId(parseDirectory->getId());
                            parseDirectoryGame->setFileName(directory + fileName);
                            parseDirectoryGame->setProcessed(0);
                            parseDirectoryGame->setTimestamp(Utils::getInstance()->nowIsoDateTime());
                            parseDirectoryGame->setName(Game::getSanitizedName(fileNameWithoutExtension));

                            if(parseDirectory->getUseMame())
                            {
                                string mameGameName = getMameGameName(fileNameWithoutExtension);
                                string sanitizedMameGameName = Game::getSanitizedName(mameGameName);                                
                                parseDirectoryGame->setMameName(sanitizedMameGameName);
                            }
                            else
                            {
                                parseDirectoryGame->setMameName("");
                            }

                            parseDirectoryGame->save();
                        }                        
                        NotificationManager::getInstance()->notify(TYPE, parseDirectoryGame->getFileName(), status);
                        
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

string ParseDirectoryProcess::getMameGameName(string fileName)
{    
    string mameName = fileName;
    string mameOutput = "";
    string command = parseDirectory->getMame() + " -lx " + fileName;
    
    if(!Utils::getInstance()->executeApplication(command, &mameOutput))
    {
        xmlDoc *xmlDocument = xmlParseMemory(mameOutput.c_str(), mameOutput.length());
        if(xmlDocument)
        {
            xmlNode *rootNode = xmlDocGetRootElement(xmlDocument);
            for (xmlNode *node = rootNode->children; node; node = node->next)
            {
                if (node->type == XML_ELEMENT_NODE && strcmp((const char *)node->name, "machine") == 0)
                {
                    char *propertyName = (char *)xmlGetProp(node, (const xmlChar *)"name");
                    
                    if(fileName.compare(string(propertyName)) == 0)
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
    purgedName = Utils::getInstance()->strReplace(purgedName, "'", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "\"", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "!", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "?", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "*", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "+", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "=", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "(", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, ")", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "/", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "°", " ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "á", "a");
    purgedName = Utils::getInstance()->strReplace(purgedName, "é", "e");
    purgedName = Utils::getInstance()->strReplace(purgedName, "í", "i");
    purgedName = Utils::getInstance()->strReplace(purgedName, "ó", "o");
    purgedName = Utils::getInstance()->strReplace(purgedName, "ú", "u");
    purgedName = Utils::getInstance()->strReplace(purgedName, "&", " and ");
    purgedName = Utils::getInstance()->strReplace(purgedName, "...", " ");
    
    return Utils::getInstance()->strSplitByWhiteSpace(purgedName);
}

int ParseDirectoryProcess::compareNumerals(string token1, string token2)
{
    // @TODO.- Find another way to compare numerals
    if(token1.compare("1") == 0 && token2.compare("i") == 0)
    {
        return 1;
    }
    else if(token1.compare("2") == 0 && token2.compare("ii") == 0)
    {
        return 1;
    }
    else if(token1.compare("3") == 0 && token2.compare("iii") == 0)
    {
        return 1;
    }
    else if(token1.compare("4") == 0 && token2.compare("iv") == 0)
    {
        return 1;
    }
    else if(token1.compare("5") == 0 && token2.compare("v") == 0)
    {
        return 1;
    }
    else if(token1.compare("6") == 0 && token2.compare("vi") == 0)
    {
        return 1;
    }
    else if(token1.compare("7") == 0 && token2.compare("vii") == 0)
    {
        return 1;
    }
    else if(token1.compare("8") == 0 && token2.compare("viii") == 0)
    {
        return 1;
    }
    else if(token1.compare("9") == 0 && token2.compare("ix") == 0)
    {
        return 1;
    }
    else if(token1.compare("10") == 0 && token2.compare("x") == 0)
    {
        return 1;
    }
    else if(token1.compare("11") == 0 && token2.compare("xi") == 0)
    {
        return 1;
    }
    else if(token1.compare("12") == 0 && token2.compare("xii") == 0)
    {
        return 1;
    }
    else if(token1.compare("13") == 0 && token2.compare("xiii") == 0)
    {
        return 1;
    }
    else if(token1.compare("14") == 0 && token2.compare("xiv") == 0)
    {
        return 1;
    }
    else if(token1.compare("15") == 0 && token2.compare("xv") == 0)
    {
        return 1;
    }
    
    else if(token2.compare("1") == 0 && token1.compare("i") == 0)
    {
        return 1;
    }
    else if(token2.compare("2") == 0 && token1.compare("ii") == 0)
    {
        return 1;
    }
    else if(token2.compare("3") == 0 && token1.compare("iii") == 0)
    {
        return 1;
    }
    else if(token2.compare("4") == 0 && token1.compare("iv") == 0)
    {
        return 1;
    }
    else if(token2.compare("5") == 0 && token1.compare("v") == 0)
    {
        return 1;
    }
    else if(token2.compare("6") == 0 && token1.compare("vi") == 0)
    {
        return 1;
    }
    else if(token2.compare("7") == 0 && token1.compare("vii") == 0)
    {
        return 1;
    }
    else if(token2.compare("8") == 0 && token1.compare("viii") == 0)
    {
        return 1;
    }
    else if(token2.compare("9") == 0 && token1.compare("ix") == 0)
    {
        return 1;
    }
    else if(token2.compare("10") == 0 && token1.compare("x") == 0)
    {
        return 1;
    }
    else if(token2.compare("11") == 0 && token1.compare("xi") == 0)
    {
        return 1;
    }
    else if(token2.compare("12") == 0 && token1.compare("xii") == 0)
    {
        return 1;
    }
    else if(token2.compare("13") == 0 && token1.compare("xiii") == 0)
    {
        return 1;
    }
    else if(token2.compare("14") == 0 && token1.compare("xiv") == 0)
    {
        return 1;
    }
    else if(token2.compare("15") == 0 && token1.compare("xv") == 0)
    {
        return 1;
    }
    
    else if(token1.compare("0") == 0 && token2.compare("zero") == 0)
    {
        return 1;
    }
    else if(token1.compare("1") == 0 && token2.compare("one") == 0)
    {
        return 1;
    }
    else if(token1.compare("2") == 0 && token2.compare("two") == 0)
    {
        return 1;
    }
    else if(token1.compare("3") == 0 && token2.compare("three") == 0)
    {
        return 1;
    }
    else if(token1.compare("4") == 0 && token2.compare("four") == 0)
    {
        return 1;
    }
    else if(token1.compare("5") == 0 && token2.compare("five") == 0)
    {
        return 1;
    }
    else if(token1.compare("6") == 0 && token2.compare("six") == 0)
    {
        return 1;
    }
    else if(token1.compare("7") == 0 && token2.compare("seven") == 0)
    {
        return 1;
    }
    else if(token1.compare("8") == 0 && token2.compare("eight") == 0)
    {
        return 1;
    }
    else if(token1.compare("9") == 0 && token2.compare("nine") == 0)
    {
        return 1;
    }
    else if(token1.compare("10") == 0 && token2.compare("ten") == 0)
    {
        return 1;
    }
    
    else if(token2.compare("0") == 0 && token1.compare("zero") == 0)
    {
        return 1;
    }
    else if(token2.compare("1") == 0 && token1.compare("one") == 0)
    {
        return 1;
    }
    else if(token2.compare("2") == 0 && token1.compare("two") == 0)
    {
        return 1;
    }
    else if(token2.compare("3") == 0 && token1.compare("three") == 0)
    {
        return 1;
    }
    else if(token2.compare("4") == 0 && token1.compare("four") == 0)
    {
        return 1;
    }
    else if(token2.compare("5") == 0 && token1.compare("five") == 0)
    {
        return 1;
    }
    else if(token2.compare("6") == 0 && token1.compare("six") == 0)
    {
        return 1;
    }
    else if(token2.compare("7") == 0 && token1.compare("seven") == 0)
    {
        return 1;
    }
    else if(token2.compare("8") == 0 && token1.compare("eight") == 0)
    {
        return 1;
    }
    else if(token2.compare("9") == 0 && token1.compare("nine") == 0)
    {
        return 1;
    }
    else if(token2.compare("10") == 0 && token1.compare("ten") == 0)
    {
        return 1;
    }
    
    return 0;
}


void ParseDirectoryProcess::getGameImagesFromDirectory(Game *game, string gameFileName, string directory, int64_t gameImageType)
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
                
                if(fileName.find(gameFileName + ".", 0) != string::npos)
                {
                    GameImage *gameImage = new GameImage((int64_t)0);
                    gameImage->setApiId(0);
                    gameImage->setGameId(game->getId());
                    gameImage->setType(gameImageType);
                    gameImage->setExternal(0);
                    gameImage->setDownloaded(1);
                    gameImage->setUrl("");                    
                    gameImage->save();
                    
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
                    
                    gameImage->save();                    
                    delete gameImage;
                }
            }
            else if(entry->d_type == DT_DIR)
            {
                getGameImagesFromDirectory(game, gameFileName, directory + string(entry->d_name), gameImageType);
            }
        }
        closedir(dir);
    }
}

void ParseDirectoryProcess::fetch(ParseDirectoryGame* parseDirectoryGame)
{
    Platform *platform = new Platform(parseDirectory->getPlatformId());
    platform->load();

    string gameName = parseDirectoryGame->getName();
    if(parseDirectory->getUseMame())
    {
        gameName = parseDirectoryGame->getMameName();
    }

    TheGamesDB::Elasticsearch::getInstance()->getGames(platform->getApiId(), gameName, [this, parseDirectoryGame](list<TheGamesDB::Game *> *apiGames) -> void {
        string gameName = parseDirectory->getUseMame() ? parseDirectoryGame->getMameName() : parseDirectoryGame->getName();
        list<string> gameNameTokens = tokenizeName(gameName);
        Game *game = NULL;

        GameEvaluation_t gameEvaluations[apiGames->size()];
        for(unsigned int index = 0; index < apiGames->size(); index++)
        {
            TheGamesDB::Game *apiGame = TheGamesDB::Game::getItem(apiGames, index);   

            gameEvaluations[index].apiGame = apiGame;
            gameEvaluations[index].nameTokens = tokenizeName(apiGame->getName());
            gameEvaluations[index].coincidences = 0;
            gameEvaluations[index].notCoincidences = 0;
            gameEvaluations[index].points = 0;

            list<string> *primaryNameTokens = NULL;
            list<string> *secondaryNameTokens = NULL;
            if(gameEvaluations[index].nameTokens.size() > gameNameTokens.size())
            {
                primaryNameTokens = &gameEvaluations[index].nameTokens;
                secondaryNameTokens = &gameNameTokens;
            }
            else
            {
                primaryNameTokens = &gameNameTokens;
                secondaryNameTokens = &gameEvaluations[index].nameTokens;
            }

            for(list<string>::iterator primaryNameToken = primaryNameTokens->begin(); primaryNameToken != primaryNameTokens->end(); primaryNameToken++)
            {
                int is = 0;
                for(list<string>::iterator secondaryNameToken = secondaryNameTokens->begin(); secondaryNameToken != secondaryNameTokens->end(); secondaryNameToken++)
                {
                    if((*secondaryNameToken).compare((*primaryNameToken)) == 0)
                    {
                        is = 1;
                        break;
                    }
                    else if(compareNumerals(*secondaryNameToken, *primaryNameToken))
                    {
                        is = 1;
                        break;
                    }
                }

                if(is)
                {
                    gameEvaluations[index].coincidences++;
                }
                else
                {
                    gameEvaluations[index].notCoincidences++;
                }
            }

            if(gameEvaluations[index].nameTokens.size() == gameEvaluations[index].coincidences && gameEvaluations[index].notCoincidences == 0)
            {
                gameEvaluations[index].points = 100;
            }
            else
            {                    
                gameEvaluations[index].points = gameEvaluations[index].coincidences - gameEvaluations[index].notCoincidences;

                if(gameEvaluations[index].coincidences > 0 && primaryNameTokens->size() == secondaryNameTokens->size())
                {
                    gameEvaluations[index].points++;
                }
            }                
        }


        GameEvaluation_t *bestGameEvaluation = NULL;
        GameEvaluation_t *winnerGameEvaluation = NULL;
        for(unsigned int index = 0; index < apiGames->size(); index++)
        {
            if(gameEvaluations[index].points <= 0)
            {
                continue;
            }

            if(!bestGameEvaluation)
            {
                bestGameEvaluation = &gameEvaluations[index];
            }
            else if(gameEvaluations[index].points > bestGameEvaluation->points)
            {
                bestGameEvaluation = &gameEvaluations[index];
            }
        }

        // @TODO.- Make additional checks
        if(bestGameEvaluation)
        {
            double percentAccuracy = (bestGameEvaluation->coincidences * 100) / bestGameEvaluation->nameTokens.size();
            if(percentAccuracy >= 70.0)
            {
                winnerGameEvaluation = bestGameEvaluation;
            }
        }


        if(winnerGameEvaluation)
        {
            int64_t esrbRatingId = 0;
            if(winnerGameEvaluation->apiGame->getEsrbRatingId())
            {
                EsrbRating *esrbRating = EsrbRating::getEsrbRating(winnerGameEvaluation->apiGame->getEsrbRatingId());
                esrbRatingId = esrbRating->getId();

                delete esrbRating;
            }

            game = new Game((int64_t)0);
            game->setApiId(winnerGameEvaluation->apiGame->getId());
            game->setPlatformId(parseDirectory->getPlatformId());
            game->setTimestamp(Utils::getInstance()->nowIsoDateTime());
            game->setName(winnerGameEvaluation->apiGame->getName());
            game->setFileName(parseDirectoryGame->getFileName());
            game->setDescription(winnerGameEvaluation->apiGame->getDescription());
            game->setEsrbRatingId(esrbRatingId);
            game->setReleaseDate(winnerGameEvaluation->apiGame->getReleaseDate());                
            game->save();

            list<TheGamesDB::GameGenre *> *apiGameGenres = bestGameEvaluation->apiGame->getGameGenres();
            for(unsigned int index = 0; index < apiGameGenres->size(); index++)
            {
                TheGamesDB::GameGenre *apiGameGenre = TheGamesDB::GameGenre::getItem(apiGameGenres, index);                    
                Genre *genre = Genre::getGenre(apiGameGenre->getGenreId());

                if(genre)
                {
                    GameGenre *gameGenre = new GameGenre(game->getId(), genre->getId());                        
                    gameGenre->save();

                    delete gameGenre;
                    delete genre;
                }
            }

            list<TheGamesDB::GamePublisher *> *apiGamePublishers = bestGameEvaluation->apiGame->getGamePublishers();
            for(unsigned int index = 0; index < apiGamePublishers->size(); index++)
            {
                TheGamesDB::GamePublisher *apiGamePublisher = TheGamesDB::GamePublisher::getItem(apiGamePublishers, index);                    
                Publisher *publisher = Publisher::getPublisher(apiGamePublisher->getPublisherId());

                if(publisher)
                {
                    GamePublisher *gamePublisher = new GamePublisher(game->getId(), publisher->getId());                        
                    gamePublisher->save();

                    delete gamePublisher;
                    delete publisher;
                }
            }                

            list<TheGamesDB::GameDeveloper *> *apiGameDevelopers = bestGameEvaluation->apiGame->getGameDevelopers();
            for(unsigned int index = 0; index < apiGameDevelopers->size(); index++)
            {
                TheGamesDB::GameDeveloper *apiGameDeveloper = TheGamesDB::GameDeveloper::getItem(apiGameDevelopers, index);                    
                Developer *developer = Developer::getDeveloper(apiGameDeveloper->getDeveloperId());

                if(developer)
                {
                    GameDeveloper *gameDeveloper = new GameDeveloper(game->getId(), developer->getId());                        
                    gameDeveloper->save();

                    delete gameDeveloper;
                    delete developer;
                }
            }
        }


        // If no game data is found, saves it with the basics
        if(!game)
        {
            game = new Game((int64_t)0);
            game->setApiId(0);
            game->setPlatformId(parseDirectory->getPlatformId());
            game->setTimestamp(Utils::getInstance()->nowIsoDateTime());
            game->setName(gameName);
            game->setFileName(parseDirectoryGame->getFileName());
            game->save();
        }


        // Loads the images from the filesystem if applies
        int areBoxFrontImages = 0;                
        if(parseDirectory->getBoxFrontImagesDirectory().length() > 0)
        {
            getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectory->getBoxFrontImagesDirectory(), GameImage::TYPE_BOX_FRONT);

            list<GameImage *> *boxFrontGameImages = GameImage::getItems(game->getId(), GameImage::TYPE_BOX_FRONT);

            areBoxFrontImages = boxFrontGameImages->size() > 0;
            GameImage::releaseItems(boxFrontGameImages);
        }

        int areBoxBackImages = 0;
        if(parseDirectory->getBoxBackImagesDirectory().length() > 0)
        {
            getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectory->getBoxBackImagesDirectory(), GameImage::TYPE_BOX_BACK);

            list<GameImage *> *boxFrontGameImages = GameImage::getItems(game->getId(), GameImage::TYPE_BOX_BACK);                

            areBoxBackImages = boxFrontGameImages->size() > 0;
            GameImage::releaseItems(boxFrontGameImages);
        }            
        if(parseDirectory->getScreenshotImagesDirectory().length() > 0)
        {
            getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectory->getScreenshotImagesDirectory(), GameImage::TYPE_SCREENSHOT);
        }
        if(parseDirectory->getLogoImagesDirectory().length() > 0)
        {
            getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectory->getLogoImagesDirectory(), GameImage::TYPE_CLEAR_LOGO);
        }
        if(parseDirectory->getBannerImagesDirectory().length() > 0)
        {
            getGameImagesFromDirectory(game, parseDirectoryGame->getName(), parseDirectory->getBannerImagesDirectory(), GameImage::TYPE_BANNER);
        }


        // If there is a match, gets the images.
        if(winnerGameEvaluation)
        {
            list<TheGamesDB::GameImage *> *apiGameImages = winnerGameEvaluation->apiGame->getGameImages();
            for(unsigned int index = 0; index < apiGameImages->size(); index++)
            {
                TheGamesDB::GameImage *apiGameImage = TheGamesDB::GameImage::getItem(apiGameImages, index);

                int64_t type = 0;
                if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_BOXART) == 0)
                {
                    if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_FRONT) == 0)
                    {
                        if(areBoxFrontImages)
                        {
                            continue;
                        }
                        type = GameImage::TYPE_BOX_FRONT;
                    }
                    else if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_BACK) == 0)
                    {
                        if(areBoxBackImages)
                        {
                            continue;
                        }

                        type = GameImage::TYPE_BOX_BACK;
                    }
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_BANNER) == 0)
                {
                    type = GameImage::TYPE_BANNER;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_FANART) == 0)
                {
                    type = GameImage::TYPE_FANART;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_SCREENSHOT) == 0)
                {
                    type = GameImage::TYPE_SCREENSHOT;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_CLEAR_LOGO) == 0)
                {
                    type = GameImage::TYPE_CLEAR_LOGO;
                }                    

                GameImage *gameImage = new GameImage((int64_t)0);
                gameImage->setApiId(apiGameImage->getId());
                gameImage->setGameId(game->getId());
                gameImage->setType(type);
                gameImage->setExternal(0);
                gameImage->setDownloaded(0);
                gameImage->setUrl(apiGameImage->getOriginal());

                // Saves before to generate the id
                gameImage->save();

                gameImage->setFileName(game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId()));
                gameImage->save();

                delete gameImage;
            }        
        }

        delete game;

        parseDirectoryGame->setProcessed(1);
        parseDirectoryGame->save();
    });

    delete platform;
}

