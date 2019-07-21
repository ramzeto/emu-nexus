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
 * @date March 7, 2019, 9:43 pm
 */

#include "Settings.h"
#include "Database.h"
#include "Utils.h"

#include <iostream>
#include <unistd.h>

Settings *Settings::instance = NULL;

const string Settings::DIRECTORY_ASSETS = "/usr/share/emu-nexus/assets/";
const string Settings::DIRECTORY_UI_TEMPLATES = "ui_templates/";
const string Settings::DIRECTORY_CSS = "css/";
const string Settings::DIRECTORY_ASSET_IMAGES = "images/";
const string Settings::DIRECTORY_MEDIA = "media/";
const string Settings::DIRECTORY_ELASTICSEARCH = "elasticsearch/";
const string Settings::ELASTICSEARCH_BINARY = "bin/elasticsearch";
const unsigned int Settings::ELASTICSEARCH_MIN_PORT = 9301;
const unsigned int Settings::ELASTICSEARCH_MAX_PORT = 9350;

    
Settings::Settings()
{
    // Creates the config directory if it does not exists (just in case)
    string configDirectory = string(getenv("HOME")) + "/.config/";
    Utils::getInstance()->makeDirectory(configDirectory);

    // Assigns the default data directory
    dataDirectory = configDirectory + "emu-nexus/";

    // Assigns default cacheDirectory
    cacheDirectory = dataDirectory + "cache/";

    // Assigns default cacheSize in MB
    cacheSize = 9359;
    
    mameExecutable = "mame";
    
    elasticsearchPort = 9329;
}

Settings::~Settings()
{
}

string Settings::getDataDirectory()
{
    return dataDirectory;
}

void Settings::setDataDirectory(string dataDirectory)
{
    this->dataDirectory = dataDirectory;
}

string Settings::getCacheDirectory()
{
    return cacheDirectory;
}

void Settings::setCacheDirectory(string cacheDirectory)
{
    this->cacheDirectory = cacheDirectory;
}

int64_t Settings::getCacheSize()
{
    return cacheSize;
}

void Settings::setCacheSize(int64_t cacheSize)
{
    this->cacheSize = cacheSize;
}

void Settings::setMameExecutable(string mameExecutable)
{
    this->mameExecutable = mameExecutable;
}

string Settings::getMameExecutable() const
{
    return mameExecutable;
}


int64_t Settings::getElasticsearchPort()
{
	return elasticsearchPort;
}

void Settings::setElasticsearchPort(int64_t elasticsearchPort)
{
	this->elasticsearchPort = elasticsearchPort;
}

int Settings::load(sqlite3 *sqlite)
{
    int result = 0;

    string query = "select dataDirectory, cacheDirectory, cacheSize, mameExecutable, elasticsearchPort from Settings";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            dataDirectory = string((const char*) sqlite3_column_text(statement, 0));
            cacheDirectory = string((const char*) sqlite3_column_text(statement, 1));
            cacheSize = (int64_t) sqlite3_column_int64(statement, 2);
            mameExecutable = string((const char*) sqlite3_column_text(statement, 3));
            elasticsearchPort = (int64_t)sqlite3_column_int64(statement, 4);
            result = 1;
        }
    }
    else
    {
        cerr << "Settings::load " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);

    return result;
}

int Settings::save(sqlite3 *sqlite)
{
    int result = 1;

    int exists = 0;
    string query = "select dataDirectory from Settings";
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(sqlite, query.c_str(), query.length(), &statement, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            exists = 1;
        }
    }
    else
    {
        cerr << "Settings::" << __FUNCTION__ << " " << sqlite3_errmsg(sqlite) << endl;
    }
    sqlite3_finalize(statement);


    string insertOrUpdate;
    if (exists)
    {
        insertOrUpdate = "update Settings set dataDirectory = ?, cacheDirectory = ?, cacheSize = ?, mameExecutable = ?, elasticsearchPort = ?";
    }
    else
    {
        insertOrUpdate = "insert into Settings (dataDirectory, cacheDirectory, cacheSize, mameExecutable, elasticsearchPort) values(?, ?, ?, ?, ?)";
    }

    if (sqlite3_prepare_v2(sqlite, insertOrUpdate.c_str(), insertOrUpdate.length(), &statement, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, dataDirectory.c_str(), dataDirectory.length(), NULL);
        sqlite3_bind_text(statement, 2, cacheDirectory.c_str(), cacheDirectory.length(), NULL);
        sqlite3_bind_int64(statement, 3, (sqlite3_int64) cacheSize);
        sqlite3_bind_text(statement, 4, mameExecutable.c_str(), mameExecutable.length(), NULL);
        sqlite3_bind_int64(statement, 5, (sqlite3_int64) elasticsearchPort);

        result = sqlite3_step(statement) == SQLITE_DONE ? 0 : 1;
    }
    else
    {
        cerr << "Settings::" << __FUNCTION__ << " " << sqlite3_errmsg(sqlite) << endl;
    }

    sqlite3_finalize(statement);

    return result;
}

string Settings::getAssetsDirectory()
{
    size_t bufferSize = 500;
    char *buffer = new char[bufferSize];

    getcwd(buffer, bufferSize);
    string localAssetsDirectory = string(buffer) + "/assets/";

    delete[] buffer;

    if (Utils::getInstance()->fileExists(localAssetsDirectory))
    {
        return localAssetsDirectory;
    }

    return DIRECTORY_ASSETS;
}

string Settings::getUiTemplatesDirectory()
{
    return getAssetsDirectory() + DIRECTORY_UI_TEMPLATES;
}

string Settings::getCssDirectory()
{
    return getAssetsDirectory() + DIRECTORY_CSS;
}

string Settings::getAssetImagesDirectory()
{
    return getAssetsDirectory() + DIRECTORY_ASSET_IMAGES;
}

string Settings::getMediaDirectory()
{
    return getDataDirectory() + DIRECTORY_MEDIA;
}

string Settings::getElasticseachDirectory()
{
    return getDataDirectory() + DIRECTORY_ELASTICSEARCH;
}

string Settings::getElasticseachBinary()
{
    return getElasticseachDirectory() + ELASTICSEARCH_BINARY;
}


Settings* Settings::getInstance()
{
    if (!instance)
    {
        instance = new Settings();
        
        if (Utils::getInstance()->fileExists(Database::getInstance()->getDatabaseFileName()))
        {
            sqlite3 *sqlite = Database::getInstance()->acquire();
            instance->load(sqlite);
            Database::getInstance()->release();
        }
        

        Utils::getInstance()->makeDirectory(instance->dataDirectory);
        Utils::getInstance()->makeDirectory(instance->cacheDirectory);
        Utils::getInstance()->makeDirectory(instance->getMediaDirectory());
    }

    return instance;
}

