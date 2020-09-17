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
 * File:   thegamesdb.h
 * Author: ram
 *
 * Created on April 14, 2019, 2:35 PM
 */

#ifndef THEGAMESDB_H
#define THEGAMESDB_H

#include <string>
#include <list>
#include <jansson.h>
#include <pthread.h>
#include <functional>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This namespace is for handling the operations and the data from the Elasticsearch engine.
 */
namespace TheGamesDB
{
    class PlatformImage
    {
    private:
            int64_t id;
            int64_t platformId;
            string type;
            string original;
            string small;
            string thumb;
            string croppedCenterThumb;
            string medium;
            string large;

            PlatformImage();

    public:
        static const string TYPE_BANNER;
        static const string TYPE_FANART;
        static const string TYPE_BOXART;
        static const string TYPE_ICON;

            PlatformImage(int64_t id);
            PlatformImage(const PlatformImage &orig);
            PlatformImage(json_t *json);

            ~PlatformImage();

            int64_t getId();
            int64_t getPlatformId();
            string getType();
            string getOriginal();
            string getSmall();
            string getThumb();
            string getCroppedCenterThumb();
            string getMedium();
            string getLarge();
            string getFileName();

            static PlatformImage *getItem(list<PlatformImage *> *items, unsigned int index);
            static void releaseItems(list<PlatformImage *> *items);
    };



    class Platform
    {
    private:
            int64_t id;
            string name;
            string alias;
            list<PlatformImage *> *platformImages;

            Platform();

    public:
            Platform(int64_t id);
            Platform(const Platform &orig);
            Platform(json_t *json);

            ~Platform();

            int64_t getId();
            string getName();
            string getAlias();
            list<PlatformImage *> *getPlatformImages();

            static Platform *getItem(list<Platform *> *items, unsigned int index);
            static void releaseItems(list<Platform *> *items);
    };                                                




    class Genre
    {
    private:
            int64_t id;
            string name;

            Genre();

    public:
            Genre(int64_t id);
            Genre(const Genre &orig);
            Genre(json_t *json);

            ~Genre();

            int64_t getId();
            string getName();

            static Genre *getItem(list<Genre *> *items, unsigned int index);
            static void releaseItems(list<Genre *> *items);
    };




    class Developer
    {
    private:
            int64_t id;
            string name;

            Developer();

    public:
            Developer(int64_t id);
            Developer(const Developer &orig);
            Developer(json_t *json);

            ~Developer();

            int64_t getId();
            string getName();

            static Developer *getItem(list<Developer *> *items, unsigned int index);
            static void releaseItems(list<Developer *> *items);
    };




    class Publisher
    {
    private:
            int64_t id;
            string name;

            Publisher();

    public:
            Publisher(int64_t id);
            Publisher(const Publisher &orig);
            Publisher(json_t *json);

            ~Publisher();

            int64_t getId();
            string getName();

            static Publisher *getItem(list<Publisher *> *items, unsigned int index);
            static void releaseItems(list<Publisher *> *items);
    };




    class EsrbRating
    {
    private:
            int64_t id;
            string name;

            EsrbRating();

    public:
            EsrbRating(int64_t id);
            EsrbRating(const EsrbRating &orig);
            EsrbRating(json_t *json);

            ~EsrbRating();

            int64_t getId();
            string getName();

            static EsrbRating *getItem(list<EsrbRating *> *items, unsigned int index);
            static void releaseItems(list<EsrbRating *> *items);
    };

    
    
    class GameImage
    {
    private:
            int64_t id;
            int64_t gameId;
            string type;
            string side;
            string original;
            string small;
            string thumb;
            string croppedCenterThumb;
            string medium;
            string large;

            GameImage();

    public:        
        static const string TYPE_BOXART;
        static const string TYPE_SCREENSHOT;
        static const string TYPE_BANNER;
        static const string TYPE_FANART;
        static const string TYPE_CLEAR_LOGO;
        
        static const string SIDE_FRONT;
        static const string SIDE_BACK;
        
            GameImage(int64_t id);
            GameImage(const GameImage &orig);
            GameImage(json_t *json);

            ~GameImage();

            int64_t getId();
            int64_t getGameId();
            string getType();
            string getSide();
            string getOriginal();
            string getSmall();
            string getThumb();
            string getCroppedCenterThumb();
            string getMedium();
            string getLarge();
            string getFileName();

            static GameImage *getItem(list<GameImage *> *items, unsigned int index);
            static void releaseItems(list<GameImage *> *items);
    };

    
    
    class GameGenre
    {
    private:
            int64_t gameId;
            int64_t genreId;

            GameGenre();

    public:
            GameGenre(int64_t gameId, int64_t genreId);
            GameGenre(const GameGenre &orig);
            GameGenre(json_t *json);

            ~GameGenre();

            int64_t getGameId();
            int64_t getGenreId();

