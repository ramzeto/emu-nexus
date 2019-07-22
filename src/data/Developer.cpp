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
 * File:   Developer.cpp
 * Author: ram
 * 
 * Created April 14, 2019, 4:56 pm
*/

#include "Developer.h"
#include <iostream>

Developer::Developer()
{
}

Developer::Developer(int64_t id)
{
	this->id = id;
}

Developer::Developer(const Developer &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

Developer::Developer(json_t *json)
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

Developer::~Developer()
{
}

int64_t Developer::getId()
{
	return id;
}

string Developer::getName()
{
	return name;
}

void Developer::setName(string name)
{
	this->name = name;
}

int64_t Developer::getApiId()
{
	return apiId;
}

void Developer::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t Developer::getApiItemId()
{
	return apiItemId;
}

void Developer::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int Developer::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select id, name, apiId, apiItemId from Developer where  id = ?";
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
		cerr << "Developer::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int Developer::save(sqlite3 *sqlite)
{
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into Developer (name, apiId, apiItemId) values(?, ?, ?)";
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
			cerr << "Developer::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Developer set name = ?, apiId = ?, apiItemId = ? where id = ?";
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
			cerr << "Developer::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	return result;
}

json_t *Developer::toJson()
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

list<Developer *> *Developer::getItems(sqlite3 *sqlite, string name)
{
    name = "%" + name + "%";
    
	list<Developer *> *items = new list<Developer *>;
	string query = "select id, name, apiId, apiItemId from Developer where name like ? order by name";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Developer *item = new Developer();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 2);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "Developer::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

Developer *Developer::getItem(list<Developer *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<Developer *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void Developer::releaseItems(list<Developer *> *items)
{
	for(list<Developer *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *Developer::toJsonArray(list<Developer *> *items)
{
	json_t *jsonArray = json_array();
	for(list<Developer *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

Developer* Developer::getDeveloper(sqlite3 *sqlite, int64_t apiId, int64_t apiItemId)
{
    Developer *developer = NULL;
    string query = "select id, name, apiId, apiItemId from Developer where  apiId = ? and apiItemId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {        
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiItemId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            developer = new Developer();
            developer->id = (int64_t)sqlite3_column_int64(statement, 0);
            developer->name = string((const char*) sqlite3_column_text(statement, 1));
            developer->apiId = (int64_t)sqlite3_column_int64(statement, 2);
            developer->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);
        }
    }
    else
    {
        cerr << "Developer::getDeveloper " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    return developer;
}


int Developer::bulkInsert(sqlite3 *sqlite, list<Developer*>* items)
{
    string insert = "insert into Developer (name, apiId, apiItemId) values ";
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
            Developer *item = Developer::getItem(items, index);
            
            sqlite3_bind_text(statement, valueIndex++, item->name.c_str(), item->name.length(), NULL);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiId);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiItemId);            
        }

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Developer::" << __FUNCTION__ << " sqlite3_errmsg: " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);    
    
    return result;
}
