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
 * File:   NotificationManager.cpp
 * Author: ram
 * 
 * Created on June 17, 2019, 5:23 PM
 */

#include "NotificationManager.h"
#include "UiThreadBridge.h"

#include <iostream>

NotificationManager *NotificationManager::instance = NULL;

NotificationManager::NotificationManager()
{
    notificationRefsMutex = PTHREAD_MUTEX_INITIALIZER;
    notificationRefsMap = new map<string, list<NotificationRef_t *> *>;
}

NotificationManager::~NotificationManager()
{
    notificationRefsMap->clear();
    delete notificationRefsMap;
}

void NotificationManager::registerToNotification(string notification, void* listener, void (*callback)(CallbackResult *), int mainThread)
{
    pthread_mutex_lock(&notificationRefsMutex);
    NotificationRef_t *notificationRef = new NotificationRef_t;
    notificationRef->listener = listener;
    notificationRef->callback = callback;
    notificationRef->mainThread = mainThread;
    notificationRef->unregistered = 0;

    if(notificationRefsMap->find(notification) != notificationRefsMap->end())
    {
        list<NotificationRef_t *> *notificationRefs = notificationRefsMap->at(notification);
        notificationRefs->push_back(notificationRef);
    }
    else
    {
        list<NotificationRef_t *> *notificationRefs = new list<NotificationRef_t *>;
        notificationRefs->push_back(notificationRef);
        notificationRefsMap->insert(pair<string, list<NotificationRef_t *> *>(notification, notificationRefs));
    }
    
    pthread_mutex_unlock(&notificationRefsMutex);
}

void NotificationManager::unregisterToNotification(string notification, void* listener)
{
    pthread_mutex_lock(&notificationRefsMutex);
    if(notificationRefsMap->find(notification) != notificationRefsMap->end())
    {
        NotificationRef_t *notificationRefToUnRegister = NULL;
        list<NotificationRef_t *> *notificationRefs = notificationRefsMap->at(notification);
        for(list<NotificationRef_t *>::iterator notificationRef = notificationRefs->begin(); notificationRef != notificationRefs->end(); notificationRef++)
        {
            if((*notificationRef)->listener == listener)
            {
                notificationRefToUnRegister = (*notificationRef);
                break;
            }
        }
        if(notificationRefToUnRegister)
        {
            notificationRefToUnRegister->unregistered = 1;
            notificationRefs->remove(notificationRefToUnRegister);
        }
    }    
    pthread_mutex_unlock(&notificationRefsMutex);
}

void NotificationManager::postNotification(CallbackResult *callbackResult)
{
    list<NotificationRef_t *> *uiThreadNotificationRefs = new list<NotificationRef_t *>;
    
    pthread_mutex_lock(&notificationRefsMutex);
    if(notificationRefsMap->find(callbackResult->getType()) != notificationRefsMap->end())
    {        
        list<NotificationRef_t *> *notificationRefs = notificationRefsMap->at(callbackResult->getType());
        callbackResult->setUsed(notificationRefs->size());
        for(list<NotificationRef_t *>::iterator notificationRef = notificationRefs->begin(); notificationRef != notificationRefs->end(); notificationRef++)
        {
            if((*notificationRef)->unregistered)
            {
                continue;
            }
            
            // @TODO .- Free the callbackResult object
            if((*notificationRef)->mainThread)
            {
                uiThreadNotificationRefs->push_back((*notificationRef));
            }
            else
            {
                callbackResult->setRequester((*notificationRef)->listener);
                (*notificationRef)->callback(callbackResult);
            }
        }
    }
    pthread_mutex_unlock(&notificationRefsMutex);
    
    if(uiThreadNotificationRefs->size() > 0)
    {
        NotificationData_t *notificationData = new NotificationData_t;
        notificationData->notificationRefs = uiThreadNotificationRefs;
        notificationData->callbackResult = callbackResult;
    
        UiThreadBridge::postToUiThread(notificationData, postToUiThread);
    }
    else
    {
        delete uiThreadNotificationRefs;
        
        callbackResult->destroy();
    }
}

NotificationManager* NotificationManager::getInstance()
{
    if(!instance)
    {
        instance = new NotificationManager();
    }
    return instance;
}

void NotificationManager::postToUiThread(void* pNotificationData)
{
    NotificationData_t *notificationData = (NotificationData_t *)pNotificationData;    
    for(list<NotificationRef_t *>::iterator notificationRef = notificationData->notificationRefs->begin(); notificationRef != notificationData->notificationRefs->end(); notificationRef++)
    {
        if((*notificationRef)->unregistered)
        {
            continue;
        }
        
        notificationData->callbackResult->setRequester((*notificationRef)->listener);
        (*notificationRef)->callback(notificationData->callbackResult);
    }
    
    
    notificationData->notificationRefs->clear();
    delete notificationData->notificationRefs;
    
    notificationData->callbackResult->destroy();
    delete notificationData;        
}
