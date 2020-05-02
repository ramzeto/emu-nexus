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
 * File:   thegamesdb.cpp
 * Author: ram
 *
 * Created on April 14, 2019, 2:35 PM
 */

#include "thegamesdb.h"
#include "Utils.h"
#include "HttpConnector.h"
#include "ApiDatabase.h"
#include "Database.h"
#include "Directory.h"
#include "Preferences.h"

#include <unistd.h>
#include <jansson.h>
#include <iostream>


TheGamesDB::Elasticsearch *TheGamesDB::Elasticsearch::instance = NULL;

const string TheGamesDB::Elasticsearch::URL = "http://localhost";
const int TheGamesDB::Elasticsearch::WAIT_TIME_ELASTICSEARCH = 10;
const int TheGamesDB::Elasticsearch::REQUEST_MAX_RETRIES = 5;
const int TheGamesDB::Elasticsearch::REQUEST_TIME_TO_RETRY = 1;
const int TheGamesDB::Elasticsearch::STATUS_OK = 0;
const int TheGamesDB::Elasticsearch::STATUS_STARTING = 1;
const int TheGamesDB::Elasticsearch::STATUS_UPDATING = 2;
const int TheGamesDB::Elasticsearch::STATUS_STOPPED = 3;

const string TheGamesDB::Elasticsearch::RESULT_TYPE_START = "start";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_GENRES = "genres";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_DEVELOPERS = "developers";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_PUBLISHERS = "publishers";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_ESRB_RATINGS = "esrb_ratings";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_PLATFORMS = "platforms";
const string TheGamesDB::Elasticsearch::RESULT_TYPE_GAMES = "games";

TheGamesDB::Elasticsearch::Elasticsearch()
{
    status = STATUS_STOPPED;
}

TheGamesDB::Elasticsearch::~Elasticsearch()
{

}

