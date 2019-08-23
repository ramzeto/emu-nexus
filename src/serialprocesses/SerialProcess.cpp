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
 * File:   SerialProcess.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 10:19 PM
 */

#include "SerialProcess.h"

#include <iostream>

const int SerialProcess::STATUS_IDLE = -2;
const int SerialProcess::STATUS_RUNNING = -1;
const int SerialProcess::STATUS_SUCCESS = 0;
const int SerialProcess::STATUS_FAIL = 1;

SerialProcess::SerialProcess(string type, void *requester, void (*statusCallback)(CallbackResult *))
{
    this->type = type;
    this->requester = requester;
    this->statusCallback = statusCallback;
    status = STATUS_IDLE;
}

SerialProcess::~SerialProcess()
{
}

void* SerialProcess::getRequester()
{
    return requester;
}

void (*SerialProcess::getStatusCallback())(CallbackResult *)
{
    return statusCallback;
}

string SerialProcess::getType()
{
    return type;
}

int SerialProcess::getStatus()
{
    return status;
}

int SerialProcess::execute()
{
    status = STATUS_SUCCESS;
    return status;
}

void SerialProcess::postStatus(string title, string message, int progress, void *data, void (destroyCallback)(CallbackResult *))
{    
    if(statusCallback)
    {
        CallbackResult *callbackResult = new CallbackResult(requester);
        callbackResult->setType(type);
        callbackResult->setStatus(status);
        callbackResult->setProgress(progress);
        callbackResult->setTitle(title);
        callbackResult->setMessage(message);
        callbackResult->setData(data);
        callbackResult->setDestroyCallback(destroyCallback);
    
        statusCallback(callbackResult);        
    }            
}



