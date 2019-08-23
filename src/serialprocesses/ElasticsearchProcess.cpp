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
 * File:   TheGamesDbStartProcess.cpp
 * Author: ram
 * 
 * Created on May 8, 2019, 10:59 PM
 */

#include "ElasticsearchProcess.h"
#include "SerialProcessExecutor.h"
#include "Database.h"
#include "ApiDatabase.h"
#include "Utils.h"
#include "FileExtractor.h"
#include "thegamesdb.h"
#include "Directory.h"

#include <iostream>
#include <jansson.h>
#include <unistd.h>
#include <fstream>

const string ElasticsearchProcess::TYPE = "ElasticsearchProcess";
//const string ElasticsearchProcess::URL_DOWNLOAD = "http://localhost/public/emu-nexus-db/downloads/thegamesdb.tar.gz";
//const string ElasticsearchProcess::URL_MD5 = "http://localhost/public/emu-nexus-db/downloads/thegamesdb.md5";
const string ElasticsearchProcess::URL_DOWNLOAD = "https://www.dropbox.com/s/4hlnjvdqu1f2ndb/thegamesdb.tar.gz?dl=1";
const string ElasticsearchProcess::URL_MD5 = "https://www.dropbox.com/s/lm6bug13yqqli7f/thegamesdb.md5?dl=1";
const string ElasticsearchProcess::TARGZ_FILE_NAME = "elasticsearch.tar.gz";
const string ElasticsearchProcess::TAR_FILE_NAME = "elasticsearch.tar";

ElasticsearchProcess::ElasticsearchProcess(void *requester, void (*statusCallback)(CallbackResult *)) : SerialProcess(TYPE, requester, statusCallback)
{
    
}

ElasticsearchProcess::~ElasticsearchProcess()
{
}

int ElasticsearchProcess::execute()
{
    status = STATUS_RUNNING;
    
    // @TODO Find a more elegant way to do this. Kills the elasticseach process in case it is running.
    system("pkill -9 elasticsearch");
    sleep(1);
    
    postStatus(string("Downloading database"));
    
    HttpConnector *md5HttpConnector = new HttpConnector(URL_MD5);
    if(md5HttpConnector->get() == HttpConnector::HTTP_OK)
    {
        string md5sum((const char *)md5HttpConnector->getResponseData(), md5HttpConnector->getResponseDataSize());        
        cout << "md5sum : " << md5sum << endl;
        
        
        sqlite3 *sqlite = Database::getInstance()->acquire();
        ApiDatabase *apiDatabase = new ApiDatabase(TheGamesDB::API_ID, md5sum);
        int isCurrentVersion = apiDatabase->load(sqlite);
        Database::getInstance()->release();
        
        if(!Utils::getInstance()->directoryExists(Directory::getInstance()->getElasticseachDirectory()))
        {
            isCurrentVersion = 0;
        }
        
        if(!isCurrentVersion)
        {
            HttpConnector *downloadHttpConnector = new HttpConnector(URL_DOWNLOAD);
            downloadHttpConnector->setDownloadProgressListener(this, httpConnectorProgressListener);
            if(downloadHttpConnector->get() == HttpConnector::HTTP_OK)
            {
                string targzFileName = Directory::getInstance()->getCacheDirectory() + TARGZ_FILE_NAME;
                unlink(targzFileName.c_str());
                
                if(!Utils::getInstance()->writeToFile(downloadHttpConnector->getResponseData(), downloadHttpConnector->getResponseDataSize(), targzFileName))
                {
                    /*string tarFileName = Settings::getInstance()->getCacheDirectory() + TAR_FILE_NAME;
                    unlink(tarFileName.c_str());
                    
                    int error = 0;
                    gzFile targzFile = gzopen(targzFileName.c_str(), "r");
                    if(targzFile)
                    {
                        postStatus(string("Uncompressing database"));
                        
                        ofstream tarStream(tarFileName.c_str(), ios::out | ios::binary);
                        if(tarStream)
                        {
                            const size_t BUFFER_LENGTH = 4095;
                            unsigned char *buffer = new unsigned char[BUFFER_LENGTH];
                            int bytesRead = 0;
                            while((bytesRead = gzread(targzFile, buffer, BUFFER_LENGTH)) > 0)
                            {
                                tarStream.write((const char *)buffer, bytesRead);
                            }
                            // tar.gz file successfully inflated
                            if(gzeof(targzFile))
                            {

                            }
                            // inflate failed
                            else
                            {                            
                                const char *errorMessage = gzerror(targzFile, &error);
                                if(error)
                                {
                                    postStatus(string(errorMessage));
                                    cerr << "ElasticsearchProcess::" << __FUNCTION__ << " gzerror: " << errorMessage << endl;
                                }
                            }
                            tarStream.close();
                        }
                        else
                        {
                            error = 1;
                        }                                                
                        gzclose(targzFile);
                    }
                    else
                    {
                        error = 1;
                    }
                    
                    if(!error)
                    {
                        postStatus(string("Extracting database"));
                        
                        TAR *tar;//O_RDONLY
                        if(!(error = tar_open(&tar, tarFileName.c_str(), NULL,  0,  0644,  TAR_GNU)))
                        {
                            Utils::getInstance()->removeDirectory(Settings::getInstance()->getElasticseachDirectory());
                            if(!(error = tar_extract_all(tar, (char *)Settings::getInstance()->getDataDirectory().c_str())))
                            {
                                sqlite3 *sqlite = Database::getInstance()->acquire();
                                apiDatabase->save(sqlite);
                                Database::getInstance()->release();
                            }
                            tar_close(tar);
                        }
                    }
                    
                    unlink(targzFileName.c_str());
                    unlink(tarFileName.c_str());*/
                    
                    postStatus(string("Extracting database"));
                    Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
                    
                    FileExtractor *fileExtractor = new FileExtractor(targzFileName);
                    if(!fileExtractor->extract(Directory::getInstance()->getDataDirectory()))
                    {
                        sqlite3 *sqlite = Database::getInstance()->acquire();
                        apiDatabase->save(sqlite);
                        Database::getInstance()->release();
                    }                    
                    delete fileExtractor;
                    
                    unlink(targzFileName.c_str());
                }
            }
            delete downloadHttpConnector;
        }
        delete apiDatabase;
    }
    delete md5HttpConnector;
    
    postStatus(string("Starting database"));
    TheGamesDB::Elasticsearch::getInstance()->start(this, callbackElasticsearchStartListener);                
    
    return status;
}

void ElasticsearchProcess::httpConnectorProgressListener(void* pElasticsearchProcess, HttpConnector* httpConnector, size_t bytesToDownload, size_t downloadedBytes)
{
    ElasticsearchProcess *elasticsearchProcess = (ElasticsearchProcess *)pElasticsearchProcess;
    int progress = (((double)downloadedBytes / (double)bytesToDownload) * 100.0);
    elasticsearchProcess->postStatus(string("Downloading database"), "", progress);
}


void ElasticsearchProcess::callbackElasticsearchStartListener(CallbackResult *callbackResult)
{
    ElasticsearchProcess *elasticsearchProcess = (ElasticsearchProcess *)callbackResult->getRequester();
    
    if(!callbackResult->getError())
    {   
        elasticsearchProcess->status = STATUS_SUCCESS;
    }
    else
    {
        elasticsearchProcess->status = STATUS_FAIL;
        Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
    }    
    
    callbackResult->destroy();    
    SerialProcessExecutor::getInstance()->finish(elasticsearchProcess);
}

