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
 * File:   Preferences.cpp
 * Author: ram
 * 
 * Created March 6, 2019, 11:04 pm
*/

#include "Preferences.h"
#include "Database.h"
#include "Logger.h"
#include "Database.h"
#include "Directory.h"

Preferences *Preferences::instance = NULL;

const string Preferences::ELASTICSEARCH_BINARY = "bin/elasticsearch";
const unsigned int Preferences::ELASTICSEARCH_MIN_PORT = 9301;
const unsigned int Preferences::ELASTICSEARCH_MAX_PORT = 9350;

Preferences::Preferences()
{
    maximized = 0;
    windowWidth = 1024;
    windowHeight = 768;
    lastPath = "";
    cacheSize = 9000;
    mameExecutable = "mame";
    elasticsearchPort = ELASTICSEARCH_MIN_PORT;
}

Preferences::~Preferences()
{
}

int64_t Preferences::getMaximized()
{
	return maximized;
}

void Preferences::setMaximized(int64_t maximized)
{
	this->maximized = maximized;
}

int64_t Preferences::getWindowWidth()
{
	return windowWidth;
}

void Preferences::setWindowWidth(int64_t windowWidth)
{
	this->windowWidth = windowWidth;
}

int64_t Preferences::getWindowHeight()
{
	return windowHeight;
}

void Preferences::setWindowHeight(int64_t windowHeight)
{
	this->windowHeight = windowHeight;
}

string Preferences::getLastPath()
{
	return lastPath;
}

void Preferences::setLastPath(string lastPath)
{
	this->lastPath = lastPath;
}

int64_t Preferences::getCacheSize()
{
    return cacheSize;
}

void Preferences::setCacheSize(int64_t cacheSize)
{
    this->cacheSize = cacheSize;
}

void Preferences::setMameExecutable(string mameExecutable)
{
    this->mameExecutable = mameExecutable;
}

string Preferences::getMameExecutable() const
{
    return mameExecutable;
}


int64_t Preferences::getElasticsearchPort()
{
    return elasticsearchPort;
}

void Preferences::setElasticsearchPort(int64_t elasticsearchPort)
{
    this->elasticsearchPort = elasticsearchPort;
}

int Preferences::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select maximized, windowWidth, windowHeight, lastPath, cacheSize, mameExecutable, elasticsearchPort from Preferences";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			maximized = (int64_t)sqlite3_column_int64(statement, 0);
			windowWidth = (int64_t)sqlite3_column_int64(statement, 1);
			windowHeight = (int64_t)sqlite3_column_int64(statement, 2);
			lastPath = string((const char*) sqlite3_column_text(statement, 3));
                        cacheSize = (int64_t) sqlite3_column_int64(statement, 4);
                        mameExecutable = string((const char*) sqlite3_column_text(statement, 5));
                        elasticsearchPort = (int64_t)sqlite3_column_int64(statement, 6);
            
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("Preferences", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return result;
}

int Preferences::save()
{
    sqlite3 *db = Database::getInstance()->acquire();
    int result = 1;
    int exists = 0;
    string query = "select maximized from Preferences";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            exists = 1;
        }
    }
    else
    {
        Logger::getInstance()->error("Preferences", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);


    string insertOrUpdate;
    if (exists)
    {
        insertOrUpdate = "update Preferences set maximized = ?, windowWidth = ?, windowHeight = ?, lastPath = ?, cacheSize = ?, mameExecutable = ?, elasticsearchPort = ?";
    }
    else
    {
        insertOrUpdate = "insert into Preferences (maximized, windowWidth, windowHeight, lastPath, cacheSize, mameExecutable, elasticsearchPort) values(?, ?, ?, ?, ?, ?, ?)";
    }

    if (sqlite3_prepare_v2(db, insertOrUpdate.c_str(), insertOrUpdate.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64) maximized);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64) windowWidth);
        sqlite3_bind_int64(statement, 3, (sqlite3_int64) windowHeight);
        sqlite3_bind_text(statement, 4, lastPath.c_str(), lastPath.length(), NULL);
        sqlite3_bind_int64(statement, 5, (sqlite3_int64) cacheSize);
        sqlite3_bind_text(statement, 6, mameExecutable.c_str(), mameExecutable.length(), NULL);
        sqlite3_bind_int64(statement, 7, (sqlite3_int64) elasticsearchPort);
        
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        Logger::getInstance()->error("Preferences", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insertOrUpdate);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
                
    return result;
}

string Preferences::getElasticseachBinary()
{
    return Directory::getInstance()->getElasticseachDirectory() + ELASTICSEARCH_BINARY;
}


Preferences* Preferences::getInstance()
{
    if(!instance)
    {
        instance = new Preferences();        
        instance->load();
    }
    
    return instance;
}
