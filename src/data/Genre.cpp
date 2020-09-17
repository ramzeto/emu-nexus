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
 * File:   Genre.cpp
 * Author: ram
 * 
 * Created April 14, 2019, 4:55 pm
*/

#include "Genre.h"
#include "Database.h"
#include "Logger.h"


const unsigned int Genre::BULK_INSERT_BATCH_SIZE = 100;

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

int Genre::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, name, apiId from Genre where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			name = string((const char*) sqlite3_column_text(statement, 1));
			apiId = (int64_t)sqlite3_column_int64(statement, 2);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int Genre::save()
{
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into Genre (name, apiId) values(?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Genre set name = ?, apiId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
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

	return json;
}

list<Genre *> *Genre::getItems(string name)
{
    name = "%" + name + "%";
    
        sqlite3 *db = Database::getInstance()->acquire();
	list<Genre *> *items = new list<Genre *>;
	string query = "select id, name, apiId from Genre where name like ? order by name";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Genre *item = new Genre();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 2);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
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

Genre* Genre::getGenre(int64_t apiId)
{
    sqlite3 *db = Database::getInstance()->acquire();
    Genre *genre = NULL;
    string query = "select id, name, apiId from Genre where  apiId = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {        
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)apiId);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            genre = new Genre();
            genre->id = (int64_t)sqlite3_column_int64(statement, 0);
            genre->name = string((const char*) sqlite3_column_text(statement, 1));
            genre->apiId = (int64_t)sqlite3_column_int64(statement, 2);
        }
    }
    else
    {
        Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
    
    return genre;
}

int Genre::bulkInsert(sqlite3 *sqlite, list<Genre*>* items)
{
    int result = 1;
    
    if(items->size() > BULK_INSERT_BATCH_SIZE)
    {
        list<Genre *> *batchItems = new list<Genre *>;
        for(unsigned int index = 0; index < items->size(); index++)
        {
            Genre *item = Genre::getItem(items, index);
        
            batchItems->push_back(item);
            if(batchItems->size() == BULK_INSERT_BATCH_SIZE)
            {
                result = Genre::bulkInsert(sqlite, batchItems);                
                batchItems->clear();
                
                if(result)
                {
                    break;
                }
            }
        }
        if(!result && batchItems->size() > 0)
        {
            result = Genre::bulkInsert(sqlite, batchItems);
        }
        
        delete batchItems;
                
        return result;
    }
    else
    {
        string insert = "insert into Genre (name, apiId) values ";
        string separator = "";
        for(unsigned int index = 0; index < items->size(); index++)
        {        
            insert += separator + "(?, ?)";
            separator = ",";
        }

        sqlite3_stmt *statement;
        if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
        {
            int valueIndex = 1;
            for(unsigned int index = 0; index < items->size(); index++)
            {
                Genre *item = Genre::getItem(items, index);

                sqlite3_bind_text(statement, valueIndex++, item->name.c_str(), item->name.length(), NULL);
                sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiId);
            }

            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            Logger::getInstance()->error("Genre", __FUNCTION__, string(sqlite3_errmsg(sqlite)));
        }

        sqlite3_finalize(statement);
    }
    
    return result;
}
