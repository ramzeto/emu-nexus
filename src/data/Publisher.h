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
 * File:   Publisher.h
 * Author: ram
 * 
 * Created April 14, 2019, 4:57 pm
*/

#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Publisher
{
private:
        static const unsigned int BULK_INSERT_BATCH_SIZE;    
	int64_t id;
	string name;
	int64_t apiId;

	Publisher();

public:
	Publisher(int64_t id);
	Publisher(const Publisher &orig);
	Publisher(json_t *json);

	~Publisher();

	int64_t getId();
	string getName();
	void setName(string name);
	int64_t getApiId();
	void setApiId(int64_t apiId);

	int load();
	int save();

	json_t *toJson();

	static list<Publisher *> *getItems(string name = "");
	static Publisher *getItem(list<Publisher *> *items, unsigned int index);
	static void releaseItems(list<Publisher *> *items);
	static json_t *toJsonArray(list<Publisher *> *items);
        static Publisher *getPublisher(int64_t apiId);
        
        /**
         * Inserts every item in the list. It assumes that every item is a new item and generates a new id for each one.
         * @param sqlite
         * @param items
         * @return 0 on success.
         */
        static int bulkInsert(sqlite3 *sqlite, list<Publisher *> *items);
};

#endif
