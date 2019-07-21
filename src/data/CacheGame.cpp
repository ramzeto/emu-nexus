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
 * File:   CacheGame.cpp
 * Author: ram
 * 
 * Created on July 19, 2019, 1:06 AM
 */


#include "CacheGame.h"
#include "Settings.h"
#include "Utils.h"
#include <iostream>

const string CacheGame::DIRECTORY_PREFIX = "cache_game_";

CacheGame::CacheGame()
{
}

CacheGame::CacheGame(int64_t id)
{
	this->id = id;
}

CacheGame::CacheGame(const CacheGame &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->timestamp = orig.timestamp;
}

CacheGame::CacheGame(json_t *json)
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

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
	}

}

CacheGame::~CacheGame()
{
}

int64_t CacheGame::getId()
{
	return id;
}

int64_t CacheGame::getGameId()
{
	return gameId;
}

void CacheGame::setGameId(int64_t gameId)
{
	this->gameId = gameId;
}

string CacheGame::getTimestamp()
{
	return timestamp;
}

void CacheGame::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int CacheGame::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select id, gameId, timestamp from CacheGame where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			gameId = (int64_t)sqlite3_column_int64(statement, 1);
			timestamp = string((const char*) sqlite3_column_text(statement, 2));
			result = 1;
		}
	}
	else
	{
		cerr << "CacheGame::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int CacheGame::save(sqlite3 *sqlite)
{
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into CacheGame (gameId, timestamp) values(?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "CacheGame::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update CacheGame set gameId = ?, timestamp = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			cerr << "CacheGame::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	return result;
}

int CacheGame::remove(sqlite3* sqlite)
{
    int result = 1;
    
    string command = "delete from CacheGame where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "CacheGame::remove " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);
    
    
    Utils::getInstance()->removeDirectory(getDirectory());
    
    return result;
}


string CacheGame::getDirectory()
{
    return Settings::getInstance()->getCacheDirectory() + DIRECTORY_PREFIX + to_string(id) + "/";
}

size_t CacheGame::getSize()
{
    return Utils::getInstance()->getDirectorySize(getDirectory());
}


json_t *CacheGame::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	return json;
}

list<CacheGame *> *CacheGame::getItems(sqlite3 *sqlite)
{
	list<CacheGame *> *items = new list<CacheGame *>;
	string query = "select id, gameId, timestamp from CacheGame order by timestamp";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			CacheGame *item = new CacheGame();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));

			items->push_back(item);
		}
	}
	else
	{
		cerr << "CacheGame::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

CacheGame *CacheGame::getItem(list<CacheGame *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<CacheGame *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void CacheGame::releaseItems(list<CacheGame *> *items)
{
	for(list<CacheGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *CacheGame::toJsonArray(list<CacheGame *> *items)
{
	json_t *jsonArray = json_array();
	for(list<CacheGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

CacheGame* CacheGame::getCacheGame(sqlite3 *sqlite,  int64_t gameId)
{
    CacheGame *cacheGame = NULL;
    string query = "select id, gameId, timestamp from CacheGame where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            cacheGame = new CacheGame();
            cacheGame->id = (int64_t)sqlite3_column_int64(statement, 0);
            cacheGame->gameId = (int64_t)sqlite3_column_int64(statement, 1);
            cacheGame->timestamp = string((const char*) sqlite3_column_text(statement, 2));
        }
    }
    else
    {
        cerr << "CacheGame::load " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    return cacheGame;
}
