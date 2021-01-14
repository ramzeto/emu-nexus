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
#include "NotificationManager.h"
#include "Notifications.h"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

SerialProcessExecutor *SerialProcessExecutor::instance = NULL;

SerialProcessExecutor::SerialProcessExecutor()
{
    serialProcessesMutex = PTHREAD_MUTEX_INITIALIZER;
    serialProcesses = new list<SerialProcess *>;
    
    isExecuting = 0;
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
    
    NotificationManager::getInstance()->notify(NOTIFICATION_PROCESS_QUEUE_CHANGED);
    
    if(isExecuting)
    {
        return;
    }    
    isExecuting = 1;
 
    ThreadManager::getInstance()->execute(0, [this]() -> void {
        while(1)
        {
            list<SerialProcess *> *safeSerialProcesses = new list<SerialProcess *>;
            
            pthread_mutex_lock(&serialProcessesMutex);
            for(list<SerialProcess *>::iterator serialProcess = serialProcesses->begin(); serialProcess != serialProcesses->end(); serialProcess++)
            {
                safeSerialProcesses->push_back(*serialProcess);
            }
            pthread_mutex_unlock(&serialProcessesMutex);
            
            for(list<SerialProcess *>::iterator serialProcess = safeSerialProcesses->begin(); serialProcess != safeSerialProcesses->end(); serialProcess++)
            {
                (*serialProcess)->execute();
            
                pthread_mutex_lock(&serialProcessesMutex);
                serialProcesses->remove(*serialProcess);
                delete *serialProcess;
                pthread_mutex_unlock(&serialProcessesMutex);
                
                NotificationManager::getInstance()->notify(NOTIFICATION_PROCESS_QUEUE_CHANGED);
            }
            
            safeSerialProcesses->clear();
            delete safeSerialProcesses;
            
            sleep(1);
        }
    });   
}

unsigned int SerialProcessExecutor::getProcessCount() 
{
    pthread_mutex_lock(&serialProcessesMutex);
    unsigned int count = serialProcesses->size();
    pthread_mutex_unlock(&serialProcessesMutex);
    
    return count;
}

SerialProcessExecutor* SerialProcessExecutor::getInstance()
{
    if(!instance)
    {
        instance = new SerialProcessExecutor();
    }
    
    return instance;
}
