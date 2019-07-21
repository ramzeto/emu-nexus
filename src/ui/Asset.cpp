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
#include "Settings.h"

#include <cstdlib>

const string Asset::ASSET_CSS = "dark.css";
const string Asset::ASSET_IMAGE_LOGO_BIG = "logo_big.png";
const string Asset::ASSET_IMAGE_HOME = "home.png";
const string Asset::ASSET_IMAGE_DOWNLOADING = "downloading.png";
const string Asset::ASSET_IMAGE_DOCUMENT = "document.png";
const string Asset::ASSET_IMAGE_GAME = "game.png";

Asset *Asset::instance = NULL;

Asset::Asset()
{
}

Asset::~Asset()
{
}

string Asset::getCss()
{
    return Settings::getInstance()->getCssDirectory() + ASSET_CSS;
}

string Asset::getImageLogoBig()
{
    return Settings::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_LOGO_BIG;
}

string Asset::getImageHome()
{
    return Settings::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_HOME;
}

string Asset::getImageDownloading()
{
    return Settings::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_DOWNLOADING;
}

string Asset::getImageDocument()
{
    return Settings::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_DOCUMENT;
}

string Asset::getImageGame()
{
    return Settings::getInstance()->getAssetImagesDirectory() + ASSET_IMAGE_GAME;
}


Asset* Asset::getInstance()
{
    if(!instance)
    {
        instance = new Asset();
    }
    
    return instance;
}


