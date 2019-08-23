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
 * File:   ElasticsearchProcess.h
 * Author: ram
 *
 * Created on May 8, 2019, 10:59 PM
 */

#ifndef ELASTICSEARCHPROCESS_H
#define ELASTICSEARCHPROCESS_H

#include "SerialProcess.h"
#include "HttpConnector.h"
#include "CallbackResult.h"

using namespace std;

/**
 * Class for checking, downloading and starting the Elasticsearch database.
 */
class ElasticsearchProcess : public SerialProcess
{
public:
    static const string TYPE;
    
    ElasticsearchProcess(void *requester, void (*statusCallback)(CallbackResult *));
    virtual ~ElasticsearchProcess();
    
    int execute() override;
private:
    static const string URL_DOWNLOAD;
    static const string URL_MD5;
    static const string TARGZ_FILE_NAME;
    static const string TAR_FILE_NAME;

    static void httpConnectorProgressListener(void *pElasticsearchProcess, HttpConnector *httpConnector, size_t bytesToDownload, size_t downloadedBytes);
    static void callbackElasticsearchStartListener(CallbackResult *callbackResult);
};

#endif /* THEGAMESDBSTARTPROCESS_H */