            static GameGenre *getItem(list<GameGenre *> *items, unsigned int index);
            static void releaseItems(list<GameGenre *> *items);
    };





    class GameDeveloper
    {
    private:
            int64_t gameId;
            int64_t developerId;

            GameDeveloper();

    public:
            GameDeveloper(int64_t gameId, int64_t developerId);
            GameDeveloper(const GameDeveloper &orig);
            GameDeveloper(json_t *json);

            ~GameDeveloper();

            int64_t getGameId();
            int64_t getDeveloperId();

            static GameDeveloper *getItem(list<GameDeveloper *> *items, unsigned int index);
            static void releaseItems(list<GameDeveloper *> *items);
    };





    class GamePublisher
    {
    private:
            int64_t gameId;
            int64_t publisherId;

            GamePublisher();

    public:
            GamePublisher(int64_t gameId, int64_t publisherId);
            GamePublisher(const GamePublisher &orig);
            GamePublisher(json_t *json);

            ~GamePublisher();

            int64_t getGameId();
            int64_t getPublisherId();
            
            json_t *toJson();

            static GamePublisher *getItem(list<GamePublisher *> *items, unsigned int index);
            static void releaseItems(list<GamePublisher *> *items);
            static json_t *toJsonArray(list<GamePublisher *> *items);
    };


    

    class Game
    {
    private:
            int64_t id;
            int64_t platformId;
            string name;
            string releaseDate;
            string description;
            int64_t esrbRatingId;
            
            list<GameImage *> *gameImages;
            list<GameGenre *> *gameGenres;
            list<GameDeveloper *> *gameDevelopers;
            list<GamePublisher *> *gamePublishers;

            Game();

    public:
            Game(int64_t id);
            Game(const Game &orig);
            Game(json_t *json);

            ~Game();

            int64_t getId();
            int64_t getPlatformId();
            string getName();
            string getReleaseDate();
            string getDescription();
            int64_t getEsrbRatingId();
            
            list<GameImage *> *getGameImages();
            list<GameGenre *> *getGameGenres();
            list<GameDeveloper *> *getGameDevelopers();
            list<GamePublisher *> *getGamePublishers();

            static Game *getItem(list<Game *> *items, unsigned int index);
            static void releaseItems(list<Game *> *items);
    };


    
    /**
     * This class handles the requests to the Elasticsearch engine.
     */
    class Elasticsearch
    {
    public:            
        static const int STATUS_OK;
        static const int STATUS_STARTING;
        static const int STATUS_STOPPED;       
        
        /**
         * Starts the Elasticsearch process.
         * @param callback Lambda function that receives the status of the process as parameter.
         */
        void start(function<void(int)> callback);
        
        /**
         * Gets EsrbRatings.
         * @param callback Lambda function that receives a list of TheGamesDB::EsrbRating (it will be freed automatically).
         */
        void getEsrbRatings(function<void(list<TheGamesDB::EsrbRating *> *)> callback);
        
        /**
         * Gets Developers.
         * @param callback Lambda function that receives a list of TheGamesDB::Developer (it will be freed automatically).
         */
        void getDevelopers(function<void(list<TheGamesDB::Developer *> *)> callback);
        
        /**
         * Gets Publishers.
         * @param callback Lambda function that receives a list of TheGamesDB::Publisher (it will be freed automatically).
         */        
        void getPublishers(function<void(list<TheGamesDB::Publisher *> *)> callback);
        
        /**
         * Gets Genres.
         * @param callback Lambda function that receives a list of TheGamesDB::Genre (it will be freed automatically).
         */          
        void getGenres(function<void(list<TheGamesDB::Genre *> *)> callback);
        
        /**
         * Gets Platforms.
         * @param callback Lambda function that receives a list of TheGamesDB::Platform (it will be freed automatically).
         */        
        void getPlatforms(function<void(list<TheGamesDB::Platform *> *)> callback);
        
        /**
         * Gets games.
         * @param apiPlatformId Id of the TheGamesDB::Platform in which the game will be looked for.
         * @param query Query to search.
         * @param callback Lambda function that receives a list of TheGamesDB::Game (it will be freed automatically).
         */
        void getGames(int64_t apiPlatformId, string query, function<void(list<TheGamesDB::Game *> *)> callback);


        /**
         * 
         * @return current status of the database.
         */
        int getStatus();

        /**
         * 
         * @return Elasticsearch instance.
         */
        static Elasticsearch *getInstance();

    private:
        static const string URL;
        static const int WAIT_TIME_ELASTICSEARCH;
        static const int REQUEST_MAX_RETRIES;
        static const int REQUEST_TIME_TO_RETRY;
        
        int status;

        Elasticsearch();
        virtual ~Elasticsearch();                        

        static Elasticsearch *instance;        
    };    
}


#ifdef __cplusplus
}
#endif

#endif /* THEGAMESDB_H */

