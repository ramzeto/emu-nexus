/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.cpp
 * Author: ram
 * 
 * Created on February 26, 2020, 11:48 AM
 */

#include "Logger.h"
#include "Utils.h"

#include <iostream>
#include <cstdarg>

Logger *Logger::instance = NULL;

Logger::Logger() 
{
}

Logger::~Logger() 
{
}

void Logger::message(string className, string functionName, string message)
{
    string output = Utils::getInstance()->nowIsoDateTime() + " | " + __FUNCTION__ + " | " + className + " | " + functionName + " | " + message;
    cout << output << endl;
}

void Logger::debug(string className, string functionName, string message)
{
    string output = Utils::getInstance()->nowIsoDateTime() + " | " + __FUNCTION__ + " | " + className + " | " + functionName + " | " + message;
    cout << output << endl;
}

void Logger::error(string className, string functionName, string message)
{
    string output = Utils::getInstance()->nowIsoDateTime() + " | " + __FUNCTION__ + " | " + className + " | " + functionName + " | " + message;
    cout << output << endl;
}

Logger* Logger::getInstance() 
{
    if(!instance)
    {
        instance = new Logger();
    }
    
    return instance;
}


