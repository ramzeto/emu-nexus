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
 * File:   Database.cpp
 * Author: ram
 * 
 * Created on February 5, 2019, 11:47 PM
 */

#include "Database.h"
#include "Utils.h"
#include "Settings.h"
#include "Build.h"
#include "ApplicationVersion.h"

#include <stdlib.h>
#include <string>
#include <fstream>
#include <list>
#include <iostream>

using namespace std;

const string Database::DATABASE_FILE_NAME =  "emu-nexus.sqlite";

Database *Database::instance = NULL;

Database::Database()
{
    sqlite = NULL;
    mutex = PTHREAD_MUTEX_INITIALIZER;
}

Database::~Database()
{
    if(sqlite)
    {
        sqlite3_close(sqlite);
    }
}

sqlite3* Database::acquire()
{
    pthread_mutex_lock(&mutex);

    if(!sqlite)
    {
        if (sqlite3_open_v2(getDatabaseFileName().c_str(), &sqlite, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
        {
            cerr << "DataBase::" << __FUNCTION__ << endl;
            exit(EXIT_FAILURE);
        }
    }
    
    return sqlite;        
}

void Database::release()
{
    pthread_mutex_unlock(&mutex);
}

void Database::close()
{
    sqlite3_close(sqlite);
    sqlite = NULL;
}


string Database::getDatabaseFileName()
{
    return Settings::getInstance()->getDataDirectory() + DATABASE_FILE_NAME;
}

void Database::init()
{
    int firstTime = 0;
    string version = "";
    if(!Utils::getInstance()->fileExists(getDatabaseFileName()))
    {
        ofstream databaseStream(getDatabaseFileName().c_str());
        databaseStream.close();
        
        firstTime = 1;
    }
    
    sqlite3 *sqlite = acquire();
    if(!firstTime)
    {
        ApplicationVersion *applicationVersion = ApplicationVersion::getCurrentVersion(sqlite);
        if(applicationVersion)
        {
            version = applicationVersion->getVersion();
            delete applicationVersion;
        }
    }
    
    
    list<string> commands;
    
    // New installation
    if(version.length() == 0)
    {
        commands.push_back("CREATE TABLE IF NOT EXISTS ApplicationVersion(version text, timestamp text, unique(version))");
        
        commands.push_back("CREATE TABLE IF NOT EXISTS Preferences(maximized integer, windowWidth integer, windowHeight integer, lastPath text)");
        commands.push_back("CREATE TABLE IF NOT EXISTS Settings(dataDirectory text, cacheDirectory text, cacheSize integer, mameExecutable text, elasticsearchPort integer)");
        
        commands.push_back("CREATE TABLE IF NOT EXISTS ApiDatabase(apiId integer, md5sum text, timestamp text, unique(apiId, md5sum) on conflict replace)");
        
        commands.push_back("CREATE TABLE IF NOT EXISTS Platform(id integer primary key, name text, description text, command text, deflate integer, deflateFileExtensions text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS PlatformImage(id integer primary key, platformId integer, type integer, fileName text, external integer, apiId integer, apiItemId integer, url text, downloaded integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS Genre(id integer primary key, name text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS Developer(id integer primary key, name text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS Publisher(id integer primary key, name text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS EsrbRating(id integer primary key, name text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS Game(id integer primary key, platformId integer, esrbRatingId integer, name text, description text, releaseDate text, fileName text, notes text, command text, deflate integer, deflateFileExtensions text, timestamp text, apiId integer, apiItemId integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS GameGenre(gameId integer, genreId integer, unique(gameId, genreId) on conflict replace)");
        commands.push_back("CREATE TABLE IF NOT EXISTS GameDeveloper(gameId integer, developerId integer, unique(gameId, developerId) on conflict replace)");
        commands.push_back("CREATE TABLE IF NOT EXISTS GamePublisher(gameId integer, publisherId integer, unique(gameId, publisherId) on conflict replace)");        
        commands.push_back("CREATE TABLE IF NOT EXISTS GameImage(id integer primary key, gameId integer, type integer, fileName text, external integer, apiId integer, apiItemId integer, url text, downloaded integer)");
        commands.push_back("CREATE TABLE IF NOT EXISTS GameDocument(id integer primary key, gameId integer, type integer, name text, fileName text, apiId integer, apiItemId integer)");
        
        commands.push_back("CREATE TABLE IF NOT EXISTS RecentGame(gameId integer, timestamp text, unique(gameId) on conflict replace)");
        commands.push_back("CREATE TABLE IF NOT EXISTS CacheGame(id integer primary key, gameId integer, timestamp text)");
    }
    
    
    
    for(list<string>::iterator command = commands.begin(); command != commands.end(); command++)
    {
        sqlite3_stmt *statement;

        if (sqlite3_prepare_v2(sqlite, (*command).c_str(), (*command).length(), &statement, NULL) == SQLITE_OK)
        {
            if(sqlite3_step(statement) != SQLITE_DONE)
            {
                cerr << "DataBase::" << __FUNCTION__ << " " << sqlite3_errmsg(sqlite) << endl;
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            cerr << "DataBase::init sqlite3_errmsg: " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }
    
    if(version.compare(BUILD_VERSION) != 0)
    {
        ApplicationVersion *applicationVersion = new ApplicationVersion(BUILD_VERSION);
        applicationVersion->setTimestamp(Utils::getInstance()->nowIsoDateTime());
        applicationVersion->save(sqlite);    
        delete applicationVersion;
    } 
    
    release();        
}

Database* Database::getInstance()
{
    if(!instance)
    {
        instance = new Database();
    }
    
    return instance;
}