void TheGamesDB::Elasticsearch::start(void *requester, void (*callback)(CallbackResult *))
{
    status = STATUS_STARTING;
    
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    if(pthread_create(&processThread, NULL, processWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
    
    pthread_t processStartListenerThread;
    if(pthread_create(&processStartListenerThread, NULL, processStartListenerWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getGenres(void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getGenresWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getDevelopers(void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getDevelopersWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getPublishers(void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getPublishersWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getEsrbRatings(void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getEsrbRatingsWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getPlatforms(void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getPlatformsWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

void TheGamesDB::Elasticsearch::getGames(int64_t apiPlatformId, string query, void* requester, void (*callback)(CallbackResult *))
{
    RequesterRef_t *requesterRef = new RequesterRef_t;
    requesterRef->requester = requester;
    requesterRef->callback = callback;
    requesterRef->apiPlatformId = apiPlatformId;
    requesterRef->query = query;
    
    pthread_t thread;
    if(pthread_create(&thread, NULL, getGamesWorker, requesterRef) != 0) 
    {
        cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << endl;
        exit(EXIT_FAILURE);
    }
}

int TheGamesDB::Elasticsearch::getStatus()
{
    return status;
}


TheGamesDB::Elasticsearch* TheGamesDB::Elasticsearch::getInstance()
{
    if(!instance)
    {
        instance = new TheGamesDB::Elasticsearch();
    }
    
    return instance;
}

void *TheGamesDB::Elasticsearch::processWorker(void *requesterRef)
{
    string command = Preferences::getInstance()->getElasticseachBinary() + " -E http.port=" + to_string(Preferences::getInstance()->getElasticsearchPort());    
    cout << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " command: " << command << endl;
    system(command.c_str());
    
    return NULL;
}

void *TheGamesDB::Elasticsearch::processStartListenerWorker(void *requesterRef)
{
    string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort());    
    cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " url: " << url << endl;

    json_t *jsonResponse = NULL;
    int maxTries = 10;
    int error = 1;
    int tries = 0;
    while(error)
    {
        sleep(WAIT_TIME_ELASTICSEARCH); 
                
        HttpConnector *httpConnector = new HttpConnector(url);
        int httpStatus = httpConnector->get();

        if(httpStatus == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            if(!jsonResponse)
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            else
            {            
                char *jsonDump = json_dumps(jsonResponse, 0);
                cout << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonResponse: " << jsonDump << endl;
                free(jsonDump);

                error = 0;
            }                   
        }
        delete httpConnector;
        
        if(error)
        {
            tries++;
            
            if(tries > maxTries)
            {
                break;
            }
        }
    }

    
    if(!error)
    {
        Elasticsearch::instance->status = STATUS_OK;
    }
    else
    {        
        Elasticsearch::instance->status = STATUS_STOPPED;
    }
    
    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_START);
    callbackResult->setError(error);
    callbackResult->setData(jsonResponse);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);
    
    delete ((RequesterRef_t *)requesterRef);

    return NULL;
}

void *TheGamesDB::Elasticsearch::getGenresWorker(void *requesterRef)
{
    int tries = 0;
    int from = 0;
    int size = 1000;
    int hits = 0;
    int error = 0;
    list<TheGamesDB::Genre *> *genres = new list<TheGamesDB::Genre *>;
    
    do
    {
        hits = 0;
        string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/genre/_search?q=*&from=" + to_string(from) + "&size=" + to_string(size);       
        
        HttpConnector *httpConnector = new HttpConnector(url);        
        if(httpConnector->get() == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            
            if(jsonResponse)
            {
                json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
                json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
                
                hits = json_array_size(jsonHits);
                for(int index = 0; index < hits; index++)
                {
                    json_t *jsonHit = json_array_get(jsonHits, index);
                    
                    genres->push_back(new TheGamesDB::Genre(json_object_get(jsonHit, "_source")));
                }
                
                json_decref(jsonResponse);
            }
            else
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            
            from += size;
        }
        else if(tries <= REQUEST_MAX_RETRIES)
        {
            sleep(REQUEST_TIME_TO_RETRY);
            tries++;
            hits = 1;
        }
        else
        {
            error = 1;
        }
        
        delete httpConnector;                
    }while(hits > 0);

    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_GENRES);
    callbackResult->setError(error);
    callbackResult->setData(genres);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);
        
    delete ((RequesterRef_t *)requesterRef);
    return NULL;
}

void *TheGamesDB::Elasticsearch::getDevelopersWorker(void *requesterRef)
{
    int tries = 0;
    int from = 0;
    int size = 1000;
    int hits = 0;
    int error = 0;
    list<TheGamesDB::Developer *> *developers = new list<TheGamesDB::Developer *>;
    
    do
    {
        hits = 0;
        string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/developer/_search?q=*&from=" + to_string(from) + "&size=" + to_string(size);
                
        HttpConnector *httpConnector = new HttpConnector(url);        
        if(httpConnector->get() == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            
            if(jsonResponse)
            {
                json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
                json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
                
                hits = json_array_size(jsonHits);
                for(int index = 0; index < hits; index++)
                {
                    json_t *jsonHit = json_array_get(jsonHits, index);
                    
                    developers->push_back(new TheGamesDB::Developer(json_object_get(jsonHit, "_source")));
                }
                
                json_decref(jsonResponse);
            }
            else
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            
            from += size;
        }
        else if(tries <= REQUEST_MAX_RETRIES)
        {
            sleep(REQUEST_TIME_TO_RETRY);
            tries++;
            hits = 1;
        }
        else
        {
            error = 1;
        }
        
        delete httpConnector;                
    }while(hits > 0);

    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_DEVELOPERS);
    callbackResult->setError(error);
    callbackResult->setData(developers);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);    
    
    delete ((RequesterRef_t *)requesterRef);
    return NULL;    
}

void *TheGamesDB::Elasticsearch::getPublishersWorker(void *requesterRef)
{
    int tries = 0;
    int from = 0;
    int size = 1000;
    int hits = 0;
    int error = 0;
    list<TheGamesDB::Publisher *> *publishers = new list<TheGamesDB::Publisher *>;
    
    do
    {
        hits = 0;
        string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/publisher/_search?q=*&from=" + to_string(from) + "&size=" + to_string(size);
                
        HttpConnector *httpConnector = new HttpConnector(url);        
        if(httpConnector->get() == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            
            if(jsonResponse)
            {                
                json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
                json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
                
                hits = json_array_size(jsonHits);
                for(int index = 0; index < hits; index++)
                {
                    json_t *jsonHit = json_array_get(jsonHits, index);
                    
                    publishers->push_back(new TheGamesDB::Publisher(json_object_get(jsonHit, "_source")));
                }
                
                json_decref(jsonResponse);
            }
            else
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            
            from += size;
        }
        else if(tries <= REQUEST_MAX_RETRIES)
        {
            sleep(REQUEST_TIME_TO_RETRY);
            tries++;
            hits = 1;
        }
        else
        {
            error = 1;
        }
        
        delete httpConnector;                
    }while(hits > 0);

    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_PUBLISHERS);
    callbackResult->setError(error);
    callbackResult->setData(publishers);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);   
        
    delete ((RequesterRef_t *)requesterRef);
    return NULL;
}

void *TheGamesDB::Elasticsearch::getEsrbRatingsWorker(void *requesterRef)
{
    int tries = 0;
    int from = 0;
    int size = 1000;
    int hits = 0;
    int error = 0;
    list<TheGamesDB::EsrbRating *> *esrbRatings = new list<TheGamesDB::EsrbRating *>;
    
    do
    {
        hits = 0;
        string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/esrbrating/_search?q=*&from=" + to_string(from) + "&size=" + to_string(size);
                
        HttpConnector *httpConnector = new HttpConnector(url);        
        if(httpConnector->get() == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            
            if(jsonResponse)
            {
                json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
                json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
                
                hits = json_array_size(jsonHits);
                for(int index = 0; index < hits; index++)
                {
                    json_t *jsonHit = json_array_get(jsonHits, index);
                    
                    esrbRatings->push_back(new TheGamesDB::EsrbRating(json_object_get(jsonHit, "_source")));
                }
                
                json_decref(jsonResponse);
            }
            else
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            
            from += size;
        }
        else if(tries <= REQUEST_MAX_RETRIES)
        {
            sleep(REQUEST_TIME_TO_RETRY);
            tries++;
            hits = 1;
        }
        else
        {
            error = 1;
        }
        
        delete httpConnector;                
    }while(hits > 0);

    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_ESRB_RATINGS);
    callbackResult->setError(error);
    callbackResult->setData(esrbRatings);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);   
    
    delete ((RequesterRef_t *)requesterRef);
    return NULL;    
}

