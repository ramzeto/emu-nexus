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
#include "GameCache.h"
#include "GameActivity.h"
#include "Utils.h"
#include "Preferences.h"
#include "Logger.h"
#include "NotificationManager.h"
#include "Notifications.h"

#include <dirent.h>
#include <cstdlib>
#include <iostream>
#include <cstring>


const int GameLauncher::ERROR_BUSY = 1;
const int GameLauncher::ERROR_INFLATE = 2;
const int GameLauncher::ERROR_INFLATE_NOT_SUPPORTED = 3;
const int GameLauncher::ERROR_FILE_NOT_FOUND = 4;
const int GameLauncher::ERROR_OTHER = 5;

const int GameLauncher::STATUS_IDLE = 0;
const int GameLauncher::STATUS_STARTING = 1;
const int GameLauncher::STATUS_INFLATING = 2;
const int GameLauncher::STATUS_SELECTING_FILE = 3;
const int GameLauncher::STATUS_FOUND_MULTIPLE_FILES = 4;
const int GameLauncher::STATUS_RUNNING = 5;
const int GameLauncher::STATUS_FINISHED = 6;
const int GameLauncher::STATUS_CANCELED = 7;

GameLauncher *GameLauncher::instance = NULL;


GameLauncher::GameLauncher()
{
    mutex = PTHREAD_MUTEX_INITIALIZER;
    status = STATUS_IDLE;
    error = 0;
    gameId = 0;
}

GameLauncher::~GameLauncher()
{
}

int GameLauncher::launch(int64_t gameId)
{
    if(status != STATUS_IDLE)
    {
        return 1;
    }    
    this->gameId = gameId;
    
    pthread_t launchThread;
    if(pthread_create(&launchThread, NULL, launchWorker, NULL) != 0) 
    {
        exit(EXIT_FAILURE);
    }
    
    return 0;
}

int GameLauncher::getError()
{
    return error;
}

int GameLauncher::getStatus()
{
    return status;
}

int64_t GameLauncher::getGameId()
{
    return gameId;
}

list<string> GameLauncher::getFileNames()
{
    return fileNames;
}

void GameLauncher::selectFileName(string fileName)
{
    this->fileName = fileName;
}

void GameLauncher::cancel()
{
    status = STATUS_CANCELED;
}

void GameLauncher::getFileNamesWithExtensions(list<string> extensions, string directory)
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
                        fileNames.push_back(directory + fileName);
                    }
                }                                
            }
            else if(entry->d_type == DT_DIR)
            {
                getFileNamesWithExtensions(extensions, directory + string(entry->d_name));
            }
        }        
        closedir(dir);
    }
}

void GameLauncher::postStatus(int progress)
{
    NotificationManager::getInstance()->postNotification(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED, NULL, status, error, progress);    
}

GameLauncher* GameLauncher::getInstance()
{
    if(instance == NULL)
    {
        instance = new GameLauncher();
    }
    
    return instance;
}

