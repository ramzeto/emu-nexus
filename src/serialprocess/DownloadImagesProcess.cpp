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
 * File:   DownloadImagesProcess.cpp
 * Author: ram
 * 
 * Created on August 11, 2019, 8:09 PM
 */

#include "DownloadImagesProcess.h"
#include "GameImage.h"
#include "HttpConnector.h"
#include "Utils.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Logger.h"
#include "PlatformImage.h"
#include "Platform.h"

#include <iostream>

const string DownloadImagesProcess::TYPE = "DownloadImagesProcess";

DownloadImagesProcess::DownloadImagesProcess() : SerialProcess(TYPE)
{
}

DownloadImagesProcess::~DownloadImagesProcess()
{
}

void DownloadImagesProcess::execute()
{
    status = STATUS_RUNNING;
    
    list<PlatformImage *> *platformImages = PlatformImage::getPendingToDownloadItems();
    list<GameImage *> *gameImages = GameImage::getPendingToDownloadItems();    
    unsigned int totalIndex = 0;
    
    for(unsigned int index = 0; index < platformImages->size(); index++)
    {
        PlatformImage *platformImage = PlatformImage::getItem(platformImages, index);
        
        Platform *platform = new Platform(platformImage->getPlatformId());
        platform->load();

        int progress = ((double)totalIndex / (double)(gameImages->size() + platformImages->size())) * 100.0;
        NotificationManager::getInstance()->notify(TYPE, platform->getName(), status, 0, NULL, progress);
        
        HttpConnector *httpConnector = new HttpConnector(platformImage->getUrl());
        httpConnector->get();
        if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
        {
            Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), platformImage->getFileName());
            platformImage->setDownloaded(1);                        
            platformImage->save();
            platformImage->saveImage();
            
            if(platformImage->getType() == PlatformImage::TYPE_ICON)
            {
                NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0, 0, new Platform(*platform));
            }
        }
        // Removes the image record if it does not exists in the TheGamesDB servers.
        else if(httpConnector->getHttpStatus() == HttpConnector::HTTP_NOT_FOUND)
        {
            platformImage->remove();
        }
        delete httpConnector;
        
        delete platform;
        
        totalIndex++;
    }
    
    
    for(unsigned int index = 0; index < gameImages->size(); index++)
    {
        GameImage *gameImage = GameImage::getItem(gameImages, index);
        
        Game *game = new Game(gameImage->getGameId());
        game->load();

        int progress = ((double)totalIndex / (double)(gameImages->size() + platformImages->size())) * 100.0;
        NotificationManager::getInstance()->notify(TYPE, game->getName(), status, 0, NULL, progress);
        
        HttpConnector *httpConnector = new HttpConnector(gameImage->getUrl());
        httpConnector->get();
        if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
        {
            Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), gameImage->getFileName());
            gameImage->setDownloaded(1);                        
            gameImage->save();
            gameImage->saveImage();
            
            if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
            {
                NotificationManager::getInstance()->notify(NOTIFICATION_GAME_UPDATED, "", 0, 0, new Game(*game));
            }
        }
        // Removes the image record if it does not exists in the TheGamesDB servers.
        else if(httpConnector->getHttpStatus() == HttpConnector::HTTP_NOT_FOUND)
        {
            gameImage->remove();
        }
        delete httpConnector;
        
        delete game;
        
        totalIndex++;
    }
        
    
    PlatformImage::releaseItems(platformImages);
    GameImage::releaseItems(gameImages);
    
    
    status = STATUS_SUCCESS;
    NotificationManager::getInstance()->notify(TYPE, "", status);
}


