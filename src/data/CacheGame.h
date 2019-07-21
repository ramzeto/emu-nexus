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
 * File:   CacheGame.h
 * Author: ram
 *
 * Created on July 19, 2019, 1:06 AM
 */


#ifndef CACHEGAME_H
#define CACHEGAME_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class CacheGame
{
private:
    static const string DIRECTORY_PREFIX;
    
	int64_t id;
	int64_t gameId;
	string timestamp;

	CacheGame();        
public:
	CacheGame(int64_t id);
	CacheGame(const CacheGame &orig);
	CacheGame(json_t *json);

	~CacheGame();

	int64_t getId();
	int64_t getGameId();
	void setGameId(int64_t gameId);
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);
        int remove(sqlite3 *sqlite);

        string getDirectory();
        size_t getSize();
        
	json_t *toJson();

	static list<CacheGame *> *getItems(sqlite3 *sqlite);
	static CacheGame *getItem(list<CacheGame *> *items, unsigned int index);
	static void releaseItems(list<CacheGame *> *items);
	static json_t *toJsonArray(list<CacheGame *> *items);        
        static CacheGame *getCacheGame(sqlite3 *sqlite, int64_t gameId);
};

#endif /* CACHEGAME_H */

