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
 * File:   Directory.cpp
 * Author: ram
 * 
 * Created on August 5, 2019, 7:12 PM
 */

#include "Directory.h"
#include "Utils.h"

#include <cstdlib>
#include <unistd.h>

Directory *Directory::instance = NULL;

const string Directory::DIRECTORY_ASSETS = "/usr/share/emu-nexus/assets/";
const string Directory::DIRECTORY_UI_TEMPLATES = "ui_templates/";
const string Directory::DIRECTORY_CSS = "css/";
const string Directory::DIRECTORY_ASSET_IMAGES = "images/";
const string Directory::DIRECTORY_MEDIA = "media/";
const string Directory::DIRECTORY_ELASTICSEARCH = "elasticsearch/";
const string Directory::DIRECTORY_CACHE = "cache/";

Directory::Directory()
{
    // Creates the config directory if it does not exists (just in case)
    string configDirectory = string(getenv("HOME")) + "/.config/";
    Utils::getInstance()->makeDirectory(configDirectory);
    
    // Assigns the default data directory
    dataDirectory = configDirectory + "emu-nexus/";
    
    Utils::getInstance()->makeDirectory(getDataDirectory());
    Utils::getInstance()->makeDirectory(getMediaDirectory());
    Utils::getInstance()->makeDirectory(getCacheDirectory());
}

Directory::~Directory()
{
}

string Directory::getDataDirectory()
{
    return dataDirectory;
}

string Directory::getAssetsDirectory()
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

string Directory::getUiTemplatesDirectory()
{
    return getAssetsDirectory() + DIRECTORY_UI_TEMPLATES;
}

string Directory::getCssDirectory()
{
    return getAssetsDirectory() + DIRECTORY_CSS;
}

string Directory::getAssetImagesDirectory()
{
    return getAssetsDirectory() + DIRECTORY_ASSET_IMAGES;
}

string Directory::getMediaDirectory()
{
    return getDataDirectory() + DIRECTORY_MEDIA;
}

string Directory::getElasticseachDirectory()
{
    return getDataDirectory() + DIRECTORY_ELASTICSEARCH;
}

string Directory::getCacheDirectory()
{
    return getDataDirectory() + DIRECTORY_CACHE;
}

Directory* Directory::getInstance()
{
    if(!instance)
    {
        instance = new Directory();
    }
    
    return instance;
}


