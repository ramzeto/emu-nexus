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
 * File:   GameLauncher.cpp
 * Author: ram
 * 
 * Created on July 19, 2019, 1:38 AM
 */

#include "GameLauncher.h"
#include "Game.h"
#include "Platform.h"
#include "Database.h"
#include "CacheGame.h"
#include "Utils.h"
#include "FileExtractor.h"
#include "RecentGame.h"
#include "Preferences.h"

#include <dirent.h>
#include <cstdlib>
#include <iostream>
#include <cstring>


const int GameLauncher::ERROR_BUSY = 1;
const int GameLauncher::ERROR_INFLATE = 2;
const int GameLauncher::ERROR_INFLATE_NOT_SUPPORTED = 3;
const int GameLauncher::ERROR_FILE_NOT_FOUND = 4;
const int GameLauncher::ERROR_OTHER = 5;

const int GameLauncher::STATE_IDLE = 1;
const int GameLauncher::STATE_INFLATING = 2;
const int GameLauncher::STATE_RUNNING = 3;
const int GameLauncher::STATE_FINISHED = 4;

GameLauncher *GameLauncher::instance = NULL;


GameLauncher::GameLauncher()
{
    mutex = PTHREAD_MUTEX_INITIALIZER;        
}

GameLauncher::~GameLauncher()
{
}

void GameLauncher::launch(int64_t gameId, void* requester, void(*callback)(void*, void*))
{
    GameLauncherData_t *gameLauncherData = new GameLauncherData_t;
    gameLauncherData->gameId = gameId;
    gameLauncherData->requester = requester;
    gameLauncherData->callback = callback;
    
    pthread_t launchThread;
    if(pthread_create(&launchThread, NULL, launchWorker, gameLauncherData) != 0) 
    {
        cerr << "GameLauncher::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

string GameLauncher::getFileNameWithExtensions(list<string> extensions, string directory)
{
    DIR *dir;
    struct dirent *entry;
    string fileNameWithExtension = "";
    
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
                        fileNameWithExtension = directory + fileName;
                        break;
                    }
                }                                
            }
            else if(entry->d_type == DT_DIR)
            {                
                fileNameWithExtension = getFileNameWithExtensions(extensions, directory + string(entry->d_name));
            }
            
            if(fileNameWithExtension.length() > 0)
            {
                break;
            }
        }        
        closedir(dir);
    }
        
    return fileNameWithExtension;
}

void GameLauncher::postStatus(GameLauncherData_t* gameLauncherData, int error, int state, int progress)
{
    Status_t *status = new Status_t;
    status->gameId = gameLauncherData->gameId;
    status->error = error;
    status->state = state;
    status->progress = progress;

    gameLauncherData->callback(gameLauncherData->requester, status);
}


GameLauncher* GameLauncher::getInstance()
{
    if(instance == NULL)
    {
        instance = new GameLauncher();
    }
    
    return instance;
}

