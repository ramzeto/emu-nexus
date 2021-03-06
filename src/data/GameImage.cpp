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
 * File:   GameImage.cpp
 * Author: ram
 * 
 * Created April 3, 2019, 10:46 pm
*/

#include "GameImage.h"
#include "Database.h"
#include "Game.h"
#include "Logger.h"
#include "Utils.h"

#include <unistd.h>

const string GameImage::FILE_PREFIX = "game_image_";

const int GameImage::TYPE_BOX_FRONT = 1;
const int GameImage::TYPE_BOX_BACK = 2;
const int GameImage::TYPE_SCREENSHOT = 3;
const int GameImage::TYPE_CLEAR_LOGO = 4;
const int GameImage::TYPE_BANNER = 5;
const int GameImage::TYPE_FANART = 6;

const int GameImage::THUMBNAIL_WIDTH = 250;
const int GameImage::THUMBNAIL_HEIGHT = 250;

GameImage::GameImage()
{
    url = "";
}

GameImage::GameImage(int64_t id)
{
	this->id = id;
        url = "";
        fileName = "";
}

GameImage::GameImage(const GameImage &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->type = orig.type;
	this->fileName = orig.fileName;
	this->external = orig.external;
	this->apiId = orig.apiId;
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

int GameImage::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;
	string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
			url = string((const char*) sqlite3_column_text(statement, 6));
			downloaded = (int64_t)sqlite3_column_int64(statement, 7);
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return result;
}

int GameImage::save()
{
	int result = 1;

        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into GameImage (gameId, type, fileName, external, apiId, url, downloaded) values(?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)external);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
                        sqlite3_bind_text(statement, 6, url.c_str(), url.length(), NULL);
                        sqlite3_bind_int64(statement, 7, (sqlite3_int64)downloaded);
                        
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update GameImage set gameId = ?, type = ?, fileName = ?, external = ?, apiId = ?, url = ?, downloaded = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
			sqlite3_bind_text(statement, 3, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_int64(statement, 4, (sqlite3_int64)external);
			sqlite3_bind_int64(statement, 5, (sqlite3_int64)apiId);
                        sqlite3_bind_text(statement, 6, url.c_str(), url.length(), NULL);
                        sqlite3_bind_int64(statement, 7, (sqlite3_int64)downloaded);
			sqlite3_bind_int64(statement, 8, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();

	return result;
}

int GameImage::remove()
{
    if(id == 0)
    {
        return 1;
    }
    
    int result = 1;
    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from GameImage where id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
            result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
            Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
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

int GameImage::saveImage() 
{
    if(!id)
    {
       return 1;
    }
    
    Game *game = new Game(getGameId());
    game->load();
    string newFileName = game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(id);
    delete game;
    
    string currentFileName = getFileName();
        
    if(currentFileName.compare(newFileName) != 0)
    {
        if(Utils::getInstance()->fileExists(currentFileName))
        {
            if(!Utils::getInstance()->copyFile(currentFileName, newFileName))
            {
                setFileName(newFileName);            

                Utils::getInstance()->scaleImage(getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, getThumbnailFileName());
                return !save();
            }
        }        
    }
    else
    {
        if(Utils::getInstance()->fileExists(currentFileName))
        {
            Utils::getInstance()->scaleImage(currentFileName, GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, getThumbnailFileName());
            return 0;
        }
    }
                
    return 1;    
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

	json_t *urlJson = json_string(url.c_str());
	json_object_set_new(json, "url", urlJson);

	json_t *downloadedJson = json_integer((json_int_t)downloaded);
	json_object_set_new(json, "downloaded", downloadedJson);
        
	return json;
}

GameImage* GameImage::getPrimaryImage(int64_t gameId)
{
    GameImage *item = NULL;
    sqlite3 *db = Database::getInstance()->acquire();
    string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where gameId = ?  and type = ? limit 1";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
        sqlite3_bind_int64(statement, 2, (sqlite3_int64)GameImage::TYPE_BOX_FRONT);

            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                    item = new GameImage();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
                    item->type = (int64_t)sqlite3_column_int64(statement, 2);
                    item->fileName = string((const char*) sqlite3_column_text(statement, 3));
                    item->external = (int64_t)sqlite3_column_int64(statement, 4);
                    item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                    item->url = string((const char*) sqlite3_column_text(statement, 6));
                    item->downloaded = (int64_t)sqlite3_column_int64(statement, 7);
            }
    }
    else
    {
            Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);    
    
    if(!item)
    {
        string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where gameId = ? limit 1";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			item = new GameImage();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->type = (int64_t)sqlite3_column_int64(statement, 2);
			item->fileName = string((const char*) sqlite3_column_text(statement, 3));
			item->external = (int64_t)sqlite3_column_int64(statement, 4);
			item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                        item->url = string((const char*) sqlite3_column_text(statement, 6));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 7);
		}
	}
	else
	{
		Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
    }
    Database::getInstance()->release();
    
    
    return item;
}


list<GameImage *> *GameImage::getItems(int64_t gameId)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameImage *> *items = new list<GameImage *>;
	string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where gameId = ? order by type";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
                        item->url = string((const char*) sqlite3_column_text(statement, 6));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 7);
                    
			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return items;
}

list<GameImage*>* GameImage::getItems(int64_t gameId, int64_t type)
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameImage *> *items = new list<GameImage *>;
	string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where gameId = ? and type = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)gameId);
            sqlite3_bind_int64(statement, 2, (sqlite3_int64)type);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameImage *item = new GameImage();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->type = (int64_t)sqlite3_column_int64(statement, 2);
			item->fileName = string((const char*) sqlite3_column_text(statement, 3));
			item->external = (int64_t)sqlite3_column_int64(statement, 4);
			item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                        item->url = string((const char*) sqlite3_column_text(statement, 6));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 7);
                    
			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return items;
}

list<GameImage*>* GameImage::getPendingToDownloadItems()
{
        sqlite3 *db = Database::getInstance()->acquire();
	list<GameImage *> *items = new list<GameImage *>;        
	string query = "select id, gameId, type, fileName, external, apiId, url, downloaded from GameImage where downloaded = 0 and url <> ''";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			GameImage *item = new GameImage();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->gameId = (int64_t)sqlite3_column_int64(statement, 1);
			item->type = (int64_t)sqlite3_column_int64(statement, 2);
			item->fileName = string((const char*) sqlite3_column_text(statement, 3));
			item->external = (int64_t)sqlite3_column_int64(statement, 4);
			item->apiId = (int64_t)sqlite3_column_int64(statement, 5);
                        item->url = string((const char*) sqlite3_column_text(statement, 6));
                        item->downloaded = (int64_t)sqlite3_column_int64(statement, 7);
                    
			items->push_back(item);
		}
	}
	else
	{
		Logger::getInstance()->error("GameImage", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

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