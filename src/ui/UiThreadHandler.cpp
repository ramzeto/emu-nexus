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
 * File:   UiThreadHandler.cpp
 * Author: ram
 * 
 * Created on April 27, 2019, 12:07 AM
 */

#include "UiThreadHandler.h"

#include <gdk/gdk.h>
#include <iostream>

using namespace std;

UiThreadHandler::UiThreadHandler(void *requesterInUiThread, int (*callbackInUiThread)(void *))
{
    this->requesterInUiThread = requesterInUiThread;
    this->callbackInUiThread = callbackInUiThread;
    datas = new list<void *>;
    dataMutex = PTHREAD_MUTEX_INITIALIZER;
    postingData = 0;
}

UiThreadHandler::~UiThreadHandler()
{
    for(list<void *>::iterator data = datas->begin(); data != datas->end(); data++)
    {
        free(*data);
    }
    datas->clear();
    delete datas;
}

void* UiThreadHandler::getRequesterInUiThread()
{
    return requesterInUiThread;
}

void UiThreadHandler::releaseResult(Result_t* result)
{
    pthread_mutex_lock(&(result->uiThreadHandler->dataMutex));
    result->uiThreadHandler->datas->remove(result->data);
    free(result->data);
    result->uiThreadHandler->postingData = 0;
    pthread_mutex_unlock(&(result->uiThreadHandler->dataMutex));
    
    result->uiThreadHandler->postToUiThread();

    delete result;
}

void UiThreadHandler::pushData(void* data)
{
    pthread_mutex_lock(&dataMutex);
    datas->push_back(data);
    pthread_mutex_unlock(&dataMutex);
    
    postToUiThread();
}

void UiThreadHandler::postToUiThread()
{
    pthread_mutex_lock(&dataMutex);
    if(!postingData)
    {
        void *data = NULL;
        if(datas->size() > 0)
        {
            data = datas->front();
        }

        if(data)
        {
            postingData = 1;

            Result_t *result = new Result_t;
            result->uiThreadHandler = this;
            result->data = data;

            gdk_threads_add_idle (callbackInUiThread, result);
        }
        else
        {
            postingData = 0;
        }        
    }
    pthread_mutex_unlock(&dataMutex);    
}

void UiThreadHandler::callback(void* pUiThreadHandler, void* data)
{
    UiThreadHandler *uiThreadHandler = (UiThreadHandler *)pUiThreadHandler;
    uiThreadHandler->pushData(data);
}


