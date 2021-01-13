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
 * File:   DownloadGameImagesProcess.cpp
 * Author: ram
 * 
 * Created on August 11, 2019, 8:09 PM
 */

#include "DownloadGameImagesProcess.h"
#include "GameImage.h"
#include "HttpConnector.h"
#include "Utils.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Logger.h"

#include <iostream>

const string DownloadGameImagesProcess::TYPE = "DownloadGameImagesProcess";

DownloadGameImagesProcess::DownloadGameImagesProcess() : SerialProcess(TYPE)
{
}

DownloadGameImagesProcess::~DownloadGameImagesProcess()
{
}

void DownloadGameImagesProcess::execute()
{
    status = STATUS_RUNNING;
    
    list<GameImage *> *gameImages = GameImage::getPendingToDownloadItems();
    
    for(unsigned int index = 0; index < gameImages->size(); index++)
    {
        GameImage *gameImage = GameImage::getItem(gameImages, index);
        
        Game *game = new Game(gameImage->getGameId());
        game->load();

        int progress = ((double)index / (double)gameImages->size()) * 100.0;
        NotificationManager::getInstance()->notify(TYPE, game->getName(), status, 0, NULL, progress);
        
        HttpConnector *httpConnector = new HttpConnector(gameImage->getUrl());
        httpConnector->get();
        if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
        {
            Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), gameImage->getFileName());
            gameImage->setDownloaded(1);
            
            Utils::getInstance()->scaleImage(gameImage->getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, gameImage->getThumbnailFileName());
            
            gameImage->save();
            
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
    }    
    GameImage::releaseItems(gameImages);
    
    status = STATUS_SUCCESS;
    NotificationManager::getInstance()->notify(TYPE, "", status);
}


