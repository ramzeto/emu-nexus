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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <sqlite3.h>
#include <string>
#include <list>
#include <jansson.h>
#include <cstdint>

using namespace std;

class Settings {
private:
    static const string DIRECTORY_ASSETS;
    static const string DIRECTORY_UI_TEMPLATES;
    static const string DIRECTORY_CSS;
    static const string DIRECTORY_ASSET_IMAGES;
    static const string DIRECTORY_MEDIA;
    static const string DIRECTORY_ELASTICSEARCH;
    static const string ELASTICSEARCH_BINARY;

    
    string dataDirectory;
    string cacheDirectory;
    int64_t cacheSize;
    string mameExecutable;
    int64_t elasticsearchPort;

    Settings();
    ~Settings();

    /**
     * Loads the object from the database.
     * @param sqlite
     * @return 1 if the object exists in the database and loads successfully, 0 otherwise.
     */
    int load(sqlite3 *sqlite);

    /**
     * 
     * @return Assets directory. It looks in the current working directory for the assets folder, if it does not exist, returns the default installation assets directory.
     */
    string getAssetsDirectory();

    static Settings *instance;

public:
    static const unsigned int ELASTICSEARCH_MIN_PORT;
    static const unsigned int ELASTICSEARCH_MAX_PORT;
    
    string getDataDirectory();
    void setDataDirectory(string dataDirectory);
    string getCacheDirectory();
    void setCacheDirectory(string cacheDirectory);
    int64_t getCacheSize();
    void setCacheSize(int64_t cacheSize);
    void setMameExecutable(string mameExecutable);
    string getMameExecutable() const;
    int64_t getElasticsearchPort();
    void setElasticsearchPort(int64_t searchEnginePort);
        
    string getUiTemplatesDirectory();
    string getCssDirectory();
    string getAssetImagesDirectory();
    string getMediaDirectory();
    string getElasticseachDirectory();
    string getElasticseachBinary();


    /**
     * Saves the object to the database.
     * @param sqlite
     * @return 0 on success 
     */
    int save(sqlite3 *sqlite);

    /**
     * 
     * @return Preferences instance.
     */
    static Settings *getInstance();
};

#endif