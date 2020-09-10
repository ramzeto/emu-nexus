/*
 * Copyright (C) 2020 ram
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
 * File:   ThreadManager.cpp
 * Author: ram
 * 
 * Created on September 6, 2020, 8:45 PM
 */

#include "ThreadManager.h"
#include "Logger.h"

#include <gdk/gdk.h>

ThreadManager *ThreadManager::instance = NULL;

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::init()
{
    mainThread = pthread_self();
}

int ThreadManager::isMainThread()
{
    return mainThread == pthread_self();
}

void ThreadManager::execute(int mainThread, function<void()> execution)
{
    ThreadData_t *threadData = new ThreadData_t;
    threadData->execution = execution;
    
    if(mainThread)
    {
        if(isMainThread())
        {
            execution();
        }
        else
        {
            gdk_threads_add_idle([](gpointer threadData)->gboolean{
                ((ThreadData_t *)threadData)->execution();
                delete (ThreadData_t *)threadData;
                
                return G_SOURCE_REMOVE;
            }, (gpointer)threadData);
        }
    }
    else
    {
        pthread_t thread;
        if(pthread_create(&thread, NULL, [](void *threadData)->void * {
            ((ThreadData_t *)threadData)->execution();
            delete (ThreadData_t *)threadData;
            
            return NULL;
        }, (void *)threadData) != 0) 
        {
            Logger::getInstance()->error("ThreadManager", __FUNCTION__, "pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}

void ThreadManager::execute(int mainThread, function<void()> execution, function<void()> finished)
{
    ThreadData_t *threadData = new ThreadData_t;
    threadData->execution = execution;
    threadData->finished = finished;
    threadData->finishedMainThread = isMainThread();
    
    if(mainThread)
    {
        if(threadData->finishedMainThread)
        {
            threadData->execution();
            threadData->finished();
        }
        else
        {                        
            gdk_threads_add_idle([](gpointer threadData)->gboolean{
                ((ThreadData_t *)threadData)->execution();               
                instance->execute(((ThreadData_t *)threadData)->finishedMainThread, ((ThreadData_t *)threadData)->finished);
                
                delete (ThreadData_t *)threadData;
                
                return G_SOURCE_REMOVE;
            }, (gpointer)threadData);
        }
    }
    else
    {
        pthread_t thread;
        if(pthread_create(&thread, NULL, [](void *threadData)->void * {
            ((ThreadData_t *)threadData)->execution();
            instance->execute(((ThreadData_t *)threadData)->finishedMainThread, ((ThreadData_t *)threadData)->finished);
            delete (ThreadData_t *)threadData;
            
            return NULL;
        }, (void *)threadData) != 0) 
        {
            Logger::getInstance()->error("ThreadManager", __FUNCTION__, "pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}



ThreadManager* ThreadManager::getInstance()
{
    if(!instance)
    {
        instance = new ThreadManager();        
    }
    
    return instance;
}



