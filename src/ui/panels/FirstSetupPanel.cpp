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
 * File:   FirstSetupPanel.cpp
 * Author: ram
 * 
 * Created on April 14, 2019, 8:45 PM
 */

#include "FirstSetupPanel.h"
#include "MessageDialog.h"
#include "Genre.h"
#include "Developer.h"
#include "Publisher.h"
#include "EsrbRating.h"
#include "Database.h"
#include "UiThreadHandler.h"
#include "SerialProcessExecutor.h"
#include "SerialProcess.h"
#include "thegamesdb.h"
#include "ElasticsearchProcess.h"
#include "UiUtils.h"
#include "Asset.h"

#include <jansson.h>
#include <iostream>

using namespace std;


FirstSetupPanel::FirstSetupPanel()  : Panel("FirstSetupPanel.ui", "FirstSetupPanel")
{    
    onSetupReadyCallback = NULL;
    onSetupReadyCallbackReferenceData = NULL;
    
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    progressBox = (GtkBox *)gtk_builder_get_object (builder, "progressBox");
    spinner = (GtkSpinner *)gtk_builder_get_object (builder, "spinner");
    progressLabel = (GtkLabel *)gtk_builder_get_object (builder, "progressLabel");
    resultLabel = (GtkLabel *)gtk_builder_get_object (builder, "resultLabel");
    
    setupButton = (GtkButton *)gtk_builder_get_object (builder, "setupButton");
    g_signal_connect (setupButton, "clicked", G_CALLBACK (signalSetupButtonClicked), this);
    
    successButton = (GtkButton *)gtk_builder_get_object (builder, "successButton");
    g_signal_connect (successButton, "clicked", G_CALLBACK (signalSuccessButtonClicked), this);
    
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogoBig(), 300, 300);
}

FirstSetupPanel::~FirstSetupPanel()
{
}

void FirstSetupPanel::setOnSetupReadyCallback(void(*onSetupReadyCallback)(void *), void *onSetupReadyCallbackReferenceData)
{
    this->onSetupReadyCallback = onSetupReadyCallback;
    this->onSetupReadyCallbackReferenceData = onSetupReadyCallbackReferenceData;
}

void FirstSetupPanel::show()
{
    prepareSetup();
}

void FirstSetupPanel::prepareSetup()
{
    gtk_widget_show_all(GTK_WIDGET(setupButton));
    
    gtk_spinner_stop(spinner);
    gtk_widget_hide(GTK_WIDGET(progressBox));
    gtk_widget_hide(GTK_WIDGET(resultLabel));
    gtk_widget_hide(GTK_WIDGET(successButton));
}

void FirstSetupPanel::startSetup()
{
    gtk_widget_hide(GTK_WIDGET(setupButton));
    gtk_widget_hide(GTK_WIDGET(resultLabel));
    gtk_widget_hide(GTK_WIDGET(successButton));
    startDatabase();
}

void FirstSetupPanel::setupFailed()
{
    gtk_widget_show_all(GTK_WIDGET(setupButton));
 
    gtk_spinner_stop(spinner);
    gtk_widget_hide(GTK_WIDGET(progressBox));    
    gtk_widget_hide(GTK_WIDGET(successButton));
    
    gtk_label_set_text(resultLabel, "Something went wrong.\nCheck your internet connection.");
    gtk_widget_show(GTK_WIDGET(resultLabel));
}

void FirstSetupPanel::setupSucceeded()
{
    gtk_widget_hide(GTK_WIDGET(setupButton));
 
    gtk_spinner_stop(spinner);    
    gtk_widget_hide(GTK_WIDGET(progressBox));        
    
    gtk_label_set_text(resultLabel, "Setup was successful");
    gtk_widget_show(GTK_WIDGET(resultLabel));
    gtk_widget_show(GTK_WIDGET(successButton));
}

void FirstSetupPanel::startDatabase()
{
    gtk_widget_show_all(GTK_WIDGET(progressBox));
    gtk_spinner_start(spinner);    
    gtk_label_set_text(progressLabel, "Preparing database");
    
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(this, callbackElasticsearchProcessListener);
    ElasticsearchProcess *elasticsearchProcess = new ElasticsearchProcess(uiThreadHandler, UiThreadHandler::callback);
    SerialProcessExecutor::getInstance()->schedule(elasticsearchProcess);
}

