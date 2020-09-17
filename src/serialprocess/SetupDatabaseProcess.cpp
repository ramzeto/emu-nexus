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
 * File:   SetupDatabaseProcess.cpp
 * Author: ram
 * 
 * Created on September 6, 2020, 7:19 PM
 */

#include "SetupDatabaseProcess.h"
#include "Database.h"
#include "Directory.h"
#include "ApiDatabase.h"
#include "Utils.h"
#include "FileExtractor.h"
#include "SerialProcessExecutor.h"
#include "EsrbRating.h"
#include "Developer.h"
#include "Publisher.h"
#include "Genre.h"
#include "thegamesdb.h"
#include "NotificationManager.h"

#include <iostream>
#include <jansson.h>
#include <map>

const string SetupDatabaseProcess::TYPE = "ElasticsearchProcess";
//const string ElasticsearchProcess::URL_DOWNLOAD = "http://localhost/public/emu-nexus-db/downloads/thegamesdb.tar.gz";
//const string ElasticsearchProcess::URL_MD5 = "http://localhost/public/emu-nexus-db/downloads/thegamesdb.md5";
const string SetupDatabaseProcess::URL_DOWNLOAD = "https://www.dropbox.com/s/4hlnjvdqu1f2ndb/thegamesdb.tar.gz?dl=1";
const string SetupDatabaseProcess::URL_MD5 = "https://www.dropbox.com/s/lm6bug13yqqli7f/thegamesdb.md5?dl=1";
const string SetupDatabaseProcess::TARGZ_FILE_NAME = "elasticsearch.tar.gz";
const string SetupDatabaseProcess::TAR_FILE_NAME = "elasticsearch.tar";

SetupDatabaseProcess::SetupDatabaseProcess() : SerialProcess(TYPE)
{
}

SetupDatabaseProcess::~SetupDatabaseProcess()
{
}

