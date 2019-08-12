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
 * File:   Platform.cpp
 * Author: ram
 * 
 * Created March 26, 2019, 8:45 pm
*/

#include "Platform.h"
#include "Utils.h"
#include "PlatformImage.h"
#include "Directory.h"
#include "CacheGame.h"

#include <iostream>

const string Platform::DIRECTORY_PREFIX = "platform_";

Platform::Platform()
{
}

Platform::Platform(int64_t id)
{
	this->id = id;
	this->name = "";
	this->description = "";
	this->command = "";
        this->deflate = 0;
        this->deflateFileExtensions = "";
	this->apiId = 0;
	this->apiItemId = 0;
}

Platform::Platform(const Platform &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->description = orig.description;
	this->command = orig.command;
	this->deflate = orig.deflate;
	this->deflateFileExtensions = orig.deflateFileExtensions;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

Platform::Platform(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *descriptionJson = json_object_get(json, "description");
	if(descriptionJson)
	{
		description = string(json_string_value(descriptionJson));
	}

	json_t *commandJson = json_object_get(json, "command");
	if(commandJson)
	{
		command = string(json_string_value(commandJson));
	}

	json_t *deflateJson = json_object_get(json, "deflate");
	if(deflateJson)
	{
		deflate = (int64_t)json_integer_value(deflateJson);
	}

	json_t *deflateFileExtensionsJson = json_object_get(json, "deflateFileExtensions");
	if(deflateFileExtensionsJson)
	{
		deflateFileExtensions = string(json_string_value(deflateFileExtensionsJson));
	}

	json_t *apiIdJson = json_object_get(json, "apiId");
	if(apiIdJson)
	{
		apiId = (int64_t)json_integer_value(apiIdJson);
	}

	json_t *apiItemIdJson = json_object_get(json, "apiItemId");
	if(apiItemIdJson)
	{
		apiItemId = (int64_t)json_integer_value(apiItemIdJson);
	}

}

Platform::~Platform()
{
}

int64_t Platform::getId()
{
	return id;
}

string Platform::getName()
{
	return name;
}

void Platform::setName(string name)
{
	this->name = name;
}

string Platform::getDescription()
{
	return description;
}

void Platform::setDescription(string description)
{
	this->description = description;
}

string Platform::getCommand()
{
	return command;
}

void Platform::setCommand(string command)
{
	this->command = command;
}

int64_t Platform::getDeflate()
{
	return deflate;
}

void Platform::setDeflate(int64_t deflate)
{
	this->deflate = deflate;
}

string Platform::getDeflateFileExtensions()
{
	return deflateFileExtensions;
}

void Platform::setDeflateFileExtensions(string deflateFileExtensions)
{
	this->deflateFileExtensions = deflateFileExtensions;
}

int64_t Platform::getApiId()
{
	return apiId;
}

void Platform::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t Platform::getApiItemId()
{
	return apiItemId;
}

void Platform::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int Platform::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select id, name, description, command, deflate, deflateFileExtensions, apiId, apiItemId from Platform where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			name = string((const char*) sqlite3_column_text(statement, 1));
			description = string((const char*) sqlite3_column_text(statement, 2));
			command = string((const char*) sqlite3_column_text(statement, 3));
			deflate = (int64_t)sqlite3_column_int64(statement, 4);
			deflateFileExtensions = string((const char*) sqlite3_column_text(statement, 5));
			apiId = (int64_t)sqlite3_column_int64(statement, 6);
			apiItemId = (int64_t)sqlite3_column_int64(statement, 7);
			result = 1;
		}
	}
	else
	{
		cerr << "Platform::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int Platform::save(sqlite3 *sqlite)
{
	int result = 1;

	if(id == 0)
	{
		string insert = "insert into Platform (name, description, command, deflate, deflateFileExtensions, apiId, apiItemId) values(?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 2, description.c_str(), description.length(), NULL);
			sqlite3_bind_text(statement, 3, command.c_str(), command.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)deflate);
			sqlite3_bind_text(statement, 5, deflateFileExtensions.c_str(), deflateFileExtensions.length(), NULL);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)apiItemId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "Platform::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);                
	}
	else
	{
		string update = "update Platform set name = ?, description = ?, command = ?, deflate = ?, deflateFileExtensions = ?, apiId = ?, apiItemId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 2, description.c_str(), description.length(), NULL);
			sqlite3_bind_text(statement, 3, command.c_str(), command.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)deflate);
			sqlite3_bind_text(statement, 5, deflateFileExtensions.c_str(), deflateFileExtensions.length(), NULL);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)apiItemId);
			sqlite3_bind_int64(statement, 8, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			cerr << "Platform::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}

        // Creates the media directory where images will be stored
        Utils::getInstance()->makeDirectory(getMediaDirectory());
        
	return result;
}

