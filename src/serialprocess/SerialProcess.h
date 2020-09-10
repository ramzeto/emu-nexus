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
 * File:   SerialProcess.h
 * Author: ram
 *
 * Created on May 6, 2019, 10:19 PM
 */

#ifndef SERIALPROCESS_H
#define SERIALPROCESS_H

#include <string>


using namespace std;

/**
 * This is the base class for processes executed by the SerialProcessExecutor
 */
class SerialProcess 
{
friend class SerialProcessExecutor;

public:
    static const int STATUS_IDLE;
    static const int STATUS_RUNNING;
    static const int STATUS_SUCCESS;
    static const int STATUS_FAIL;       

    /**
     * 
     * @param type String that identifies the type of the process.
     */
    SerialProcess(string type);
    virtual ~SerialProcess();
    
    string getType();
    int getStatus();            
        
protected:
    string type;
    int status;
    
    /**
     * Executes the process. It should be called only by SerialProcessExecutor. 
     * If the process executes atomically, it should return STATUS_SUCCEED or STATUS_FAILED so SerialProcessExecutor acknowledges that the process finished and can execute the next one.
     * If the process executes in a different thread, it should return STATUS_RUNNING so SerialProcessExecutor acknowledges that the process is running. When finished, this object has to set status=STATUS_SUCCEED or status=STATUS_FAILED and call the SerialProcessExecutor::finish method so SerialProcessExecutor acknowledges that the process finished. SerialProcessExecutor will execute a final postStatus to announce that the process has finished.
     * This method has to assign the status value.
     * @return status
     */
    virtual int execute();    
};

#endif /* SERIALPROCESS_H */

