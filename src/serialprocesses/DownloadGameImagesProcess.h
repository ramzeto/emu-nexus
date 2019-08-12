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
 * File:   DownloadGameImagesProcess.h
 * Author: ram
 *
 * Created on August 11, 2019, 8:09 PM
 */

#ifndef DOWNLOADGAMEIMAGESPROCESS_H
#define DOWNLOADGAMEIMAGESPROCESS_H

#include "SerialProcess.h"
#include "UiThreadHandler.h"
#include "Game.h"

#include <list>

using namespace std;

class DownloadGameImagesProcess : public SerialProcess
{
public:
    static const string TYPE;
        
    DownloadGameImagesProcess(void *requester, void (*statusCallback)(void *, void*));
    virtual ~DownloadGameImagesProcess();
    
    int execute() override;
    
private:
    UiThreadHandler *postNotificationThreadHandler;
    
    void postGameChangedNotification(Game *game);
    
    static void callbackPostNotificationReady(void *pUiThreadHandlerResult);
};

#endif /* DOWNLOADGAMEIMAGESPROCESS_H */