int Platform::remove(sqlite3 *sqlite)
{        
    int result = 1;
 
    // Removes GameDeveloper records
    string command = "delete from GameDeveloper where gameId in (select Game.id from GameDeveloper join Game on Game.id = GameDeveloper.gameId join Platform on Platform.id = Game.platformId where Platform.id = ?)";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);
    
    if(!result)
    {
        // Removes GamePublisher records
        command = "delete from GamePublisher where gameId in (select Game.id from GamePublisher join Game on Game.id = GamePublisher.gameId join Platform on Platform.id = Game.platformId where Platform.id = ?)";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }
    
    if(!result)
    {
        // Removes GameGenre records
        command = "delete from GameGenre where gameId in (select Game.id from GameGenre join Game on Game.id = GameGenre.gameId join Platform on Platform.id = Game.platformId where Platform.id = ?)";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }    

    if(!result)
    {
        // Removes GameImage records
        command = "delete from GameImage where gameId in (select Game.id from GameImage join Game on Game.id = GameImage.gameId join Platform on Platform.id = Game.platformId where Platform.id = ?)";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }
    
    if(!result)
    {
        // Removes RecentGame records
        command = "delete from RecentGame where gameId in (select Game.id from RecentGame join Game on Game.id = RecentGame.gameId join Platform on Platform.id = Game.platformId where Platform.id = ?)";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);
    }
    
    // Removes CacheGame records
    list<CacheGame *> *cacheGames = CacheGame::getItems(sqlite, id);
    for(unsigned int index = 0; index < cacheGames->size(); index++)
    {
        CacheGame *cacheGame = CacheGame::getItem(cacheGames, index);
        cacheGame->remove(sqlite);
    }
    CacheGame::releaseItems(cacheGames);
    
    if(!result)
    {
        // Removes Game records
        command = "delete from Game where platformId = ?";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }

    if(!result)
    {
        // Removes Platform record
        command = "delete from Platform where id = ?";
        if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            cerr << "Platform::remove " << sqlite3_errmsg(sqlite) << endl;
        }
        sqlite3_finalize(statement);        
    }
    
    if(!result)
    {
        // Removes PlatformImage records
        list<PlatformImage *> *platformImages = PlatformImage::getItems(sqlite, id);
        for(unsigned int c = 0; c < platformImages->size(); c++)
        {
            PlatformImage *platformImage = PlatformImage::getItem(platformImages, c);
            platformImage->remove(sqlite);
        }
        PlatformImage::releaseItems(platformImages);
        
        Utils::getInstance()->removeDirectory(getMediaDirectory());
    }
    
    return result;
}

string Platform::getMediaDirectory()
{
    return Directory::getInstance()->getMediaDirectory() + DIRECTORY_PREFIX + to_string(id) + "/";
}


json_t *Platform::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *nameJson = json_string(name.c_str());
	json_object_set_new(json, "name", nameJson);

	json_t *descriptionJson = json_string(description.c_str());
	json_object_set_new(json, "description", descriptionJson);

	json_t *commandJson = json_string(command.c_str());
	json_object_set_new(json, "command", commandJson);

	json_t *deflateJson = json_integer((json_int_t)deflate);
	json_object_set_new(json, "deflate", deflateJson);

	json_t *deflateFileExtensionsJson = json_string(deflateFileExtensions.c_str());
	json_object_set_new(json, "deflateFileExtensions", deflateFileExtensionsJson);

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *apiItemIdJson = json_integer((json_int_t)apiItemId);
	json_object_set_new(json, "apiItemId", apiItemIdJson);

	return json;
}

list<Platform *> *Platform::getItems(sqlite3 *sqlite)
{
	list<Platform *> *items = new list<Platform *>;

	string query = "select id, name, description, command, deflate, deflateFileExtensions, apiId, apiItemId from Platform";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Platform *item = new Platform();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->description = string((const char*) sqlite3_column_text(statement, 2));
			item->command = string((const char*) sqlite3_column_text(statement, 3));
			item->deflate = (int64_t)sqlite3_column_int64(statement, 4);
			item->deflateFileExtensions = string((const char*) sqlite3_column_text(statement, 5));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 6);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 7);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "Platform::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

Platform *Platform::getItem(list<Platform *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<Platform *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void Platform::releaseItems(list<Platform *> *items)
{
	for(list<Platform *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *Platform::toJsonArray(list<Platform *> *items)
{
	json_t *jsonArray = json_array();
	for(list<Platform *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
