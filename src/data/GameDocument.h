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
 * File:   GameDocument.h
 * Author: ram
 *
 * Created on June 29, 2019, 5:46 PM
 */

#ifndef GAMEDOCUMENT_H
#define GAMEDOCUMENT_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class GameDocument 
{
private:
	int64_t id;
	int64_t gameId;
	int64_t type;
	string name;
	string fileName;
	int64_t apiId;
        
        /**
         * Used for convenience. It is not part of the sqlite table.
         */
        string previewImageFileName;

	GameDocument();

public:
    static const string FILE_PREFIX;
    
    static const int TYPE_MANUAL;
    static const int TYPE_MAGAZINE;
    static const int TYPE_BOOK;
    static const int TYPE_GUIDE;
    static const int TYPE_OTHER;
    
	GameDocument(int64_t id);
	GameDocument(const GameDocument &orig);
	GameDocument(json_t *json);

	~GameDocument();

	int64_t getId();
	int64_t getGameId();
	void setGameId(int64_t gameId);
	int64_t getType();
	void setType(int64_t type);
	string getName();
	void setName(string name);
	string getFileName();
	void setFileName(string fileName);
	int64_t getApiId();
	void setApiId(int64_t apiId);
        
        void setPreviewImageFileName(string previewImageFileName);
        string getPreviewImageFileName();

	int load();
	int save();
        int remove();

	json_t *toJson();

	static list<GameDocument *> *getItems(int64_t gameId);
	static GameDocument *getItem(list<GameDocument *> *items, unsigned int index);
	static void releaseItems(list<GameDocument *> *items);
	static json_t *toJsonArray(list<GameDocument *> *items);

};

#endif /* GAMEDOCUMENT_H */

