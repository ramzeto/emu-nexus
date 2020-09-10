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
 * File:   SerialProcessExecutor.h
 * Author: ram
 *
 * Created on May 6, 2019, 10:19 PM
 */

#ifndef SERIALPROCESSEXECUTOR_H
#define SERIALPROCESSEXECUTOR_H

#include "SerialProcess.h"

#include <pthread.h>
#include <list>

using namespace std;

/**
 * This class executes SerialProcess objects in order of schedule.
 */
class SerialProcessExecutor {
public:
    
    /**
     * Schedules a serialProcess object for execution. This serial process will be executed in a background thread.
     * @param serialProcess
     */
    void schedule(SerialProcess *serialProcess);
    
    /**
     * Ends the execution of the serialProcess. Only has effect if the serialProcess parameter is the same as currentSerialProcess. The requester has to free the SerialProcess object.
     * @param serialProcess
     */
    void finish(SerialProcess *serialProcess);
    
    /**
     * 
     * @return Instance of the executor.
     */
    static SerialProcessExecutor *getInstance();
    
private:
    SerialProcessExecutor();
    virtual ~SerialProcessExecutor();
    
    list<SerialProcess *> *serialProcesses;
    pthread_mutex_t serialProcessesMutex;
    SerialProcess *currentSerialProcess;
    pthread_t executorThread;
    
    void executeNext();
    
    static SerialProcessExecutor *instance;
};

#endif /* SERIALPROCESSEXECUTOR_H */

