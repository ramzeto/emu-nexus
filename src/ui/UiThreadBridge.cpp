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
 * File:   UiThreadBridge.cpp
 * Author: ram
 * 
 * Created on April 27, 2019, 12:07 AM
 */

#include "UiThreadBridge.h"

#include <gdk/gdk.h>
#include <iostream>

using namespace std;

list<UiThreadBridge *> *UiThreadBridge::uiThreadBridges = new list<UiThreadBridge *>;
pthread_mutex_t UiThreadBridge::uiThreadBridgesMutex = PTHREAD_MUTEX_INITIALIZER;

UiThreadBridge::UiThreadBridge(void *requester, void (*requesterCallbackInUiThread)(CallbackResult *))
{
    this->requester = requester;
    this->requesterCallbackInUiThread = requesterCallbackInUiThread;
    
    callbackResults = new list<CallbackResult *>;
    callbackResultsMutex = PTHREAD_MUTEX_INITIALIZER;    
    unregistered = 0;
    postingToUiThread = 0;
}

UiThreadBridge::~UiThreadBridge()
{
    callbackResults->clear();
    delete callbackResults;
}

UiThreadBridge *UiThreadBridge::registerBridge(void* requester, void(*requesterCallbackInUiThread)(CallbackResult*))
{
    UiThreadBridge *uiThreadBridge = new UiThreadBridge(requester, requesterCallbackInUiThread);
    
    pthread_mutex_lock(&uiThreadBridgesMutex);
    uiThreadBridges->push_back(uiThreadBridge);
    pthread_mutex_unlock(&uiThreadBridgesMutex);
    
    return uiThreadBridge;
}

void UiThreadBridge::unregisterBridge(UiThreadBridge *uiThreadBridge)
{
    uiThreadBridge->unregistered = 1;
    
    pthread_mutex_lock(&uiThreadBridgesMutex);
    uiThreadBridges->remove(uiThreadBridge);
    pthread_mutex_unlock(&uiThreadBridgesMutex);
}

void UiThreadBridge::postToUiThread(void* data, void(*functionInUiThread)(void*))
{
    PostToUiThreadData_t *postToUiThreadData = new PostToUiThreadData_t;
    postToUiThreadData->data = data;
    postToUiThreadData->functionInUiThread = functionInUiThread;
    gdk_threads_add_idle(callbackAuxUiThreadReady, postToUiThreadData);
}

void UiThreadBridge::postToUiThread()
{    
    pthread_mutex_lock(&callbackResultsMutex);
    if(!postingToUiThread)
    {
        CallbackResult *callbackResult = NULL;
        if(callbackResults->size() > 0)
        {
            callbackResult = callbackResults->front();
        }

        if(callbackResult)
        {
            postingToUiThread = 1;
            gdk_threads_add_idle (callbackUiThreadReady, callbackResult);
        }
        else
        {
            postingToUiThread = 0;
        }        
    }
    pthread_mutex_unlock(&callbackResultsMutex);    
}


void UiThreadBridge::callback(CallbackResult *callbackResult)
{
    UiThreadBridge *uiThreadBridge = (UiThreadBridge *)callbackResult->getRequester();
    if(uiThreadBridge->unregistered)
    {
        callbackResult->destroy();
        return; 
    }
    
    pthread_mutex_lock(&(uiThreadBridge->callbackResultsMutex));
    uiThreadBridge->callbackResults->push_back(callbackResult);
    pthread_mutex_unlock(&(uiThreadBridge->callbackResultsMutex));
    
    uiThreadBridge->postToUiThread();
}

int UiThreadBridge::callbackUiThreadReady(void *pCallbackResult)
{
    CallbackResult *callbackResult = (CallbackResult *)pCallbackResult;
    
    UiThreadBridge *uiThreadBridge = (UiThreadBridge *)callbackResult->getRequester();
    callbackResult->setRequester(uiThreadBridge->requester);
    uiThreadBridge->requesterCallbackInUiThread(callbackResult);

    pthread_mutex_lock(&(uiThreadBridge->callbackResultsMutex));
    uiThreadBridge->callbackResults->remove(callbackResult);
    callbackResult->destroy();
    uiThreadBridge->postingToUiThread = 0;
    pthread_mutex_unlock(&(uiThreadBridge->callbackResultsMutex));
    
    uiThreadBridge->postToUiThread();
    
    return G_SOURCE_REMOVE;
}

int UiThreadBridge::callbackAuxUiThreadReady(void *pPostToUiThreadData)
{    
    PostToUiThreadData_t *postToUiThreadData = (PostToUiThreadData_t *)pPostToUiThreadData;
    postToUiThreadData->functionInUiThread(postToUiThreadData->data);
    
    delete postToUiThreadData;
    
    return G_SOURCE_REMOVE;
}
