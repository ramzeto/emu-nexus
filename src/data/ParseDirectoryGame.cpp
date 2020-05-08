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
 * File:   ParseDirectoryGame.cpp
 * Author: ram
 * 
 * Created on August 6, 2019, 10:53 PM
 */

#include "ParseDirectoryGame.h"
#include "Database.h"
#include "Logger.h"


ParseDirectoryGame::ParseDirectoryGame()
{
}

ParseDirectoryGame::ParseDirectoryGame(int64_t id)
{
	this->id = id;
}

ParseDirectoryGame::ParseDirectoryGame(const ParseDirectoryGame &orig)
{
	this->id = orig.id;
	this->parseDirectoryId = orig.parseDirectoryId;
	this->timestamp = orig.timestamp;
	this->fileName = orig.fileName;
	this->name = orig.name;
	this->mameName = orig.mameName;
	this->processed = orig.processed;
}

ParseDirectoryGame::ParseDirectoryGame(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *parseDirectoryIdJson = json_object_get(json, "parseDirectoryId");
	if(parseDirectoryIdJson)
	{
		parseDirectoryId = (int64_t)json_integer_value(parseDirectoryIdJson);
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
	}

	json_t *fileNameJson = json_object_get(json, "fileName");
	if(fileNameJson)
	{
		fileName = string(json_string_value(fileNameJson));
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *mameNameJson = json_object_get(json, "mameName");
	if(mameNameJson)
	{
		mameName = string(json_string_value(mameNameJson));
	}

	json_t *processedJson = json_object_get(json, "processed");
	if(processedJson)
	{
		processed = (int64_t)json_integer_value(processedJson);
	}

}

ParseDirectoryGame::~ParseDirectoryGame()
{
}

int64_t ParseDirectoryGame::getId()
{
	return id;
}

int64_t ParseDirectoryGame::getParseDirectoryId()
{
	return parseDirectoryId;
}

void ParseDirectoryGame::setParseDirectoryId(int64_t parseDirectoryId)
{
	this->parseDirectoryId = parseDirectoryId;
}

string ParseDirectoryGame::getTimestamp()
{
	return timestamp;
}

void ParseDirectoryGame::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

string ParseDirectoryGame::getFileName()
{
	return fileName;
}

void ParseDirectoryGame::setFileName(string fileName)
{
	this->fileName = fileName;
}

string ParseDirectoryGame::getName()
{
	return name;
}

void ParseDirectoryGame::setName(string name)
{
	this->name = name;
}

string ParseDirectoryGame::getMameName()
{
	return mameName;
}

void ParseDirectoryGame::setMameName(string mameName)
{
	this->mameName = mameName;
}

int64_t ParseDirectoryGame::getProcessed()
{
	return processed;
}

void ParseDirectoryGame::setProcessed(int64_t processed)
{
	this->processed = processed;
}

int ParseDirectoryGame::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, parseDirectoryId, timestamp, fileName, name, mameName, processed from ParseDirectoryGame where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			parseDirectoryId = (int64_t)sqlite3_column_int64(statement, 1);
			timestamp = string((const char*) sqlite3_column_text(statement, 2));
			fileName = string((const char*) sqlite3_column_text(statement, 3));
			name = string((const char*) sqlite3_column_text(statement, 4));
			mameName = string((const char*) sqlite3_column_text(statement, 5));
			processed = (int64_t)sqlite3_column_int64(statement, 6);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int ParseDirectoryGame::save()
{
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into ParseDirectoryGame (parseDirectoryId, timestamp, fileName, name, mameName, processed) values(?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)parseDirectoryId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_text(statement, 4, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 5, mameName.c_str(), mameName.length(), NULL);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)processed);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update ParseDirectoryGame set parseDirectoryId = ?, timestamp = ?, fileName = ?, name = ?, mameName = ?, processed = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)parseDirectoryId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_text(statement, 4, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 5, mameName.c_str(), mameName.length(), NULL);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)processed);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
	return result;
}