void FirstSetupPanel::startDatabaseFinished(int error)
{
    if(!error)
    {
        preloadData();
    }
    else
    {
        setupFailed();                
    }
}

void FirstSetupPanel::preloadData()
{
    gtk_widget_show_all(GTK_WIDGET(progressBox));
    gtk_spinner_start(spinner);    
    gtk_label_set_text(progressLabel, "Preloading data");

    UiThreadHandler *uiThreadHandler = new UiThreadHandler(this, callbackElasticsearchGenres);
    TheGamesDB::Elasticsearch::getInstance()->getGenres(uiThreadHandler, UiThreadHandler::callback);
}

void FirstSetupPanel::preloadDataFinished(int error)
{
    if(!error)
    {
        setupSucceeded();
    }
    else
    {
        setupFailed();                
    }
}

void FirstSetupPanel::setupReady()
{
    if(onSetupReadyCallback)
    {
        onSetupReadyCallback(onSetupReadyCallbackReferenceData);
    }
}



void FirstSetupPanel::signalSetupButtonClicked(GtkButton* button, gpointer firstSetupPanel)
{
    ((FirstSetupPanel *)firstSetupPanel)->startSetup();
}

void FirstSetupPanel::signalSuccessButtonClicked(GtkButton* button, gpointer firstSetupPanel)
{
    ((FirstSetupPanel *)firstSetupPanel)->setupReady();
}

