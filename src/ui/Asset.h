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
 * File:   Asset.h
 * Author: ram
 *
 * Created on June 4, 2019, 9:17 PM
 */

#ifndef ASSET_H
#define ASSET_H

#include <string>

using namespace std;

/**
 * This class is for QoL. Provides an easy access to the assets of the application.
 */
class Asset {

public:
    static const string ASSET_CSS;
    static const string ASSET_IMAGE_LOGO_BIG;
    static const string ASSET_IMAGE_HOME;
    static const string ASSET_IMAGE_DOWNLOADING;
    static const string ASSET_IMAGE_DOCUMENT;
    static const string ASSET_IMAGE_GAME;

    /**
     * 
     * @return CSS filename.
     */
    string getCss();
    
    /**
     * 
     * @return Big image logo filename.
     */
    string getImageLogoBig();
    
    /**
     * 
     * @return Home image filename.
     */
    string getImageHome();
    
    /**
     * 
     * @return "Represents downloading" image filename.
     */
    string getImageDownloading();
    
    /**
     * 
     * @return Document image filename.
     */
    string getImageDocument();
    
    /**
     * 
     * @return Game image filename.
     */
    string getImageGame();
    
    /**
     * 
     * @return Instance of the object. Only one object is created in runtime.
     */
    static Asset *getInstance();
    
private:
    Asset();
    virtual ~Asset();

    static Asset *instance;
};

#endif /* ASSET_H */

