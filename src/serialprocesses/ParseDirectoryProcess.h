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
 * File:   ParseDirectoryProcess.h
 * Author: ram
 *
 * Created on August 8, 2019, 7:57 PM
 */

#ifndef PARSEDIRECTORYPROCESS_H
#define PARSEDIRECTORYPROCESS_H

#include "SerialProcess.h"
#include "ParseDirectory.h"
#include "ParseDirectoryGame.h"
#include "Game.h"

#include <list>

class ParseDirectoryProcess : public SerialProcess
{
public:
    static const string TYPE;
    
    ParseDirectoryProcess(void *requester, void (*statusCallback)(void *, void*));
    virtual ~ParseDirectoryProcess();
    
    int execute() override;
    
    ParseDirectory *getParseDirectory();
        
private:
    ParseDirectory *parseDirectory;
    list<ParseDirectoryGame *> *parseDirectoryGames;
    unsigned int parseDirectoryGamesIndex;
    
    void parseGameFiles(list<string> extensions, string directory);
    string cleanName(string rawName, string extension);
    string queryMame(string name);
    list<string> tokenizeName(string name);
    void getGameImagesFromDirectory(Game *game, string name, string directory, int64_t gameImageType);
    void fetchGameInformation();    
    
    static void callbackElasticsearchGames(void *pParseDirectoryProcess, void *pResult);
};

#endif /* PARSEDIRECTORYPROCESS_H */

