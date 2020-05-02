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
 * File:   HttpConnector.h
 * Author: ram
 *
 * Created on August 21, 2016, 5:17 PM
 */

#ifndef HTTPCONNECTOR_H
#define HTTPCONNECTOR_H

#include <curl/curl.h>
#include <string>
#include <map>

using namespace std;

/**
 * This class handles the HTTP requests. A libcurl with SSL support is required (libcurl4-openssl-dev or libcurl-nss-dev or libcurl4-gnutls-dev).
 */
class HttpConnector
{
public:    
    static const string METHOD_POST;
    static const string METHOD_GET;
    
    static const int HTTP_OK;
    
    HttpConnector(string url);
    virtual ~HttpConnector();
    
    unsigned char* getResponseData() const;
    size_t getResponseDataSize() const;
    int getHttpStatus() const;
    string getMethod() const;
    string getUrl() const;
    void setTimeout(long timeout);
    long getTimeout() const;

    /**
     * @param downloadProgressListener Object that will receive the progress updates.
     * @param downloadProgressListenerCallback Pointer to a callback function that will be executed when a progress update happens. The function receives this parameters (pointer to the progressListener, pointer to the HttpConnector object, bytesToDownload, downloadedBytes).
     */
    void setDownloadProgressListener(void *downloadProgressListener, void (*downloadProgressListenerCallback)(void*, HttpConnector*, size_t, size_t));
    
    /**
     * Sets a header.
     * @param name
     * @param value
     */
    void setHeader(string name, string value);
    
    /**
     * Sets a parameter.
     * @param name
     * @param value
     */
    void setParameter(string name, string value);
    
    /**
     * Executes a get request.
     * @return Http status.
     */
    int get();
    
    /**
     * @TODO Executes a post request.
     * @param body Body of the request.
     * @param bodySize Size of the body.
     * @return Http status.
     */
    int post(unsigned char *body, size_t bodySize);
    
    /**
     * Destroy the data received, this should be called after every get or post.
     */
    void clean();
    
    
private:
    string url;
    string method;
    int httpStatus;
    long timeout;
    size_t responseDataSize;
    unsigned char *responseData;
    map<string, string> headers;
    map<string, string> parameters;
    CURL *curl;
    size_t contentLength;
    void *downloadProgressListener;
    void (*downloadProgressListenerCallback)(void *, HttpConnector *, size_t, size_t);
    
    static size_t curlResponseCallback(unsigned char *data, size_t size, size_t nmemb, void *pHttpConnector);
};

#endif /* HTTPCONNECTOR_H */

