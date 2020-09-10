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
 * File:   SerialProcessExecutor.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 10:19 PM
 */

#include "SerialProcessExecutor.h"
#include "ThreadManager.h"
#include "Logger.h"

#include <cstdlib>
#include <iostream>

SerialProcessExecutor *SerialProcessExecutor::instance = NULL;

SerialProcessExecutor::SerialProcessExecutor()
{
    serialProcessesMutex = PTHREAD_MUTEX_INITIALIZER;
    serialProcesses = new list<SerialProcess *>;
    currentSerialProcess = NULL;
}

SerialProcessExecutor::~SerialProcessExecutor()
{
    serialProcesses->clear();
    delete serialProcesses;
}

void SerialProcessExecutor::schedule(SerialProcess* serialProcess)
{    
    pthread_mutex_lock(&serialProcessesMutex);
    serialProcesses->push_back(serialProcess);
    pthread_mutex_unlock(&serialProcessesMutex);
    
    executeNext();
}

void SerialProcessExecutor::finish(SerialProcess* serialProcess)
{
    pthread_mutex_lock(&serialProcessesMutex);
    if(serialProcess == currentSerialProcess)
    {
        serialProcesses->remove(currentSerialProcess);        
        currentSerialProcess = NULL;
                
        delete serialProcess;
    }
    pthread_mutex_unlock(&serialProcessesMutex);
    
    executeNext();
}

void SerialProcessExecutor::executeNext()
{
    pthread_mutex_lock(&serialProcessesMutex);
    if(!currentSerialProcess && serialProcesses->size() > 0)
    {
        currentSerialProcess = serialProcesses->front();
        ThreadManager::getInstance()->execute(0, [this]() -> void {
            SerialProcess *serialProcess = currentSerialProcess;
            Logger::getInstance()->debug("SerialProcessExecutor", __FUNCTION__, currentSerialProcess->getType());
            int status = serialProcess->execute();

            if(status != SerialProcess::STATUS_RUNNING)
            {
                finish(serialProcess);
            }
        }, [this]() -> void {
        });
    }
    pthread_mutex_unlock(&serialProcessesMutex);
}


SerialProcessExecutor* SerialProcessExecutor::getInstance()
{
    if(!instance)
    {
        instance = new SerialProcessExecutor();
    }
    
    return instance;
}
