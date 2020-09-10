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

#include "Notification.h"

using namespace std;

/**
 * This class handles in app notifications. Are used to broadcast an event to registered listeners.
 */
class NotificationManager {
public:

    /**
     * Registers a listener to a notification
     * @param name Notification name to register to.
     * @param listener Pointer to the object that listens to the notification.
     * @param callback Pointer to the function that will be called when the notification is posted. Receives a pointer to a Notification object (It will be freed automatically).
     * @param mainThread States that the notification should be delivered in the main thread.
     */
    void registerToNotification(string name, void *listener, void (*callback)(Notification *), int mainThread);    
    
    /**
     * Unregisters a listener from a notification.
     * @param name Notification name to unregister from.
     * @param listener Pointer to the object that currently listens to the notification.
     */
    void unregisterToNotification(string name, void *listener);    

    /**
     * Posts a notification to the registered listeners.
     * @param name Notification name to post.
     * @param message
     * @param status
     * @param error
     * @param data Data to be sent to the notification listeners (If not NULL, it will be automatically freed).
     * @param progress
     */
    void notify(string name, string message = "", int status = -1, int error = -1, void* data = NULL, int progress = -1);
    
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
        void (*callback)(Notification *);
        int unregistered;
        int mainThread;
    }NotificationListenerData_t;
    
    pthread_mutex_t notificationListenersDataMutex;
    map<string, list<NotificationListenerData_t *> *> *notificationListenersDataMap;
    
    static NotificationManager *instance;
};

#endif /* NOTIFICATIONMANAGER_H */

