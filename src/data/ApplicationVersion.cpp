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
 * File:   ApplicationVersion.cpp
 * Author: ram
 * 
 * Created April 14, 2019, 3:33 pm
*/

#include "ApplicationVersion.h"
#include "Database.h"
#include "Logger.h"

#include <iostream>

ApplicationVersion::ApplicationVersion()
{
}

ApplicationVersion::ApplicationVersion(string version)
{
	this->version = version;
}

ApplicationVersion::ApplicationVersion(const ApplicationVersion &orig)
{
	this->version = orig.version;
	this->timestamp = orig.timestamp;
}

ApplicationVersion::ApplicationVersion(json_t *json)
{
	json_t *versionJson = json_object_get(json, "version");
	if(versionJson)
	{
		version = string(json_string_value(versionJson));
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
	}

}

ApplicationVersion::~ApplicationVersion()
{
}

string ApplicationVersion::getVersion()
{
	return version;
}

string ApplicationVersion::getTimestamp()
{
	return timestamp;
}

void ApplicationVersion::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int ApplicationVersion::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select version, timestamp from ApplicationVersion where  version = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(statement, 1, version.c_str(), version.length(), NULL);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			version = string((const char*) sqlite3_column_text(statement, 0));
			timestamp = string((const char*) sqlite3_column_text(statement, 1));
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("ApiDatabase", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int ApplicationVersion::save()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 1;
	string insert = "insert into ApplicationVersion (version, timestamp) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(statement, 1, version.c_str(), version.length(), NULL);
		sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		Logger::getInstance()->error("ApiDatabase", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

json_t *ApplicationVersion::toJson()
{
	json_t *json = json_object();

	json_t *versionJson = json_string(version.c_str());
	json_object_set_new(json, "version", versionJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	return json;
}

list<ApplicationVersion *> *ApplicationVersion::getItems()
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<ApplicationVersion *> *items = new list<ApplicationVersion *>;
	string query = "select version, timestamp from ApplicationVersion order by timestamp";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			ApplicationVersion *item = new ApplicationVersion();
			item->version = string((const char*) sqlite3_column_text(statement, 0));
			item->timestamp = string((const char*) sqlite3_column_text(statement, 1));

			items->push_back(item);
		}
	}
	else
	{
                Logger::getInstance()->error("ApiDatabase", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}

ApplicationVersion *ApplicationVersion::getItem(list<ApplicationVersion *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<ApplicationVersion *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void ApplicationVersion::releaseItems(list<ApplicationVersion *> *items)
{
	for(list<ApplicationVersion *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *ApplicationVersion::toJsonArray(list<ApplicationVersion *> *items)
{
	json_t *jsonArray = json_array();
	for(list<ApplicationVersion *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}


ApplicationVersion* ApplicationVersion::getCurrentVersion()
{    
    ApplicationVersion *applicationVersion = NULL;
    list<ApplicationVersion *> *applicationVersions = getItems();
    if(applicationVersions->size() > 0)
    {        
        applicationVersion = new ApplicationVersion(*getItem(applicationVersions, applicationVersions->size() - 1));
    }
    releaseItems(applicationVersions);
    
    return applicationVersion;
}