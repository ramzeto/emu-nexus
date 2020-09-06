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

#include "CallbackResult.h"

using namespace std;

/**
 * This class handles in app notifications. Are used to broadcast an event to registered listeners.
 */
class NotificationManager {
public:
    
    /**
     * Registers a listener to a notification
     * @param notification Notification to register.
     * @param listener Pointer to the object that listens to the notification.
     * @param callback Pointer to the function that will be called when the notification gets posted. Receives a pointer to a CallbackResult object (CallbackResult->getType() is the notification name).
     * @param mainThread States that the notification should be delivered in the main thread.
     */
    void registerToNotification(string notification, void *listener, void (*callback)(CallbackResult *), int mainThread);
    
    /**
     * Unregisters a listener from a notification.
     * @param notification Notification to unregister.
     * @param listener Pointer to the object that currently listens to the notification.
     */
    void unregisterToNotification(string notification, void *listener);
    
    /**
     * Posts a notification to the registered listeners. If notification listener mainThread = 0, the notification will be posted in the same thread from which this method is called.
     * @param notification Notification to post
     * @param data Data to be sent to the notification listeners (It will be automatically freed).
     * @param status
     * @param error
     * @param progress
     */
    void postNotification(string notification, void *data, int status = -1, int error = 0, int progress = 0);
            
    
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
        void (*callback)(CallbackResult *);
        int unregistered;
        int mainThread;
    }NotificationRef_t;
    
    typedef struct{
        list<NotificationRef_t *> *notificationRefs;
        CallbackResult *callbackResult;
    }NotificationData_t;

    pthread_mutex_t notificationRefsMutex;
    map<string, list<NotificationRef_t *> *> *notificationRefsMap;
    
    static NotificationManager *instance;
    static void postToUiThread(void *pNotificationData);
};

#endif /* NOTIFICATIONMANAGER_H */

