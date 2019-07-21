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
* @date April 14, 2019, 4:55 pm
*/

#include "Genre.h"
#include <iostream>

Genre::Genre()
{
}

Genre::Genre(int64_t id)
{
	this->id = id;
}

Genre::Genre(const Genre &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

Genre::Genre(json_t *json)
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

Genre::~Genre()
{
}

int64_t Genre::getId()
{
	return id;
}

string Genre::getName()
{
	return name;
}

void Genre::setName(string name)
{
	this->name = name;
}

int64_t Genre::getApiId()
{
	return apiId;
}

void Genre::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t Genre::getApiItemId()
{
	return apiItemId;
}

void Genre::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int Genre::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select id, name, apiId, apiItemId from Genre where  id = ?";
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
		cerr << "Genre::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int Genre::save(sqlite3 *sqlite)
{
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into Genre (name, apiId, apiItemId) values(?, ?, ?)";
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
			cerr << "Genre::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Genre set name = ?, apiId = ?, apiItemId = ? where id = ?";
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
			cerr << "Genre::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	return result;
}

json_t *Genre::toJson()
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

list<Genre *> *Genre::getItems(sqlite3 *sqlite, string name)
{
    name = "%" + name + "%";
    
	list<Genre *> *items = new list<Genre *>;
	string query = "select id, name, apiId, apiItemId from Genre where name like ? order by name";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Genre *item = new Genre();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 2);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "Genre::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

Genre *Genre::getItem(list<Genre *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<Genre *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void Genre::releaseItems(list<Genre *> *items)
{
	for(list<Genre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *Genre::toJsonArray(list<Genre *> *items)
{
	json_t *jsonArray = json_array();
	for(list<Genre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

Genre* Genre::getGenre(sqlite3* sqlite, int64_t apiId, int64_t apiItemId)
{
    Genre *genre = NULL;
    string query = "select id, name, apiId, apiItemId from Genre where  apiId = ? and apiItemId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {        
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiItemId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            genre = new Genre();
            genre->id = (int64_t)sqlite3_column_int64(statement, 0);
            genre->name = string((const char*) sqlite3_column_text(statement, 1));
            genre->apiId = (int64_t)sqlite3_column_int64(statement, 2);
            genre->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);
        }
    }
    else
    {
        cerr << "Genre::getGenre " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    return genre;
}

int Genre::bulkInsert(sqlite3 *sqlite, list<Genre*>* items)
{
    string insert = "insert into Genre (name, apiId, apiItemId) values ";
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
            Genre *item = Genre::getItem(items, index);
            
            sqlite3_bind_text(statement, valueIndex++, item->name.c_str(), item->name.length(), NULL);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiId);
            sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiItemId);            
        }

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Genre::" << __FUNCTION__ << " sqlite3_errmsg: " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);    
    
    return result;
}
