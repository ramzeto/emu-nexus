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
* @date April 14, 2019, 4:55 pm
*/

#ifndef GENRE_H
#define GENRE_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Genre
{
private:
	int64_t id;
	string name;
	int64_t apiId;
	int64_t apiItemId;

	Genre();

public:
	Genre(int64_t id);
	Genre(const Genre &orig);
	Genre(json_t *json);

	~Genre();

	int64_t getId();
	string getName();
	void setName(string name);
	int64_t getApiId();
	void setApiId(int64_t apiId);
	int64_t getApiItemId();
	void setApiItemId(int64_t apiItemId);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);

	json_t *toJson();

	static list<Genre *> *getItems(sqlite3 *sqlite, string name);	
        static Genre *getItem(list<Genre *> *items, unsigned int index);	
        static void releaseItems(list<Genre *> *items);                        
	static json_t *toJsonArray(list<Genre *> *items);
        static Genre *getGenre(sqlite3 *sqlite, int64_t apiId, int64_t apiItemId);
        
        /**
         * Inserts every item in the list. It assumes that every item is a new item and generates a new id for each one.
         * @param sqlite
         * @param items
         * @return 0 on success.
         */
        static int bulkInsert(sqlite3 *sqlite, list<Genre *> *items);
};

#endif