int platformsSort(TheGamesDB::Platform *platform1, TheGamesDB::Platform *platform2)
{
    return Utils::getInstance()->strToLowerCase(platform1->getName()).compare(Utils::getInstance()->strToLowerCase(platform2->getName())) < 0;
}

void *TheGamesDB::Elasticsearch::getPlatformsWorker(void *requesterRef)
{
    int tries = 0;
    int from = 0;
    int size = 1000;
    int hits = 0;
    int error = 0;
    list<TheGamesDB::Platform *> *platforms = new list<TheGamesDB::Platform *>;
    
    do
    {
        hits = 0;
        string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/platform/_search?q=*&from=" + to_string(from) + "&size=" + to_string(size);
                
        HttpConnector *httpConnector = new HttpConnector(url);        
        if(httpConnector->get() == HttpConnector::HTTP_OK)
        {
            json_error_t jsonError;
            json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
            if(jsonResponse)
            {
                json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
                json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
                
                hits = json_array_size(jsonHits);
                for(int index = 0; index < hits; index++)
                {
                    json_t *jsonHit = json_array_get(jsonHits, index);                                        
                    platforms->push_back(new TheGamesDB::Platform(json_object_get(jsonHit, "_source")));                    
                }
                
                json_decref(jsonResponse);
            }
            else
            {
                cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
            }
            
            from += size;
        }
        else if(tries <= REQUEST_MAX_RETRIES)
        {
            sleep(REQUEST_TIME_TO_RETRY);
            tries++;
            hits = 1;
        }
        else
        {
            error = 1;
        }
        
        delete httpConnector;                
    }while(hits > 0);

    platforms->sort(platformsSort);
    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_PLATFORMS);
    callbackResult->setError(error);
    callbackResult->setData(platforms);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);   
    
    delete ((RequesterRef_t *)requesterRef);
    return NULL;    
}


void *TheGamesDB::Elasticsearch::getGamesWorker(void *requesterRef)
{
    int64_t apiPlatformId = ((RequesterRef_t *)requesterRef)->apiPlatformId;
    string query = ((RequesterRef_t *)requesterRef)->query;
    query = Utils::getInstance()->strReplace(query, "!", " ");
    query = Utils::getInstance()->strReplace(query, "?", " ");
    query = Utils::getInstance()->strReplace(query, "/", " ");
    query = Utils::getInstance()->strReplace(query, ":", " ");

    
    int error = 0;
    list<TheGamesDB::Game *> *games = new list<TheGamesDB::Game *>;
    string url = URL + ":" + to_string(Preferences::getInstance()->getElasticsearchPort()) + "/platform" + to_string(apiPlatformId) + "/_search?q=name:" + Utils::getInstance()->urlEncode(query) + "&from=0&size=20";

    HttpConnector *httpConnector = new HttpConnector(url);        
    if(httpConnector->get() == HttpConnector::HTTP_OK)
    {
        json_error_t jsonError;
        json_t *jsonResponse = json_loadb((char *)httpConnector->getResponseData(), httpConnector->getResponseDataSize(), 0, &jsonError);
        if(jsonResponse)
        {
            json_t *jsonHitsSection = json_object_get(jsonResponse, "hits");
            json_t *jsonHits = json_object_get(jsonHitsSection, "hits");
            for(unsigned int index = 0; index < json_array_size(jsonHits); index++)
            {
                json_t *jsonHit = json_array_get(jsonHits, index);
                games->push_back(new TheGamesDB::Game(json_object_get(jsonHit, "_source")));                    
            }

            json_decref(jsonResponse);
        }
        else
        {
            cerr << "TheGamesDB::Elasticsearch::" << __FUNCTION__ << " jsonError: " << jsonError.text << endl;
        }
    }
    else
    {
        error = 1;
    }

    delete httpConnector;

    CallbackResult *callbackResult = new CallbackResult(((RequesterRef_t *)requesterRef)->requester);
    callbackResult->setType(RESULT_TYPE_GAMES);
    callbackResult->setError(error);
    callbackResult->setData(games);
    callbackResult->setDestroyCallback(callbackResultDestroy);
    ((RequesterRef_t *)requesterRef)->callback(callbackResult);   
    
    delete ((RequesterRef_t *)requesterRef);
    return NULL; 
}

