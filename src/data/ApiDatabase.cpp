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
 * File:   ApiDatabase.cpp
 * Author: ram
 * 
 * Created on July 8, 2019, 9:50 PM
 */


#include "ApiDatabase.h"
#include <iostream>

ApiDatabase::ApiDatabase()
{
}

ApiDatabase::ApiDatabase(int64_t apiId, string md5sum)
{
	this->apiId = apiId;
	this->md5sum = md5sum;
}

ApiDatabase::ApiDatabase(const ApiDatabase &orig)
{
	this->apiId = orig.apiId;
	this->md5sum = orig.md5sum;
	this->timestamp = orig.timestamp;
}

ApiDatabase::ApiDatabase(json_t *json)
{
	json_t *apiIdJson = json_object_get(json, "apiId");
	if(apiIdJson)
	{
		apiId = (int64_t)json_integer_value(apiIdJson);
	}

	json_t *md5sumJson = json_object_get(json, "md5sum");
	if(md5sumJson)
	{
		md5sum = string(json_string_value(md5sumJson));
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
	}

}

ApiDatabase::~ApiDatabase()
{
}

int64_t ApiDatabase::getApiId()
{
	return apiId;
}

string ApiDatabase::getMd5sum()
{
	return md5sum;
}

string ApiDatabase::getTimestamp()
{
	return timestamp;
}

void ApiDatabase::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int ApiDatabase::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select apiId, md5sum, timestamp from ApiDatabase where  apiId = ? and  md5sum = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
		sqlite3_bind_text(statement, 2, md5sum.c_str(), md5sum.length(), NULL);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			apiId = (int64_t)sqlite3_column_int64(statement, 0);
			md5sum = string((const char*) sqlite3_column_text(statement, 1));
			timestamp = string((const char*) sqlite3_column_text(statement, 2));
			result = 1;
		}
	}
	else
	{
		cerr << "ApiDatabase::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int ApiDatabase::save(sqlite3 *sqlite)
{
	int result = 1;
	string insert = "insert into ApiDatabase (apiId, md5sum, timestamp) values(?, ?, ?)";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
		sqlite3_bind_text(statement, 2, md5sum.c_str(), md5sum.length(), NULL);
		sqlite3_bind_text(statement, 3, timestamp.c_str(), timestamp.length(), NULL);
		result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
	}
	else
	{
		cerr << "ApiDatabase::save " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

json_t *ApiDatabase::toJson()
{
	json_t *json = json_object();

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *md5sumJson = json_string(md5sum.c_str());
	json_object_set_new(json, "md5sum", md5sumJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	return json;
}

list<ApiDatabase *> *ApiDatabase::getItems(sqlite3 *sqlite)
{
	list<ApiDatabase *> *items = new list<ApiDatabase *>;
	string query = "select apiId, md5sum, timestamp from ApiDatabase";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			ApiDatabase *item = new ApiDatabase();
			item->apiId = (int64_t)sqlite3_column_int64(statement, 0);
			item->md5sum = string((const char*) sqlite3_column_text(statement, 1));
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));

			items->push_back(item);
		}
	}
	else
	{
		cerr << "ApiDatabase::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

ApiDatabase *ApiDatabase::getItem(list<ApiDatabase *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<ApiDatabase *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void ApiDatabase::releaseItems(list<ApiDatabase *> *items)
{
	for(list<ApiDatabase *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *ApiDatabase::toJsonArray(list<ApiDatabase *> *items)
{
	json_t *jsonArray = json_array();
	for(list<ApiDatabase *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

