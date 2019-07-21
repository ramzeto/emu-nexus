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

/**
*
* @author Tlachia auto generador
* @date March 27, 2019, 9:51 pm
*/

#include "GamePublisher.h"

#include <iostream>

GamePublisher::GamePublisher()
{
}

GamePublisher::GamePublisher(int64_t gameId, int64_t publisherId)
{
	this->gameId = gameId;
	this->publisherId = publisherId;
}

GamePublisher::GamePublisher(const GamePublisher &orig)
{
	this->gameId = orig.gameId;
	this->publisherId = orig.publisherId;
}

GamePublisher::GamePublisher(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *publisherIdJson = json_object_get(json, "publisherId");
	if(publisherIdJson)
	{
		publisherId = (int64_t)json_integer_value(publisherIdJson);
	}

}

GamePublisher::~GamePublisher()
{
}

int64_t GamePublisher::getGameId()
{
	return gameId;
}

void GamePublisher::setGameId(int64_t gameId)
{
    this->gameId = gameId;
}

int64_t GamePublisher::getPublisherId()
{
	return publisherId;
}

int GamePublisher::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select gameId, publisherId from GamePublisher where  gameId = ? and  publisherId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)publisherId);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			gameId = (int64_t)sqlite3_column_int64(statement, 0);
			publisherId = (int64_t)sqlite3_column_int64(statement, 1);
			result = 1;
		}
	}
	else
	{
		cerr << "GamePublisher::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int GamePublisher::save(sqlite3 *sqlite)
{
	int result = 1;

	string insert = "insert into GamePublisher (gameId, publisherId) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)publisherId);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		cerr << "GamePublisher::save " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

json_t *GamePublisher::toJson()
{
	json_t *json = json_object();

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *publisherIdJson = json_integer((json_int_t)publisherId);
	json_object_set_new(json, "publisherId", publisherIdJson);

	return json;
}

list<GamePublisher *> *GamePublisher::getItems(sqlite3 *sqlite, int64_t gameId)
{
	list<GamePublisher *> *items = new list<GamePublisher *>;

	string query = "select gameId, publisherId from GamePublisher where gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GamePublisher *item = new GamePublisher();
			item->gameId = (int64_t)sqlite3_column_int64(statement, 0);
			item->publisherId = (int64_t)sqlite3_column_int64(statement, 1);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "GamePublisher::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

GamePublisher *GamePublisher::getItem(list<GamePublisher *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GamePublisher *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GamePublisher::releaseItems(list<GamePublisher *> *items)
{
	for(list<GamePublisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

int GamePublisher::remove(sqlite3 *sqlite, int64_t gameId)
{
    int result = 1;

    string command = "delete from GamePublisher where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "GamePublisher::remove " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    
    return result;
}

json_t *GamePublisher::toJsonArray(list<GamePublisher *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GamePublisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
