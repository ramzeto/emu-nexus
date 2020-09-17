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
#include <list>

ThreadManager *ThreadManager::instance = NULL;

ThreadManager::ThreadManager()
{
    mainThreadDataMutex = PTHREAD_MUTEX_INITIALIZER;
    mainThreadData = new list<ThreadData_t *>;
    mainThreadPostHandler = 0;
}

ThreadManager::~ThreadManager()
{
    for(list<ThreadData_t *>::iterator data = mainThreadData->begin(); data != mainThreadData->end(); data++)
    {
        delete (*data);
    }
    mainThreadData->clear();
    delete mainThreadData;
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
    threadData->shouldFinish = 0;
    
    if(mainThread)
    {
        if(isMainThread())
        {
            threadData->execution();
            
            delete threadData;
        }
        else
        {
            postToMainThread(threadData);
        }
    }
    else
    {
        if(isMainThread())
        {
            pthread_t thread;
            int error;
            if((error = pthread_create(&thread, NULL, [](void *threadData)->void * {
                ((ThreadData_t *)threadData)->execution();
                delete (ThreadData_t *)threadData;

                return NULL;
            }, (void *)threadData)) != 0) 
            {
                string errorName = "";
                if(error == EAGAIN)
                {
                    errorName = "EAGAIN";
                }
                else if(error == EINVAL)
                {
                    errorName = "EINVAL";
                }
                else if(error == EPERM)
                {
                    errorName = "EPERM";
                }

                Logger::getInstance()->error("ThreadManager", __FUNCTION__, "1 pthread_create error: " + to_string(error) + " errorName: " + errorName);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            ((ThreadData_t *)threadData)->execution();            
            delete (ThreadData_t *)threadData;
        }
    }
}

void ThreadManager::execute(int mainThread, function<void()> execution, function<void()> finish)
{
    ThreadData_t *threadData = new ThreadData_t;
    threadData->execution = execution;
    threadData->shouldFinish = 1;
    threadData->finish = finish;
    threadData->finishInMainThread = isMainThread();
    
    if(mainThread)
    {
        if(isMainThread())
        {
            threadData->execution();
            
            if(threadData->finishInMainThread)
            {            
                threadData->finish();
            }
            else
            {
                execute(threadData->finishInMainThread, threadData->finish);
            }
            
            delete threadData;
        }        
        else
        {
            postToMainThread(threadData);
        }
    }
    else
    {
        if(isMainThread())
        {
            pthread_t thread;
            int error;
            if((error = pthread_create(&thread, NULL, [](void *threadData)->void * {
                ((ThreadData_t *)threadData)->execution();

                if(((ThreadData_t *)threadData)->finishInMainThread)
                {
                    instance->execute(((ThreadData_t *)threadData)->finishInMainThread, ((ThreadData_t *)threadData)->finish);
                }
                else
                {
                    ((ThreadData_t *)threadData)->finish();
                }

                delete (ThreadData_t *)threadData;

                return NULL;
            }, (void *)threadData)) != 0) 
            {
                string errorName = "";
                if(error == EAGAIN)
                {
                    errorName = "EAGAIN";
                }
                else if(error == EINVAL)
                {
                    errorName = "EINVAL";
                }
                else if(error == EPERM)
                {
                    errorName = "EPERM";
                }

                Logger::getInstance()->error("ThreadManager", __FUNCTION__, "2 pthread_create error: " + to_string(error) + " errorName: " + errorName);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            ((ThreadData_t *)threadData)->execution();
            
            if(((ThreadData_t *)threadData)->finishInMainThread)
            {
                instance->execute(((ThreadData_t *)threadData)->finishInMainThread, ((ThreadData_t *)threadData)->finish);
            }
            else
            {
                ((ThreadData_t *)threadData)->finish();
            }

            delete (ThreadData_t *)threadData;
        }
    }
}

void ThreadManager::postToMainThread(ThreadData_t *threadData)
{
    pthread_mutex_lock(&mainThreadDataMutex);
    mainThreadData->push_back(threadData);
    pthread_mutex_unlock(&mainThreadDataMutex);
    
    if(mainThreadPostHandler)
    {
        return;
    }
    
    //https://developer.gnome.org/gdk3/stable/gdk3-Threads.html
    mainThreadPostHandler = gdk_threads_add_timeout(100, [](gpointer)->gboolean{
        list<ThreadData_t *> *secureMainThreadData = new list<ThreadData_t *>;
        
        pthread_mutex_lock(&(instance->mainThreadDataMutex));
        for(list<ThreadData_t *>::iterator data = instance->mainThreadData->begin(); data != instance->mainThreadData->end(); data++)
        {
            secureMainThreadData->push_back(*data);
        }
        pthread_mutex_unlock(&(instance->mainThreadDataMutex));
        
        
        for(list<ThreadData_t *>::iterator data = secureMainThreadData->begin(); data != secureMainThreadData->end(); data++)
        {
            ThreadData_t *threadData = *data;
            
            threadData->execution();           
            if(threadData->shouldFinish)
            {
                if(threadData->finishInMainThread)
                {
                    threadData->finish();
                }
                else
                {
                    instance->execute(0, threadData->finish);
                }
            }
        }
        
        pthread_mutex_lock(&(instance->mainThreadDataMutex));
        for(list<ThreadData_t *>::iterator data = secureMainThreadData->begin(); data != secureMainThreadData->end(); data++)
        {
            instance->mainThreadData->remove(*data);
            delete *data;
        }
        pthread_mutex_unlock(&(instance->mainThreadDataMutex));
        
        secureMainThreadData->clear();
        delete secureMainThreadData;
        
        return G_SOURCE_CONTINUE;
    }, NULL);
    
}

ThreadManager* ThreadManager::getInstance()
{
    if(!instance)
    {
        instance = new ThreadManager();        
    }
    
    return instance;
}