void* GameLauncher::launchWorker(void *)
{    
    // Launcher is busy
    if(pthread_mutex_trylock(&instance->mutex) == EBUSY)
    {        
        return NULL;
    }        

    instance->error = 0;
    instance->status = STATUS_STARTING;
    instance->postStatus();
    
    Game *game = new Game(instance->gameId);
    game->load();
    
    Platform *platform = new Platform(game->getPlatformId());        
    platform->load();
    
    string command = platform->getCommand();   
    int deflate = platform->getDeflate();
    string deflateFileExtensions = platform->getDeflateFileExtensions();
    
    if(game->getCommand().length() > 0)
    {
        command = game->getCommand();
        deflate = game->getDeflate();
        deflateFileExtensions = game->getDeflateFileExtensions();        
    }
    
    if(deflate)
    {                       
        // Clears cache if necessary
        list<GameCache *> *gameCaches = GameCache::getItems();
        size_t cacheSize = 0;
        for(unsigned int c = 0; c < gameCaches->size(); c++)
        {
            GameCache *aGameCache = GameCache::getItem(gameCaches, c);
            cacheSize += aGameCache->getSize();
        }
        
        size_t allowdCacheSize = Preferences::getInstance()->getCacheSize() * 1024 * 1024; //MB to Bytes
        if(cacheSize > allowdCacheSize)
        {
            size_t size = 0;
            size_t sizeToClear = cacheSize - allowdCacheSize;
            for(unsigned int c = 0; c < gameCaches->size(); c++)
            {
                GameCache *aGameCache = GameCache::getItem(gameCaches, c);
                if(aGameCache->getGameId() == game->getId())
                {
                    continue;
                }
                
                size += aGameCache->getSize();
                
                aGameCache->remove();
                
                if(size >= sizeToClear)
                {
                    break;
                }
            }
        }
        GameCache::releaseItems(gameCaches);
        
        
        // Caches the game
        GameCache *gameCache = GameCache::getGameCache(game->getId());
        if(!gameCache)
        {
            gameCache = new GameCache((int64_t)0);
            gameCache->setGameId(game->getId());
            gameCache->setTimestamp(Utils::getInstance()->nowIsoDateTime());
            gameCache->save();
        }
        
        // If cached game directory does not exist, then it should be created and the game ROM should be extracted
        if(!gameCache || !Utils::getInstance()->directoryExists(gameCache->getDirectory()))
        {
            instance->status = STATUS_INFLATING;
            instance->postStatus();
            
            string directory = Utils::getInstance()->getTempFileName() + "/";
            Utils::getInstance()->makeDirectory(directory);                        
            
            FileExtractor *fileExtractor = new FileExtractor(game->getFileName());
            fileExtractor->setProgressListener(instance, fileExtractorProgressListenerCallback);
            if(fileExtractor->extract(directory))
            {
                instance->error = ERROR_INFLATE_NOT_SUPPORTED;
                instance->postStatus();
            }
            delete fileExtractor;
            
            if(!gameCache)
            {
                gameCache = new GameCache((int64_t)0);
                gameCache->setGameId(game->getId());
                gameCache->setTimestamp(Utils::getInstance()->nowIsoDateTime());
                gameCache->save();
            }
            
            Utils::getInstance()->moveDirectory(directory, gameCache->getDirectory());
        }                
        
        
        // Tries to find a file with the proper extension
        if(!instance->error)
        {
            instance->status = STATUS_SELECTING_FILE;
            instance->postStatus();
            
            instance->fileName = "";
            instance->fileNames.clear();
            instance->getFileNamesWithExtensions(Utils::getInstance()->strSplitByWhiteSpace(deflateFileExtensions), gameCache->getDirectory());
            
            // No suitable file found
            if(instance->fileNames.size() == 0)
            {
                instance->error = ERROR_FILE_NOT_FOUND;
                instance->postStatus();
            }
            // More than one suitable files found
            else if(instance->fileNames.size() > 1)
            {
                // For multidisk ROMs, if multiple .cue files are found, a .m3u file is created, so emulators like Retroarch with the Beetle Saturn core can use it (https://libretro.readthedocs.io/en/latest/library/beetle_saturn/).
                instance->fileNames.sort();
                list<string> cueFileNames;
                for(list<string>::iterator fileName = instance->fileNames.begin(); fileName != instance->fileNames.end(); fileName++)
                {
                    if(Utils::getInstance()->strToLowerCase(*fileName).find(".m3u") != string::npos)
                    {
                        cueFileNames.clear();
                        break;
                    }
                    
                    if(Utils::getInstance()->strToLowerCase(*fileName).find(".cue") != string::npos)
                    {
                        cueFileNames.push_back(*fileName);
                    }
                }
                
                if(cueFileNames.size() > 1)
                {
                    string m3uFileName = Utils::getInstance()->getFileDirectory(*cueFileNames.begin()) + "00000_autogenerated.m3u";
                    string m3uContent = "";
                    for(list<string>::iterator fileName = cueFileNames.begin(); fileName != cueFileNames.end(); fileName++)
                    {
                        m3uContent += Utils::getInstance()->getFileBasename(*fileName) + "\n";
                    }
                    Utils::getInstance()->writeToFile((unsigned char *)m3uContent.c_str(), m3uContent.length(), m3uFileName);
                    instance->fileNames.push_back(m3uFileName);
                    instance->fileNames.sort();
                }
                
                instance->status = STATUS_FOUND_MULTIPLE_FILES;
                instance->postStatus();
                
                //Wait for file selection or cancellation
                while(instance->fileName.length() == 0 && instance->status != STATUS_CANCELED)
                {
                    sleep(1);
                }
            }
            else
            {
                instance->fileName = *(instance->fileNames.begin());
            }
        }
        
        
        // If an error happens, the cached game is removed
        if(instance->error)
        {
            gameCache->remove();
        }
        
        delete gameCache;
    }
    else
    {
        instance->fileName = game->getFileName();
    }
    
    
    // Executes
    if(instance->status != STATUS_CANCELED && !instance->error)
    {
        if(instance->fileName.length() > 0)
        {
            instance->status = STATUS_RUNNING;
            instance->postStatus();
         
            // Escapes problematic characters in the game file name.
            string fileName = instance->fileName;
            fileName = Utils::getInstance()->strReplace(fileName, " ", "\\ ");
            fileName = Utils::getInstance()->strReplace(fileName, "\"", "\\\"");
            fileName = Utils::getInstance()->strReplace(fileName, "'", "\\'");
            fileName = Utils::getInstance()->strReplace(fileName, "(", "\\(");
            fileName = Utils::getInstance()->strReplace(fileName, ")", "\\)");
            fileName = Utils::getInstance()->strReplace(fileName, "[", "\\[");
            fileName = Utils::getInstance()->strReplace(fileName, "]", "\\]");
            fileName = Utils::getInstance()->strReplace(fileName, "{", "\\{");
            fileName = Utils::getInstance()->strReplace(fileName, "}", "\\}");
            
            // Replaces %FILE% for the actual game file name
            command = Utils::getInstance()->strReplace(command, "%FILE%", fileName);
            
            GameActivity *gameActivity = new GameActivity((int64_t)0);
            gameActivity->setGameId(game->getId());
            gameActivity->setTimestamp(Utils::getInstance()->nowIsoDateTime());            
            time_t start = time(NULL);
            
            Logger::getInstance()->message("GameLauncher", __FUNCTION__, command);
            system(command.c_str());
            
            gameActivity->setDuration(time(NULL) - start);
            gameActivity->save();
            delete gameActivity;
            
            instance->status = STATUS_FINISHED;
            instance->error = 0;        
            instance->postStatus();
            
            // Notifies the game activity
            NotificationManager::getInstance()->postNotification(NOTIFICATION_GAME_ACTIVITY_UPDATED, new Game(*game));
        }
        else
        {
            instance->status = STATUS_FINISHED;
            instance->error = ERROR_FILE_NOT_FOUND;
            instance->postStatus();
        }                
    }   
    
    delete game;
    delete platform;
    

    instance->status = STATUS_IDLE;
    instance->postStatus();
    pthread_mutex_unlock(&instance->mutex);
    
    return NULL;
}


void GameLauncher::fileExtractorProgressListenerCallback(void *, FileExtractor* fileExtractor, size_t fileSize, size_t progressBytes)
{
    instance->status = STATUS_INFLATING;
    instance->postStatus(((double) progressBytes / (double)fileSize) * 100.0);
}
