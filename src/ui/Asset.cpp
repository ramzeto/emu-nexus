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
 * File:   Asset.cpp
 * Author: ram
 * 
 * Created on June 4, 2019, 9:17 PM
 */

#include "Asset.h"
#include "Directory.h"

#include <cstdlib>

const string Asset::ASSET_IMAGE_LOGO = "logo.png";
const string Asset::ASSET_IMAGE_HOME = ASSET_IMAGE_LOGO;
const string Asset::ASSET_IMAGE_DOWNLOADING = ASSET_IMAGE_LOGO;
const string Asset::ASSET_IMAGE_DOCUMENT = ASSET_IMAGE_LOGO;
const string Asset::ASSET_IMAGE_FAVORITE = "favorite.png";
const string Asset::ASSET_IMAGE_RECENT = "recent.png";
const string Asset::ASSET_HOME_PML = "home.pml";

Asset *Asset::instance = NULL;

Asset::Asset()
{
}

Asset::~Asset()
{
}

string Asset::getImageLogo()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_LOGO;
}

string Asset::getImageHome()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_HOME;
}

string Asset::getImageDownloading()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_DOWNLOADING;
}

string Asset::getImageDocument()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_DOCUMENT;
}

string Asset::getImageFavorite()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_FAVORITE;
}

string Asset::getImageRecent()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_RECENT;
}

string Asset::getHomePml()
{
    return Directory::getInstance()->getAssetsDirectory() + ASSET_HOME_PML;
}


Asset* Asset::getInstance()
{
    if(!instance)
    {
        instance = new Asset();
    }
    
    return instance;
}


