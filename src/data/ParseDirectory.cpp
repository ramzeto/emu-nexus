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
 * File:   ParseDirectory.cpp
 * Author: ram
 * 
 * Created on August 6, 2019, 10:51 PM
 */

#include "ParseDirectory.h"
#include <iostream>

ParseDirectory::ParseDirectory()
{
}

ParseDirectory::ParseDirectory(int64_t id)
{
	this->id = id;
        this->timestamp = "";
        this->start = "";
        this->end = "";
}

ParseDirectory::ParseDirectory(const ParseDirectory &orig)
{
	this->id = orig.id;
	this->platformId = orig.platformId;
	this->timestamp = orig.timestamp;
	this->start = orig.start;
	this->end = orig.end;
	this->directory = orig.directory;
	this->fileExtensions = orig.fileExtensions;
	this->useMame = orig.useMame;
	this->mame = orig.mame;
	this->boxFrontImagesDirectory = orig.boxFrontImagesDirectory;
	this->boxBackImagesDirectory = orig.boxBackImagesDirectory;
	this->screenshotImagesDirectory = orig.screenshotImagesDirectory;
	this->logoImagesDirectory = orig.logoImagesDirectory;
	this->bannerImagesDirectory = orig.bannerImagesDirectory;
}

ParseDirectory::ParseDirectory(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *platformIdJson = json_object_get(json, "platformId");
	if(platformIdJson)
	{
		platformId = (int64_t)json_integer_value(platformIdJson);
	}

	json_t *timestampJson = json_object_get(json, "timestamp");
	if(timestampJson)
	{
		timestamp = string(json_string_value(timestampJson));
	}

	json_t *startJson = json_object_get(json, "start");
	if(startJson)
	{
		start = string(json_string_value(startJson));
	}

	json_t *endJson = json_object_get(json, "end");
	if(endJson)
	{
		end = string(json_string_value(endJson));
	}

	json_t *directoryJson = json_object_get(json, "directory");
	if(directoryJson)
	{
		directory = string(json_string_value(directoryJson));
	}

	json_t *fileExtensionsJson = json_object_get(json, "fileExtensions");
	if(fileExtensionsJson)
	{
		fileExtensions = string(json_string_value(fileExtensionsJson));
	}

	json_t *useMameJson = json_object_get(json, "useMame");
	if(useMameJson)
	{
		useMame = (int64_t)json_integer_value(useMameJson);
	}

	json_t *mameJson = json_object_get(json, "mame");
	if(mameJson)
	{
		mame = string(json_string_value(mameJson));
	}

	json_t *boxFrontImagesDirectoryJson = json_object_get(json, "boxFrontImagesDirectory");
	if(boxFrontImagesDirectoryJson)
	{
		boxFrontImagesDirectory = string(json_string_value(boxFrontImagesDirectoryJson));
	}

	json_t *boxBackImagesDirectoryJson = json_object_get(json, "boxBackImagesDirectory");
	if(boxBackImagesDirectoryJson)
	{
		boxBackImagesDirectory = string(json_string_value(boxBackImagesDirectoryJson));
	}

	json_t *screenshotImagesDirectoryJson = json_object_get(json, "screenshotImagesDirectory");
	if(screenshotImagesDirectoryJson)
	{
		screenshotImagesDirectory = string(json_string_value(screenshotImagesDirectoryJson));
	}

	json_t *logoImagesDirectoryJson = json_object_get(json, "logoImagesDirectory");
	if(logoImagesDirectoryJson)
	{
		logoImagesDirectory = string(json_string_value(logoImagesDirectoryJson));
	}

	json_t *bannerImagesDirectoryJson = json_object_get(json, "bannerImagesDirectory");
	if(bannerImagesDirectoryJson)
	{
		bannerImagesDirectory = string(json_string_value(bannerImagesDirectoryJson));
	}

}

ParseDirectory::~ParseDirectory()
{
}

int64_t ParseDirectory::getId()
{
	return id;
}

int64_t ParseDirectory::getPlatformId()
{
	return platformId;
}

void ParseDirectory::setPlatformId(int64_t platformId)
{
	this->platformId = platformId;
}

string ParseDirectory::getTimestamp()
{
	return timestamp;
}

void ParseDirectory::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

string ParseDirectory::getStart()
{
	return start;
}

void ParseDirectory::setStart(string start)
{
	this->start = start;
}

string ParseDirectory::getEnd()
{
	return end;
}

