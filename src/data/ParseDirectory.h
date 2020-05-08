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
 * File:   ParseDirectory.h
 * Author: ram
 *
 * Created on August 6, 2019, 10:51 PM
 */

#ifndef PARSEDIRECTORY_H
#define PARSEDIRECTORY_H

#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class ParseDirectory
{
private:
	int64_t id;
	int64_t platformId;
	string timestamp;
	string start;
	string end;
	string directory;
	string fileExtensions;
	int64_t useMame;
	string mame;
	string boxFrontImagesDirectory;
	string boxBackImagesDirectory;
	string screenshotImagesDirectory;
	string logoImagesDirectory;
	string bannerImagesDirectory;

	ParseDirectory();

public:
	ParseDirectory(int64_t id);
	ParseDirectory(const ParseDirectory &orig);
	ParseDirectory(json_t *json);

	~ParseDirectory();

	int64_t getId();
	int64_t getPlatformId();
	void setPlatformId(int64_t platformId);
	string getTimestamp();
	void setTimestamp(string timestamp);
	string getStart();
	void setStart(string start);
	string getEnd();
	void setEnd(string end);
	string getDirectory();
	void setDirectory(string directory);
	string getFileExtensions();
	void setFileExtensions(string fileExtensions);
	int64_t getUseMame();
	void setUseMame(int64_t useMame);
	string getMame();
	void setMame(string mame);
	string getBoxFrontImagesDirectory();
	void setBoxFrontImagesDirectory(string boxFrontImagesDirectory);
	string getBoxBackImagesDirectory();
	void setBoxBackImagesDirectory(string boxBackImagesDirectory);
	string getScreenshotImagesDirectory();
	void setScreenshotImagesDirectory(string screenshotImagesDirectory);
	string getLogoImagesDirectory();
	void setLogoImagesDirectory(string logoImagesDirectory);
	string getBannerImagesDirectory();
	void setBannerImagesDirectory(string bannerImagesDirectory);

	int load();
	int save();

	json_t *toJson();

        static ParseDirectory *getPengingItem();
	static list<ParseDirectory *> *getItems();
	static ParseDirectory *getItem(list<ParseDirectory *> *items, unsigned int index);
	static void releaseItems(list<ParseDirectory *> *items);
	static json_t *toJsonArray(list<ParseDirectory *> *items);
};

#endif /* PARSEPROCESS_H */

