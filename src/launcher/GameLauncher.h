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

#include "FileExtractor.h"
#include "CallbackResult.h"

using namespace std;

/**
 * Handles the execution of games/programs.
 */
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
    static const int STATE_SELECTING_FILE;
    static const int STATE_FOUND_MULTIPLE_FILES;
    static const int STATE_RUNNING;
    static const int STATE_FINISHED;
    static const int STATE_CANCELED;
    
    /**
     * Launches a game.
     * @param gameId Game id to launch.
     * @param requester Pointer to the object that requested the launch.
     * @param callback Callback that receives status updates. The parameter of this callback is a pointer to a CallbackResult object (the requester is responsible for freeing this pointer).
     */
    void launch(int64_t gameId, void *requester, void (*callback)(CallbackResult *));
    
    /**
     * 
     * @return Current error code of the launch.
     */
    int getError();
    
    /**
     * 
     * @return Current state of the launch.
     */
    int getState();
    
    /**
     * This method should be called when state = STATE_FOUND_MULTIPLE_FILES. This state is set when multiple filenames with the matching criteria are found after uncompressing/unpacking a ROM file.
     * @return List of filenames.
     */
    list<string> getFileNames();
    
    /**
     * This method should be called to select a filename from the list returned by the getFileNames() method.
     * @param fileName to use.
     */
    void selectFileName(string fileName);
    
    /**
     * Cancels the execution.
     */
    void cancel();

    /**
     * 
     * @return Current instance.
     */
    static GameLauncher *getInstance();
    
private:
    int error;
    int state;
    list<string> fileNames;
    string fileName;
    
    GameLauncher();
    virtual ~GameLauncher();

    typedef struct
    {
        int64_t gameId;
        void *requester;
        void (*callback)(CallbackResult *);
    }GameLauncherData_t;    
    pthread_mutex_t mutex;
    
    /**
     * Recursively searches for files that match the criteria. Stores the filenames in the 'fileNames' variable.
     * @param extensions List of supported extensions.
     * @param directory Directory to search.
     */
    void getFileNamesWithExtensions(list<string> extensions, string directory);
    
    /**
     * Posts status updates to the requester object.
     * @param gameLauncherData
     * @param error
     * @param state
     * @param progress
     */
    void postStatus(GameLauncherData_t *gameLauncherData, int error, int state, int progress);
    
    static GameLauncher *instance;
    
    static void *launchWorker(void *pGameLauncherData);    
    static void fileExtractorProgressListenerCallback(void *pGameLauncherData, FileExtractor *fileExtractor, size_t fileSize, size_t progressBytes);
};

#endif /* GAMELAUNCHER_H */