void ParseDirectory::setEnd(string end)
{
	this->end = end;
}

string ParseDirectory::getDirectory()
{
	return directory;
}

void ParseDirectory::setDirectory(string directory)
{
	this->directory = directory;
}

string ParseDirectory::getFileExtensions()
{
	return fileExtensions;
}

void ParseDirectory::setFileExtensions(string fileExtensions)
{
	this->fileExtensions = fileExtensions;
}

int64_t ParseDirectory::getUseMame()
{
	return useMame;
}

void ParseDirectory::setUseMame(int64_t useMame)
{
	this->useMame = useMame;
}

string ParseDirectory::getMame()
{
	return mame;
}

void ParseDirectory::setMame(string mame)
{
	this->mame = mame;
}

string ParseDirectory::getBoxFrontImagesDirectory()
{
	return boxFrontImagesDirectory;
}

void ParseDirectory::setBoxFrontImagesDirectory(string boxFrontImagesDirectory)
{
	this->boxFrontImagesDirectory = boxFrontImagesDirectory;
}

string ParseDirectory::getBoxBackImagesDirectory()
{
	return boxBackImagesDirectory;
}

void ParseDirectory::setBoxBackImagesDirectory(string boxBackImagesDirectory)
{
	this->boxBackImagesDirectory = boxBackImagesDirectory;
}

string ParseDirectory::getScreenshotImagesDirectory()
{
	return screenshotImagesDirectory;
}

void ParseDirectory::setScreenshotImagesDirectory(string screenshotImagesDirectory)
{
	this->screenshotImagesDirectory = screenshotImagesDirectory;
}

string ParseDirectory::getLogoImagesDirectory()
{
	return logoImagesDirectory;
}

void ParseDirectory::setLogoImagesDirectory(string logoImagesDirectory)
{
	this->logoImagesDirectory = logoImagesDirectory;
}

string ParseDirectory::getBannerImagesDirectory()
{
	return bannerImagesDirectory;
}

void ParseDirectory::setBannerImagesDirectory(string bannerImagesDirectory)
{
	this->bannerImagesDirectory = bannerImagesDirectory;
}

int ParseDirectory::load(sqlite3 *sqlite)
{
	int result = 0;
	string query = "select id, platformId, timestamp, start, end, directory, fileExtensions, useMame, mame, boxFrontImagesDirectory, boxBackImagesDirectory, screenshotImagesDirectory, logoImagesDirectory, bannerImagesDirectory from ParseDirectory where  id = ?";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, (sqlite3_int64)id);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			id = (int64_t)sqlite3_column_int64(statement, 0);
			platformId = (int64_t)sqlite3_column_int64(statement, 1);
			timestamp = string((const char*) sqlite3_column_text(statement, 2));
			start = string((const char*) sqlite3_column_text(statement, 3));
			end = string((const char*) sqlite3_column_text(statement, 4));
			directory = string((const char*) sqlite3_column_text(statement, 5));
			fileExtensions = string((const char*) sqlite3_column_text(statement, 6));
			useMame = (int64_t)sqlite3_column_int64(statement, 7);
			mame = string((const char*) sqlite3_column_text(statement, 8));
			boxFrontImagesDirectory = string((const char*) sqlite3_column_text(statement, 9));
			boxBackImagesDirectory = string((const char*) sqlite3_column_text(statement, 10));
			screenshotImagesDirectory = string((const char*) sqlite3_column_text(statement, 11));
			logoImagesDirectory = string((const char*) sqlite3_column_text(statement, 12));
			bannerImagesDirectory = string((const char*) sqlite3_column_text(statement, 13));
			result = 1;
		}
	}
	else
	{
		cerr << "ParseDirectory::load " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return result;
}

