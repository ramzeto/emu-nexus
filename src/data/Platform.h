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
 * File:   Platform.h
 * Author: ram
 * 
 * Created March 26, 2019, 8:45 pm
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Platform
{
private:
	int64_t id;
	string name;
	string description;
	string command;
	int64_t deflate;
	string deflateFileExtensions;
	int64_t apiId;
	int64_t apiItemId;

	Platform();

public:
    static const string DIRECTORY_PREFIX;
    
	Platform(int64_t id);
	Platform(const Platform &orig);
	Platform(json_t *json);

	~Platform();

	int64_t getId();
	string getName();
	void setName(string name);
	string getDescription();
	void setDescription(string description);
	string getCommand();
	void setCommand(string command);
	int64_t getDeflate();
	void setDeflate(int64_t deflate);
	string getDeflateFileExtensions();
	void setDeflateFileExtensions(string deflateFileExtensions);
	int64_t getApiId();
	void setApiId(int64_t apiId);
	int64_t getApiItemId();
	void setApiItemId(int64_t apiItemId);

	int load();
	int save();
        int remove();
        
        /**
         * 
         * @return Directory where images for this platform are stored.
         */
        string getMediaDirectory();

	json_t *toJson();

	static list<Platform *> *getItems();
	static Platform *getItem(list<Platform *> *items, unsigned int index);
	static void releaseItems(list<Platform *> *items);
	static json_t *toJsonArray(list<Platform *> *items);
};

#endif
