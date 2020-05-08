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
 * File:   PlatformImage.cpp
 * Author: ram
 * 
 * Created April 3, 2019, 10:42 pm
*/

#include "PlatformImage.h"
#include "Database.h"
#include "Logger.h"

#include <unistd.h>

const string PlatformImage::FILE_PREFIX = "platform_image_";

const int64_t PlatformImage::TYPE_FANART = 1;
const int64_t PlatformImage::TYPE_BANNER = 2;
const int64_t PlatformImage::TYPE_BOXART = 3;
const int64_t PlatformImage::TYPE_ICON = 4;

const int PlatformImage::THUMBNAIL_WIDTH = 200;
const int PlatformImage::THUMBNAIL_HEIGHT = 100;

PlatformImage::PlatformImage()
{
    url = "";
}

PlatformImage::PlatformImage(int64_t id)
{
	this->id = id;
        url = "";
}

PlatformImage::PlatformImage(const PlatformImage &orig)
{
	this->id = orig.id;
	this->platformId = orig.platformId;
	this->type = orig.type;
	this->fileName = orig.fileName;
	this->external = orig.external;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
	this->url = orig.url;
	this->downloaded = orig.downloaded;        
}

PlatformImage::PlatformImage(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *platformIdJson = json_object_get(json, "platformId");
	if(platformIdJson)
	{
		platformId = (int64_t)json_integer_value(platformIdJson);
	}

	json_t *typeJson = json_object_get(json, "type");
	if(typeJson)
	{
		type = (int64_t)json_integer_value(typeJson);
	}

	json_t *fileNameJson = json_object_get(json, "fileName");
	if(fileNameJson)
	{
		fileName = string(json_string_value(fileNameJson));
	}

	json_t *externalJson = json_object_get(json, "external");
	if(externalJson)
	{
		external = (int64_t)json_integer_value(externalJson);
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
                
	json_t *urlJson = json_object_get(json, "url");
	if(urlJson)
	{
		url = string(json_string_value(urlJson));
	}

	json_t *downloadedJson = json_object_get(json, "downloaded");
	if(downloadedJson)
	{
		downloaded = (int64_t)json_integer_value(downloadedJson);
	}

}

PlatformImage::~PlatformImage()
{
}

int64_t PlatformImage::getId()
{
	return id;
}

int64_t PlatformImage::getPlatformId()
{
	return platformId;
}

void PlatformImage::setPlatformId(int64_t platformId)
{
	this->platformId = platformId;
}

int64_t PlatformImage::getType()
{
	return type;
}

void PlatformImage::setType(int64_t type)
{
	this->type = type;
}

string PlatformImage::getFileName()
{
	return fileName;
}

void PlatformImage::setFileName(string fileName)
{
	this->fileName = fileName;
}

int64_t PlatformImage::getExternal()
{
	return external;
}

void PlatformImage::setExternal(int64_t external)
{
	this->external = external;
}

int64_t PlatformImage::getApiId()
{
	return apiId;
}

void PlatformImage::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t PlatformImage::getApiItemId()
{
	return apiItemId;
}

void PlatformImage::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

string PlatformImage::getUrl()
{
	return url;
}

void PlatformImage::setUrl(string url)
{
	this->url = url;
}

int64_t PlatformImage::getDownloaded()
{
	return downloaded;
}

void PlatformImage::setDownloaded(int64_t downloaded)
{
	this->downloaded = downloaded;
}

int PlatformImage::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, platformId, type, fileName, external, apiId, apiItemId, url, downloaded from PlatformImage where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			platformId = (int64_t)sqlite3_column_int64(statement, 1);
			type = (int64_t)sqlite3_column_int64(statement, 2);
			fileName = string((const char*) sqlite3_column_text(statement, 3));
			external = (int64_t)sqlite3_column_int64(statement, 4);
			apiId = (int64_t)sqlite3_column_int64(statement, 5);
			apiItemId = (int64_t)sqlite3_column_int64(statement, 6);
			url = string((const char*) sqlite3_column_text(statement, 7));
			downloaded = (int64_t)sqlite3_column_int64(statement, 8);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();
        
	return result;
}

int PlatformImage::save()
{
	int result = 1;
        
        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into PlatformImage (platformId, type, fileName, external, apiId, apiItemId, url, downloaded) values(?, ?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)external);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiItemId);
			sqlite3_bind_text(statement, 7, url.c_str(), url.length(), NULL);
			sqlite3_bind_int64(statement, 8, (sqlite3_int64)downloaded);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update PlatformImage set platformId = ?, type = ?, fileName = ?, external = ?, apiId = ?, apiItemId = ?, url = ?, downloaded = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)external);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiItemId);
			sqlite3_bind_text(statement, 7, url.c_str(), url.length(), NULL);
			sqlite3_bind_int64(statement, 8, (sqlite3_int64)downloaded);
			sqlite3_bind_int64(statement, 9, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();
        
	return result;
}