void SetupDatabaseProcess::execute()
{
    status = STATUS_RUNNING;
    
    // @TODO Find a more elegant way to do this. Kills the elasticseach process in case it is running.
    system("pkill -9 elasticsearch");
    sleep(1);
    
    NotificationManager::getInstance()->notify(TYPE, "Downloading database", status);
    
    HttpConnector *md5HttpConnector = new HttpConnector(URL_MD5);
    if(md5HttpConnector->get() == HttpConnector::HTTP_OK)
    {
        string md5sum((const char *)md5HttpConnector->getResponseData(), md5HttpConnector->getResponseDataSize());
        
        ApiDatabase *apiDatabase = new ApiDatabase(md5sum);
        int isCurrentVersion = apiDatabase->load();
        
        if(!Utils::getInstance()->directoryExists(Directory::getInstance()->getElasticseachDirectory()))
        {
            isCurrentVersion = 0;
        }
        
        if(!isCurrentVersion)
        {
            HttpConnector *downloadHttpConnector = new HttpConnector(URL_DOWNLOAD);
            downloadHttpConnector->setDownloadProgressListener(this, [](void* setupDatabaseProcess, HttpConnector* httpConnector, size_t bytesToDownload, size_t downloadedBytes) -> void {
                int progress = (((double)downloadedBytes / (double)bytesToDownload) * 100.0);
                NotificationManager::getInstance()->notify(TYPE, "Downloading database", ((SetupDatabaseProcess *)setupDatabaseProcess)->status, 0, NULL, progress);
            });
            
            if(downloadHttpConnector->get() == HttpConnector::HTTP_OK)
            {
                string targzFileName = Directory::getInstance()->getCacheDirectory() + TARGZ_FILE_NAME;
                unlink(targzFileName.c_str());
                
                if(!Utils::getInstance()->writeToFile(downloadHttpConnector->getResponseData(), downloadHttpConnector->getResponseDataSize(), targzFileName))
                {
                    NotificationManager::getInstance()->notify(TYPE, "Extracting database", status);
                    
                    Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
                    
                    FileExtractor *fileExtractor = new FileExtractor(targzFileName);
                    if(!fileExtractor->extract(Directory::getInstance()->getDataDirectory()))
                    {
                        apiDatabase->save();
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
    
    NotificationManager::getInstance()->notify(TYPE, "Starting database", status);
    
    TheGamesDB::Elasticsearch::getInstance()->start([this](int status) -> void {
        if(status == TheGamesDB::Elasticsearch::STATUS_OK)
        {
            if(Database::getInstance()->isSetup())
            {
                update();
            }
            else
            {
                initialize();
            }            
        }
        else
        {
            this->status = STATUS_FAIL;
            Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
            NotificationManager::getInstance()->notify(TYPE, "Database setup failed", this->status);
        }
    });
}

void SetupDatabaseProcess::initialize()
{
    NotificationManager::getInstance()->notify(TYPE, "Preloading data", status);
    
    TheGamesDB::Elasticsearch::getInstance()->getEsrbRatings([this](list<TheGamesDB::EsrbRating *> *apiItems) -> void {
        list<EsrbRating *> *items = new list<EsrbRating *>;
        for(unsigned int index = 0; index < apiItems->size(); index++)
        {
            TheGamesDB::EsrbRating *apiItem = TheGamesDB::EsrbRating::getItem(apiItems, index);

            EsrbRating *item = new EsrbRating((int64_t)0);
            item->setName(apiItem->getName());
            item->setApiId(apiItem->getId());

            items->push_back(item);
        }

        sqlite3 *sqlite = Database::getInstance()->acquire();
        int error = EsrbRating::bulkInsert(sqlite, items);
        Database::getInstance()->release();
        EsrbRating::releaseItems(items);

        if(error)
        {
            this->status = STATUS_FAIL;
            Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
            NotificationManager::getInstance()->notify(TYPE, "Database setup failed", this->status);
            return;
        }

        TheGamesDB::Elasticsearch::getInstance()->getDevelopers([this](list<TheGamesDB::Developer *> *apiItems) -> void {
            list<Developer *> *items = new list<Developer *>;
            for(unsigned int index = 0; index < apiItems->size(); index++)
            {
                TheGamesDB::Developer *apiItem = TheGamesDB::Developer::getItem(apiItems, index);

                Developer *item = new Developer((int64_t)0);
                item->setName(apiItem->getName());
                item->setApiId(apiItem->getId());

                items->push_back(item);
            }

            sqlite3 *sqlite = Database::getInstance()->acquire();
            int error = Developer::bulkInsert(sqlite, items);
            Database::getInstance()->release();
            Developer::releaseItems(items);

            if(error)
            {
                this->status = STATUS_FAIL;
                Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
                NotificationManager::getInstance()->notify(TYPE, "Database setup failed", this->status);
                return;
            }

            TheGamesDB::Elasticsearch::getInstance()->getPublishers([this](list<TheGamesDB::Publisher *> *apiItems) -> void {
                list<Publisher *> *items = new list<Publisher *>;
                for(unsigned int index = 0; index < apiItems->size(); index++)
                {
                    TheGamesDB::Publisher *apiItem = TheGamesDB::Publisher::getItem(apiItems, index);

                    Publisher *item = new Publisher((int64_t)0);
                    item->setName(apiItem->getName());
                    item->setApiId(apiItem->getId());

                    items->push_back(item);
                }

                sqlite3 *sqlite = Database::getInstance()->acquire();
                int error = Publisher::bulkInsert(sqlite, items);
                Database::getInstance()->release();
                Publisher::releaseItems(items);

                if(error)
                {
                    this->status = STATUS_FAIL;
                    Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
                    NotificationManager::getInstance()->notify(TYPE, "Database setup failed", this->status);
                    return;
                }

                TheGamesDB::Elasticsearch::getInstance()->getGenres([this](list<TheGamesDB::Genre *> *apiItems) -> void {
                    list<Genre *> *items = new list<Genre *>;
                    for(unsigned int index = 0; index < apiItems->size(); index++)
                    {
                        TheGamesDB::Genre *apiItem = TheGamesDB::Genre::getItem(apiItems, index);

                        Genre *item = new Genre((int64_t)0);
                        item->setName(apiItem->getName());
                        item->setApiId(apiItem->getId());

                        items->push_back(item);
                    }

                    sqlite3 *sqlite = Database::getInstance()->acquire();
                    int error = Genre::bulkInsert(sqlite, items);
                    Database::getInstance()->release();
                    Genre::releaseItems(items);

                    if(error)
                    {
                        this->status = STATUS_FAIL;
                        Utils::getInstance()->removeDirectory(Directory::getInstance()->getElasticseachDirectory());
                        NotificationManager::getInstance()->notify(TYPE, "Database setup failed", this->status);
                    }
                    else
                    {
                        this->status = STATUS_SUCCESS;
                        NotificationManager::getInstance()->notify(TYPE, "Database setup successfully", this->status);
                    }
                });                                                        
            });
        });                    
    });
}

void SetupDatabaseProcess::update()
{
    NotificationManager::getInstance()->notify(TYPE, "Updating data", status);
    
    TheGamesDB::Elasticsearch::getInstance()->getEsrbRatings([this](list<TheGamesDB::EsrbRating *> *apiItems) -> void {
        list<EsrbRating *> *items = EsrbRating::getItems();
        map<int64_t, EsrbRating *> *itemsMap = new map<int64_t, EsrbRating *>();
        for(unsigned int index = 0; index < items->size(); index++)
        {
            EsrbRating *item = EsrbRating::getItem(items, index);
            itemsMap->insert(pair<int64_t, EsrbRating *>(item->getApiId(), item));
        }
        
        for(unsigned int index = 0; index < apiItems->size(); index++)
        {
            TheGamesDB::EsrbRating *apiItem = TheGamesDB::EsrbRating::getItem(apiItems, index);

            if(itemsMap->find(apiItem->getId()) != itemsMap->end())
            {
                EsrbRating *item = itemsMap->at(apiItem->getId());
                if(item->getName().compare(apiItem->getName()) != 0)
                {
                    item->setName(apiItem->getName());
                    item->save();
                }
            }
            else
            {
                EsrbRating *item = new EsrbRating((int64_t)0);
                item->setName(apiItem->getName());
                item->setApiId(apiItem->getId());
                item->save();
                
                delete item;
            }
        }        
        itemsMap->clear();
        delete itemsMap;
        EsrbRating::releaseItems(items);

        TheGamesDB::Elasticsearch::getInstance()->getDevelopers([this](list<TheGamesDB::Developer *> *apiItems) -> void {
            list<Developer *> *items = Developer::getItems();
            map<int64_t, Developer *> *itemsMap = new map<int64_t, Developer *>();
            for(unsigned int index = 0; index < items->size(); index++)
            {
                Developer *item = Developer::getItem(items, index);
                itemsMap->insert(pair<int64_t, Developer *>(item->getApiId(), item));
            }

            for(unsigned int index = 0; index < apiItems->size(); index++)
            {
                TheGamesDB::Developer *apiItem = TheGamesDB::Developer::getItem(apiItems, index);

                if(itemsMap->find(apiItem->getId()) != itemsMap->end())
                {
                    Developer *item = itemsMap->at(apiItem->getId());
                    if(item->getName().compare(apiItem->getName()) != 0)
                    {
                        item->setName(apiItem->getName());
                        item->save();
                    }
                }
                else
                {
                    Developer *item = new Developer((int64_t)0);
                    item->setName(apiItem->getName());
                    item->setApiId(apiItem->getId());
                    item->save();
                    
                    delete item;
                }
            }
            itemsMap->clear();
            delete itemsMap;
            Developer::releaseItems(items);

            TheGamesDB::Elasticsearch::getInstance()->getPublishers([this](list<TheGamesDB::Publisher *> *apiItems) -> void {
                list<Publisher *> *items = Publisher::getItems();
                map<int64_t, Publisher *> *itemsMap = new map<int64_t, Publisher *>();
                for(unsigned int index = 0; index < items->size(); index++)
                {
                    Publisher *item = Publisher::getItem(items, index);
                    itemsMap->insert(pair<int64_t, Publisher *>(item->getApiId(), item));
                }
                
                for(unsigned int index = 0; index < apiItems->size(); index++)
                {
                    TheGamesDB::Publisher *apiItem = TheGamesDB::Publisher::getItem(apiItems, index);

                    if(itemsMap->find(apiItem->getId()) != itemsMap->end())
                    {
                        Publisher *item = itemsMap->at(apiItem->getId());
                        if(item->getName().compare(apiItem->getName()) != 0)
                        {
                            item->setName(apiItem->getName());
                            item->save();
                        }
                    }
                    else
                    {
                        Publisher *item = new Publisher((int64_t)0);
                        item->setName(apiItem->getName());
                        item->setApiId(apiItem->getId());
                        item->save();

                        delete item;
                    }
                }
                itemsMap->clear();
                delete itemsMap;
                Publisher::releaseItems(items);               

                TheGamesDB::Elasticsearch::getInstance()->getGenres([this](list<TheGamesDB::Genre *> *apiItems) -> void {
                    list<Genre *> *items = Genre::getItems();
                    map<int64_t, Genre *> *itemsMap = new map<int64_t, Genre *>();
                    for(unsigned int index = 0; index < items->size(); index++)
                    {
                        Genre *item = Genre::getItem(items, index);
                        itemsMap->insert(pair<int64_t, Genre *>(item->getApiId(), item));
                    }

                    for(unsigned int index = 0; index < apiItems->size(); index++)
                    {
                        TheGamesDB::Genre *apiItem = TheGamesDB::Genre::getItem(apiItems, index);

                        if(itemsMap->find(apiItem->getId()) != itemsMap->end())
                        {
                            Genre *item = itemsMap->at(apiItem->getId());
                            if(item->getName().compare(apiItem->getName()) != 0)
                            {
                                item->setName(apiItem->getName());
                                item->save();
                            }
                        }
                        else
                        {
                            Genre *item = new Genre((int64_t)0);
                            item->setName(apiItem->getName());
                            item->setApiId(apiItem->getId());
                            item->save();

                            delete item;
                        }
                    }
                    itemsMap->clear();
                    delete itemsMap;
                    Genre::releaseItems(items);
                    

                    this->status = STATUS_SUCCESS;
                    NotificationManager::getInstance()->notify(TYPE, "Database setup successfully", this->status);
                });                                                        
            });
        });                    
    });
}
