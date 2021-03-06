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

const string Asset::ASSET_CSS = "default.css";
const string Asset::ASSET_IMAGE_LOGO = "logo.png";
const string Asset::ASSET_IMAGE_BASE_BANNER = "base_banner.jpg";
const string Asset::ASSET_IMAGE_NON_FAVORITE = "non_favorite.png";
const string Asset::ASSET_IMAGE_NO_GAME_IMAGE = "no_game_image.png";
const string Asset::ASSET_HOME_PML = "home.pml";

Asset *Asset::instance = NULL;

Asset::Asset()
{
}

Asset::~Asset()
{
}

string Asset::getCss()
{
    return Directory::getInstance()->getCssDirectory() + ASSET_CSS;
}

string Asset::getImageLogo()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_LOGO;
}

string Asset::getImageBaseBanner()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_BASE_BANNER;
}


string Asset::getImageNonFavorite()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_NON_FAVORITE;
}

string Asset::getImageNoGameImage()
{
    return Directory::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_NO_GAME_IMAGE;
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


