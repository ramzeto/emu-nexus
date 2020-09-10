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
    
    static const int STATUS_IDLE;
    static const int STATUS_STARTING;
    static const int STATUS_INFLATING;
    static const int STATUS_SELECTING_FILE;
    static const int STATUS_FOUND_MULTIPLE_FILES;
    static const int STATUS_RUNNING;
    static const int STATUS_FINISHED;
    static const int STATUS_CANCELED;
    
    /**
     * Launches a game. Should be called from the main thread.
     * @param gameId Game id to launch.
     * @return 0 if launch process will start; > 0 otherwise.
     */
    int launch(int64_t gameId);
    
    /**
     * 
     * @return Current error code of the launch.
     */
    int getError();
    
    /**
     * 
     * @return Current status of the launch.
     */
    int getStatus();
    
    /**
     * 
     * @return Game id.
     */
    int64_t getGameId();
    
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
    int status;
    int64_t gameId;
    list<string> fileNames;
    string fileName;
    
    GameLauncher();
    virtual ~GameLauncher();

    pthread_mutex_t mutex;
    
    /**
     * Recursively searches for files that match the criteria. Stores the filenames in the 'fileNames' variable.
     * @param extensions List of supported extensions.
     * @param directory Directory to search.
     */
    void getFileNamesWithExtensions(list<string> extensions, string directory);
    
    /**
     * Posts status updates to the requester object.
     * @param progress
     */
    void postStatus(int progress = -1);
    
    static GameLauncher *instance;
};

#endif /* GAMELAUNCHER_H */

