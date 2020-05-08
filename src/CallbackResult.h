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
 * File:   CallbackResult.h
 * Author: ram
 *
 * Created on August 18, 2019, 6:01 PM
 */

#ifndef CALLBACKRESULT_H
#define CALLBACKRESULT_H

#include <string>

using namespace std;

/**
 * Class made for exchanging data between objects. It is designed ti be thread safe.
 */
class CallbackResult 
{
public:
    
    /**
     * 
     * @param requester Pointer to the requester object that will receive the result.
     */
    CallbackResult(void *requester);
    
    /**
     * If the data pointer requires custom deletion, this callback should be assigned. It will be called during the destruction process of the CallbackResult object. The 'destroyCallback' should only destroy the data pointer.
     * @param destroyCallback
     */
    void setDestroyCallback(void(*destroyCallback) (CallbackResult *));
    
    void(*getDestroyCallback())(CallbackResult *) const;
    void setUsed(int used);
    int getUsed() const;
    void setTitle(string title);
    string getTitle() const;
    void setMessage(string message);
    string getMessage() const;
    void setProgress(int progress);
    int getProgress() const;
    void setData(void* data);
    void* getData() const;
    void setError(int error);
    int getError() const;
    void setStatus(int status);
    int getStatus() const;
    void setType(string type);
    string getType() const;
    void setRequester(void* requester);
    void* getRequester() const;
    
    void destroy();
    
    
private:    
    virtual ~CallbackResult();
    
    void *requester;
    string type;
    int status;
    int error;
    void *data;
    int progress;
    string title;
    string message;
    int used;    
    void (*destroyCallback)(CallbackResult *);        
    
    static void destroy(CallbackResult *callbackResult);
};

#endif /* CALLBACKRESULT_H */

