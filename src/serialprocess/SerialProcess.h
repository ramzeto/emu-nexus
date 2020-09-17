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
     * Executes the process in a background thread. It should be called only by SerialProcessExecutor.
     */
    virtual void execute();    
};

#endif /* SERIALPROCESS_H */

