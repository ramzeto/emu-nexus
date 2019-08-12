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
 * File:   RecentGame.cpp
 * Author: ram
 * 
 * Created on July 21, 2019, 8:46 PM
 */

#include "RecentGame.h"
#include <iostream>

const unsigned int RecentGame::LIMIT = 20;

RecentGame::RecentGame()
{
}

RecentGame::RecentGame(int64_t gameId)
{
	this->gameId = gameId;
}

RecentGame::RecentGame(const RecentGame &orig)
{
	this->gameId = orig.gameId;
	this->timestamp = orig.timestamp;
}

RecentGame::RecentGame(json_t *json)
{
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

RecentGame::~RecentGame()
{
}

int64_t RecentGame::getGameId()
{
	return gameId;
}

string RecentGame::getTimestamp()
{
	return timestamp;
}

void RecentGame::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int RecentGame::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select gameId, timestamp from RecentGame where  gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
		cerr << "RecentGame::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int RecentGame::save(sqlite3 *sqlite)
{
	int result = 1;
	string insert = "insert into RecentGame (gameId, timestamp) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		cerr << "RecentGame::save " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int RecentGame::remove(sqlite3* sqlite)
{
    int result = 1;
    
    string command = "delete from RecentGame where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "RecentGame::remove " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);
           
    return result;
}

json_t *RecentGame::toJson()
{
	json_t *json = json_object();

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	return json;
}

list<RecentGame *> *RecentGame::getItems(sqlite3 *sqlite, int ascending)
{
	list<RecentGame *> *items = new list<RecentGame *>;
	string query = "select gameId, timestamp from RecentGame order by timestamp";
        if(!ascending)
        {
            query += " desc";
        }
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			RecentGame *item = new RecentGame();
			item->gameId = (int64_t)sqlite3_column_int64(statement, 0);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 1));

			items->push_back(item);
		}
	}
	else
	{
		cerr << "RecentGame::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

RecentGame *RecentGame::getItem(list<RecentGame *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<RecentGame *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void RecentGame::releaseItems(list<RecentGame *> *items)
{
	for(list<RecentGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *RecentGame::toJsonArray(list<RecentGame *> *items)
{
	json_t *jsonArray = json_array();
	for(list<RecentGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}


