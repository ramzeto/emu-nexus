/*
 * Copyright (C) 2020 ram
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
 * File:   GameFavorite.cpp
 * Author: ram
 * 
 * Created on May 7, 2020, 10:44 PM
 */

#include "GameFavorite.h"
#include "Database.h"
#include "Logger.h"

GameFavorite::GameFavorite()
{
	this->gameId = 0;
	this->timestamp = "";
}

GameFavorite::GameFavorite(int64_t gameId)
{
	this->gameId = 0;
	this->timestamp = "";

	this->gameId = gameId;
}

GameFavorite::GameFavorite(const GameFavorite &orig)
{
	this->gameId = orig.gameId;
	this->timestamp = orig.timestamp;
}

GameFavorite::GameFavorite(json_t *json)
{
	this->gameId = 0;
	this->timestamp = "";

	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
		if(gameId == 0 && json_string_value(gameIdJson))
		{
			gameId = (int64_t)atol(json_string_value(gameIdJson));
		}
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		if(json_string_value(timestampJson))
		{
			timestamp = string(json_string_value(timestampJson));
		}
		else
		{
			timestamp = "";
		}
	}

}

GameFavorite::~GameFavorite()
{
}

int64_t GameFavorite::getGameId()
{
	return gameId;
}

string GameFavorite::getTimestamp()
{
	return timestamp;
}

void GameFavorite::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int GameFavorite::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select gameId, timestamp from GameFavorite where  gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			gameId = (int64_t)sqlite3_column_int64(statement, 0);
			timestamp = string((const char*) sqlite3_column_text(statement, 1));
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("GameFavorite", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return result;
}

int GameFavorite::save()
{
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	string insert = "insert into GameFavorite (gameId, timestamp) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
                Logger::getInstance()->error("GameFavorite", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return result;
}

int GameFavorite::remove()
{
    int result = 1;
    
    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from GameFavorite where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        Logger::getInstance()->error("GameFavorite", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return result;
}

json_t *GameFavorite::toJson()
{
	json_t *json = json_object();

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	return json;
}

list<GameFavorite *> *GameFavorite::getItems(int64_t gameId, string timestamp)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameFavorite *> *items = new list<GameFavorite *>;

	string query = "select gameId, timestamp from GameFavorite ";
	string selection = "";
	if(gameId >= 0)
	{
		if(selection.length() == 0)
		{
			selection += " where ";
		}
		else
		{
			selection += " and ";
		}
		selection += " gameId = ? ";
	}
	if(timestamp.length() > 0)
	{
		if(selection.length() == 0)
		{
			selection += " where ";
		}
		else
		{
			selection += " and ";
		}
		selection += " timestamp = ? ";
	}
	query += selection;
        query += " order by timestamp desc";
        
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		int paramCount = 1;
		if(gameId >= 0)
		{
			sqlite3_bind_int64(statement, paramCount++, (sqlite3_int64)gameId);
		}
		if(timestamp.length() > 0)
		{
			sqlite3_bind_text(statement, paramCount++, timestamp.c_str(), timestamp.length(), NULL);
		}
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameFavorite *item = new GameFavorite();
			item->gameId = (int64_t)sqlite3_column_int64(statement, 0);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 1));

			items->push_back(item);
		}
	}
	else
	{
            Logger::getInstance()->error("GameFavorite", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return items;
}

GameFavorite *GameFavorite::getItem(list<GameFavorite *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameFavorite *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameFavorite::releaseItems(list<GameFavorite *> *items)
{
	for(list<GameFavorite *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *GameFavorite::toJsonArray(list<GameFavorite *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameFavorite *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
