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
* @date March 27, 2019, 9:50 pm
*/

#include "GameDeveloper.h"

#include <iostream>

GameDeveloper::GameDeveloper()
{
}

GameDeveloper::GameDeveloper(int64_t gameId, int64_t developerId)
{
	this->gameId = gameId;
	this->developerId = developerId;
}

GameDeveloper::GameDeveloper(const GameDeveloper &orig)
{
	this->gameId = orig.gameId;
	this->developerId = orig.developerId;
}

GameDeveloper::GameDeveloper(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *developerIdJson = json_object_get(json, "developerId");
	if(developerIdJson)
	{
		developerId = (int64_t)json_integer_value(developerIdJson);
	}

}

GameDeveloper::~GameDeveloper()
{
}

int64_t GameDeveloper::getGameId()
{
	return gameId;
}

int64_t GameDeveloper::getDeveloperId()
{
	return developerId;
}

void GameDeveloper::setGameId(int64_t gameId)
{
    this->gameId = gameId;
}

int GameDeveloper::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select gameId, developerId from GameDeveloper where  gameId = ? and  developerId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)developerId);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			gameId = (int64_t)sqlite3_column_int64(statement, 0);
			developerId = (int64_t)sqlite3_column_int64(statement, 1);
			result = 1;
		}
	}
	else
	{
		cerr << "GameDeveloper::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int GameDeveloper::save(sqlite3 *sqlite)
{
	int result = 1;

	string insert = "insert into GameDeveloper (gameId, developerId) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)developerId);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		cerr << "GameDeveloper::save " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

json_t *GameDeveloper::toJson()
{
	json_t *json = json_object();

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *developerIdJson = json_integer((json_int_t)developerId);
	json_object_set_new(json, "developerId", developerIdJson);

	return json;
}

list<GameDeveloper *> *GameDeveloper::getItems(sqlite3 *sqlite, int64_t gameId)
{
	list<GameDeveloper *> *items = new list<GameDeveloper *>;

	string query = "select gameId, developerId from GameDeveloper where gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameDeveloper *item = new GameDeveloper();
			item->gameId = (int64_t)sqlite3_column_int64(statement, 0);
			item->developerId = (int64_t)sqlite3_column_int64(statement, 1);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "GameDeveloper::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

GameDeveloper *GameDeveloper::getItem(list<GameDeveloper *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameDeveloper *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameDeveloper::releaseItems(list<GameDeveloper *> *items)
{
	for(list<GameDeveloper *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

int GameDeveloper::remove(sqlite3 *sqlite, int64_t gameId)
{
    int result = 1;

    string command = "delete from GameDeveloper where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "GameDeveloper::remove " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    
    return result;
}

json_t *GameDeveloper::toJsonArray(list<GameDeveloper *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameDeveloper *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
