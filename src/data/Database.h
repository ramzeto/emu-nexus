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
 * File:   Database.h
 * Author: ram
 *
 * Created on February 5, 2019, 11:47 PM
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <pthread.h>
#include <string>

using namespace std;

class Database
{
public:    
    
    /**
     * Acquires the reference to the database and locks the mutex to prevent concurrent access. Method "release()" should be called immediately after operating the database.
     * @return Reference to the database.
     */
    sqlite3 *acquire();
    
    /**
     * Releases the mutex to allow consequent acquires.
     */
    void release();
    
    /**
     * Closes the database file access.
     */
    void close();
    
    /**
     * 
     * @return Database absolute path.
     */
    string getDatabaseFileName();

    /**
     * Starts the database. Handles the changes between application versions.
     */
    void init();    
    
    /**
     * 
     * @return Database instance.
     */
    static Database *getInstance();
    
private:
    static const string DATABASE_FILE_NAME;
    
    Database();
    virtual ~Database();    
    
    sqlite3 *sqlite;
    pthread_mutex_t mutex;
    
    static Database *instance;
};

#endif /* DATABASE_H */

