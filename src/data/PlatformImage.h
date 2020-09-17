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
 * File:   PlatformImage.h
 * Author: ram
 * 
 * Created April 3, 2019, 10:42 pm
*/

#ifndef PLATFORMIMAGE_H
#define PLATFORMIMAGE_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class PlatformImage
{
private:
	int64_t id;
	int64_t platformId;
	int64_t type;
	string fileName;
	int64_t external;
	int64_t apiId;
	string url;
	int64_t downloaded;


	PlatformImage();

public:
    static const string FILE_PREFIX;
    
    static const int64_t TYPE_FANART;
    static const int64_t TYPE_BANNER;
    static const int64_t TYPE_BOXART;
    static const int64_t TYPE_ICON;
    
    static const int THUMBNAIL_WIDTH;
    static const int THUMBNAIL_HEIGHT;
    
	PlatformImage(int64_t id);
	PlatformImage(const PlatformImage &orig);
	PlatformImage(json_t *json);

	~PlatformImage();


	int64_t getId();
	int64_t getPlatformId();
	void setPlatformId(int64_t platformId);
	int64_t getType();
	void setType(int64_t type);
	string getFileName();
	void setFileName(string fileName);
	int64_t getExternal();
	void setExternal(int64_t external);
	int64_t getApiId();
	void setApiId(int64_t apiId);
	string getUrl();
	void setUrl(string url);
	int64_t getDownloaded();
	void setDownloaded(int64_t downloaded);

	int load();
	int save();
        int remove();
        
        string getThumbnailFileName();
	json_t *toJson();

        static PlatformImage *getPlatformImage(int64_t platformId, int64_t type);
	static list<PlatformImage *> *getItems(int64_t platformId);
	static PlatformImage *getItem(list<PlatformImage *> *items, unsigned int index);
	static void releaseItems(list<PlatformImage *> *items);
	static json_t *toJsonArray(list<PlatformImage *> *items);
};

#endif
