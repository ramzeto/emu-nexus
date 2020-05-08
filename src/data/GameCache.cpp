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
 * File:   GameCache.cpp
 * Author: ram
 * 
 * Created on July 19, 2019, 1:06 AM
 */


#include "GameCache.h"
#include "Database.h"
#include "Logger.h"
#include "Utils.h"
#include "Directory.h"

const string GameCache::DIRECTORY_PREFIX = "game_cache_";

GameCache::GameCache()
{
}

GameCache::GameCache(int64_t id)
{
	this->id = id;
}

GameCache::GameCache(const GameCache &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->timestamp = orig.timestamp;
}

GameCache::GameCache(json_t *json)
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

GameCache::~GameCache()
{
}

int64_t GameCache::getId()
{
	return id;
}

int64_t GameCache::getGameId()
{
	return gameId;
}

void GameCache::setGameId(int64_t gameId)
{
	this->gameId = gameId;
}

string GameCache::getTimestamp()
{
	return timestamp;
}

void GameCache::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int GameCache::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, gameId, timestamp from GameCache where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
		Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int GameCache::save()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into GameCache (gameId, timestamp) values(?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update GameCache set gameId = ?, timestamp = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
	return result;
}

int GameCache::remove()
{
    sqlite3 *db = Database::getInstance()->acquire();
    int result = 1;    
    string command = "delete from GameCache where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    Utils::getInstance()->removeDirectory(getDirectory());
    
    
    return result;
}


string GameCache::getDirectory()
{
    return Directory::getInstance()->getCacheDirectory() + DIRECTORY_PREFIX + to_string(id) + "/";
}

size_t GameCache::getSize()
{
    return Utils::getInstance()->getDirectorySize(getDirectory());
}


json_t *GameCache::toJson()
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

list<GameCache *> *GameCache::getItems()
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameCache *> *items = new list<GameCache *>;
	string query = "select id, gameId, timestamp from GameCache order by timestamp";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameCache *item = new GameCache();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}

list<GameCache*>* GameCache::getItems(int64_t platformId)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameCache *> *items = new list<GameCache *>;
	string query = "select GameCache.id, GameCache.gameId, GameCache.timestamp from GameCache join Game on GameCache.gameId = Game.id where Game.platformId = ? order by GameCache.timestamp";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameCache *item = new GameCache();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}


GameCache *GameCache::getItem(list<GameCache *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameCache *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameCache::releaseItems(list<GameCache *> *items)
{
	for(list<GameCache *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *GameCache::toJsonArray(list<GameCache *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameCache *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

GameCache* GameCache::getGameCache(int64_t gameId)
{
    sqlite3 *db = Database::getInstance()->acquire();
    GameCache *gameCache = NULL;
    string query = "select id, gameId, timestamp from GameCache where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            gameCache = new GameCache();
            gameCache->id = (int64_t)sqlite3_column_int64(statement, 0);
            gameCache->gameId = (int64_t)sqlite3_column_int64(statement, 1);
            gameCache->timestamp = string((const char*) sqlite3_column_text(statement, 2));
        }
    }
    else
    {
        Logger::getInstance()->error("GameCache", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return gameCache;
}
