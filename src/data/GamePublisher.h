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
* @date March 27, 2019, 9:51 pm
*/

#ifndef GAMEPUBLISHER_H
#define GAMEPUBLISHER_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GamePublisher
{
private:
	int64_t gameId;
	int64_t publisherId;

	GamePublisher();

public:
	GamePublisher(int64_t gameId, int64_t publisherId);
	GamePublisher(const GamePublisher &orig);
	GamePublisher(json_t *json);

	~GamePublisher();

	int64_t getGameId();
        void setGameId(int64_t gameId);
        
	int64_t getPublisherId();

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);

	json_t *toJson();

	static list<GamePublisher *> *getItems(sqlite3 *sqlite, int64_t gameId);
	static GamePublisher *getItem(list<GamePublisher *> *items, unsigned int index);
	static void releaseItems(list<GamePublisher *> *items);
        static int remove(sqlite3 *sqlite, int64_t gameId);
	static json_t *toJsonArray(list<GamePublisher *> *items);
};

#endif
