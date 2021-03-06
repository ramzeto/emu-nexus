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
 * File:   ThreadManager.h
 * Author: ram
 *
 * Created on September 6, 2020, 8:45 PM
 */

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <pthread.h>
#include <functional>
#include <list>

using namespace std;

/**
 * Provides an easy way to execute code in different threads. For execution in the main thread, the GTK approach is used.
 */
class ThreadManager 
{
public:
    ThreadManager();
    virtual ~ThreadManager();
    
    /**
     * Initializes the object. Should be called only once at the beginning of the application in the main thread.
     */
    void init();
    
    /**
     * 
     * @return 1 if was called from the main thread; 0 otherwise.
     */
    int isMainThread();
    
    /**
     * Executes a lambda function in a thread.
     * @param mainThread If > 0, then 'execution' is executed in the main thread, otherwise is executed in the same thread in which this method is called.
     * @param execution Lambda function to be executed.
     */
    void execute(int mainThread, function<void()> execution);
    
    /**
     * Executes a lambda function in a thread.
     * @param mainThread If > 0, then 'execution' is executed in the main thread, otherwise is executed in the same thread in which this method is called.
     * @param execution Lambda function to be executed.
     * @param finish Lambda function to be executed when the 'execution' function finishes. It will be executed in the same thread in which this method is called.
     */
    void execute(int mainThread, function<void()> execution, function<void()> finish);
    
    /**
     * 
     * @return Object instance. Only one object is created for the lifetime of the application.
     */
    static ThreadManager *getInstance();
    
private:
    typedef struct{        
        function<void()> execution;
        int shouldFinish;
        function<void()> finish;        
        int finishInMainThread;
    }ThreadData_t;
    
    pthread_mutex_t mainThreadDataMutex;
    list<ThreadData_t *> *mainThreadData;
    
    unsigned int mainThreadPostHandler;
    pthread_t mainThread;
    
    void postToMainThread(ThreadData_t *threadData);
    
    static ThreadManager *instance;
};

#endif /* THREADMANAGER_H */

