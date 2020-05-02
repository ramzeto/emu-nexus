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
 * File:   FileExtractor.h
 * Author: ram
 *
 * Created on July 20, 2019, 12:12 AM
 */

#ifndef FILEEXTRACTOR_H
#define FILEEXTRACTOR_H

#include <string>
#include <archive.h>
#include <archive_entry.h>

using namespace std;

/**
 * Provides a way to easily and atomically extract files to a desired location. Supports TAR.GZ, 7z, ZIP and partially supports RAR.
 */
class FileExtractor 
{
public:
    
    /**
     * 
     * @param fileName Compressed and/or packed file name to extract.
     */
    FileExtractor(string fileName);
        
    virtual ~FileExtractor();
    
    /**
     * 
     * @param progressListener Object that will receive the progress updates.
     * @param progressListenerCallback Pointer to a callback function that will be executed when a progress update happens. The function receives this parameters (pointer to the progressListener, pointer to the FileExtractor object, fileSize, progressBytes)
     */
    void setProgressListener(void *progressListener, void (*progressListenerCallback)(void*, FileExtractor*, size_t, size_t));
    
    /**
     * Extracts to a desired directory.
     * @param directory Directory where files will be extracted.
     * @return 0 on success
     */
    int extract(string directory);
    
private:
    string fileName;
    size_t fileSize;
    size_t progressBytes;
    void *progressListener;
    void (*progressListenerCallback)(void *, FileExtractor *, size_t, size_t);
    
    int copyData(struct archive *ar, struct archive *aw);
};

#endif /* FILEEXTRACTOR_H */

