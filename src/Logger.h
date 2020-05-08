/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.h
 * Author: ram
 *
 * Created on February 26, 2020, 11:48 AM
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

using namespace std;

class Logger {
public:
    void message(string className, string functionName, string message = "");
    void debug(string className, string functionName, string message = "");
    void error(string className, string functionName, string error = "");
    
    static Logger *getInstance();    
    
private:
    Logger();
    virtual ~Logger();
    
    static Logger *instance;        
};

#endif /* LOGGER_H */

