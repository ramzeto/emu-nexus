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
 * File:   RecentGame.h
 * Author: ram
 *
 * Created on July 21, 2019, 8:46 PM
 */

#ifndef RECENTGAME_H
#define RECENTGAME_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class RecentGame
{
private:
	int64_t gameId;
	string timestamp;

	RecentGame();

public:
    static const unsigned int LIMIT;
    
	RecentGame(int64_t gameId);
	RecentGame(const RecentGame &orig);
	RecentGame(json_t *json);

	~RecentGame();

	int64_t getGameId();
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);
        int remove(sqlite3 *sqlite);

	json_t *toJson();

	static list<RecentGame *> *getItems(sqlite3 *sqlite);
	static RecentGame *getItem(list<RecentGame *> *items, unsigned int index);
	static void releaseItems(list<RecentGame *> *items);
	static json_t *toJsonArray(list<RecentGame *> *items);
};

#endif

