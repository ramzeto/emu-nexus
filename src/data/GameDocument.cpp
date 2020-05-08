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
 * File:   GameDocument.cpp
 * Author: ram
 * 
 * Created on June 29, 2019, 5:46 PM
 */

#include "GameDocument.h"
#include "Database.h"
#include "Logger.h"

#include <unistd.h>

const string GameDocument::FILE_PREFIX = "document_";

const int GameDocument::TYPE_MANUAL = 1;
const int GameDocument::TYPE_MAGAZINE = 2;
const int GameDocument::TYPE_BOOK = 3;
const int GameDocument::TYPE_GUIDE = 4;
const int GameDocument::TYPE_OTHER = 5;

GameDocument::GameDocument()
{
    previewImageFileName = "";
}

GameDocument::GameDocument(int64_t id)
{
    this->id = id;
    previewImageFileName = "";
}

GameDocument::GameDocument(const GameDocument &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->type = orig.type;
	this->name = orig.name;
	this->fileName = orig.fileName;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
        this->previewImageFileName = orig.previewImageFileName;
}

GameDocument::GameDocument(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *typeJson = json_object_get(json, "type");
	if(typeJson)
	{
		type = (int64_t)json_integer_value(typeJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *fileNameJson = json_object_get(json, "fileName");
	if(fileNameJson)
	{
		fileName = string(json_string_value(fileNameJson));
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
        
        previewImageFileName = "";
}

GameDocument::~GameDocument()
{
}

int64_t GameDocument::getId()
{
	return id;
}

int64_t GameDocument::getGameId()
{
	return gameId;
}

void GameDocument::setGameId(int64_t gameId)
{
	this->gameId = gameId;
}

int64_t GameDocument::getType()
{
	return type;
}

void GameDocument::setType(int64_t type)
{
	this->type = type;
}

string GameDocument::getName()
{
	return name;
}

void GameDocument::setName(string name)
{
	this->name = name;
}

string GameDocument::getFileName()
{
	return fileName;
}

void GameDocument::setFileName(string fileName)
{
	this->fileName = fileName;
}

int64_t GameDocument::getApiId()
{
	return apiId;
}

void GameDocument::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t GameDocument::getApiItemId()
{
	return apiItemId;
}

void GameDocument::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

void GameDocument::setPreviewImageFileName(string previewImageFileName)
{
    this->previewImageFileName = previewImageFileName;
}

string GameDocument::getPreviewImageFileName()
{
    if(previewImageFileName.length() == 0)
    {
        return fileName + "_preview";
    }
    return previewImageFileName;
}
        
int GameDocument::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, gameId, type, name, fileName, apiId, apiItemId from GameDocument where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			gameId = (int64_t)sqlite3_column_int64(statement, 1);
			type = (int64_t)sqlite3_column_int64(statement, 2);
			name = string((const char*) sqlite3_column_text(statement, 3));
			fileName = string((const char*) sqlite3_column_text(statement, 4));
			apiId = (int64_t)sqlite3_column_int64(statement, 5);
			apiItemId = (int64_t)sqlite3_column_int64(statement, 6);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("GameDocument", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int GameDocument::save()
{        
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into GameDocument (gameId, type, name, fileName, apiId, apiItemId) values(?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 4, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiItemId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("GameDocument", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update GameDocument set gameId = ?, type = ?, name = ?, fileName = ?, apiId = ?, apiItemId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 4, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiItemId);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("GameDocument", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
	return result;
}

int GameDocument::remove()
{
    if(id == 0)
    {
        return 1;
    }
    
    int result = 1;
    
    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from GameDocument where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
            Logger::getInstance()->error("GameDocument", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
        
    unlink(fileName.c_str());
    unlink(getPreviewImageFileName().c_str());
    
    return result;    
}

json_t *GameDocument::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *typeJson = json_integer((json_int_t)type);
	json_object_set_new(json, "type", typeJson);

	json_t *nameJson = json_string(name.c_str());
	json_object_set_new(json, "name", nameJson);

	json_t *fileNameJson = json_string(fileName.c_str());
	json_object_set_new(json, "fileName", fileNameJson);

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *apiItemIdJson = json_integer((json_int_t)apiItemId);
	json_object_set_new(json, "apiItemId", apiItemIdJson);

	return json;
}

list<GameDocument *> *GameDocument::getItems(int64_t gameId)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameDocument *> *items = new list<GameDocument *>;
	string query = "select id, gameId, type, name, fileName, apiId, apiItemId from GameDocument where gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameDocument *item = new GameDocument();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->type = (int64_t)sqlite3_column_int64(statement, 2);
			item->name = string((const char*) sqlite3_column_text(statement, 3));
			item->fileName = string((const char*) sqlite3_column_text(statement, 4));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 6);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameDocument", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}

GameDocument *GameDocument::getItem(list<GameDocument *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameDocument *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameDocument::releaseItems(list<GameDocument *> *items)
{
	for(list<GameDocument *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *GameDocument::toJsonArray(list<GameDocument *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameDocument *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

