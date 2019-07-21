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

#include <iostream>

NotificationManager *NotificationManager::instance = NULL;

NotificationManager::NotificationManager()
{
    notificationsListenersMutex = PTHREAD_MUTEX_INITIALIZER;
    notificationsListeners = new map<string, list<NotificationListener_t *> *>;
}

NotificationManager::~NotificationManager()
{
    notificationsListeners->clear();
    delete notificationsListeners;
}

void NotificationManager::registerToNotification(string notification, void* listener, void(*listenerFunction)(string, void*, void*))
{
    pthread_mutex_lock(&notificationsListenersMutex);
    NotificationListener_t *notificationListener = new NotificationListener_t;
    notificationListener->listener = listener;
    notificationListener->listenerFunction = listenerFunction;

    if(notificationsListeners->find(notification) != notificationsListeners->end())
    {
        list<NotificationListener_t *> *notificationListeners = notificationsListeners->at(notification);
        notificationListeners->push_back(notificationListener);
    }
    else
    {
        list<NotificationListener_t *> *notificationListeners = new list<NotificationListener_t *>;
        notificationListeners->push_back(notificationListener);
        notificationsListeners->insert(pair<string, list<NotificationListener_t *> *>(notification, notificationListeners));
    }
    
    pthread_mutex_unlock(&notificationsListenersMutex);
}

void NotificationManager::unRegisterToNotification(string notification, void* listener)
{
    pthread_mutex_lock(&notificationsListenersMutex);
    if(notificationsListeners->find(notification) != notificationsListeners->end())
    {
        NotificationListener_t *notificationListenerToUnRegister = NULL;
        list<NotificationListener_t *> *notificationListeners = notificationsListeners->at(notification);
        for(list<NotificationListener_t *>::iterator notificationListener = notificationListeners->begin(); notificationListener != notificationListeners->end(); notificationListener++)
        {
            if((*notificationListener)->listener == listener)
            {
                notificationListenerToUnRegister = (*notificationListener);
                break;
            }
        }
        if(notificationListenerToUnRegister)
        {
            notificationListeners->remove(notificationListenerToUnRegister);
        }
        
        if(notificationListeners->size() == 0)
        {
            delete notificationListeners;
            notificationsListeners->erase(notification);
        }
    }    
    pthread_mutex_unlock(&notificationsListenersMutex);
}

void NotificationManager::postNotification(string notification, void* data)
{    
    pthread_mutex_lock(&notificationsListenersMutex);
    if(notificationsListeners->find(notification) != notificationsListeners->end())
    {
        list<NotificationListener_t *> *notificationListeners = notificationsListeners->at(notification);
        for(list<NotificationListener_t *>::iterator notificationListener = notificationListeners->begin(); notificationListener != notificationListeners->end(); notificationListener++)
        {
            (*notificationListener)->listenerFunction(notification, (*notificationListener)->listener, data);
        }
    }    
    pthread_mutex_unlock(&notificationsListenersMutex);
}


NotificationManager* NotificationManager::getInstance()
{
    if(!instance)
    {
        instance = new NotificationManager();
    }
    return instance;
}
