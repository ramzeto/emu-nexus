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
 * File:   Utils.cpp
 * Author: ram
 * 
 * Created on August 20, 2016, 2:38 PM
 */

#include "Utils.h"

#include <stdio.h>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>
#include <dirent.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iterator>
#include <algorithm>

#include <dlib/image_io.h>
#include <dlib/image_transforms.h>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>

#include <curl/curl.h>


using namespace dlib;

Utils *Utils::instance = NULL;

Utils::Utils()
{
}

Utils::~Utils()
{
}

int Utils::copyFile(string sourceFileName, string destinyFileName)
{    
    ofstream out(destinyFileName.c_str(), ios::binary);    
    if(!out)
    {
        return 1;
    }
    
    ifstream in(sourceFileName.c_str(), ios::binary);
    if(!in)
    {
        return 1;
    }

    istreambuf_iterator<char> beginSource(in);
    istreambuf_iterator<char> endSource;
    ostreambuf_iterator<char> beginDest(out); 
    copy(beginSource, endSource, beginDest);
    
    in.close();
    out.close();
    
    return 0;
}

string Utils::getFileDirectory(string fileName)
{
    string path = "";
    char *lastDivider = strrchr((char*)fileName.c_str(), '/');
    if(lastDivider)
    {
        char directory[fileName.length()];
        memset(directory, 0, fileName.length());
        strncpy(directory, fileName.c_str(), lastDivider - fileName.c_str() + 1);
        
        path = string(directory);
    }
    
    return path;
}

string Utils::getFileBasename(string fileName)
{
    string path = fileName;
    char *lastDivider = strrchr((char*)fileName.c_str(), '/');
    if(lastDivider)
    {
        char name[fileName.length()];
        memset(name, 0, fileName.length());
        strncpy(name, lastDivider + 1, strlen(lastDivider));
        
        path = string(name);
    }
    
    return path;
}


int Utils::fileExists(string fileName)
{
    ifstream f(fileName.c_str());
    
    return f.good() ? 1 : 0;
}

int Utils::writeToFile(unsigned char* data, size_t dataSize, string fileName)
{
    ofstream out(fileName.c_str(), ios::out | ios::binary);
    if(!out)
    {
        return 1;
    }
    
    out.write((const char *)data, dataSize);
    out.close();
    return 0;
}

string Utils::getFileContents(string fileName)
{
    string contents = "";
    ifstream ifs (fileName.c_str());
    if(ifs)
    {        
        getline (ifs, contents, (char) ifs.eof());
        ifs.close();
    }

    return contents;
}

int Utils::directoryExists(string directoryName)
{
    struct stat statInfo;
    
    if(stat(directoryName.c_str(), &statInfo) != 0)
    {
        return 0;
    }
    
    if(statInfo.st_mode & S_IFDIR)
    {
        return 1;
    }
    
    return 0;
}

size_t Utils::getDirectorySize(string directory)
{
    size_t size = 0;
    DIR *dir;
    struct dirent *entry;
    
    if(directory.rfind("/") == string::npos || directory.rfind("/") != directory.length() - 1)
    {
        directory += "/";
    }
    
    if((dir = opendir(directory.c_str())))
    {
        while ((entry = readdir(dir)))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            
            if(entry->d_type == DT_REG)
            { 
                size += getFileSize(directory + string(entry->d_name));
            }
            else if(entry->d_type == DT_DIR)
            {                
                size += getDirectorySize(directory + string(entry->d_name));
            }
        }
        closedir(dir);
    }
        
    return size;
}

string Utils::getTempDirectory()
{
    // @TODO Find an elegant way to do this
    return "/tmp";
}

string Utils::getTempFileName()
{
    char *cTempFileName = new char[100];
    sprintf(cTempFileName, "%s/XXXXXX", getTempDirectory().c_str());
    mkstemp(cTempFileName);
    unlink(cTempFileName);
    
    string tempFileName(cTempFileName);
    delete cTempFileName;
    
    return tempFileName;
}

size_t Utils::getFileSize(string fileName)
{
    ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
    if(in)
    {
        return in.tellg();
    }
    
    return -1;
}

string Utils::trim(string str)
{
    int start = -1;
    for(int c = 0; c < (int)str.length(); c++)
    {
        char letter = str.c_str()[c];
        
        if(letter != ' ' && letter != '\t')
        {
            start = c;            
            break;
        }        
    }
    
    if(start < 0)
    {
        return "";
    }
    
    int end = start;
    if(end >= 0)
    {
        for(int c = (int)str.length() - 1; c >= start; c--)
        {
            char letter = str.c_str()[c];

            if(letter != ' ' && letter != '\t')
            {
                end = c;
                break;
            }
        }
    }
    
    return str.substr(start, end - start + 1);
}

