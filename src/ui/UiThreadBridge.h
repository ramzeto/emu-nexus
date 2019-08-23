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
 * File:   UiThreadBridge.h
 * Author: ram
 *
 * Created on April 27, 2019, 12:07 AM
 */

#ifndef UITHREADHBRIDGE_H
#define UITHREADHBRIDGE_H

#include <list>
#include <pthread.h>

#include "CallbackResult.h"

using namespace std;

/**
 * Class used to post events and to share data from background threads to the main thread. Uses GTK3 approach (https://developer.gnome.org/gdk3/stable/gdk3-Threads.html).
 * @TODO Find a way to safely delete UiThreadBridge objects when there are unregistered.
 */
class UiThreadBridge {
public:
    
    /**
     * Registers a UiThreadBridge object to exchange data from a background thread to the main thread.
     * @param requester Pointer to the object that requests the bridge (main thread object).
     * @param requesterCallbackInUiThread Callback that will be called in the main thread when the background thread calls the UiThreadBridge::callback. Receives a pointer to a CallbackResult object, this pointer will be freed automatically at any time after after calling requesterCallbackInUiThread.
     * @return 
     */
    static UiThreadBridge *registerBridge(void *requester, void (*requesterCallbackInUiThread)(CallbackResult *));
    
    /**
     * Unregisters previously registered UiThreadBridge object.
     * @param uiThreadBridge
     */
    static void unregisterBridge(UiThreadBridge *uiThreadBridge);
    
    /**
     * Convenience method for executing a function from a background thread into the main thread.
     * @param data Pointer that will be passed as reference to 'functionInUiThread'. 'functionInUiThread' is responsible for freeing this pointer.
     * @param functionInUiThread Function that will be called.
     */
    static void postToUiThread(void *data, void (*functionInUiThread)(void *));    
        
    
    /**
     * Callback that should be called by the background thread.
     * @param callbackResult CallbackResult reference. This pointer will be freed automatically.
     */
    static void callback(CallbackResult *callbackResult);
    
private:
    
    UiThreadBridge(void *requester, void (*requesterCallbackInUiThread)(CallbackResult *));
    virtual ~UiThreadBridge();
    
    void *requester;
    void (*requesterCallbackInUiThread)(CallbackResult *);
    int unregistered;    
    

    list<CallbackResult *> *callbackResults;    
    pthread_mutex_t callbackResultsMutex;
    int postingToUiThread;
    
    void postToUiThread();
        
   
    static list<UiThreadBridge *> *uiThreadBridges;    
    static pthread_mutex_t uiThreadBridgesMutex;
    
    typedef struct{
        void (*functionInUiThread)(void *);
        void *data;
    }PostToUiThreadData_t;
    
    static int callbackUiThreadReady(void *pCallbackResult);
    static int callbackAuxUiThreadReady(void *pPostToUiThreadData);
};

#endif /* UITHREADHANDLER_H */

