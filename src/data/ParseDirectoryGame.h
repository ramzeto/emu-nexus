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
 * File:   ParseDirectoryGame.h
 * Author: ram
 *
 * Created on August 6, 2019, 10:53 PM
 */

#ifndef PARSEDIRECTORYGAME_H
#define PARSEDIRECTORYGAME_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class ParseDirectoryGame
{
private:
	int64_t id;
	int64_t parseDirectoryId;
	string timestamp;
	string fileName;
	string name;
	string mameName;
	int64_t processed;

	ParseDirectoryGame();

public:
	ParseDirectoryGame(int64_t id);
	ParseDirectoryGame(const ParseDirectoryGame &orig);
	ParseDirectoryGame(json_t *json);

	~ParseDirectoryGame();

	int64_t getId();
	int64_t getParseDirectoryId();
	void setParseDirectoryId(int64_t parseDirectoryId);
	string getTimestamp();
	void setTimestamp(string timestamp);
	string getFileName();
	void setFileName(string fileName);
	string getName();
	void setName(string name);
	string getMameName();
	void setMameName(string mameName);
	int64_t getProcessed();
	void setProcessed(int64_t processed);

	int load();
	int save();

	json_t *toJson();

        static ParseDirectoryGame *getItem(int64_t parseDirectoryId, string fileName);
        static list<ParseDirectoryGame *> *getPendingItems(int64_t parseDirectoryId);
	static list<ParseDirectoryGame *> *getItems();
	static ParseDirectoryGame *getItem(list<ParseDirectoryGame *> *items, unsigned int index);
	static void releaseItems(list<ParseDirectoryGame *> *items);
	static json_t *toJsonArray(list<ParseDirectoryGame *> *items);
};

#endif /* PARSEPROCESSGAME_H */