void Utils::openFileWithDefaultApplication(string fileName)
{
    system(string("xdg-open \"" + fileName + "\" &").c_str());
}

int Utils::executeApplication(string application, string* output)
{
    FILE *fp;
    int length = 1023;
    char buffer[length];

    fp = popen(application.c_str(), "r");
    
    if (!fp)
    {
        return 1;
    }
        
    while (fgets(buffer, length, fp) != NULL)
    {
        *output += buffer;      
    }
    pclose(fp);
    
    return 0;
}

string Utils::strReplace(string str, string from, string to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

string Utils::strToLowerCase(string input)
{
    string lowerCase = input;
    
    transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::tolower);
    
    return lowerCase;
}

list<string> Utils::strSplitByWhiteSpace(string input)
{
    istringstream buf(input);
    istream_iterator<string> beg(buf), end;
    std::vector<string> parts(beg, end);
    
    list<string> tokens;
    for(string& part: parts)
    {
        if(part.length() > 0)
        {
            tokens.push_back(part);
        }        
    }
    
    return tokens;
}

string Utils::htmlEntities(string text)
{
    string escapedText = text;
    escapedText = strReplace(escapedText, "&",  "&amp;");
    escapedText = strReplace(escapedText, "\"", "&quot;");
    escapedText = strReplace(escapedText, "\'", "&apos;");
    escapedText = strReplace(escapedText, "<",  "&lt;");
    escapedText = strReplace(escapedText, ">",  "&gt;");
    
    return escapedText;
}

int Utils::makeDirectory(string directory)
{
    return mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

string Utils::nowIsoDateTime()
{
    time_t rawTime;
    struct tm * timeinfo;
    char buffer[50];

    time (&rawTime);
    timeinfo = localtime(&rawTime);
    
    strftime (buffer, 50, "%F %T", timeinfo);

    return string(buffer);
}

long long Utils::timestampInMilliSeconds()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec/1000;
    return milliseconds;
}

int Utils::scaleImage(string inFileName, int outWidth, int outHeight, string outFileName)
{
    try
    {
        array2d<rgb_pixel> image;
        load_image(image, inFileName);

        if(image.size() > 0)
        {
            double aspectRatio = 1.0;
            long width = image.nc();
            long height = image.nr();

            if(width > height)
            {
                aspectRatio = (double)width / (double)height;
                outWidth = outHeight * aspectRatio;
            }
            else
            {
                aspectRatio = (double)height / (double)width;
                outHeight = outWidth * aspectRatio;
            }

            array2d<rgb_pixel> resizedImage(outHeight, outWidth);
            resize_image(image, resizedImage, interpolate_bilinear());
            save_jpeg(resizedImage, outFileName);
            //save_png(resizedImage, outFileName);

            return 0;
        }        
    }
    catch(exception ex)
    {
        
    }
    
    return 1;
}

int Utils::createPreviewImageFromPdf(string pdfFileName, string imageFileName)
{
    poppler::document *pdfDocument = poppler::document::load_from_file(pdfFileName);
    if(!pdfDocument)
    {
        return 1;
    }
    if(pdfDocument->pages() <= 0)
    {
        delete pdfDocument;
        return 1;
    }
    
    poppler::page *portraitPage = pdfDocument->create_page(0);
    poppler::page_renderer *pageRenderer = new poppler::page_renderer();
    poppler::image previewImage = pageRenderer->render_page(portraitPage);
    
    int ok = previewImage.save(imageFileName, "jpeg");

    delete pageRenderer;    
    delete portraitPage;
    delete pdfDocument;
    
    return ok ? 0 : 1;
    return 0;
}

string Utils::urlEncode(string text)
{
    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    char *cEncodedText = curl_easy_escape(curl, text.c_str(), 0);
    string encodedText = string(cEncodedText);
    
    curl_free(cEncodedText);
    curl_easy_cleanup(curl);
    
    return encodedText;
}

void Utils::removeDirectory(string directoryName)
{
    system(string("rm -rf " + directoryName).c_str());
}

void Utils::moveDirectory(string sourceDirectoryName, string destinationDirectoryName)
{
    system(string("mv \"" + sourceDirectoryName + "\" \"" + destinationDirectoryName + "\"").c_str());
}


Utils* Utils::getInstance()
{
    if(!instance)        
    {
        instance = new Utils();
    }
    
    return instance;
}