int ParseDirectory::save(sqlite3 *sqlite)
{
	int result = 1;
	if(id == 0)
	{
		string insert = "insert into ParseDirectory (platformId, timestamp, start, end, directory, fileExtensions, useMame, mame, boxFrontImagesDirectory, boxBackImagesDirectory, screenshotImagesDirectory, logoImagesDirectory, bannerImagesDirectory) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, insert.c_str(), insert.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_text(statement, 3, start.c_str(), start.length(), NULL);
			sqlite3_bind_text(statement, 4, end.c_str(), end.length(), NULL);
			sqlite3_bind_text(statement, 5, directory.c_str(), directory.length(), NULL);
			sqlite3_bind_text(statement, 6, fileExtensions.c_str(), fileExtensions.length(), NULL);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)useMame);
			sqlite3_bind_text(statement, 8, mame.c_str(), mame.length(), NULL);
			sqlite3_bind_text(statement, 9, boxFrontImagesDirectory.c_str(), boxFrontImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 10, boxBackImagesDirectory.c_str(), boxBackImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 11, screenshotImagesDirectory.c_str(), screenshotImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 12, logoImagesDirectory.c_str(), logoImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 13, bannerImagesDirectory.c_str(), bannerImagesDirectory.length(), NULL);
			
			if(!(result = (sqlite3_step(statement) == SQLITE_DONE ? 0 : 1)))
			{
				id = sqlite3_last_insert_rowid(sqlite);
			}
		}
		else
		{
			cerr << "ParseProcess::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	else
	{
		string update = "update ParseDirectory set platformId = ?, timestamp = ?, start = ?, end = ?, directory = ?, fileExtensions = ?, useMame = ?, mame = ?, boxFrontImagesDirectory = ?, boxBackImagesDirectory = ?, screenshotImagesDirectory = ?, logoImagesDirectory = ?, bannerImagesDirectory = ? where id = ?";
		sqlite3_stmt *statement;
		if (sqlite3_prepare_v2(sqlite, update.c_str(), update.length(), &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64)platformId);
			sqlite3_bind_text(statement, 2, timestamp.c_str(), timestamp.length(), NULL);
			sqlite3_bind_text(statement, 3, start.c_str(), start.length(), NULL);
			sqlite3_bind_text(statement, 4, end.c_str(), end.length(), NULL);
			sqlite3_bind_text(statement, 5, directory.c_str(), directory.length(), NULL);
			sqlite3_bind_text(statement, 6, fileExtensions.c_str(), fileExtensions.length(), NULL);
			sqlite3_bind_int64(statement, 7, (sqlite3_int64)useMame);
			sqlite3_bind_text(statement, 8, mame.c_str(), mame.length(), NULL);
			sqlite3_bind_text(statement, 9, boxFrontImagesDirectory.c_str(), boxFrontImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 10, boxBackImagesDirectory.c_str(), boxBackImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 11, screenshotImagesDirectory.c_str(), screenshotImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 12, logoImagesDirectory.c_str(), logoImagesDirectory.length(), NULL);
			sqlite3_bind_text(statement, 13, bannerImagesDirectory.c_str(), bannerImagesDirectory.length(), NULL);
			sqlite3_bind_int64(statement, 14, (sqlite3_int64)id);
			
			result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
		}
		else
		{
			cerr << "ParseProcess::save " << sqlite3_errmsg(sqlite) << endl;
		}

		sqlite3_finalize(statement);
	}
	return result;
}

json_t *ParseDirectory::toJson()
{
	json_t *json = json_object();

	json_t *idJson = json_integer((json_int_t)id);
	json_object_set_new(json, "id", idJson);

	json_t *platformIdJson = json_integer((json_int_t)platformId);
	json_object_set_new(json, "platformId", platformIdJson);

	json_t *timestampJson = json_string(timestamp.c_str());
	json_object_set_new(json, "timestamp", timestampJson);

	json_t *startJson = json_string(start.c_str());
	json_object_set_new(json, "start", startJson);

	json_t *endJson = json_string(end.c_str());
	json_object_set_new(json, "end", endJson);

	json_t *directoryJson = json_string(directory.c_str());
	json_object_set_new(json, "directory", directoryJson);

	json_t *fileExtensionsJson = json_string(fileExtensions.c_str());
	json_object_set_new(json, "fileExtensions", fileExtensionsJson);

	json_t *useMameJson = json_integer((json_int_t)useMame);
	json_object_set_new(json, "useMame", useMameJson);

	json_t *mameJson = json_string(mame.c_str());
	json_object_set_new(json, "mame", mameJson);

	json_t *boxFrontImagesDirectoryJson = json_string(boxFrontImagesDirectory.c_str());
	json_object_set_new(json, "boxFrontImagesDirectory", boxFrontImagesDirectoryJson);

	json_t *boxBackImagesDirectoryJson = json_string(boxBackImagesDirectory.c_str());
	json_object_set_new(json, "boxBackImagesDirectory", boxBackImagesDirectoryJson);

	json_t *screenshotImagesDirectoryJson = json_string(screenshotImagesDirectory.c_str());
	json_object_set_new(json, "screenshotImagesDirectory", screenshotImagesDirectoryJson);

	json_t *logoImagesDirectoryJson = json_string(logoImagesDirectory.c_str());
	json_object_set_new(json, "logoImagesDirectory", logoImagesDirectoryJson);

	json_t *bannerImagesDirectoryJson = json_string(bannerImagesDirectory.c_str());
	json_object_set_new(json, "bannerImagesDirectory", bannerImagesDirectoryJson);

	return json;
}

