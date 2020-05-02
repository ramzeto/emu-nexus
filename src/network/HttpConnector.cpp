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
 * File:   HttpConnector.cpp
 * Author: ram
 * 
 * Created on August 21, 2016, 5:17 PM
 */

#include "HttpConnector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

const string HttpConnector::METHOD_POST = "POST";
const string HttpConnector::METHOD_GET = "GET";
const int HttpConnector::HTTP_OK = 200;
    
HttpConnector::HttpConnector(string url)
{
    this->url = url;    
    responseData = NULL;
    responseDataSize = 0;
    timeout = 600L;
    downloadProgressListener = NULL;
    downloadProgressListenerCallback = NULL;
}

HttpConnector::~HttpConnector()
{
    clean();
}

unsigned char* HttpConnector::getResponseData() const {
    return responseData;
}

size_t HttpConnector::getResponseDataSize() const {
    return responseDataSize;
}

int HttpConnector::getHttpStatus() const {
    return httpStatus;
}

string HttpConnector::getMethod() const {
    return method;
}

string HttpConnector::getUrl() const {
    return url;
}

void HttpConnector::setTimeout(long timeout) {
    this->timeout = timeout;
}

long HttpConnector::getTimeout() const {
    return timeout;
}

void HttpConnector::setDownloadProgressListener(void *downloadProgressListener, void (*downloadProgressListenerCallback)(void*, HttpConnector*, size_t, size_t))
{
    this->downloadProgressListener = downloadProgressListener;
    this->downloadProgressListenerCallback = downloadProgressListenerCallback;
}

void HttpConnector::setHeader(string name, string value)
{
    headers[name] = value;
}

void HttpConnector::setParameter(string name, string value)
{
    parameters[name] = value;
}


int HttpConnector::get()
{
    httpStatus = 0;
    contentLength = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(!curl)
    {
        return httpStatus;
    }
    
    struct curl_slist *headerList = NULL;    
    for (map<string, string>::iterator header = headers.begin(); header != headers.end(); header++)
    {
        headerList = curl_slist_append(headerList, (header->first + ": " + header->second).c_str());
    }
    
    string query = "";
    for (map<string, string>::iterator parameter = parameters.begin(); parameter != parameters.end(); parameter++)
    {
        if(query.length() > 0)
        {
            query += "&";
        }
        
        char *escapeValue = curl_easy_escape(curl, parameter->second.c_str(), parameter->second.length());
        query += parameter->first + "=" + escapeValue;
        curl_free(escapeValue);
    }
    if(query.length() > 0)
    {
        url += "?" + query;
    }
    
    this->responseData = (unsigned char *)malloc(1);
    this->responseDataSize = 0;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    
    if(headerList)
    {
        curl_easy_setopt(curl, CURLOPT_HEADER, headerList);
    }
    
    CURLcode curlCode = curl_easy_perform(curl);
    if(curlCode == CURLE_OK)
    {        
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);        
    }
        
    if(headerList)
    {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);

    return httpStatus;
}

int HttpConnector::post(unsigned char *body, size_t bodySize)
{
    httpStatus = 0;
    contentLength = 0;
    
    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(!curl)
    {
        return httpStatus;
    }
    
    struct curl_slist *headerList = NULL;    
    for (map<string, string>::iterator header = headers.begin(); header != headers.end(); header++)
    {
        headerList = curl_slist_append(headerList, (header->first + ": " + header->second).c_str());
    }
    
    this->responseData = (unsigned char *)malloc(1);
    this->responseDataSize = 0;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, bodySize);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    
    if(headerList)
    {
        curl_easy_setopt(curl, CURLOPT_HEADER, headerList);
    }
    
    CURLcode curlCode = curl_easy_perform(curl);
    if(curlCode == CURLE_OK)
    {        
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    }        
    
    if(headerList)
    {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    return httpStatus;    
}

void HttpConnector::clean()
{
    if(responseData)
    {
        free(responseData);
        responseDataSize = 0;
        responseData = NULL;
    }
}


size_t HttpConnector::curlResponseCallback(unsigned char* data, size_t size, size_t nmemb, void* pHttpConnector)
{
    HttpConnector *httpConnector = (HttpConnector *)pHttpConnector;
    if(httpConnector->contentLength == 0)
    {
        double contentLength = 0;
        curl_easy_getinfo(httpConnector->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
        httpConnector->contentLength = (size_t)contentLength;
    }
    
    size_t realSize = size * nmemb;
    httpConnector->responseData = (unsigned char *)realloc((void *)httpConnector->responseData, httpConnector->responseDataSize + realSize);
    
    if(!httpConnector->responseData)
    {
        cerr << "HttpConnector::" << __FUNCTION__ << " realloc failded" << endl;
        return 0;
    }
    
    memcpy(&httpConnector->responseData[httpConnector->responseDataSize], data, realSize);
    httpConnector->responseDataSize += realSize;
       
    if(httpConnector->downloadProgressListener && httpConnector->downloadProgressListenerCallback)
    {
        httpConnector->downloadProgressListenerCallback(httpConnector->downloadProgressListener, httpConnector, httpConnector->contentLength, httpConnector->responseDataSize);
    }
    
    return realSize;
}