/*
 * Copyright (C) 2020 ram
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
 * File:   SetupDatabaseProcess.h
 * Author: ram
 *
 * Created on September 6, 2020, 7:19 PM
 */

#ifndef SETUPDATABASEPROCESS_H
#define SETUPDATABASEPROCESS_H

#include "SerialProcess.h"
#include "HttpConnector.h"

using namespace std;

class SetupDatabaseProcess : public SerialProcess
{
public:
    static const string TYPE;
    
    SetupDatabaseProcess();
    virtual ~SetupDatabaseProcess();
        
private:
    static const string URL_DOWNLOAD;
    static const string URL_MD5;
    static const string TARGZ_FILE_NAME;
    static const string TAR_FILE_NAME;
    
    void execute() override;
    void initialize();
    void update();
};

#endif /* SETUPDATABASEPROCESS_H */

