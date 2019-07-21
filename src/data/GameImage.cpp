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
* @date April 3, 2019, 10:46 pm
*/

#include "GameImage.h"

#include <iostream>
#include <unistd.h>

const string GameImage::FILE_PREFIX = "game_image_";

const int GameImage::TYPE_BOX_FRONT = 1;
const int GameImage::TYPE_BOX_BACK = 2;
const int GameImage::TYPE_SCREENSHOT = 3;
const int GameImage::TYPE_CLEAR_LOGO = 4;
const int GameImage::TYPE_BANNER = 5;

const int GameImage::THUMBNAIL_WIDTH = 200;
const int GameImage::THUMBNAIL_HEIGHT = 100;

GameImage::GameImage()
{
    url = "";
}

GameImage::GameImage(int64_t id)
{
	this->id = id;
        url = "";
}

GameImage::GameImage(const GameImage &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->type = orig.type;
	this->fileName = orig.fileName;
	this->external = orig.external;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
	this->url = orig.url;
	this->downloaded = orig.downloaded;         
}

GameImage::GameImage(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
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

GameImage::~GameImage()
{
}

int64_t GameImage::getId()
{
	return id;
}

int64_t GameImage::getGameId()
{
	return gameId;
}

void GameImage::setGameId(int64_t gameId)
{
	this->gameId = gameId;
}

int64_t GameImage::getType()
{
	return type;
}

void GameImage::setType(int64_t type)
{
	this->type = type;
}

string GameImage::getFileName()
{
	return fileName;
}

void GameImage::setFileName(string fileName)
{
	this->fileName = fileName;
}

int64_t GameImage::getExternal()
{
	return external;
}

void GameImage::setExternal(int64_t external)
{
	this->external = external;
}

int64_t GameImage::getApiId()
{
	return apiId;
}

void GameImage::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t GameImage::getApiItemId()
{
	return apiItemId;
}

void GameImage::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

string GameImage::getUrl()
{
	return url;
}

void GameImage::setUrl(string url)
{
	this->url = url;
}

int64_t GameImage::getDownloaded()
{
	return downloaded;
}

void GameImage::setDownloaded(int64_t downloaded)
{
	this->downloaded = downloaded;
}

int GameImage::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select id, gameId, type, fileName, external, apiId, apiItemId, url, downloaded from GameImage where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			gameId = (int64_t)sqlite3_column_int64(statement, 1);
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
		cerr << "GameImage::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int GameImage::save(sqlite3 *sqlite)
{
	int result = 1;

	if(id == 0)
	{
		string insert = "insert into GameImage (gameId, type, fileName, external, apiId, apiItemId, url, downloaded) values(?, ?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)external);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 6, (sqlite3_int64)apiItemId);
                        sqlite3_bind_text(statement, 7, url.c_str(), url.length(), NULL);
                        sqlite3_bind_int64(statement, 8, (sqlite3_int64)downloaded);
                        
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "GameImage::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update GameImage set gameId = ?, type = ?, fileName = ?, external = ?, apiId = ?, apiItemId = ?, url = ?, downloaded = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
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
			cerr << "GameImage::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}

	return result;
}

int GameImage::remove(sqlite3 *sqlite)
{
    if(id == 0)
    {
        return 1;
    }
    
    int result = 1;
    
    string command = "delete from GameImage where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
            cerr << "GameImage::remove " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);
        
    if(!external)
    {
        unlink(fileName.c_str());
    }
    
    unlink(getThumbnailFileName().c_str());
    
    return result;    
}

string GameImage::getThumbnailFileName()
{
    return fileName + "_thumb";
}

json_t *GameImage::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *gameIdJson = json_integer((json_int_t)gameId);
	json_object_set_new(json, "gameId", gameIdJson);

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

GameImage* GameImage::getPrimaryImage(sqlite3 *sqlite, int64_t gameId)
{
    GameImage *item = NULL;

    string query = "select id, gameId, type, fileName, external, apiId, apiItemId, url, downloaded from GameImage where gameId = ? and type = ? limit 1";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)TYPE_BOX_FRONT);

            if (sqlite3_step(statement) == SQLITE_ROW)
            {
                    item = new GameImage();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
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
            cerr << "GameImage::getPrimaryImage " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);
    
    return item;
}


list<GameImage *> *GameImage::getItems(sqlite3 *sqlite, int64_t gameId)
{
	list<GameImage *> *items = new list<GameImage *>;

	string query = "select id, gameId, type, fileName, external, apiId, apiItemId, url, downloaded from GameImage where gameId = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameImage *item = new GameImage();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
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
		cerr << "GameImage::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

GameImage *GameImage::getItem(list<GameImage *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<GameImage *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void GameImage::releaseItems(list<GameImage *> *items)
{
	for(list<GameImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *GameImage::toJsonArray(list<GameImage *> *items)
{
	json_t *jsonArray = json_array();
	for(list<GameImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}