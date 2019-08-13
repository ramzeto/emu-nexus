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

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace TheGamesDB
{
    static const int API_ID = 1;

    class Elasticsearch
    {
    public:            
        static const int STATUS_OK;
        static const int STATUS_STARTING;
        static const int STATUS_UPDATING;
        static const int STATUS_STOPPED;

        typedef struct{
            int error;
            void *data;
        }Result_t;
             

        /**
         * Starts the engine. This method will return immediately. The requester should listen to the callback to start using the engine.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that indicates that the engine has started or not. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is the response from the engine in a json_t pointer (The requester is responsible to free Result_t->data using json_decref and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void start(void *requester, void (*callback)(void *, void*));

        /**
         * Gets the genres in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the genres. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::Genre *> object (The requester is responsible to free the list using TheGamesDB::Genre::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getGenres(void *requester, void (*callback)(void *, void*));

        /**
         * Gets the developers in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the developers. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::Developer *> object (The requester is responsible to free the list using TheGamesDB::Developer::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getDevelopers(void *requester, void (*callback)(void *, void*));

        /**
         * Gets the publishers in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the publishers. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::Publisher *> object (The requester is responsible to free the list using TheGamesDB::Publisher::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getPublishers(void *requester, void (*callback)(void *, void*));

        /**
         * Gets the ESRB ratings in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the ESRB ratings. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::EsrbRating *> object (The requester is responsible to free the list using TheGamesDB::EsrbRating::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getEsrbRatings(void *requester, void (*callback)(void *, void*));

        /**
         * Gets the platforms in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the platforms. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::Platform *> object (The requester is responsible to free the list using TheGamesDB::Platform::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getPlatforms(void *requester, void (*callback)(void *, void*));
                
        /**
         * Gets matching games (according to the query parameter) in TheGamesDB. This method will return immediately. The requester should listen to the callback to get the items.
         * @param apiPlatformId Id of the TheGamesDB::Platform in which the game will be looked for.
         * @param query Query to search.
         * @param requester Pointer to the object that requested the method.
         * @param callback Callback that receives the games. Receives the pointer of the requester and a pointer to a Result_t structure. Result_t->data is a pointer to a list<TheGamesDB::Game *> object (The requester is responsible to free the list using TheGamesDB::Game::releaseItems and to free the pointer to the Result_t structure). It will be called in a new thread.
         */
        void getGames(int64_t apiPlatformId, string query, void *requester, void (*callback)(void *, void*));


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

        typedef struct{
            void *requester;
            void (*callback)(void *, void*);
            int64_t apiPlatformId;
            string query;
        }RequesterRef_t;           

        pthread_t processThread;
        int status;

        Elasticsearch();
        virtual ~Elasticsearch();                        

        static Elasticsearch *instance;
        static void *processWorker(void *pRequesterRef);
        static void *processStartListenerWorker(void *pRequesterRef);
        static void *getGenresWorker(void *pRequesterRef);
        static void *getDevelopersWorker(void *pRequesterRef);
        static void *getPublishersWorker(void *pRequesterRef);
        static void *getEsrbRatingsWorker(void *pRequesterRef);            
        static void *getPlatformsWorker(void *pRequesterRef);
        static void *getGamesWorker(void *pRequesterRef);
    };



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


}


#ifdef __cplusplus
}
#endif

#endif /* THEGAMESDB_H */