void* GameLauncher::launchWorker(void* pGameLauncherData)
{
    GameLauncherData_t *gameLauncherData = (GameLauncherData_t *)pGameLauncherData;
    
    // Launcher is busy
    if(pthread_mutex_trylock(&instance->mutex) == EBUSY)
    {
        instance->postStatus(gameLauncherData, ERROR_BUSY, STATE_IDLE, -1);
        return NULL;
    }
        
    int error = 0;
    instance->postStatus(gameLauncherData, error, STATE_IDLE, -1);
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    Game *game = new Game(gameLauncherData->gameId);
    game->load(sqlite);
    
    Platform *platform = new Platform(game->getPlatformId());        
    platform->load(sqlite);
    Database::getInstance()->release();
    
    string command = platform->getCommand();   
    int deflate = platform->getDeflate();
    string deflateFileExtensions = platform->getDeflateFileExtensions();
    string gameFileName = "";
    
    if(game->getCommand().length() > 0)
    {
        command = game->getCommand();
        deflate = game->getDeflate();
        deflateFileExtensions = game->getDeflateFileExtensions();        
    }
    
    if(deflate)
    {
        sqlite3 *sqlite = Database::getInstance()->acquire();
        CacheGame *cacheGame = CacheGame::getCacheGame(sqlite, game->getId());
        if(!cacheGame)
        {
            cacheGame = new CacheGame((int64_t)0);
            cacheGame->setGameId(game->getId());
            cacheGame->setTimestamp(Utils::getInstance()->nowIsoDateTime());
            cacheGame->save(sqlite);
        }
        Database::getInstance()->release();
        
        // If cached game directory does not exist, then it should be created and the game ROM should be extracted
        if(!Utils::getInstance()->directoryExists(cacheGame->getDirectory()))
        {
            Utils::getInstance()->makeDirectory(cacheGame->getDirectory());
            instance->postStatus(gameLauncherData, error, STATE_INFLATING, -1);
            
            FileExtractor *fileExtractor = new FileExtractor(game->getFileName());
            if(fileExtractor->extract(cacheGame->getDirectory()))
            {
               error = ERROR_INFLATE_NOT_SUPPORTED; 
            }
            delete fileExtractor;                        
        }
        
        // Clears cache if necessary
        sqlite = Database::getInstance()->acquire();
        list<CacheGame *> *cacheGames = CacheGame::getItems(sqlite);
        Database::getInstance()->release();
        size_t cacheSize = 0;
        for(unsigned int c = 0; c < cacheGames->size(); c++)
        {
            CacheGame *aCacheGame = CacheGame::getItem(cacheGames, c);
            cacheSize += aCacheGame->getSize();
        }
        
        size_t allowdCacheSize = Preferences::getInstance()->getCacheSize() * 1024 * 1024; //MB to Bytes
        if(cacheSize > allowdCacheSize)
        {
            size_t size = 0;
            size_t sizeToClear = cacheSize - allowdCacheSize;
            for(unsigned int c = 0; c < cacheGames->size(); c++)
            {
                CacheGame *aCacheGame = CacheGame::getItem(cacheGames, c);
                if(aCacheGame->getId() == cacheGame->getId())
                {
                    continue;
                }
                
                size += aCacheGame->getSize();
                
                sqlite = Database::getInstance()->acquire();
                aCacheGame->remove(sqlite);
                Database::getInstance()->release();
                
                if(size >= sizeToClear)
                {
                    break;
                }
            }
        }
        CacheGame::releaseItems(cacheGames);
        
                
        // Tries to find a file with the proper extension
        if(!error)
        {
            gameFileName = instance->getFileNameWithExtensions(Utils::getInstance()->strSplitByWhiteSpace(deflateFileExtensions), cacheGame->getDirectory());
            if(gameFileName.length() == 0)
            {
                error = ERROR_FILE_NOT_FOUND;
            }
        }
        
        
        // If an error happens, the cache game is removed
        if(error)
        {
            sqlite = Database::getInstance()->acquire();
            cacheGame->remove(sqlite);
            Database::getInstance()->release();            
        }
    }
    else
    {
        gameFileName = game->getFileName();
    }
    
    // Executes
    if(!error)
    {
        if(gameFileName.length() > 0)
        {
            instance->postStatus(gameLauncherData, error, STATE_RUNNING, -1);
         
            // Escapes problematic characters in the game file name.
            gameFileName = Utils::getInstance()->strReplace(gameFileName, " ", "\\ ");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "\"", "\\\"");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "'", "\\'");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "(", "\\(");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, ")", "\\)");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "[", "\\[");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "]", "\\]");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "{", "\\{");
            gameFileName = Utils::getInstance()->strReplace(gameFileName, "}", "\\}");
            
            // Replaces %FILE% for the actual game file name
            command = Utils::getInstance()->strReplace(command, "%FILE%", gameFileName);
            
            cout << "GameLauncher::" << __FUNCTION__ << " command: " << command << endl;
            system(command.c_str());
        }
        else
        {
            error = ERROR_FILE_NOT_FOUND;
        }
    }
    
    // If the execution was successful, saves to recents
    if(!error)
    {
        RecentGame *recentGame = new RecentGame(game->getId());
        recentGame->setTimestamp(Utils::getInstance()->nowIsoDateTime());
        sqlite = Database::getInstance()->acquire();
        recentGame->save(sqlite);
        list<RecentGame *> *recentGames = RecentGame::getItems(sqlite, 1);
        Database::getInstance()->release();
        
        if(recentGames->size() > RecentGame::LIMIT)
        {
            unsigned int itemsToRemove = recentGames->size() - RecentGame::LIMIT;
            for(unsigned int c = 0; c < itemsToRemove; c++)
            {
                RecentGame *recentGame = RecentGame::getItem(recentGames, c);
                sqlite = Database::getInstance()->acquire();
                recentGame->remove(sqlite);
                Database::getInstance()->release();
            }
        }
        RecentGame::releaseItems(recentGames);
    }
    
    delete game;
    delete platform;
    pthread_mutex_unlock(&instance->mutex);

    instance->postStatus(gameLauncherData, error, STATE_FINISHED, -1);
    
    return NULL;
}
 