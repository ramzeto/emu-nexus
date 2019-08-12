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
 * File:   Directory.h
 * Author: ram
 *
 * Created on August 5, 2019, 7:12 PM
 */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>

using namespace std;

/**
 * This class provides easy access to the directories of the application.
 */
class Directory 
{
public:
    string getDataDirectory();
    string getAssetsDirectory();
    string getUiTemplatesDirectory();
    string getCssDirectory();
    string getAssetImagesDirectory();
    string getMediaDirectory();
    string getElasticseachDirectory();    
    string getCacheDirectory();
    
    static Directory *getInstance();
    
private:
    static const string DIRECTORY_ASSETS;
    static const string DIRECTORY_UI_TEMPLATES;
    static const string DIRECTORY_CSS;
    static const string DIRECTORY_ASSET_IMAGES;
    static const string DIRECTORY_MEDIA;
    static const string DIRECTORY_ELASTICSEARCH;    
    static const string DIRECTORY_CACHE;
    
    string dataDirectory;
    
    Directory();
    virtual ~Directory();
    
    static Directory *instance;
};

#endif /* DIRECTORY_H */

