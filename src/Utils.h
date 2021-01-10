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
 * File:   Utils.h
 * Author: ram
 *
 * Created on August 20, 2016, 2:38 PM
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <list>

using namespace std;

/**
 * Provides useful methods.
 */
class Utils 
{
public:
            
    /**
     * Saves a file into the media directory.
     * @param sourceFileName
     * @param destinyFileName
     * @return 0 on success.
     */
    int copyFile(string sourceFileName, string destinyFileName);    
    
    /**
     * Gets the directory from a filename.
     * @param fileName
     * @return path
     */
    string getFileDirectory(string fileName);
    
    /**
     * Gets the base name of a file.
     * @param fileName
     * @return base file name.
     */
    string getFileBasename(string fileName);
    
    /**
     * Checks if a file exists.
     * @param fileName
     * @return 1 if exists, 0 otherwise.
     */
    int fileExists(string fileName);    
    
    /**
     * Writes an array of bytes to a file.
     * @param data
     * @param dataSize
     * @param fileName
     * @return 0 on success
     */
    int writeToFile(unsigned char *data, size_t dataSize, string fileName);
    
    /**
     * Gets the contents of a text file.
     * @param fileName
     * @return Contents of the file.
     */
    string getFileContents(string fileName);
    
    /**
     * Checks if a directory exists.
     * @param directoryName
     * @return 1 if exists, 0 otherwise.
     */
    int directoryExists(string directoryName);
    
    /**
     * 
     * @param directory
     * @return Size of the contents of the directory in bytes.
     */
    size_t getDirectorySize(string directory);
    
    /**
     * 
     * @return System temporary directory.
     */
    string getTempDirectory();
    
    /**
     * 
     * @return A unique file name for a temporary file.
     */
    string getTempFileName();
    
    /**
     * 
     * @param fileName
     * @return Size of the file in bytes.
     */
    size_t getFileSize(string fileName);
    
    /**
     * Trims a string.
     * @param str string to trim.
     * @return Trimmed string.
     */
    string trim(string str);
    
    /**
     * Opens a file using the default application.
     * @param fileName
     */
    void openFileWithDefaultApplication(string fileName);
    
    /**
     * Shows a file in the file namager.
     * @param fileName
     */
    void showFileInFileManager(string fileName);
    
    /**
     * Executes an application.
     * @param application Application path and parameters.
     * @param output Input/Output parameter. The output of the application.
     * @return Application return code @TODO
     */
    int executeApplication(string application, string *output);
            
    /**
     * Replaces all the occurrences of 'from' to 'to' in 'str'.
     * @param str
     * @param from
     * @param to
     * @return String with all 'from's replaced with 'to's.
     */
    string strReplace(string str, string from, string to);
    
    /**
     * Gets the lowercase string of input.
     * @param input
     * @return Lowercase string of input.
     */
    string strToLowerCase(string input);
    
    /**
     * Splits a string by whitespaces.
     * @param input
     * @return List of tokens that form input.
     */
    list<string> strSplitByWhiteSpace(string input);
    
    /**
     * Replaces special characters for their html/xml entities.
     * @param text to process.
     * @return 
     */
    string htmlEntities(string text);
    
    /**
     * Creates a directory.
     * @param directory
     * @return 0 on success.
     */
    int makeDirectory(string directory);
    
    /**
     * 
     * @return Timestamp in format yyyy-mm-dd HH:MM:SS
     */
    string nowIsoDateTime();
    
    /**
     * 
     * @return Timestamp in millis.
     */
    long long timestampInMilliSeconds();
    
    /**
     * Scales an image preserving its aspect ratio.
     * @param inFileName
     * @param outWidth
     * @param outHeight
     * @param outFileName
     * @return 0 on success.
     */
    int scaleImage(string inFileName, int outWidth, int outHeight, string outFileName);
    
    /**
     * Creates a JPEG image from the first page of a PDF document.
     * @param pdfFileName PDF document file name.
     * @param imageFileName File name where the image will be stored.
     * @return 0 on success.
     */
    int createPreviewImageFromPdf(string pdfFileName, string imageFileName);
    
    /**
     * Encodes a string in url format.
     * @param text string to encode.
     * @return text encoded.
     */
    string urlEncode(string text);
    
    /**
     * Removes a directory.
     * @param directoryName Directory to remove.
     */
    void removeDirectory(string directoryName);
    
    /**
     * 
     * @param sourceDirectoryName
     * @param destinationDirectoryName
     */
    void moveDirectory(string sourceDirectoryName, string destinationDirectoryName);
    
    
    /**
     * 
     * @return Utils instance.
     */
    static Utils *getInstance();
    
private:
    Utils();
    virtual ~Utils();
    
    static Utils *instance;

};

#endif /* UTILS_H */

