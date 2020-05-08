/*
 * Copyright (C) 2020 ram
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
 * File:   GameActivity.h
 * Author: ram
 *
 * Created on May 8, 2020, 11:12 AM
 */

#ifndef GAMEACTIVITY_H
#define GAMEACTIVITY_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameActivity
{
private:
	int64_t id;
	int64_t gameId;
	string timestamp;
	int64_t duration;

	GameActivity();

public:
	GameActivity(int64_t id);
	GameActivity(const GameActivity &orig);
	GameActivity(json_t *json);

	~GameActivity();

	int64_t getId();
	int64_t getGameId();
	void setGameId(int64_t gameId);
	string getTimestamp();
	void setTimestamp(string timestamp);
	int64_t getDuration();
	void setDuration(int64_t duration);

	int load();
	int save();

	json_t *toJson();

	static list<GameActivity *> *getItems(int64_t id = -1, int64_t gameId = -1, string timestamp = "", int64_t duration = -1);
        static list<GameActivity *> *getRecentItems();
	static GameActivity *getItem(list<GameActivity *> *items, unsigned int index);
	static void releaseItems(list<GameActivity *> *items);        
	static json_t *toJsonArray(list<GameActivity *> *items);
        static int remove(int64_t gameId);
};

#endif
