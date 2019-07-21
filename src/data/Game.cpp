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
* @date March 27, 2019, 9:46 pm
*/

#include "Game.h"
#include "Settings.h"
#include "Utils.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameImage.h"
#include "Platform.h"

#include <iostream>

const string Game::DIRECTORY_PREFIX = "game_";

Game::Game()
{
    this->apiId = 0;
    this->apiItemId = 0;
}

Game::Game(int64_t id)
{
	this->id = id;
        this->platformId = 0;
	this->esrbRatingId = 0;
	this->name = "";
	this->description = "";
	this->releaseDate = "";
	this->fileName = "";
	this->notes = "";
	this->command = "";
	this->deflate = 0;
	this->deflateFileExtensions = "";
	this->timestamp = "";
	this->apiId = 0;
	this->apiItemId = 0;
}

Game::Game(const Game &orig)
{
	this->id = orig.id;
	this->platformId = orig.platformId;
	this->esrbRatingId = orig.esrbRatingId;
	this->name = orig.name;
	this->description = orig.description;
	this->releaseDate = orig.releaseDate;
	this->fileName = orig.fileName;
	this->notes = orig.notes;
	this->command = orig.command;
	this->deflate = orig.deflate;
	this->deflateFileExtensions = orig.deflateFileExtensions;
	this->timestamp = orig.timestamp;
	this->apiId = orig.apiId;
	this->apiItemId = orig.apiItemId;
}

