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
 * File:   NotificationManager.h
 * Author: ram
 *
 * Created on June 17, 2019, 5:23 PM
 */

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <pthread.h>

#include <string>
#include <map>
#include <list>

using namespace std;

/**
 * This class handles in app notifications. This notifications broadcast an event to registered listeners.
 */
class NotificationManager {
public:
    
    /**
     * Registers a listener to a notification
     * @param notification Notification to register.
     * @param listener Pointer to the object that listens to the notification.
     * @param listenerFunction Pointer to the function that will be called when the notification gets posted. Receives 3 parameters (string notification, void *listener, void* notificationData)
     */
    void registerToNotification(string notification, void *listener, void (*listenerFunction)(string, void *, void*));
    
    /**
     * Unregisters a listener from a notification.
     * @param notification Notification to unregister.
     * @param listener Pointer to the object that currently listens to the notification.
     */
    void unRegisterToNotification(string notification, void *listener);
    
    /**
     * Posts a notification to the registered listeners. The notification is posted in the same thread from which this method is called.
     * @param notification Notification to post.
     * @param data Data sent by the poster. This data will be received bye every listener through the notificationData parameter in the listenerFunction. The poster is responsible to free this data.
     */
    void postNotification(string notification, void *data);
    
    /**
     * 
     * @return Instance of the NotificationManager.
     */
    static NotificationManager *getInstance();
    
private:
    NotificationManager();
    virtual ~NotificationManager();

    
    typedef struct{
        void *listener;
        void (*listenerFunction)(string, void *, void*); //notification, listener, notification data
    }NotificationListener_t;

    pthread_mutex_t notificationsListenersMutex;
    map<string, list<NotificationListener_t *> *> *notificationsListeners;
    
    static NotificationManager *instance;
};

#endif /* NOTIFICATIONMANAGER_H */