int PlatformImage::remove()
{
    if(id == 0)
    {
        return 1;
    }
    
    int result = 1;
    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from PlatformImage where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
            Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();
        
    if(!external)
    {
        unlink(fileName.c_str());
    }
    unlink(getThumbnailFileName().c_str());
    
    return result;    
}

string PlatformImage::getThumbnailFileName()
{
    return fileName + "_thumb";
}

json_t *PlatformImage::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *platformIdJson = json_integer((json_int_t)platformId);
	json_object_set_new(json, "platformId", platformIdJson);

	json_t *typeJson = json_integer((json_int_t)type);
	json_object_set_new(json, "type", typeJson);

	json_t *fileNameJson = json_string(fileName.c_str());
	json_object_set_new(json, "fileName", fileNameJson);

	json_t *externalJson = json_integer((json_int_t)external);
	json_object_set_new(json, "external", externalJson);

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *apiItemIdJson = json_integer((json_int_t)apiItemId);
	json_object_set_new(json, "apiItemId", apiItemIdJson);

	json_t *urlJson = json_string(url.c_str());
	json_object_set_new(json, "url", urlJson);

	json_t *downloadedJson = json_integer((json_int_t)downloaded);
	json_object_set_new(json, "downloaded", downloadedJson);
        
	return json;
}

PlatformImage* PlatformImage::getPrimaryImage(int64_t platformId)
{
    sqlite3 *db = Database::getInstance()->acquire();
    PlatformImage *item = NULL;
    string query = "select id, platformId, type, fileName, external, apiId, apiItemId, url, downloaded from PlatformImage where platformId = ? and type = ? limit 1";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)TYPE_BOXART);

            if (sqlite3_step(statement) == SQLITE_ROW)
            {
                    item = new PlatformImage();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
                    item->type = (int64_t)sqlite3_column_int64(statement, 2);
                    item->fileName = string((const char*) sqlite3_column_text(statement, 3));
                    item->external = (int64_t)sqlite3_column_int64(statement, 4);
                    item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                    item->apiItemId = (int64_t)sqlite3_column_int64(statement, 6);
                    item->url = string((const char*) sqlite3_column_text(statement, 7));
                    item->downloaded = (int64_t)sqlite3_column_int64(statement, 8);                    
            }
    }
    else
    {
            Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    
    if(!item)
    {
        string query = "select id, platformId, type, fileName, external, apiId, apiItemId, url, downloaded from PlatformImage where platformId = ? limit 1";
        sqlite3_stmt *statement;
        if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);

                if (sqlite3_step(statement) == SQLITE_ROW)
                {
                        item = new PlatformImage();
                        item->id = (int64_t)sqlite3_column_int64(statement, 0);
                        item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
                        item->type = (int64_t)sqlite3_column_int64(statement, 2);
                        item->fileName = string((const char*) sqlite3_column_text(statement, 3));
                        item->external = (int64_t)sqlite3_column_int64(statement, 4);
                        item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                        item->apiItemId = (int64_t)sqlite3_column_int64(statement, 6);
                        item->url = string((const char*) sqlite3_column_text(statement, 7));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 8);                          
                }
        }
        else
        {
                Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
        }
        sqlite3_finalize(statement);
    }
    Database::getInstance()->release();
    
    return item;
}


list<PlatformImage *> *PlatformImage::getItems(int64_t platformId)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<PlatformImage *> *items = new list<PlatformImage *>;
	string query = "select id, platformId, type, fileName, external, apiId, apiItemId, url, downloaded from PlatformImage where platformId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			PlatformImage *item = new PlatformImage();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
			item->type = (int64_t)sqlite3_column_int64(statement, 2);
			item->fileName = string((const char*) sqlite3_column_text(statement, 3));
			item->external = (int64_t)sqlite3_column_int64(statement, 4);
			item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 6);
                        item->url = string((const char*) sqlite3_column_text(statement, 7));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 8);    
                        
			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("PlatformImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return items;
}

PlatformImage *PlatformImage::getItem(list<PlatformImage *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<PlatformImage *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void PlatformImage::releaseItems(list<PlatformImage *> *items)
{
	for(list<PlatformImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *PlatformImage::toJsonArray(list<PlatformImage *> *items)
{
	json_t *jsonArray = json_array();
	for(list<PlatformImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