ParseDirectory* ParseDirectory::getPengingItem(sqlite3* sqlite)
{
    ParseDirectory *item = NULL;
    string query = "select id, platformId, timestamp, start, end, directory, fileExtensions, useMame, mame, boxFrontImagesDirectory, boxBackImagesDirectory, screenshotImagesDirectory, logoImagesDirectory, bannerImagesDirectory from ParseDirectory where end = '' order by timestamp desc limit 1";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                    item = new ParseDirectory();
                    item->id = (int64_t)sqlite3_column_int64(statement, 0);
                    item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
                    item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
                    item->start = string((const char*) sqlite3_column_text(statement, 3));
                    item->end = string((const char*) sqlite3_column_text(statement, 4));
                    item->directory = string((const char*) sqlite3_column_text(statement, 5));
                    item->fileExtensions = string((const char*) sqlite3_column_text(statement, 6));
                    item->useMame = (int64_t)sqlite3_column_int64(statement, 7);
                    item->mame = string((const char*) sqlite3_column_text(statement, 8));
                    item->boxFrontImagesDirectory = string((const char*) sqlite3_column_text(statement, 9));
                    item->boxBackImagesDirectory = string((const char*) sqlite3_column_text(statement, 10));
                    item->screenshotImagesDirectory = string((const char*) sqlite3_column_text(statement, 11));
                    item->logoImagesDirectory = string((const char*) sqlite3_column_text(statement, 12));
                    item->bannerImagesDirectory = string((const char*) sqlite3_column_text(statement, 13));
            }
    }
    else
    {
            cerr << "ParseDirectory::getPengingItem " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);

        
    return item;
}

list<ParseDirectory *> *ParseDirectory::getItems(sqlite3 *sqlite)
{
	list<ParseDirectory *> *items = new list<ParseDirectory *>;
	string query = "select id, platformId, timestamp, start, end, directory, fileExtensions, useMame, mame, boxFrontImagesDirectory, boxBackImagesDirectory, screenshotImagesDirectory, logoImagesDirectory, bannerImagesDirectory from ParseDirectory";
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			ParseDirectory *item = new ParseDirectory();
			item->id = (int64_t)sqlite3_column_int64(statement, 0);
			item->platformId = (int64_t)sqlite3_column_int64(statement, 1);
			item->timestamp = string((const char*) sqlite3_column_text(statement, 2));
			item->start = string((const char*) sqlite3_column_text(statement, 3));
			item->end = string((const char*) sqlite3_column_text(statement, 4));
			item->directory = string((const char*) sqlite3_column_text(statement, 5));
			item->fileExtensions = string((const char*) sqlite3_column_text(statement, 6));
			item->useMame = (int64_t)sqlite3_column_int64(statement, 7);
			item->mame = string((const char*) sqlite3_column_text(statement, 8));
			item->boxFrontImagesDirectory = string((const char*) sqlite3_column_text(statement, 9));
			item->boxBackImagesDirectory = string((const char*) sqlite3_column_text(statement, 10));
			item->screenshotImagesDirectory = string((const char*) sqlite3_column_text(statement, 11));
			item->logoImagesDirectory = string((const char*) sqlite3_column_text(statement, 12));
			item->bannerImagesDirectory = string((const char*) sqlite3_column_text(statement, 13));

			items->push_back(item);
		}
	}
	else
	{
		cerr << "ParseDirectory::getItems " << sqlite3_errmsg(sqlite) << endl;
	}

	sqlite3_finalize(statement);
	return items;
}

ParseDirectory *ParseDirectory::getItem(list<ParseDirectory *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<ParseDirectory *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void ParseDirectory::releaseItems(list<ParseDirectory *> *items)
{
	for(list<ParseDirectory *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}

json_t *ParseDirectory::toJsonArray(list<ParseDirectory *> *items)
{
	json_t *jsonArray = json_array();
	for(list<ParseDirectory *>::iterator item = items->begin(); item != items->end(); item++)
	{
		json_array_append_new(jsonArray, (*item)->toJson());
	}

	return jsonArray;
}