void TheGamesDB::Elasticsearch::callbackResultDestroy(CallbackResult *callbackResult)
{
    if(callbackResult->getType().compare(RESULT_TYPE_START) == 0)
    {
        json_decref((json_t *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_GENRES) == 0)
    {
        TheGamesDB::Genre::releaseItems((list<TheGamesDB::Genre *> *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_DEVELOPERS) == 0)
    {
        TheGamesDB::Developer::releaseItems((list<TheGamesDB::Developer *> *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_PUBLISHERS) == 0)
    {
        TheGamesDB::Publisher::releaseItems((list<TheGamesDB::Publisher *> *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_ESRB_RATINGS) == 0)
    {
        TheGamesDB::EsrbRating::releaseItems((list<TheGamesDB::EsrbRating *> *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_PLATFORMS) == 0)
    {
        TheGamesDB::Platform::releaseItems((list<TheGamesDB::Platform *> *)callbackResult->getData());
    }
    else if(callbackResult->getType().compare(RESULT_TYPE_GAMES) == 0)
    {
        TheGamesDB::Game::releaseItems((list<TheGamesDB::Game *> *)callbackResult->getData());
    }
}






const string TheGamesDB::PlatformImage::TYPE_BANNER = "banner";
const string TheGamesDB::PlatformImage::TYPE_FANART = "fanart";
const string TheGamesDB::PlatformImage::TYPE_BOXART = "boxart";
const string TheGamesDB::PlatformImage::TYPE_ICON = "icon";

TheGamesDB::PlatformImage::PlatformImage()
{
}

TheGamesDB::PlatformImage::PlatformImage(int64_t id)
{
	this->id = id;
}

TheGamesDB::PlatformImage::PlatformImage(const TheGamesDB::PlatformImage &orig)
{
	this->id = orig.id;
	this->platformId = orig.platformId;
	this->type = orig.type;
	this->original = orig.original;
	this->small = orig.small;
	this->thumb = orig.thumb;
	this->croppedCenterThumb = orig.croppedCenterThumb;
	this->medium = orig.medium;
	this->large = orig.large;
}

TheGamesDB::PlatformImage::PlatformImage(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *platformIdJson = json_object_get(json, "platformId");
	if(platformIdJson)
	{
		platformId = (int64_t)json_integer_value(platformIdJson);
	}

	json_t *typeJson = json_object_get(json, "type");
	if(typeJson)
	{
		type = string(json_string_value(typeJson));
	}

	json_t *originalJson = json_object_get(json, "original");
	if(originalJson)
	{
		original = string(json_string_value(originalJson));
	}

	json_t *smallJson = json_object_get(json, "small");
	if(smallJson)
	{
		small = string(json_string_value(smallJson));
	}

	json_t *thumbJson = json_object_get(json, "thumb");
	if(thumbJson)
	{
		thumb = string(json_string_value(thumbJson));
	}

	json_t *croppedCenterThumbJson = json_object_get(json, "croppedCenterThumb");
	if(croppedCenterThumbJson)
	{
		croppedCenterThumb = string(json_string_value(croppedCenterThumbJson));
	}

	json_t *mediumJson = json_object_get(json, "medium");
	if(mediumJson)
	{
		medium = string(json_string_value(mediumJson));
	}

	json_t *largeJson = json_object_get(json, "large");
	if(largeJson)
	{
		large = string(json_string_value(largeJson));
	}

}

TheGamesDB::PlatformImage::~PlatformImage()
{
}

int64_t TheGamesDB::PlatformImage::getId()
{
	return id;
}

int64_t TheGamesDB::PlatformImage::getPlatformId()
{
	return platformId;
}

string TheGamesDB::PlatformImage::getType()
{
	return type;
}

string TheGamesDB::PlatformImage::getOriginal()
{
	return original;
}

string TheGamesDB::PlatformImage::getSmall()
{
	return small;
}

string TheGamesDB::PlatformImage::getThumb()
{
	return thumb;
}

string TheGamesDB::PlatformImage::getCroppedCenterThumb()
{
	return croppedCenterThumb;
}

string TheGamesDB::PlatformImage::getMedium()
{
	return medium;
}

string TheGamesDB::PlatformImage::getLarge()
{
	return large;
}

string TheGamesDB::PlatformImage::getFileName()
{
    return Directory::getInstance()->getCacheDirectory() + "apiPlatformImage_" + to_string(TheGamesDB::API_ID) + "_" + to_string(id);
}

TheGamesDB::PlatformImage *TheGamesDB::PlatformImage::getItem(list<TheGamesDB::PlatformImage *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::PlatformImage *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::PlatformImage::releaseItems(list<TheGamesDB::PlatformImage *> *items)
{
	for(list<TheGamesDB::PlatformImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}















TheGamesDB::Platform::Platform()
{
}

TheGamesDB::Platform::Platform(int64_t id)
{
	this->id = id;
        this->platformImages = new list<TheGamesDB::PlatformImage*>;
}

TheGamesDB::Platform::Platform(const TheGamesDB::Platform &orig)
{
	this->id = orig.id;
	this->name = orig.name;
	this->alias = orig.alias;
        this->platformImages = new list<TheGamesDB::PlatformImage *>;
        
        for(unsigned int index = 0; index < orig.platformImages->size(); index++)
        {
            TheGamesDB::PlatformImage *platformImage = new TheGamesDB::PlatformImage(*PlatformImage::getItem(orig.platformImages, index));
            this->platformImages->push_back(platformImage);
        }
}

TheGamesDB::Platform::Platform(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *aliasJson = json_object_get(json, "alias");
	if(aliasJson)
	{
		alias = string(json_string_value(aliasJson) ? json_string_value(aliasJson) : "");
	}
        
        platformImages = new list<TheGamesDB::PlatformImage *>;
        json_t *jsonPlatformImages = json_object_get(json, "platformImages");
        for(unsigned int index = 0; index < json_array_size(jsonPlatformImages); index++)
        {
            json_t *jsonPlatformImage = json_array_get(jsonPlatformImages, index);
            
            TheGamesDB::PlatformImage *platformImage = new TheGamesDB::PlatformImage(jsonPlatformImage);
            platformImages->push_back(platformImage);
        }
}

TheGamesDB::Platform::~Platform()
{
    TheGamesDB::PlatformImage::releaseItems(platformImages);
}

int64_t TheGamesDB::Platform::getId()
{
	return id;
}

string TheGamesDB::Platform::getName()
{
	return name;
}

string TheGamesDB::Platform::getAlias()
{
	return alias;
}

list<TheGamesDB::PlatformImage*> *TheGamesDB::Platform::getPlatformImages()
{
    return platformImages;
}

TheGamesDB::Platform *TheGamesDB::Platform::getItem(list<TheGamesDB::Platform *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::Platform *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::Platform::releaseItems(list<TheGamesDB::Platform *> *items)
{
	for(list<TheGamesDB::Platform *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}












TheGamesDB::Genre::Genre()
{
}

TheGamesDB::Genre::Genre(int64_t id)
{
	this->id = id;
}

TheGamesDB::Genre::Genre(const TheGamesDB::Genre &orig)
{
	this->id = orig.id;
	this->name = orig.name;
}

TheGamesDB::Genre::Genre(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

}

TheGamesDB::Genre::~Genre()
{
}

int64_t TheGamesDB::Genre::getId()
{
	return id;
}

string TheGamesDB::Genre::getName()
{
	return name;
}

TheGamesDB::Genre *TheGamesDB::Genre::getItem(list<TheGamesDB::Genre *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::Genre *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::Genre::releaseItems(list<TheGamesDB::Genre *> *items)
{
	for(list<TheGamesDB::Genre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}









TheGamesDB::Developer::Developer()
{
}

TheGamesDB::Developer::Developer(int64_t id)
{
	this->id = id;
}

TheGamesDB::Developer::Developer(const TheGamesDB::Developer &orig)
{
	this->id = orig.id;
	this->name = orig.name;
}

TheGamesDB::Developer::Developer(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

}

TheGamesDB::Developer::~Developer()
{
}

int64_t TheGamesDB::Developer::getId()
{
	return id;
}

string TheGamesDB::Developer::getName()
{
	return name;
}

TheGamesDB::Developer *TheGamesDB::Developer::getItem(list<TheGamesDB::Developer *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::Developer *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::Developer::releaseItems(list<TheGamesDB::Developer *> *items)
{
	for(list<TheGamesDB::Developer *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}









TheGamesDB::Publisher::Publisher()
{
}

TheGamesDB::Publisher::Publisher(int64_t id)
{
	this->id = id;
}

TheGamesDB::Publisher::Publisher(const TheGamesDB::Publisher &orig)
{
	this->id = orig.id;
	this->name = orig.name;
}

TheGamesDB::Publisher::Publisher(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

}

TheGamesDB::Publisher::~Publisher()
{
}

int64_t TheGamesDB::Publisher::getId()
{
	return id;
}

string TheGamesDB::Publisher::getName()
{
	return name;
}

TheGamesDB::Publisher *TheGamesDB::Publisher::getItem(list<TheGamesDB::Publisher *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::Publisher *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::Publisher::releaseItems(list<TheGamesDB::Publisher *> *items)
{
	for(list<TheGamesDB::Publisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}










TheGamesDB::EsrbRating::EsrbRating()
{
}

TheGamesDB::EsrbRating::EsrbRating(int64_t id)
{
	this->id = id;
}

TheGamesDB::EsrbRating::EsrbRating(const TheGamesDB::EsrbRating &orig)
{
	this->id = orig.id;
	this->name = orig.name;
}

TheGamesDB::EsrbRating::EsrbRating(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

}

TheGamesDB::EsrbRating::~EsrbRating()
{
}

int64_t TheGamesDB::EsrbRating::getId()
{
	return id;
}

string TheGamesDB::EsrbRating::getName()
{
	return name;
}

TheGamesDB::EsrbRating *TheGamesDB::EsrbRating::getItem(list<TheGamesDB::EsrbRating *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::EsrbRating *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::EsrbRating::releaseItems(list<TheGamesDB::EsrbRating *> *items)
{
	for(list<TheGamesDB::EsrbRating *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}






const string TheGamesDB::GameImage::TYPE_BOXART = "boxart";
const string TheGamesDB::GameImage::SIDE_FRONT = "front";
const string TheGamesDB::GameImage::SIDE_BACK = "back";

TheGamesDB::GameImage::GameImage()
{
}

TheGamesDB::GameImage::GameImage(int64_t id)
{
	this->id = id;
}

TheGamesDB::GameImage::GameImage(const TheGamesDB::GameImage &orig)
{
	this->id = orig.id;
	this->gameId = orig.gameId;
	this->type = orig.type;
	this->side = orig.side;
	this->original = orig.original;
	this->small = orig.small;
	this->thumb = orig.thumb;
	this->croppedCenterThumb = orig.croppedCenterThumb;
	this->medium = orig.medium;
	this->large = orig.large;
}

TheGamesDB::GameImage::GameImage(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *typeJson = json_object_get(json, "type");
	if(typeJson)
	{
		type = string(json_string_value(typeJson));
	}

	json_t *sideJson = json_object_get(json, "side");
	if(sideJson)
	{
		side = string(json_string_value(sideJson));
	}

	json_t *originalJson = json_object_get(json, "original");
	if(originalJson)
	{
		original = string(json_string_value(originalJson));
	}

	json_t *smallJson = json_object_get(json, "small");
	if(smallJson)
	{
		small = string(json_string_value(smallJson));
	}

	json_t *thumbJson = json_object_get(json, "thumb");
	if(thumbJson)
	{
		thumb = string(json_string_value(thumbJson));
	}

	json_t *croppedCenterThumbJson = json_object_get(json, "croppedCenterThumb");
	if(croppedCenterThumbJson)
	{
		croppedCenterThumb = string(json_string_value(croppedCenterThumbJson));
	}

	json_t *mediumJson = json_object_get(json, "medium");
	if(mediumJson)
	{
		medium = string(json_string_value(mediumJson));
	}

	json_t *largeJson = json_object_get(json, "large");
	if(largeJson)
	{
		large = string(json_string_value(largeJson));
	}

}

TheGamesDB::GameImage::~GameImage()
{
}

int64_t TheGamesDB::GameImage::getId()
{
	return id;
}

int64_t TheGamesDB::GameImage::getGameId()
{
	return gameId;
}

string TheGamesDB::GameImage::getType()
{
	return type;
}

string TheGamesDB::GameImage::getSide()
{
	return side;
}

string TheGamesDB::GameImage::getOriginal()
{
	return original;
}

string TheGamesDB::GameImage::getSmall()
{
	return small;
}

string TheGamesDB::GameImage::getThumb()
{
	return thumb;
}

string TheGamesDB::GameImage::getCroppedCenterThumb()
{
	return croppedCenterThumb;
}

string TheGamesDB::GameImage::getMedium()
{
	return medium;
}

string TheGamesDB::GameImage::getLarge()
{
	return large;
}

string TheGamesDB::GameImage::getFileName()
{
    return Directory::getInstance()->getCacheDirectory() + "apiGameImage_" + to_string(TheGamesDB::API_ID) + "_" + to_string(id);
}

TheGamesDB::GameImage *TheGamesDB::GameImage::getItem(list<TheGamesDB::GameImage *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::GameImage *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::GameImage::releaseItems(list<TheGamesDB::GameImage *> *items)
{
	for(list<TheGamesDB::GameImage *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}










TheGamesDB::GameGenre::GameGenre()
{
}

TheGamesDB::GameGenre::GameGenre(int64_t gameId, int64_t genreId)
{
	this->gameId = gameId;
	this->genreId = genreId;
}

TheGamesDB::GameGenre::GameGenre(const TheGamesDB::GameGenre &orig)
{
	this->gameId = orig.gameId;
	this->genreId = orig.genreId;
}

TheGamesDB::GameGenre::GameGenre(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *genreIdJson = json_object_get(json, "genreId");
	if(genreIdJson)
	{
		genreId = (int64_t)json_integer_value(genreIdJson);
	}

}

TheGamesDB::GameGenre::~GameGenre()
{
}

int64_t TheGamesDB::GameGenre::getGameId()
{
	return gameId;
}

int64_t TheGamesDB::GameGenre::getGenreId()
{
	return genreId;
}

TheGamesDB::GameGenre *TheGamesDB::GameGenre::getItem(list<TheGamesDB::GameGenre *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::GameGenre *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::GameGenre::releaseItems(list<TheGamesDB::GameGenre *> *items)
{
	for(list<TheGamesDB::GameGenre *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}












TheGamesDB::GameDeveloper::GameDeveloper()
{
}

TheGamesDB::GameDeveloper::GameDeveloper(int64_t gameId, int64_t developerId)
{
	this->gameId = gameId;
	this->developerId = developerId;
}

TheGamesDB::GameDeveloper::GameDeveloper(const TheGamesDB::GameDeveloper &orig)
{
	this->gameId = orig.gameId;
	this->developerId = orig.developerId;
}

TheGamesDB::GameDeveloper::GameDeveloper(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *developerIdJson = json_object_get(json, "developerId");
	if(developerIdJson)
	{
		developerId = (int64_t)json_integer_value(developerIdJson);
	}

}

TheGamesDB::GameDeveloper::~GameDeveloper()
{
}

int64_t TheGamesDB::GameDeveloper::getGameId()
{
	return gameId;
}

int64_t TheGamesDB::GameDeveloper::getDeveloperId()
{
	return developerId;
}

TheGamesDB::GameDeveloper *TheGamesDB::GameDeveloper::getItem(list<TheGamesDB::GameDeveloper *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::GameDeveloper *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::GameDeveloper::releaseItems(list<TheGamesDB::GameDeveloper *> *items)
{
	for(list<TheGamesDB::GameDeveloper *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}










TheGamesDB::GamePublisher::GamePublisher()
{
}

TheGamesDB::GamePublisher::GamePublisher(int64_t gameId, int64_t publisherId)
{
	this->gameId = gameId;
	this->publisherId = publisherId;
}

TheGamesDB::GamePublisher::GamePublisher(const TheGamesDB::GamePublisher &orig)
{
	this->gameId = orig.gameId;
	this->publisherId = orig.publisherId;
}

TheGamesDB::GamePublisher::GamePublisher(json_t *json)
{
	json_t *gameIdJson = json_object_get(json, "gameId");
	if(gameIdJson)
	{
		gameId = (int64_t)json_integer_value(gameIdJson);
	}

	json_t *publisherIdJson = json_object_get(json, "publisherId");
	if(publisherIdJson)
	{
		publisherId = (int64_t)json_integer_value(publisherIdJson);
	}

}

TheGamesDB::GamePublisher::~GamePublisher()
{
}

int64_t TheGamesDB::GamePublisher::getGameId()
{
	return gameId;
}

int64_t TheGamesDB::GamePublisher::getPublisherId()
{
	return publisherId;
}

TheGamesDB::GamePublisher *TheGamesDB::GamePublisher::getItem(list<TheGamesDB::GamePublisher *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::GamePublisher *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::GamePublisher::releaseItems(list<TheGamesDB::GamePublisher *> *items)
{
	for(list<TheGamesDB::GamePublisher *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}













TheGamesDB::Game::Game()
{
    gameImages = new list<GameImage *>;
    gameGenres = new list<GameGenre *>;
    gameDevelopers = new list<GameDeveloper *>;
    gamePublishers = new list<GamePublisher *>;
}

TheGamesDB::Game::Game(int64_t id)
{
	this->id = id;
        
    gameImages = new list<GameImage *>;
    gameGenres = new list<GameGenre *>;
    gameDevelopers = new list<GameDeveloper *>;
    gamePublishers = new list<GamePublisher *>;        
}

TheGamesDB::Game::Game(const TheGamesDB::Game &orig)
{
	this->id = orig.id;
	this->platformId = orig.platformId;
	this->name = orig.name;
	this->releaseDate = orig.releaseDate;
	this->description = orig.description;
	this->esrbRatingId = orig.esrbRatingId;
        
    gameImages = new list<TheGamesDB::GameImage *>;
    for(unsigned int index = 0; index < orig.gameImages->size(); index++)
    {
        TheGamesDB::GameImage *gameImage = new TheGamesDB::GameImage(*GameImage::getItem(orig.gameImages, index));
        this->gameImages->push_back(gameImage);
    }
    
    gameGenres = new list<TheGamesDB::GameGenre *>;
    for(unsigned int index = 0; index < orig.gameGenres->size(); index++)
    {
        TheGamesDB::GameGenre *gameGenre = new TheGamesDB::GameGenre(*GameGenre::getItem(orig.gameGenres, index));
        this->gameGenres->push_back(gameGenre);
    }
    
    gameDevelopers = new list<TheGamesDB::GameDeveloper *>;
    for(unsigned int index = 0; index < orig.gameDevelopers->size(); index++)
    {
        TheGamesDB::GameDeveloper *gameDeveloper = new TheGamesDB::GameDeveloper(*GameDeveloper::getItem(orig.gameDevelopers, index));
        this->gameDevelopers->push_back(gameDeveloper);
    }
            
    gamePublishers = new list<TheGamesDB::GamePublisher *>;
    for(unsigned int index = 0; index < orig.gamePublishers->size(); index++)
    {
        TheGamesDB::GamePublisher *gamePublisher = new TheGamesDB::GamePublisher(*GamePublisher::getItem(orig.gamePublishers, index));
        this->gamePublishers->push_back(gamePublisher);
    }    
}

TheGamesDB::Game::Game(json_t *json)
{
	json_t *idJson = json_object_get(json, "id");
	if(idJson)
	{
		id = (int64_t)json_integer_value(idJson);
	}

	json_t *platformIdJson = json_object_get(json, "platformId");
	if(platformIdJson)
	{
		platformId = (int64_t)json_integer_value(platformIdJson);
	}

	json_t *nameJson = json_object_get(json, "name");
	if(nameJson)
	{
		name = string(json_string_value(nameJson));
	}

	json_t *releaseDateJson = json_object_get(json, "releaseDate");
	if(releaseDateJson)
	{
		releaseDate = string(json_string_value(releaseDateJson) ? json_string_value(releaseDateJson) : "");
	}

	json_t *descriptionJson = json_object_get(json, "description");
	if(descriptionJson)
	{
            description = string(json_string_value(descriptionJson) ? json_string_value(descriptionJson) : "");
	}

	json_t *esrbRatingIdJson = json_object_get(json, "esrbRatingId");
	if(esrbRatingIdJson)
	{
		esrbRatingId = (int64_t)json_integer_value(esrbRatingIdJson);
	}
        
        gameImages = new list<TheGamesDB::GameImage *>;
        json_t *jsonGameImages = json_object_get(json, "gameImages");
        for(unsigned int index = 0; index < json_array_size(jsonGameImages); index++)
        {
            json_t *jsonGameImage = json_array_get(jsonGameImages, index);
            
            TheGamesDB::GameImage *gameImage = new TheGamesDB::GameImage(jsonGameImage);
            gameImages->push_back(gameImage);
        }
        
        gameGenres = new list<TheGamesDB::GameGenre *>;
        json_t *jsonGameGenres = json_object_get(json, "gameGenres");
        for(unsigned int index = 0; index < json_array_size(jsonGameGenres); index++)
        {
            json_t *jsonGameGenre = json_array_get(jsonGameGenres, index);
            
            TheGamesDB::GameGenre *gameGenre = new TheGamesDB::GameGenre(jsonGameGenre);
            gameGenres->push_back(gameGenre);
        }

        gameDevelopers = new list<TheGamesDB::GameDeveloper *>;
        json_t *jsonGameDevelopers = json_object_get(json, "gameDevelopers");
        for(unsigned int index = 0; index < json_array_size(jsonGameDevelopers); index++)
        {
            json_t *jsonGameDeveloper = json_array_get(jsonGameDevelopers, index);
            
            TheGamesDB::GameDeveloper *gameDeveloper = new TheGamesDB::GameDeveloper(jsonGameDeveloper);
            gameDevelopers->push_back(gameDeveloper);
        }

        gamePublishers = new list<TheGamesDB::GamePublisher *>;
        json_t *jsonGamePublishers = json_object_get(json, "gamePublishers");
        for(unsigned int index = 0; index < json_array_size(jsonGamePublishers); index++)
        {
            json_t *jsonGamePublisher = json_array_get(jsonGamePublishers, index);
            
            TheGamesDB::GamePublisher *gamePublisher = new TheGamesDB::GamePublisher(jsonGamePublisher);
            gamePublishers->push_back(gamePublisher);
        }        
}

TheGamesDB::Game::~Game()
{
    TheGamesDB::GameImage::releaseItems(gameImages);
    TheGamesDB::GameGenre::releaseItems(gameGenres);
    TheGamesDB::GameDeveloper::releaseItems(gameDevelopers);
    TheGamesDB::GamePublisher::releaseItems(gamePublishers);
}

int64_t TheGamesDB::Game::getId()
{
	return id;
}

int64_t TheGamesDB::Game::getPlatformId()
{
	return platformId;
}

string TheGamesDB::Game::getName()
{
	return name;
}

string TheGamesDB::Game::getReleaseDate()
{
	return releaseDate;
}

string TheGamesDB::Game::getDescription()
{
	return description;
}

int64_t TheGamesDB::Game::getEsrbRatingId()
{
	return esrbRatingId;
}

list<TheGamesDB::GameImage *> *TheGamesDB::Game::getGameImages()
{
    return gameImages;
}

list<TheGamesDB::GameGenre *> *TheGamesDB::Game::getGameGenres()
{
    return gameGenres;
}

list<TheGamesDB::GameDeveloper *> *TheGamesDB::Game::getGameDevelopers()
{
    return gameDevelopers;
}

list<TheGamesDB::GamePublisher *> *TheGamesDB::Game::getGamePublishers()
{
    return gamePublishers;
}

TheGamesDB::Game *TheGamesDB::Game::getItem(list<TheGamesDB::Game *> *items, unsigned int index)
{
	if(index >= items->size())
	{	
		return NULL;
	}

	list<TheGamesDB::Game *>::iterator item = items->begin();
	advance(item, index);

	return (*item);
}

void TheGamesDB::Game::releaseItems(list<TheGamesDB::Game *> *items)
{
	for(list<TheGamesDB::Game *>::iterator item = items->begin(); item != items->end(); item++)
	{
		delete (*item);
	}

	items->clear();
	delete items;
}