json_t *ParseDirectoryGame::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *parseDirectoryIdJson = json_integer((json_int_t)parseDirectoryId);
	json_object_set_new(json, "parseDirectoryId", parseDirectoryIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	json_t *fileNameJson = json_string(fileName.c_str());
	json_object_set_new(json, "fileName", fileNameJson);

	json_t *nameJson = json_string(name.c_str());
	json_object_set_new(json, "name", nameJson);

	json_t *mameNameJson = json_string(mameName.c_str());
	json_object_set_new(json, "mameName", mameNameJson);

	json_t *processedJson = json_integer((json_int_t)processed);
	json_object_set_new(json, "processed", processedJson);

	return json;
}

ParseDirectoryGame* ParseDirectoryGame::getItem(int64_t parseDirectoryId, string fileName)
{
    sqlite3 *db = Database::getInstance()->acquire();
    ParseDirectoryGame *item = NULL;
    string query = "select id, parseDirectoryId, timestamp, fileName, name, mameName, processed from ParseDirectoryGame where parseDirectoryId = ? and fileName = ? limit 1";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)parseDirectoryId);
        sqlite3_bind_text(statement, 2, fileName.c_str(), fileName.length(), NULL);
        
            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                    item = new ParseDirectoryGame();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->parseDirectoryId = (int64_t)sqlite3_column_int64(statement, 1);
                    item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
                    item->fileName = string((const char*) sqlite3_column_text(statement, 3));
                    item->name = string((const char*) sqlite3_column_text(statement, 4));
                    item->mameName = string((const char*) sqlite3_column_text(statement, 5));
                    item->processed = (int64_t)sqlite3_column_int64(statement, 6);
            }
    }
    else
    {
            Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return item;
}

list<ParseDirectoryGame*>* ParseDirectoryGame::getPendingItems(int64_t parseDirectoryId)
{
    sqlite3 *db = Database::getInstance()->acquire();
    list<ParseDirectoryGame *> *items = new list<ParseDirectoryGame *>;
    string query = "select id, parseDirectoryId, timestamp, fileName, name, mameName, processed from ParseDirectoryGame where parseDirectoryId = ? and processed = 0";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)parseDirectoryId);
        
            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                    ParseDirectoryGame *item = new ParseDirectoryGame();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->parseDirectoryId = (int64_t)sqlite3_column_int64(statement, 1);
                    item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
                    item->fileName = string((const char*) sqlite3_column_text(statement, 3));
                    item->name = string((const char*) sqlite3_column_text(statement, 4));
                    item->mameName = string((const char*) sqlite3_column_text(statement, 5));
                    item->processed = (int64_t)sqlite3_column_int64(statement, 6);

                    items->push_back(item);
            }
    }
    else
    {
            Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return items;
}

list<ParseDirectoryGame *> *ParseDirectoryGame::getItems()
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<ParseDirectoryGame *> *items = new list<ParseDirectoryGame *>;
	string query = "select id, parseDirectoryId, timestamp, fileName, name, mameName, processed from ParseDirectoryGame";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			ParseDirectoryGame *item = new ParseDirectoryGame();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->parseDirectoryId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
			item->fileName = string((const char*) sqlite3_column_text(statement, 3));
			item->name = string((const char*) sqlite3_column_text(statement, 4));
			item->mameName = string((const char*) sqlite3_column_text(statement, 5));
			item->processed = (int64_t)sqlite3_column_int64(statement, 6);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("ParseDirectoryGame", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}

ParseDirectoryGame *ParseDirectoryGame::getItem(list<ParseDirectoryGame *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<ParseDirectoryGame *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void ParseDirectoryGame::releaseItems(list<ParseDirectoryGame *> *items)
{
	for(list<ParseDirectoryGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *ParseDirectoryGame::toJsonArray(list<ParseDirectoryGame *> *items)
{
	json_t *jsonArray = json_array();
	for(list<ParseDirectoryGame *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

