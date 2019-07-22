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
 * File:   GameDeveloper.h
 * Author: ram
 * 
 * Created March 27, 2019, 9:50 pm
*/

#ifndef GAMEDEVELOPER_H
#define GAMEDEVELOPER_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameDeveloper
{
private:
	int64_t gameId;
	int64_t developerId;

	GameDeveloper();

public:
	GameDeveloper(int64_t gameId, int64_t developerId);
	GameDeveloper(const GameDeveloper &orig);
	GameDeveloper(json_t *json);

	~GameDeveloper();

	int64_t getGameId();
        void setGameId(int64_t gameId);
        
	int64_t getDeveloperId();        

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);

	json_t *toJson();

	static list<GameDeveloper *> *getItems(sqlite3 *sqlite, int64_t gameId);
	static GameDeveloper *getItem(list<GameDeveloper *> *items, unsigned int index);
	static void releaseItems(list<GameDeveloper *> *items);
        static int remove(sqlite3 *sqlite, int64_t gameId);
	static json_t *toJsonArray(list<GameDeveloper *> *items);
};

#endif
