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
* @date April 14, 2019, 4:57 pm
*/

#include "Publisher.h"
#include <iostream>

Publisher::Publisher()
{
}

Publisher::Publisher(int64_t id)
{
	this->id = id;
}

Publisher::Publisher(const Publisher &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

Publisher::Publisher(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *apiIdJson = json_object_get(json, "apiId");
	if(apiIdJson)
	{
		apiId = (int64_t)json_integer_value(apiIdJson);
	}

	json_t *apiItemIdJson = json_object_get(json, "apiItemId");
	if(apiItemIdJson)
	{
		apiItemId = (int64_t)json_integer_value(apiItemIdJson);
	}

}

Publisher::~Publisher()
{
}

int64_t Publisher::getId()
{
	return id;
}

string Publisher::getName()
{
	return name;
}

void Publisher::setName(string name)
{
	this->name = name;
}

int64_t Publisher::getApiId()
{
	return apiId;
}

void Publisher::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t Publisher::getApiItemId()
{
	return apiItemId;
}

void Publisher::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int Publisher::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select id, name, apiId, apiItemId from Publisher where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			name = string((const char*) sqlite3_column_text(statement, 1));
			apiId = (int64_t)sqlite3_column_int64(statement, 2);
			apiItemId = (int64_t)sqlite3_column_int64(statement, 3);
			result = 1;
		}
	}
	else
	{
		cerr << "Publisher::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int Publisher::save(sqlite3 *sqlite)
{
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into Publisher (name, apiId, apiItemId) values(?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)apiItemId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "Publisher::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Publisher set name = ?, apiId = ?, apiItemId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)apiItemId);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			cerr << "Publisher::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	return result;
}

json_t *Publisher::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *nameJson = json_string(name.c_str());
	json_object_set_new(json, "name", nameJson);

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *apiItemIdJson = json_integer((json_int_t)apiItemId);
	json_object_set_new(json, "apiItemId", apiItemIdJson);

	return json;
}

list<Publisher *> *Publisher::getItems(sqlite3 *sqlite, string name)
{
    name = "%" + name + "%";
    
	list<Publisher *> *items = new list<Publisher *>;
	string query = "select id, name, apiId, apiItemId from Publisher where name like ? order by name";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Publisher *item = new Publisher();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 2);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "Publisher::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

Publisher *Publisher::getItem(list<Publisher *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<Publisher *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void Publisher::releaseItems(list<Publisher *> *items)
{
	for(list<Publisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *Publisher::toJsonArray(list<Publisher *> *items)
{
	json_t *jsonArray = json_array();
	for(list<Publisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

Publisher* Publisher::getPublisher(sqlite3* sqlite, int64_t apiId, int64_t apiItemId)
{
    Publisher *publisher = NULL;
    string query = "select id, name, apiId, apiItemId from Publisher where  apiId = ? and apiItemId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {        
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiItemId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            publisher = new Publisher();
            publisher->id = (int64_t)sqlite3_column_int64(statement, 0);
            publisher->name = string((const char*) sqlite3_column_text(statement, 1));
            publisher->apiId = (int64_t)sqlite3_column_int64(statement, 2);
            publisher->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);
        }
    }
    else
    {
        cerr << "Publisher::getPublisher " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    return publisher;
}


int Publisher::bulkInsert(sqlite3 *sqlite, list<Publisher*>* items)
{
    string insert = "insert into Publisher (name, apiId, apiItemId) values ";
    string separator = "";
    for(unsigned int index = 0; index < items->size(); index++)
    {        
        insert += separator + "(?, ?, ?)";
        separator = ",";
    }
 
    int result = 1;
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
    {
        int valueIndex = 1;
        for(unsigned int index = 0; index < items->size(); index++)
        {
            Publisher *item = Publisher::getItem(items, index);
            
            sqlite3_bind_text(statement, valueIndex++, item->name.c_str(), item->name.length(), NULL);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiId);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiItemId);            
        }

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Publisher::" << __FUNCTION__ << " sqlite3_errmsg: " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);    
    
    return result;
}
