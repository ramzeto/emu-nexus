/*
 * Copyright (C) 2020 ram
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
 * File:   Notification.cpp
 * Author: ram
 * 
 * Created on September 7, 2020, 12:35 PM
 */

#include "Notification.h"

Notification::Notification()
{
    listener = NULL;
    data = NULL;
}

Notification::~Notification()
{
    if(data)
    {
        free(data);
    }
}

string Notification::getMessage() const
{
    return message;
}


int Notification::getProgress() const
{
    return progress;
}

void* Notification::getData() const
{
    return data;
}

int Notification::getError() const
{
    return error;
}

int Notification::getStatus() const
{
    return status;
}

string Notification::getName() const
{
    return name;
}

void* Notification::getListener() const
{
    return listener;
}

