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
 * File:   GameActivity.cpp
 * Author: ram
 * 
 * Created on May 8, 2020, 11:12 AM
 */

#include "GameActivity.h"
#include "Database.h"
#include "Logger.h"

GameActivity::GameActivity()
{
	this->id = 0;
	this->gameId = 0;
	this->timestamp = "";
	this->duration = 0;
}

GameActivity::GameActivity(int64_t id)
{
	this->id = 0;
	this->gameId = 0;
	this->timestamp = "";
	this->duration = 0;

	this->id = id;
}

GameActivity::GameActivity(const GameActivity &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->timestamp = orig.timestamp;
	this->duration = orig.duration;
}

GameActivity::GameActivity(json_t *json)
{
	this->id = 0;
	this->gameId = 0;
	this->timestamp = "";
	this->duration = 0;

	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
		if(id == 0 && json_string_value(idJson))
		{
			id = (int64_t)atol(json_string_value(idJson));
		}
	}

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

	json_t *durationJson = json_object_get(json, "duration");
	if(durationJson)
	{
		duration = (int64_t)json_integer_value(durationJson);
		if(duration == 0 && json_string_value(durationJson))
		{
			duration = (int64_t)atol(json_string_value(durationJson));
		}
	}

}

GameActivity::~GameActivity()
{
}

int64_t GameActivity::getId()
{
	return id;
}

int64_t GameActivity::getGameId()
{
	return gameId;
}

void GameActivity::setGameId(int64_t gameId)
{
	this->gameId = gameId;
}

string GameActivity::getTimestamp()
{
	return timestamp;
}

void GameActivity::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int64_t GameActivity::getDuration()
{
	return duration;
}

void GameActivity::setDuration(int64_t duration)
{
	this->duration = duration;
}

int GameActivity::load()
{
	int result = 0;
	sqlite3 *db = Database::getInstance()->acquire();

	string query = "select id, gameId, timestamp, duration from GameActivity where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			gameId = (int64_t)sqlite3_column_int64(statement, 1);
			timestamp = string((const char*) sqlite3_column_text(statement, 2));
			duration = (int64_t)sqlite3_column_int64(statement, 3);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("GameActivity", string(__FUNCTION__), string(sqlite3_errmsg(db)) + " " + query);
	}

	sqlite3_finalize(statement);
	Database::getInstance()->release();

	return result;
}

int GameActivity::save()
{
	int result = 1;
	sqlite3 *db = Database::getInstance()->acquire();

	if(id == 0)
	{
		string insert = "insert into GameActivity (gameId, timestamp, duration) values(?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)duration);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("GameActivity", string(__FUNCTION__), string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update GameActivity set gameId = ?, timestamp = ?, duration = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)duration);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("GameActivity", string(__FUNCTION__), string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}

	Database::getInstance()->release();

	return result;
}

json_t *GameActivity::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	json_t *durationJson = json_integer((json_int_t)duration);
	json_object_set_new(json, "duration", durationJson);

	return json;
}

list<GameActivity *> *GameActivity::getItems(int64_t id, int64_t gameId, string timestamp, int64_t duration)
{
	list<GameActivity *> *items = new list<GameActivity *>;
	sqlite3 *db = Database::getInstance()->acquire();

	string query = "select id, gameId, timestamp, duration from GameActivity ";
	string selection = "";
	if(id >= 0)
	{
		if(selection.length() == 0)
		{
			selection += " where ";
		}
		else
		{
			selection += " and ";
		}
		selection += " id = ? ";
	}
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
	if(duration >= 0)
	{
		if(selection.length() == 0)
		{
			selection += " where ";
		}
		else
		{
			selection += " and ";
		}
		selection += " duration = ? ";
	}
	query += selection;
        query += " order by timestamp";
        
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		int paramCount = 1;
		if(id >= 0)
		{
			sqlite3_bind_int64(statement, paramCount++, (sqlite3_int64)id);
		}
		if(gameId >= 0)
		{
			sqlite3_bind_int64(statement, paramCount++, (sqlite3_int64)gameId);
		}
		if(timestamp.length() > 0)
		{
			sqlite3_bind_text(statement, paramCount++, timestamp.c_str(), timestamp.length(), NULL);
		}
		if(duration >= 0)
		{
			sqlite3_bind_int64(statement, paramCount++, (sqlite3_int64)duration);
		}
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameActivity *item = new GameActivity();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
			item->duration = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameActivity", string(__FUNCTION__), string(sqlite3_errmsg(db)) + " " + query);
	}

	sqlite3_finalize(statement);
	Database::getInstance()->release();

	return items;
}

list<GameActivity *> *GameActivity::getRecentItems()
{
	list<GameActivity *> *items = new list<GameActivity *>;
	sqlite3 *db = Database::getInstance()->acquire();

	string query = "select id, gameId, timestamp, duration from GameActivity ";
        query += " group by gameId order by timestamp desc";
        
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameActivity *item = new GameActivity();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
			item->duration = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameActivity", string(__FUNCTION__), string(sqlite3_errmsg(db)) + " " + query);
	}

	sqlite3_finalize(statement);
	Database::getInstance()->release();

	return items;
}


GameActivity *GameActivity::getItem(list<GameActivity *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameActivity *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameActivity::releaseItems(list<GameActivity *> *items)
{
	for(list<GameActivity *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *GameActivity::toJsonArray(list<GameActivity *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameActivity *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

int GameActivity::remove(int64_t gameId)
{
    int result = 1;

    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from GameActivity where gameId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        Logger::getInstance()->error("GameActivity", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return result;
}

