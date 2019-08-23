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
 * File:   CallbackResult.cpp
 * Author: ram
 * 
 * Created on August 18, 2019, 6:01 PM
 */

#include "CallbackResult.h"

CallbackResult::CallbackResult(void *requester)
{
    this->requester = requester;
    destroyCallback = NULL;
    data = NULL;
    used = 0;
    title = "";
    message = "";
    progress = 0;
    error = 0;
    status = 0;
    type = "";
}

CallbackResult::~CallbackResult()
{
    if(destroyCallback && data)
    {
        destroyCallback(this);
    }
    else if(data)
    {
        free(data);
    }
}

void CallbackResult::setDestroyCallback(void(*destroyCallback) (CallbackResult *))
{
    this->destroyCallback = destroyCallback;
}

void(*CallbackResult::getDestroyCallback())(CallbackResult *) const
{
    return destroyCallback;
}

void CallbackResult::setUsed(int used)
{
    this->used = used;
}

int CallbackResult::getUsed() const
{
    return used;
}

void CallbackResult::setTitle(string title)
{
    this->title = title;
}

string CallbackResult::getTitle() const
{
    return title;
}

void CallbackResult::setMessage(string message)
{
    this->message = message;
}

string CallbackResult::getMessage() const
{
    return message;
}

void CallbackResult::setProgress(int progress)
{
    this->progress = progress;
}

int CallbackResult::getProgress() const
{
    return progress;
}

void CallbackResult::setData(void* data)
{
    this->data = data;
}

void* CallbackResult::getData() const
{
    return data;
}

void CallbackResult::setError(int error)
{
    this->error = error;
}

int CallbackResult::getError() const
{
    return error;
}

void CallbackResult::setStatus(int status)
{
    this->status = status;
}

int CallbackResult::getStatus() const
{
    return status;
}

void CallbackResult::setType(string type)
{
    this->type = type;
}

string CallbackResult::getType() const
{
    return type;
}

void CallbackResult::setRequester(void* requester)
{
    this->requester = requester;
}

void* CallbackResult::getRequester() const
{
    return requester;
}

void CallbackResult::destroy()
{    
    destroy(this);
}

void CallbackResult::destroy(CallbackResult* callbackResult)
{
    delete callbackResult;
}

