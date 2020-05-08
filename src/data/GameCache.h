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
 * File:   GameCache.h
 * Author: ram
 *
 * Created on July 19, 2019, 1:06 AM
 */


#ifndef GAMECACHE_H
#define GAMECACHE_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameCache
{
private:
    static const string DIRECTORY_PREFIX;
    
	int64_t id;
	int64_t gameId;
	string timestamp;

	GameCache();        
public:
	GameCache(int64_t id);
	GameCache(const GameCache &orig);
	GameCache(json_t *json);

	~GameCache();

	int64_t getId();
	int64_t getGameId();
	void setGameId(int64_t gameId);
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load();
	int save();
        int remove();

        string getDirectory();
        size_t getSize();
        
	json_t *toJson();

	static list<GameCache *> *getItems();
        static list<GameCache *> *getItems(int64_t platformId);
	static GameCache *getItem(list<GameCache *> *items, unsigned int index);
	static void releaseItems(list<GameCache *> *items);
	static json_t *toJsonArray(list<GameCache *> *items);        
        static GameCache *getGameCache(int64_t gameId);
};

#endif /* GAMECACHE_H */

