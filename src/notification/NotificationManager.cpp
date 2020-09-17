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
#include "ThreadManager.h"

#include <iostream>

NotificationManager *NotificationManager::instance = NULL;

NotificationManager::NotificationManager()
{    
    notificationListenersDataMutex = PTHREAD_MUTEX_INITIALIZER;
    notificationListenersDataMap = new map<string, list<NotificationListenerData_t *> *>;
}

NotificationManager::~NotificationManager()
{
    for (map<string, list<NotificationListenerData_t *> *>::iterator item = notificationListenersDataMap->begin(); item != notificationListenersDataMap->end(); item++)
    {
        for(list<NotificationListenerData_t *>::iterator notificationListenerData = item->second->begin(); notificationListenerData != item->second->end(); notificationListenerData++)
        {
            delete *notificationListenerData;
        }
        delete item->second;
    }
    notificationListenersDataMap->clear();
    delete notificationListenersDataMap;
}

void NotificationManager::registerToNotification(string name, void *listener, void (*callback)(Notification *), int mainThread)
{
    pthread_mutex_lock(&notificationListenersDataMutex);
    
    NotificationListenerData_t *notificationListenerData = new NotificationListenerData_t;
    notificationListenerData->listener = listener;
    notificationListenerData->callback = callback;
    notificationListenerData->mainThread = mainThread;
    notificationListenerData->unregistered = 0;

    if(notificationListenersDataMap->find(name) != notificationListenersDataMap->end())
    {
        list<NotificationListenerData_t *> *items = notificationListenersDataMap->at(name);
        items->push_back(notificationListenerData);
    }
    else
    {
        list<NotificationListenerData_t *> *items = new list<NotificationListenerData_t *>;
        items->push_back(notificationListenerData);
        notificationListenersDataMap->insert(pair<string, list<NotificationListenerData_t *> *>(name, items));
    }
    
    pthread_mutex_unlock(&notificationListenersDataMutex);
}

void NotificationManager::unregisterToNotification(string name, void* listener)
{
    pthread_mutex_lock(&notificationListenersDataMutex);
    if(notificationListenersDataMap->find(name) != notificationListenersDataMap->end())
    {
        NotificationListenerData_t *notificationListenerDataToUnRegister = NULL;
        list<NotificationListenerData_t *> *notificationListenersData = notificationListenersDataMap->at(name);
        for(list<NotificationListenerData_t *>::iterator notificationListenerData = notificationListenersData->begin(); notificationListenerData != notificationListenersData->end(); notificationListenerData++)
        {
            if((*notificationListenerData)->listener == listener)
            {
                notificationListenerDataToUnRegister = (*notificationListenerData);
                break;
            }
        }
        if(notificationListenerDataToUnRegister)
        {
            notificationListenerDataToUnRegister->unregistered = 1;
            notificationListenersData->remove(notificationListenerDataToUnRegister);
        }
    }    
    pthread_mutex_unlock(&notificationListenersDataMutex);    
}

void NotificationManager::notify(string name, string message, int status, int error, void* data, int progress)
{
    Notification *notification = new Notification();
    notification->name = name;
    notification->message = message;    
    notification->status = status;
    notification->error = error;
    notification->data = data;
    notification->progress = progress;
    
    list<NotificationListenerData_t *> *notificationListenersDataInMainThread = new list<NotificationListenerData_t *>;
    
    pthread_mutex_lock(&notificationListenersDataMutex);
    if(notificationListenersDataMap->find(name) != notificationListenersDataMap->end())
    {
        list<NotificationListenerData_t *> *notificationListenersData = notificationListenersDataMap->at(name);
        for(list<NotificationListenerData_t *>::iterator notificationListenerData = notificationListenersData->begin(); notificationListenerData != notificationListenersData->end(); notificationListenerData++)
        {
            if((*notificationListenerData)->unregistered)
            {
                continue;
            }
            
            if((*notificationListenerData)->mainThread)
            {
                notificationListenersDataInMainThread->push_back((*notificationListenerData));
            }
            else
            {
                notification->listener = (*notificationListenerData)->listener;
                (*notificationListenerData)->callback(notification);
            }
        }
    }
    pthread_mutex_unlock(&notificationListenersDataMutex);
    
    if(notificationListenersDataInMainThread->size() > 0)
    {
        ThreadManager::getInstance()->execute(1, [notification, notificationListenersDataInMainThread]() -> void {
            for(list<NotificationListenerData_t *>::iterator notificationListenerData = notificationListenersDataInMainThread->begin(); notificationListenerData != notificationListenersDataInMainThread->end(); notificationListenerData++)
        {
                if((*notificationListenerData)->unregistered)
                {
                    continue;
                }

                notification->listener = (*notificationListenerData)->listener;
                (*notificationListenerData)->callback(notification);
            }            
            notificationListenersDataInMainThread->clear();
            
            delete notificationListenersDataInMainThread;
            delete notification;
        });
    }
    else
    {
        delete notificationListenersDataInMainThread;
        delete notification;
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
