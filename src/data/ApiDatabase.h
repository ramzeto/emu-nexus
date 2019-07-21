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
 * File:   ApiDatabase.h
 * Author: ram
 *
 * Created on July 8, 2019, 9:50 PM
 */

#ifndef APIDATABASE_H
#define APIDATABASE_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class ApiDatabase
{
private:
	int64_t apiId;
	string md5sum;
	string timestamp;

	ApiDatabase();

public:
	ApiDatabase(int64_t apiId, string md5sum);
	ApiDatabase(const ApiDatabase &orig);
	ApiDatabase(json_t *json);

	~ApiDatabase();

	int64_t getApiId();
	string getMd5sum();
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);

	json_t *toJson();

	static list<ApiDatabase *> *getItems(sqlite3 *sqlite);
	static ApiDatabase *getItem(list<ApiDatabase *> *items, unsigned int index);
	static void releaseItems(list<ApiDatabase *> *items);
	static json_t *toJsonArray(list<ApiDatabase *> *items);
};

#endif