Game::Game(json_t *json)
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

	json_t *esrbRatingIdJson = json_object_get(json, "esrbRatingId");
	if(esrbRatingIdJson)
	{
		esrbRatingId = (int64_t)json_integer_value(esrbRatingIdJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *descriptionJson = json_object_get(json, "description");
	if(descriptionJson)
	{
		description = string(json_string_value(descriptionJson));
	}

	json_t *releaseDateJson = json_object_get(json, "releaseDate");
	if(releaseDateJson)
	{
		releaseDate = string(json_string_value(releaseDateJson));
	}

	json_t *fileNameJson = json_object_get(json, "fileName");
	if(fileNameJson)
	{
		fileName = string(json_string_value(fileNameJson));
	}

	json_t *notesJson = json_object_get(json, "notes");
	if(notesJson)
	{
		notes = string(json_string_value(notesJson));
	}

	json_t *commandJson = json_object_get(json, "command");
	if(commandJson)
	{
		command = string(json_string_value(commandJson));
	}

	json_t *deflateJson = json_object_get(json, "deflate");
	if(deflateJson)
	{
		deflate = (int64_t)json_integer_value(deflateJson);
	}

	json_t *deflateFileExtensionsJson = json_object_get(json, "deflateFileExtensions");
	if(deflateFileExtensionsJson)
	{
		deflateFileExtensions = string(json_string_value(deflateFileExtensionsJson));
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
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

Game::~Game()
{
}

int64_t Game::getId()
{
	return id;
}

int64_t Game::getPlatformId()
{
	return platformId;
}

void Game::setPlatformId(int64_t platformId)
{
	this->platformId = platformId;
}

int64_t Game::getEsrbRatingId()
{
	return esrbRatingId;
}

void Game::setEsrbRatingId(int64_t esrbRatingId)
{
	this->esrbRatingId = esrbRatingId;
}

string Game::getName()
{
	return name;
}

void Game::setName(string name)
{
	this->name = name;
}

string Game::getDescription()
{
	return description;
}

void Game::setDescription(string description)
{
	this->description = description;
}

string Game::getReleaseDate()
{
	return releaseDate;
}

void Game::setReleaseDate(string releaseDate)
{
	this->releaseDate = releaseDate;
}

string Game::getFileName()
{
	return fileName;
}

void Game::setFileName(string fileName)
{
	this->fileName = fileName;
}

string Game::getNotes()
{
	return notes;
}

void Game::setNotes(string notes)
{
	this->notes = notes;
}

string Game::getCommand()
{
	return command;
}

void Game::setCommand(string command)
{
	this->command = command;
}

int64_t Game::getDeflate()
{
	return deflate;
}

void Game::setDeflate(int64_t deflate)
{
	this->deflate = deflate;
}

string Game::getDeflateFileExtensions()
{
	return deflateFileExtensions;
}

void Game::setDeflateFileExtensions(string deflateFileExtensions)
{
	this->deflateFileExtensions = deflateFileExtensions;
}

string Game::getTimestamp()
{
	return timestamp;
}

void Game::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

int64_t Game::getApiId()
{
	return apiId;
}

void Game::setApiId(int64_t apiId)
{
	this->apiId = apiId;
}

int64_t Game::getApiItemId()
{
	return apiItemId;
}

void Game::setApiItemId(int64_t apiItemId)
{
	this->apiItemId = apiItemId;
}

int Game::load(sqlite3 *sqlite)
{
	int result = 0;

	string query = "select id, platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId, apiItemId from Game where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			platformId = (int64_t)sqlite3_column_int64(statement, 1);
			esrbRatingId = (int64_t)sqlite3_column_int64(statement, 2);
			name = string((const char*) sqlite3_column_text(statement, 3));
			description = string((const char*) sqlite3_column_text(statement, 4));
			releaseDate = string((const char*) sqlite3_column_text(statement, 5));
			fileName = string((const char*) sqlite3_column_text(statement, 6));
			notes = string((const char*) sqlite3_column_text(statement, 7));
			command = string((const char*) sqlite3_column_text(statement, 8));
			deflate = (int64_t)sqlite3_column_int64(statement, 9);
			deflateFileExtensions = string((const char*) sqlite3_column_text(statement, 10));
			timestamp = string((const char*) sqlite3_column_text(statement, 11));
			apiId = (int64_t)sqlite3_column_int64(statement, 12);
			apiItemId = (int64_t)sqlite3_column_int64(statement, 13);
			result = 1;
		}
	}
	else
	{
		cerr << "Game::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return result;
}

int Game::save(sqlite3 *sqlite)
{
	int result = 1;

	if(id == 0)
	{
		string insert = "insert into Game (platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId, apiItemId) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)esrbRatingId);
			sqlite3_bind_text(statement, 3, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 4, description.c_str(), description.length(), NULL);
			sqlite3_bind_text(statement, 5, releaseDate.c_str(), releaseDate.length(), NULL);
			sqlite3_bind_text(statement, 6, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_text(statement, 7, notes.c_str(), notes.length(), NULL);
			sqlite3_bind_text(statement, 8, command.c_str(), command.length(), NULL);
			sqlite3_bind_int64(statement, 9, (sqlite3_int64)deflate);
			sqlite3_bind_text(statement, 10, deflateFileExtensions.c_str(), deflateFileExtensions.length(), NULL);
			sqlite3_bind_text(statement, 11, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 12, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 13, (sqlite3_int64)apiItemId);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "Game::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Game set platformId = ?, esrbRatingId = ?, name = ?, description = ?, releaseDate = ?, fileName = ?, notes = ?, command = ?, deflate = ?, deflateFileExtensions = ?, timestamp = ?, apiId = ?, apiItemId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_int64(statement, 2, (sqlite3_int64)esrbRatingId);
			sqlite3_bind_text(statement, 3, name.c_str(), name.length(), NULL);
			sqlite3_bind_text(statement, 4, description.c_str(), description.length(), NULL);
			sqlite3_bind_text(statement, 5, releaseDate.c_str(), releaseDate.length(), NULL);
			sqlite3_bind_text(statement, 6, fileName.c_str(), fileName.length(), NULL);
			sqlite3_bind_text(statement, 7, notes.c_str(), notes.length(), NULL);
			sqlite3_bind_text(statement, 8, command.c_str(), command.length(), NULL);
			sqlite3_bind_int64(statement, 9, (sqlite3_int64)deflate);
			sqlite3_bind_text(statement, 10, deflateFileExtensions.c_str(), deflateFileExtensions.length(), NULL);
			sqlite3_bind_text(statement, 11, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_int64(statement, 12, (sqlite3_int64)apiId);
			sqlite3_bind_int64(statement, 13, (sqlite3_int64)apiItemId);
			sqlite3_bind_int64(statement, 14, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			cerr << "Game::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}

        // Creates the media directory where images will be stored
        Utils::getInstance()->makeDirectory(getMediaDirectory());
        
	return result;
}

int Game::remove(sqlite3 *sqlite)
{
    GamePublisher::remove(sqlite, id);
    GameDeveloper::remove(sqlite, id);
    
    list<GameImage *> *gamesImages = GameImage::getItems(sqlite, id);
    for(unsigned int c = 0; c < gamesImages->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gamesImages, c);
        gameImage->remove(sqlite);
    }
    GameImage::releaseItems(gamesImages);
    
    Utils::getInstance()->removeDirectory(getMediaDirectory());    
    
    int result = 1;

    string command = "delete from Game where  id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Game::remove " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);

    return result;
}

string Game::getMediaDirectory()
{
    return Settings::getInstance()->getMediaDirectory() + Platform::DIRECTORY_PREFIX + to_string(platformId) + "/" + DIRECTORY_PREFIX + to_string(id) + "/";
}


json_t *Game::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *platformIdJson = json_integer((json_int_t)platformId);
	json_object_set_new(json, "platformId", platformIdJson);

	json_t *esrbRatingIdJson = json_integer((json_int_t)esrbRatingId);
	json_object_set_new(json, "esrbRatingId", esrbRatingIdJson);

	json_t *nameJson = json_string(name.c_str());
	json_object_set_new(json, "name", nameJson);

	json_t *descriptionJson = json_string(description.c_str());
	json_object_set_new(json, "description", descriptionJson);

	json_t *releaseDateJson = json_string(releaseDate.c_str());
	json_object_set_new(json, "releaseDate", releaseDateJson);

	json_t *fileNameJson = json_string(fileName.c_str());
	json_object_set_new(json, "fileName", fileNameJson);

	json_t *notesJson = json_string(notes.c_str());
	json_object_set_new(json, "notes", notesJson);

	json_t *commandJson = json_string(command.c_str());
	json_object_set_new(json, "command", commandJson);

	json_t *deflateJson = json_integer((json_int_t)deflate);
	json_object_set_new(json, "deflate", deflateJson);

	json_t *deflateFileExtensionsJson = json_string(deflateFileExtensions.c_str());
	json_object_set_new(json, "deflateFileExtensions", deflateFileExtensionsJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	json_t *apiIdJson = json_integer((json_int_t)apiId);
	json_object_set_new(json, "apiId", apiIdJson);

	json_t *apiItemIdJson = json_integer((json_int_t)apiItemId);
	json_object_set_new(json, "apiItemId", apiItemIdJson);

	return json;
}

list<Game *> *Game::getItems(sqlite3 *sqlite, int64_t platformId, string queryString)
{
    queryString = "%" + queryString + "%";
    
	list<Game *> *items = new list<Game *>;

	string query = "select id, platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId, apiItemId from Game where platformId = ? and name like ? order by name";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
            sqlite3_bind_text(statement, 2, queryString.c_str(), queryString.length(), NULL);
            
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			Game *item = new Game();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
			item->esrbRatingId = (int64_t)sqlite3_column_int64(statement, 2);
			item->name = string((const char*) sqlite3_column_text(statement, 3));
			item->description = string((const char*) sqlite3_column_text(statement, 4));
			item->releaseDate = string((const char*) sqlite3_column_text(statement, 5));
			item->fileName = string((const char*) sqlite3_column_text(statement, 6));
			item->notes = string((const char*) sqlite3_column_text(statement, 7));
			item->command = string((const char*) sqlite3_column_text(statement, 8));
			item->deflate = (int64_t)sqlite3_column_int64(statement, 9);
			item->deflateFileExtensions = string((const char*) sqlite3_column_text(statement, 10));
			item->timestamp = string((const char*) sqlite3_column_text(statement, 11));
			item->apiId = (int64_t)sqlite3_column_int64(statement, 12);
			item->apiItemId = (int64_t)sqlite3_column_int64(statement, 13);

			items->push_back(item);
		}
	}
	else
	{
		cerr << "Game::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);

	return items;
}

Game *Game::getItem(list<Game *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<Game *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void Game::releaseItems(list<Game *> *items)
{
	for(list<Game *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *Game::toJsonArray(list<Game *> *items)
{
	json_t *jsonArray = json_array();
	for(list<Game *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}
