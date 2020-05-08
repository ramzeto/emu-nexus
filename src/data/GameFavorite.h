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
 * File:   GameFavorite.h
 * Author: ram
 *
 * Created on May 7, 2020, 10:44 PM
 */

#ifndef FAVORITEGAME_H
#define FAVORITEGAME_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameFavorite
{
private:
	int64_t gameId;
	string timestamp;

	GameFavorite();

public:
	GameFavorite(int64_t gameId);
	GameFavorite(const GameFavorite &orig);
	GameFavorite(json_t *json);

	~GameFavorite();

	int64_t getGameId();
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load();
	int save();
        int remove();

	json_t *toJson();

	static list<GameFavorite *> *getItems(int64_t gameId = -1, string timestamp = "");
	static GameFavorite *getItem(list<GameFavorite *> *items, unsigned int index);
	static void releaseItems(list<GameFavorite *> *items);
	static json_t *toJsonArray(list<GameFavorite *> *items);
};

#endif


