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
 * File:   Preferences.h
 * Author: ram
 * 
 * Created March 6, 2019, 11:04 pm
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Preferences 
{
private:
    static const string ELASTICSEARCH_BINARY;
    
    int64_t maximized;
    int64_t windowWidth;
    int64_t windowHeight;
    string lastPath;
    int64_t cacheSize;
    string mameExecutable;
    int64_t elasticsearchPort;
    
    Preferences();
    ~Preferences();

    /**
     * Loads the object from the database
     * @param sqlite
     * @return 1 if the object exists in the database and loads successfully, 0 otherwise.
     */
    int load(sqlite3 *sqlite);

    static Preferences *instance;

public:
    static const unsigned int ELASTICSEARCH_MIN_PORT;
    static const unsigned int ELASTICSEARCH_MAX_PORT;
    
    int64_t getMaximized();
    void setMaximized(int64_t maximized);
    int64_t getWindowWidth();
    void setWindowWidth(int64_t windowWidth);
    int64_t getWindowHeight();
    void setWindowHeight(int64_t windowHeight);
    string getLastPath();
    void setLastPath(string lastPath);
    int64_t getCacheSize();
    void setCacheSize(int64_t cacheSize);
    void setMameExecutable(string mameExecutable);
    string getMameExecutable() const;
    int64_t getElasticsearchPort();
    void setElasticsearchPort(int64_t searchEnginePort);
    string getElasticseachBinary();

    
    /**
     * Saves the object to the database.
     * @param sqlite
     * @return  0 on success
     */
    int save(sqlite3 *sqlite);
    
    /**
     * 
     * @return Settings instance.
     */
    static Preferences *getInstance();    
};

#endif