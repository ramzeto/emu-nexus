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
 * File:   Notifications.h
 * Author: ram
 *
 * Created on June 17, 2019, 8:22 PM
 */

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <string>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

    static const string NOTIFICATION_PLATFORM_UPDATED = "NOTIFICATION_PLATFORM_UPDATED";
    static const string NOTIFICATION_GAME_UPDATED = "NOTIFICATION_GAME_UPDATED";
    static const string NOTIFICATION_DIRECTORY_ADDED = "NOTIFICATION_DIRECTORY_ADDED";
    static const string NOTIFICATION_FAVORITES_UPDATED = "NOTIFICATION_FAVORITES_UPDATED";
    static const string NOTIFICATION_GAME_ACTIVITY_UPDATED = "NOTIFICATION_GAME_ACTIVITY_UPDATED";



#ifdef __cplusplus
}
#endif

#endif /* NOTIFICATIONS_H */

