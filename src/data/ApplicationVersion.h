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
* @date April 14, 2019, 3:33 pm
*/

#ifndef APPLICATIONVERSION_H
#define APPLICATIONVERSION_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class ApplicationVersion
{
private:
	string version;
	string timestamp;

	ApplicationVersion();

public:
	ApplicationVersion(string version);
	ApplicationVersion(const ApplicationVersion &orig);
	ApplicationVersion(json_t *json);

	~ApplicationVersion();

	string getVersion();
	string getTimestamp();
	void setTimestamp(string timestamp);

	int load(sqlite3 *sqlite);
	int save(sqlite3 *sqlite);

	json_t *toJson();

	static list<ApplicationVersion *> *getItems(sqlite3 *sqlite);
	static ApplicationVersion *getItem(list<ApplicationVersion *> *items, unsigned int index);
	static void releaseItems(list<ApplicationVersion *> *items);
	static json_t *toJsonArray(list<ApplicationVersion *> *items);
        static ApplicationVersion* getCurrentVersion(sqlite3 *sqlite);
};

#endif