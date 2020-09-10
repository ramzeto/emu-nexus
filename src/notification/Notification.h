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
 * File:   Notification.h
 * Author: ram
 *
 * Created on September 7, 2020, 12:35 PM
 */

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>

using namespace std;

class Notification 
{
friend class NotificationManager;

public:
    Notification();
    virtual ~Notification();
    
    string getMessage() const;
    int getProgress() const;
    void* getData() const;
    int getError() const;
    int getStatus() const;
    string getName() const;
    void *getListener() const;
            
private:
    string name;
    void *listener;
    int status;
    int error;
    void *data;
    int progress;
    string message;
};

#endif /* NOTIFICATION_H */

