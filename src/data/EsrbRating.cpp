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
 * File:   EsrbRating.cpp
 * Author: ram
 * 
 * Created April 14, 2019, 4:58 pm
*/

#include "EsrbRating.h"
#include "Database.h"
#include "Logger.h"


const unsigned int EsrbRating::BULK_INSERT_BATCH_SIZE = 100;

EsrbRating::EsrbRating()
{
}

EsrbRating::EsrbRating(int64_t id)
{
	this->id = id;
}

EsrbRating::EsrbRating(const EsrbRating &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

EsrbRating::EsrbRating(json_t *json)
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

EsrbRating::~EsrbRating()
{
}

int64_t EsrbRating::getId()
{
	return id;
}

string EsrbRating::getName()
{
	return name;
}

void EsrbRating::setName(string name)
{
	this->name = name;
}

int64_t EsrbRating::getApiId()
{
	return apiId;
}

void EsrbRating::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t EsrbRating::getApiItemId()
{
	return apiItemId;
}

void EsrbRating::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int EsrbRating::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, name, apiId, apiItemId from EsrbRating where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
		Logger::getInstance()->error("EsrbRating", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int EsrbRating::save()
{
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into EsrbRating (name, apiId, apiItemId) values(?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)apiItemId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("EsrbRating", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update EsrbRating set name = ?, apiId = ?, apiItemId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_text(statement, 1, name.c_str(), name.length(), NULL);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 3, (sqlite3_int64)apiItemId);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("EsrbRating", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
	return result;
}

json_t *EsrbRating::toJson()
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

list<EsrbRating *> *EsrbRating::getItems()
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<EsrbRating *> *items = new list<EsrbRating *>;
	string query = "select id, name, apiId, apiItemId from EsrbRating";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			EsrbRating *item = new EsrbRating();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->name = string((const char*) sqlite3_column_text(statement, 1));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 2);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 3);

			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("EsrbRating", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return items;
}

EsrbRating *EsrbRating::getItem(list<EsrbRating *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<EsrbRating *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void EsrbRating::releaseItems(list<EsrbRating *> *items)
{
	for(list<EsrbRating *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *EsrbRating::toJsonArray(list<EsrbRating *> *items)
{
	json_t *jsonArray = json_array();
	for(list<EsrbRating *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}

int EsrbRating::bulkInsert(sqlite3 *sqlite, list<EsrbRating*>* items)
{
    int result = 1;
    
    if(items->size() > BULK_INSERT_BATCH_SIZE)
    {
        list<EsrbRating *> *batchItems = new list<EsrbRating *>;
        for(unsigned int index = 0; index < items->size(); index++)
        {
            EsrbRating *item = EsrbRating::getItem(items, index);
        
            batchItems->push_back(item);
            if(batchItems->size() == BULK_INSERT_BATCH_SIZE)
            {
                result = EsrbRating::bulkInsert(sqlite, batchItems);                
                batchItems->clear();
                
                if(result)
                {
                    break;
                }
            }
        }
        if(!result && batchItems->size() > 0)
        {
            result = EsrbRating::bulkInsert(sqlite, batchItems);
        }
        
        delete batchItems;
                
        return result;
    }
    else
    {
        string insert = "insert into EsrbRating (name, apiId, apiItemId) values ";
        string separator = "";
        for(unsigned int index = 0; index < items->size(); index++)
        {        
            insert += separator + "(?, ?, ?)";
            separator = ",";
        }

        sqlite3_stmt *statement;
        if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
        {
            int valueIndex = 1;
            for(unsigned int index = 0; index < items->size(); index++)
            {
                EsrbRating *item = EsrbRating::getItem(items, index);

                sqlite3_bind_text(statement, valueIndex++, item->name.c_str(), item->name.length(), NULL);
                sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiId);
                sqlite3_bind_int64(statement, valueIndex++, (sqlite3_int64)item->apiItemId);            
            }

            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
        }
        else
        {
            Logger::getInstance()->error("EsrbRating", __FUNCTION__, string(sqlite3_errmsg(sqlite)));
        }

        sqlite3_finalize(statement);
    }
    
    return result;
}

