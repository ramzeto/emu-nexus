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
#include "thegamesdb.h"

#include <list>

/**
 * Class for getting the contents of a directory, selecting the ROMs, cleaning the names and fetching the game data and images from Elasticsearh.
 */
class ParseDirectoryProcess : public SerialProcess
{
public:
    static const string TYPE;
    
    ParseDirectoryProcess();
    virtual ~ParseDirectoryProcess();    
    
    ParseDirectory *getParseDirectory();
        
private:
    typedef struct{
        TheGamesDB::Game *apiGame;
        list<string> nameTokens;
        unsigned int coincidences;
        unsigned int notCoincidences;
        int points;
    }GameEvaluation_t;
    
    ParseDirectory *parseDirectory;    
    void execute() override;
    
    /**
     * Parses the files from a directory.
     * @param extensions List of file extensions to look for.
     * @param directory Directory to process.
     */
    void parseGameFiles(list<string> extensions, string directory);
    
    /**
     * Sanitizes the name of a file. Removes undesired characters, region and version information.
     * @param rawName
     * @param extension
     * @return Sanitized name of the file.
     */
    string sanitizeName(string rawName, string extension);
    
    /**
     * Queries MAME for the game name from a filename.
     * @param fileName
     * @return Game name.
     */
    string getMameGameName(string fileName);
    
    /**
     * Splits the name into words.
     * @param name
     * @return List of words (tokens).
     */
    list<string> tokenizeName(string name);
    
    /**
     * Compares two tokens (words) looking for numerals (4 == iv, 4 == four).
     * @param token1
     * @param token2
     * @return 1 if numerals are the same, 0 otherwise
     */
    int compareNumerals(string token1, string token2);
    
    /**
     * Gets the game images from a directory. The image must have the same name as the game filename.
     * @param game
     * @param gameFileName
     * @param directory
     * @param gameImageType Constant type from GameImage.
     */
    void getGameImagesFromDirectory(Game *game, string gameFileName, string directory, int64_t gameImageType);
        
    /**
     * Fetches the information of the game from the ParseDirectoryGame object.
     * @param parseDirectoryGame
     */
    void fetch(ParseDirectoryGame *parseDirectoryGame);    

};

#endif /* PARSEDIRECTORYPROCESS_H */

