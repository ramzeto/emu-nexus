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
#include "Database.h"
#include "HttpConnector.h"
#include "Utils.h"
#include "NotificationManager.h"
#include "Notifications.h"

#include <iostream>

const string DownloadGameImagesProcess::TYPE = "DownloadGameImagesProcess";

DownloadGameImagesProcess::DownloadGameImagesProcess(void *requester, void (*statusCallback)(void *, void*)) : SerialProcess(TYPE, requester, statusCallback)
{
    postNotificationThreadHandler = new UiThreadHandler(this, callbackPostNotificationReady);
}

DownloadGameImagesProcess::~DownloadGameImagesProcess()
{
    delete postNotificationThreadHandler;
}

int DownloadGameImagesProcess::execute()
{
    status = STATUS_RUNNING;
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    list<GameImage *> *gameImages = GameImage::getPendingToDownloadItems(sqlite);
    Database::getInstance()->release();        
    
    for(unsigned int index = 0; index < gameImages->size(); index++)
    {
        GameImage *gameImage = GameImage::getItem(gameImages, index);
        
        Game *game = new Game(gameImage->getGameId());
        sqlite = Database::getInstance()->acquire();
        game->load(sqlite);
        Database::getInstance()->release();

        int progress = ((double)index / (double)gameImages->size()) * 100.0;
        cout << "index: " << index << " gameImages->size(): " << gameImages->size() << " progress: " << progress << endl; 
        postStatus("Downloading images", game->getName(), progress);

        
        HttpConnector *httpConnector = new HttpConnector(gameImage->getUrl());
        httpConnector->get();
        cout  << "DownloadGameImagesProcess::" << __FUNCTION__ << " fileName: " <<gameImage->getFileName() << " url: " << gameImage->getUrl() << " httpStatus: " << httpConnector->getHttpStatus() << endl;
        if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
        {
            Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), gameImage->getFileName());
            gameImage->setDownloaded(1);
            
            Utils::getInstance()->scaleImage(gameImage->getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, gameImage->getThumbnailFileName());
            
            sqlite = Database::getInstance()->acquire();
            gameImage->save(sqlite);
            Database::getInstance()->release();
            
            if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
            {
                postGameChangedNotification(game);
            }
        }
        delete httpConnector;                        
    }            
    GameImage::releaseItems(gameImages);
    
    status = STATUS_SUCCESS;
    
    return status;
}

void DownloadGameImagesProcess::postGameChangedNotification(Game* game)
{
    UiThreadHandler::callback(postNotificationThreadHandler, game);
}

void DownloadGameImagesProcess::callbackPostNotificationReady(void* pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;      
    Game *game = (Game *)uiThreadHandlerResult->data;
    
    NotificationManager::getInstance()->postNotification(NOTIFICATION_GAME_UPDATED, game);
}

