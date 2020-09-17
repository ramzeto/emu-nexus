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
 * File:   SerialProcess.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 10:19 PM
 */

#include "SerialProcess.h"

#include <iostream>

const int SerialProcess::STATUS_IDLE = -2;
const int SerialProcess::STATUS_RUNNING = -1;
const int SerialProcess::STATUS_SUCCESS = 0;
const int SerialProcess::STATUS_FAIL = 1;

SerialProcess::SerialProcess(string type)
{
    this->type = type;
    status = STATUS_IDLE;
}

SerialProcess::~SerialProcess()
{
}


string SerialProcess::getType()
{
    return type;
}

int SerialProcess::getStatus()
{
    return status;
}

void SerialProcess::execute()
{
}



