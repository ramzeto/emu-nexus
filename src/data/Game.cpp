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
 * File:   Game.cpp
 * Author: ram
 * 
 * Created March 27, 2019, 9:46 pm
*/

#include "Game.h"
#include "Database.h"
#include "Logger.h"
#include "Utils.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameImage.h"
#include "Platform.h"
#include "Directory.h"
#include "GameActivity.h"
#include "GameCache.h"
#include "GameFavorite.h"

#include <iostream>
#include <regex>

const string Game::DIRECTORY_PREFIX = "game_";

Game::Game()
{
    this->apiId = 0;
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

int Game::load()
{
        sqlite3 *db = Database::getInstance()->acquire();
	int result = 0;

	string query = "select id, platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId from Game where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
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
			result = 1;
		}
	}
	else
	{
		Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

	return result;
}

int Game::save()
{
	int result = 1;

        sqlite3 *db = Database::getInstance()->acquire();
	if(id == 0)
	{
		string insert = "insert into Game (platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
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
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(db);
			}
		}
		else
		{
			Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + insert);
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update Game set platformId = ?, esrbRatingId = ?, name = ?, description = ?, releaseDate = ?, fileName = ?, notes = ?, command = ?, deflate = ?, deflateFileExtensions = ?, timestamp = ?, apiId = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(db, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
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
			sqlite3_bind_int64(statement, 13, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + update);
		}

		sqlite3_finalize(statement);
	}
        Database::getInstance()->release();

        // Creates the media directory where images will be stored
        Utils::getInstance()->makeDirectory(getMediaDirectory());
        
	return result;
}

int Game::remove()
{
    GamePublisher::remove(id);
    GameDeveloper::remove(id);
    
    list<GameImage *> *gamesImages = GameImage::getItems(id);
    for(unsigned int c = 0; c < gamesImages->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gamesImages, c);
        gameImage->remove();
    }
    GameImage::releaseItems(gamesImages);
        
    GameActivity::remove(id);
    
    GameFavorite *gameFavorite = new GameFavorite(id);
    if(gameFavorite->load())
    {
        gameFavorite->remove();
    }    
    delete gameFavorite;
    
    GameCache *gameCache = GameCache::getGameCache(id);
    if(gameCache)
    {
        gameCache->remove();
        delete gameCache;
    }
    
    Utils::getInstance()->removeDirectory(getMediaDirectory());
    
    int result = 1;
    sqlite3 *db = Database::getInstance()->acquire();
    string command = "delete from Game where  id = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + command);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();

    return result;
}

string Game::getMediaDirectory()
{
    return Directory::getInstance()->getMediaDirectory() + Platform::DIRECTORY_PREFIX + to_string(platformId) + "/" + DIRECTORY_PREFIX + to_string(id) + "/";
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

	return json;
}

list<Game *> *Game::getItems(int64_t platformId, string queryString)
{
        sqlite3 *db = Database::getInstance()->acquire();
        queryString = "%" + queryString + "%";
    
	list<Game *> *items = new list<Game *>;

	//string query = "select id, platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId, apiItemId from Game where platformId = ? and name like ? order by name";
        string query = "select Game.id, Game.platformId, Game.esrbRatingId, Game.name, Game.description, Game.releaseDate, Game.fileName, Game.notes, Game.command, Game.deflate, Game.deflateFileExtensions, Game.timestamp, Game.apiId "; 
        query += " from Game ";
        query += " left join GameDeveloper on GameDeveloper.gameId = Game.id ";
        query += " left join Developer on GameDeveloper.developerId = Developer.id ";
        query += " left join GamePublisher on GamePublisher.gameId = Game.id ";
        query += " left join Publisher on GamePublisher.publisherId = Publisher.id ";
        query += " left join GameGenre on GameGenre.gameId = Game.id ";
        query += " left join Genre on GameGenre.genreId = Genre.id ";
        query += " where Game.platformId = ? and (Game.name like ? or Developer.name like ? or Publisher.name like ? or Genre.name like ?) ";
        query += " group by Game.id order by Game.name, Game.fileName";
        
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
            sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
            sqlite3_bind_text(statement, 2, queryString.c_str(), queryString.length(), NULL);
            sqlite3_bind_text(statement, 3, queryString.c_str(), queryString.length(), NULL);
            sqlite3_bind_text(statement, 4, queryString.c_str(), queryString.length(), NULL);
            sqlite3_bind_text(statement, 5, queryString.c_str(), queryString.length(), NULL);
            
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

                    items->push_back(item);
            }
	}
	else
	{
		Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
	}
	sqlite3_finalize(statement);
        Database::getInstance()->release();

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

Game* Game::getGameWithFileName(int64_t platformId, string fileName)
{
    sqlite3 *db = Database::getInstance()->acquire();
    Game *game = NULL;
    string query = "select id, platformId, esrbRatingId, name, description, releaseDate, fileName, notes, command, deflate, deflateFileExtensions, timestamp, apiId from Game where platformId = ? and fileName = ?";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(db, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
        sqlite3_bind_text(statement, 2, fileName.c_str(), fileName.length(), NULL);

            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                    game = new Game();
                    game->id = (int64_t)sqlite3_column_int64(statement, 0);
                    game->platformId = (int64_t)sqlite3_column_int64(statement, 1);
                    game->esrbRatingId = (int64_t)sqlite3_column_int64(statement, 2);
                    game->name = string((const char*) sqlite3_column_text(statement, 3));
                    game->description = string((const char*) sqlite3_column_text(statement, 4));
                    game->releaseDate = string((const char*) sqlite3_column_text(statement, 5));
                    game->fileName = string((const char*) sqlite3_column_text(statement, 6));
                    game->notes = string((const char*) sqlite3_column_text(statement, 7));
                    game->command = string((const char*) sqlite3_column_text(statement, 8));
                    game->deflate = (int64_t)sqlite3_column_int64(statement, 9);
                    game->deflateFileExtensions = string((const char*) sqlite3_column_text(statement, 10));
                    game->timestamp = string((const char*) sqlite3_column_text(statement, 11));
                    game->apiId = (int64_t)sqlite3_column_int64(statement, 12);
            }
    }
    else
    {
            Logger::getInstance()->error("Game", __FUNCTION__, string(sqlite3_errmsg(db)) + " " + query);
    }
    sqlite3_finalize(statement);
    Database::getInstance()->release();

    return game;
}

string Game::getSanitizedName(string fileName)
{
    string name = "";
    int open = 0;
    for(unsigned int c = 0; c < fileName.length(); c++)
    {
        char letter = fileName.c_str()[c];
        
        if(letter == '(' || letter == '[' || letter == '{')
        {
            open++;
        }
        else if(open > 0 && (letter == ')' || letter == ']' || letter == '}'))
        {
            open--;
        }                
        else if(!open)
        {
            name += letter;
        }
    }
    
    // Sometimes ROM names have the version not in parenthesis, it should be removed
    regex regEx("[a-zA-Z0-9_+?/!\\.(),&:;' -]*[\n\r\f\t ]\\{1,\\}\\(v[0-9]\\{1,\\}\\.\\{1,\\}[0-9]\\{1,\\}\\).*", regex_constants::basic);
    cmatch matches;
    regex_match(name.c_str(), matches, regEx, regex_constants::match_default);
    if(matches.size() == 2)
    {
        name = Utils::getInstance()->strReplace(name, matches.str(1), "");
    }
    //_________
    
    return Utils::getInstance()->trim(name);
}