void FirstSetupPanel::callbackElasticsearchProcessListener(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    FirstSetupPanel *firstSetupPanel = (FirstSetupPanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    SerialProcess::Status_t *status = (SerialProcess::Status_t *)uiThreadHandlerResult->data;
    
    if(status->serialProcess->getStatus() == SerialProcess::STATUS_RUNNING)
    {
        gtk_widget_show_all(GTK_WIDGET(firstSetupPanel->progressBox));
        gtk_spinner_start(firstSetupPanel->spinner);        

        string statusMessage = status->title;            
        if(status->progress >= 0)
        {
            statusMessage += " (" + to_string(status->progress) + "%)";
        }
        
        gtk_label_set_text(firstSetupPanel->progressLabel, statusMessage.c_str());
    }
    else if(status->serialProcess->getStatus() == SerialProcess::STATUS_FAIL)
    {
        firstSetupPanel->startDatabaseFinished(1);
    }
    else if(status->serialProcess->getStatus() == SerialProcess::STATUS_SUCCESS)
    {
        firstSetupPanel->startDatabaseFinished(0);
    }
}

void FirstSetupPanel::callbackElasticsearchGenres(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    FirstSetupPanel *firstSetupPanel = (FirstSetupPanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;
    
    list<TheGamesDB::Genre *> *apiGenres = (list<TheGamesDB::Genre *> *)dbResult->data;
    if(!dbResult->error)
    {
        list<Genre *> *genres = new list<Genre *>;
        for(unsigned int index = 0; index < apiGenres->size(); index++)
        {
            TheGamesDB::Genre *apiGenre = TheGamesDB::Genre::getItem(apiGenres, index);
            //cout << __FUNCTION__ << " index: " << index << " id: " << apiGenre->getId() << " name: " << apiGenre->getName() << endl;

            Genre *genre = new Genre((int64_t)0);
            genre->setName(apiGenre->getName());
            genre->setApiId(TheGamesDB::API_ID);
            genre->setApiItemId(apiGenre->getId());

            genres->push_back(genre);
        }

        sqlite3 *sqlite = Database::getInstance()->acquire();
        int error = Genre::bulkInsert(sqlite, genres);
        Database::getInstance()->release();
        Genre::releaseItems(genres);

        if(!error)
        {
            UiThreadHandler *uiThreadHandler = new UiThreadHandler(firstSetupPanel, callbackElasticsearchDevelopers);
            TheGamesDB::Elasticsearch::getInstance()->getDevelopers(uiThreadHandler, UiThreadHandler::callback);
        }
        else
        {
            firstSetupPanel->preloadDataFinished(error);
        }                
    }
    else
    {
        firstSetupPanel->preloadDataFinished(dbResult->error);
    }
    
    
    TheGamesDB::Genre::releaseItems(apiGenres);
}

void FirstSetupPanel::callbackElasticsearchDevelopers(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    FirstSetupPanel *firstSetupPanel = (FirstSetupPanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;
    
    list<TheGamesDB::Developer *> *apiDevelopers = (list<TheGamesDB::Developer *> *)dbResult->data;
    if(!dbResult->error)
    {
        list<Developer *> *developers = new list<Developer *>;
        for(unsigned int index = 0; index < apiDevelopers->size(); index++)
        {
            TheGamesDB::Developer *apiDeveloper = TheGamesDB::Developer::getItem(apiDevelopers, index);

            Developer *developer = new Developer((int64_t)0);
            developer->setName(apiDeveloper->getName());
            developer->setApiId(TheGamesDB::API_ID);
            developer->setApiItemId(apiDeveloper->getId());

            developers->push_back(developer);
        }


        sqlite3 *sqlite = Database::getInstance()->acquire();
        int error = Developer::bulkInsert(sqlite, developers);
        Database::getInstance()->release();
        Developer::releaseItems(developers);

        if(!error)
        {
            UiThreadHandler *uiThreadHandler = new UiThreadHandler(firstSetupPanel, callbackElasticsearchPublishers);
            TheGamesDB::Elasticsearch::getInstance()->getPublishers(uiThreadHandler, UiThreadHandler::callback);
        }
        else
        {
            firstSetupPanel->preloadDataFinished(error);
        }        
    }
    else
    {
        firstSetupPanel->preloadDataFinished(dbResult->error);
    }
    
      
    TheGamesDB::Developer::releaseItems(apiDevelopers);
}

void FirstSetupPanel::callbackElasticsearchPublishers(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;
    FirstSetupPanel *firstSetupPanel = (FirstSetupPanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;
    
    list<TheGamesDB::Publisher *> *apiPublishers = (list<TheGamesDB::Publisher *> *)dbResult->data;    
    if(!dbResult->error)
    {
        list<Publisher *> *publishers = new list<Publisher *>;
        for(unsigned int index = 0; index < apiPublishers->size(); index++)
        {
            TheGamesDB::Publisher *apiPublisher = TheGamesDB::Publisher::getItem(apiPublishers, index);

            Publisher *publisher = new Publisher((int64_t)0);
            publisher->setName(apiPublisher->getName());
            publisher->setApiId(TheGamesDB::API_ID);
            publisher->setApiItemId(apiPublisher->getId());

            publishers->push_back(publisher);
        }


        sqlite3 *sqlite = Database::getInstance()->acquire();
        int error = Publisher::bulkInsert(sqlite, publishers);
        Database::getInstance()->release();
        Publisher::releaseItems(publishers);

        if(!error)
        {    
            UiThreadHandler *uiThreadHandler = new UiThreadHandler(firstSetupPanel, callbackElasticsearchEsrbRatings);
            TheGamesDB::Elasticsearch::getInstance()->getEsrbRatings(uiThreadHandler, UiThreadHandler::callback);
        }
        else
        {
            firstSetupPanel->preloadDataFinished(error);
        }        
    }
    else
    {
        firstSetupPanel->preloadDataFinished(dbResult->error);
    }
    
   
    TheGamesDB::Publisher::releaseItems(apiPublishers);
}

void FirstSetupPanel::callbackElasticsearchEsrbRatings(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    FirstSetupPanel *firstSetupPanel = (FirstSetupPanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;
    
    list<TheGamesDB::EsrbRating *> *apiEsrbRatings = (list<TheGamesDB::EsrbRating *> *)dbResult->data;
    if(!dbResult->error)
    {
        list<EsrbRating *> *esrbRatings = new list<EsrbRating *>;
        for(unsigned int index = 0; index < apiEsrbRatings->size(); index++)
        {
            TheGamesDB::EsrbRating *apiEsrbRating = TheGamesDB::EsrbRating::getItem(apiEsrbRatings, index);

            EsrbRating *esrbRating = new EsrbRating((int64_t)0);
            esrbRating->setName(apiEsrbRating->getName());
            esrbRating->setApiId(TheGamesDB::API_ID);
            esrbRating->setApiItemId(apiEsrbRating->getId());

            esrbRatings->push_back(esrbRating);
        }


        sqlite3 *sqlite = Database::getInstance()->acquire();
        int error = EsrbRating::bulkInsert(sqlite, esrbRatings);
        Database::getInstance()->release();
        EsrbRating::releaseItems(esrbRatings);

        firstSetupPanel->preloadDataFinished(error);
    }
    else
    {
        firstSetupPanel->preloadDataFinished(dbResult->error);
    }
       

    TheGamesDB::EsrbRating::releaseItems(apiEsrbRatings);
}
