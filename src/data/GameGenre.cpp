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
* @date March 27, 2019, 9:49 pm
*/

#include "GameGenre.h"

#include <iostream>

GameGenre::GameGenre()
{
}

GameGenre::GameGenre(int64_t gameId, int64_t genreId)
{
	this->gameId = gameId;
	this->genreId = genreId;
}

GameGenre::GameGenre(const GameGenre &orig)
{
	this->gameId = orig.gameId;
	this->genreId = orig.genreId;
}

GameGenre::GameGenre(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *genreIdJson = json_object_get(json, "genreId");
	if(genreIdJson)
	{
		genreId = (int64_t)json_integer_value(genreIdJson);
	}

}

GameGenre::~GameGenre()
{
}

int64_t GameGenre::getGameId()
{
	return gameId;
}

void GameGenre::setGameId(int64_t gameId)
{
    this->gameId = gameId;
}


int64_t GameGenre::getGenreId()
{
	return genreId;
}

int GameGenre::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select gameId, genreId from GameGenre where  gameId = ? and  genreId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)genreId);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			gameId = (int64_t)sqlite3_column_int64(statement, 0);
			genreId = (int64_t)sqlite3_column_int64(statement, 1);
			result = 1;
		}
	}
	else
	{
		cerr << "GameGenre::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int GameGenre::save(sqlite3 *sqlite)
{
	int result = 1;

	string insert = "insert into GameGenre (gameId, genreId) values(?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64)genreId);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		cerr << "GameGenre::save " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

json_t *GameGenre::toJson()
{
	json_t *json = json_object();

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *genreIdJson = json_integer((json_int_t)genreId);
	json_object_set_new(json, "genreId", genreIdJson);

	return json;
}

list<GameGenre *> *GameGenre::getItems(sqlite3 *sqlite, int64_t gameId)
{
	list<GameGenre *> *items = new list<GameGenre *>;

	string query = "select gameId, genreId from GameGenre where gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameGenre *item = new GameGenre();
			item->gameId = (int64_t)sqlite3_column_int64(statement, 0);
			item->genreId = (int64_t)sqlite3_column_int64(statement, 1);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "GameGenre::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

GameGenre *GameGenre::getItem(list<GameGenre *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameGenre *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameGenre::releaseItems(list<GameGenre *> *items)
{
	for(list<GameGenre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

int GameGenre::remove(sqlite3 *sqlite, int64_t gameId)
{
    int result = 1;

    string command = "delete from GameGenre where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "GameGenre::remove " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    
    return result;
}

json_t *GameGenre::toJsonArray(list<GameGenre *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameGenre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
