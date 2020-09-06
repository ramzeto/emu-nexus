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
 * File:   FileExtractor.cpp
 * Author: ram
 * 
 * Created on July 20, 2019, 12:12 AM
 */

#include "FileExtractor.h"
#include "Utils.h"

#include <iostream>


FileExtractor::FileExtractor(string fileName)
{
    this->fileName = fileName;
    fileSize = Utils::getInstance()->getFileSize(fileName);
    progressBytes = 0;
    
    progressListener = NULL;
    progressListenerCallback = NULL;
}

FileExtractor::~FileExtractor()
{
}

void FileExtractor::setProgressListener(void* progressListener, void(*progressListenerCallback)(void*, FileExtractor*, size_t, size_t))
{
    this->progressListener = progressListener;
    this->progressListenerCallback = progressListenerCallback;
}


int FileExtractor::extract(string directory)
{
    progressBytes = 0;
    
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);


    if ((r = archive_read_open_filename(a, fileName.c_str(), 10240)))
    {
        cerr << "FileExtractor::" << __FUNCTION__ << " " << archive_error_string(a) << endl;
        return 1;
    }

    for (;;) 
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
        {
            break;
        }
        if (r != ARCHIVE_OK)
        {
            cerr << "FileExtractor::" << __FUNCTION__ << " " << archive_error_string(a) << endl;
        }

        string outputFileName = directory + string(archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, outputFileName.c_str());
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
        {
            cerr << "FileExtractor::" << __FUNCTION__ << " " << archive_error_string(a) << endl;
        }
        else
        {
            copyData(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
            {
                cerr << "FileExtractor::" << __FUNCTION__ << " " << archive_error_string(ext) << endl;
            }
        }
    }
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);
    
    return 0;
}

int FileExtractor::copyData(archive* ar, archive* aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
            r = archive_read_data_block(ar, &buff, &size, &offset);
            if (r == ARCHIVE_EOF)
                    return (ARCHIVE_OK);
            if (r != ARCHIVE_OK)
                    return (r);
            r = archive_write_data_block(aw, buff, size, offset);
            if (r != ARCHIVE_OK) {
                cerr << "FileExtractor::" << __FUNCTION__ << " " << archive_error_string(aw) << endl;
                return (r);
            }
            
            progressBytes += size;
            if(progressListener && progressListenerCallback)
            {
                if(progressBytes > fileSize * 2)
                {
                    progressBytes = fileSize * 2;
                }
                progressListenerCallback(progressListener, this, fileSize * 2 /*Uncompressed size aprox.*/, progressBytes);                
            }            
    }
}



