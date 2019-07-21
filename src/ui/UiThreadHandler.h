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
 * File:   UiThreadHandler.h
 * Author: ram
 *
 * Created on April 27, 2019, 12:07 AM
 */

#ifndef UITHREADHANDLER_H
#define UITHREADHANDLER_H

#include <list>
#include <pthread.h>

using namespace std;

/**
 * Class used to post events and to share data from background threads to the main thread. Uses GTK3 approach (https://developer.gnome.org/gdk3/stable/gdk3-Threads.html).
 * @TODO Find a way to safely delete UiThreadHandler from different threads.
 */
class UiThreadHandler {
public:
    typedef struct
    {
        UiThreadHandler *uiThreadHandler;
        void *data;
    }Result_t;
    
    
    /**
     * 
     * @param requesterInUiThread Pointer to the object that requested the operation in the UI thread.
     * @param callbackInUiThread Pointer to the callback that will receive the result from the operation. Receives a pointer to a Result_t structure. The requester in the UI thread is responsible to free this pointer, the data pointer of Result_t is freed by calling releaseResult. The callbackInUiThread should return G_SOURCE_REMOVE.
     */
    UiThreadHandler(void *requesterInUiThread, int (*callbackInUiThread)(void *));
    
    virtual ~UiThreadHandler();
    
    /**
     * 
     * @return Pointer to the requester object in the UI thread.
     */
    void *getRequesterInUiThread();

    
    /**
     * Deletes the Result_t from the operation. The Result_t->data pointer will be freed automatically. If it contains pointers to dynamically allocated objects, this should be freed by the requester. This method IS REQUIRED to be called immediately after the requester uses the data to allow the next data (if available) to be processed.
     */
    static void releaseResult(Result_t *result);
        
    
    
    /**
     * Callback that should be called by the background thread. This function will execute the uiThreadHandler->callback in the UI thread.
     * @param uiThreadHandler UiThreadHandler reference. The requester in the UI thread is responsible to free it.
     * @param data Data from the operation executed in the background thread. It will be freed when the UiThreadHandler gets deleted, but the requester in the UI thread is responsible to free any dynamically allocated data that contains. CANNOT be NULL.
     */
    static void callback(void *pUiThreadHandler, void *data);
    
    
private:
    void *requesterInUiThread;
    int (*callbackInUiThread)(void *);
    list<void *> *datas;
    pthread_mutex_t dataMutex;
    int postingData;
    
    void pushData(void *data);
    void postToUiThread();
        
};

#endif /* UITHREADHANDLER_H */

