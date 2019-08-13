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
 * File:   GameLauncher.h
 * Author: ram
 *
 * Created on July 19, 2019, 1:38 AM
 */

#ifndef GAMELAUNCHER_H
#define GAMELAUNCHER_H

#include <pthread.h>
#include <cstdint>
#include <string>
#include <list>

using namespace std;

class GameLauncher 
{
public:
    static const int ERROR_BUSY;
    static const int ERROR_INFLATE;
    static const int ERROR_INFLATE_NOT_SUPPORTED;
    static const int ERROR_FILE_NOT_FOUND;
    static const int ERROR_OTHER;
    
    static const int STATE_IDLE;
    static const int STATE_INFLATING;
    static const int STATE_RUNNING;
    static const int STATE_FINISHED;
    
    typedef struct
    {
        int64_t gameId;
        int error;
        int state;
        int progress;
    }Status_t;
    
    void launch(int64_t gameId, void *requester, void (*callback)(void *, void *));
        
    static GameLauncher *getInstance();
    
private:
    GameLauncher();
    virtual ~GameLauncher();

    typedef struct
    {
        int64_t gameId;
        void *requester;
        void (*callback)(void *, void *);
    }GameLauncherData_t;
    
    pthread_mutex_t mutex;
    
    string getFileNameWithExtensions(list<string> extensions, string directory);
    void postStatus(GameLauncherData_t *gameLauncherData, int error, int state, int progress);
    
    static GameLauncher *instance;    
    static void *launchWorker(void *pGameLauncherData);
};

#endif /* GAMELAUNCHER_H */
