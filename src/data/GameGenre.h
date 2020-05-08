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
 * File:   GameGenre.h
 * Author: ram
 * 
 * Created March 27, 2019, 9:49 pm
*/

#ifndef GAMEGENRE_H
#define GAMEGENRE_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameGenre
{
private:
	int64_t gameId;
	int64_t genreId;

	GameGenre();

public:
	GameGenre(int64_t gameId, int64_t genreId);
	GameGenre(const GameGenre &orig);
	GameGenre(json_t *json);

	~GameGenre();

	int64_t getGameId();
        void setGameId(int64_t gameId);
	int64_t getGenreId();

	int load();
	int save();

	json_t *toJson();

	static list<GameGenre *> *getItems(int64_t gameId);
	static GameGenre *getItem(list<GameGenre *> *items, unsigned int index);
	static void releaseItems(list<GameGenre *> *items);
        static int remove(int64_t gameId);
	static json_t *toJsonArray(list<GameGenre *> *items);
};

#endif
