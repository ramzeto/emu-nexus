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
 * File:   Game.h
 * Author: ram
 * 
 * Created March 27, 2019, 9:46 pm
*/

#ifndef GAME_H
#define GAME_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Game
{
private:
	int64_t id;
	int64_t platformId;
	int64_t esrbRatingId;
	string name;
	string description;
	string releaseDate;
	string fileName;
	string notes;
	string command;
	int64_t deflate;
	string deflateFileExtensions;
	string timestamp;
	int64_t apiId;
	int64_t apiItemId;

	Game();

public:
        static const string DIRECTORY_PREFIX;
        
	Game(int64_t id);
	Game(const Game &orig);
	Game(json_t *json);

	~Game();

	int64_t getId();
	int64_t getPlatformId();
	void setPlatformId(int64_t platformId);
	int64_t getEsrbRatingId();
	void setEsrbRatingId(int64_t esrbRatingId);
	string getName();
	void setName(string name);
	string getDescription();
	void setDescription(string description);
	string getReleaseDate();
	void setReleaseDate(string releaseDate);
	string getFileName();
	void setFileName(string fileName);
	string getNotes();
	void setNotes(string notes);
	string getCommand();
	void setCommand(string command);
	int64_t getDeflate();
	void setDeflate(int64_t deflate);
	string getDeflateFileExtensions();
	void setDeflateFileExtensions(string deflateFileExtensions);
	string getTimestamp();
	void setTimestamp(string timestamp);
	int64_t getApiId();
	void setApiId(int64_t apiId);
	int64_t getApiItemId();
	void setApiItemId(int64_t apiItemId);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);
        int remove(sqlite3 *sqlite);
       
        /**
         * 
         * @return Directory where images for this game are stored.
         */
        string getMediaDirectory();

	json_t *toJson();

        static Game* getGameWithFileName(sqlite3 *sqlite, int64_t platformId, string fileName);
	static list<Game *> *getItems(sqlite3 *sqlite, int64_t platformId, string query);
	static Game *getItem(list<Game *> *items, unsigned int index);
	static void releaseItems(list<Game *> *items);
	static json_t *toJsonArray(list<Game *> *items);
};

#endif

