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
 * File:   HomePanel.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 8:49 PM
 */

#include "HomePanel.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Game.h"
#include "Database.h"
#include "GameImage.h"
#include "GameLauncher.h"
#include "UiThreadHandler.h"
#include "Directory.h"
#include "Asset.h"

#include <iostream>

const int HomePanel::GAME_GRID_ITEM_WIDTH = 180;
const int HomePanel::GAME_GRID_ITEM_HEIGHT = 300;

const int HomePanel::GAME_GRID_ITEM_IMAGE_WIDTH = GAME_GRID_ITEM_WIDTH - 20;
const int HomePanel::GAME_GRID_ITEM_IMAGE_HEIGHT = GAME_GRID_ITEM_IMAGE_WIDTH * 1.443;


HomePanel::HomePanel() : Panel("HomePanel.ui", "homeBox")
{
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    selectedGameId = 0;
    recentGames = NULL;
    gameGridItems = new map<int64_t, GtkWidget *>;
    launchDialog = NULL;
    
    recentsBox = (GtkBox *)gtk_builder_get_object (builder, "recentsBox");
    recentsGridBox = (GtkBox *)gtk_builder_get_object (builder, "recentsGridBox");
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    
    gtk_label_set_markup(informationLabel, Utils::getInstance()->getFileContents(Asset::getInstance()->getHomePml()).c_str());
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalRecentsGridSizeAllocate), this);
    
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogoBig(), 300, 300);
}

HomePanel::~HomePanel()
{
    if(recentGames)
    {
        RecentGame::releaseItems(recentGames);
    }
    
    gameGridItems->clear();
    delete gameGridItems;
}

void HomePanel::loadRecentsGrid()
{
    if(!isShown)
    {
        return;
    }
    
    cout << "HomePanel::" << __FUNCTION__ << endl;
    
    gameGridItems->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(recentsGridBox), 1);
    
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(getPanelBox()));
    unsigned int columns = width / GAME_GRID_ITEM_WIDTH;
    if(!columns)
    {
        gtk_widget_hide(GTK_WIDGET(recentsBox));
        return;
    }    

    if(recentGames)
    {
        RecentGame::releaseItems(recentGames);
    }
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    recentGames = RecentGame::getItems(sqlite, 0);
    Database::getInstance()->release();
    
    if(recentGames->size() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(recentsBox));
        return;
    }        
    
    for(unsigned int column = 0; column < columns; column++)
    {
        if(column < recentGames->size())
        {
            RecentGame *recentGame = RecentGame::getItem(recentGames, column);
            Game *game = new Game(recentGame->getGameId());
            
            sqlite3 *sqlite = Database::getInstance()->acquire();
            game->load(sqlite);
            Database::getInstance()->release();
            
            GtkBuilder *gameGridItemBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "GameGridItemBox.ui").c_str());
            GtkEventBox *gameGridItemBox = (GtkEventBox *)gtk_builder_get_object (gameGridItemBuilder, "gameGridItemBox");
            GtkImage *gameImage = (GtkImage *)gtk_builder_get_object (gameGridItemBuilder, "gameImage");
            GtkLabel *nameLabel = (GtkLabel *)gtk_builder_get_object (gameGridItemBuilder, "nameLabel");

            gtk_widget_set_name(GTK_WIDGET(gameGridItemBox), to_string(game->getId()).c_str());                                
            g_signal_connect (gameGridItemBox, "button-press-event", G_CALLBACK(signalGameItemBoxButtonPressedEvent), this);


            gtk_label_set_text(nameLabel, game->getName().c_str());

            GameImage *primaryImage = GameImage::getPrimaryImage(sqlite, game->getId());
            Database::getInstance()->release();

            if(primaryImage)
            {
                if(Utils::getInstance()->fileExists(primaryImage->getThumbnailFileName()))
                {
                    UiUtils::getInstance()->loadImage(gameImage, primaryImage->getThumbnailFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                }
                else if(Utils::getInstance()->fileExists(primaryImage->getFileName()))
                {
                    UiUtils::getInstance()->loadImage(gameImage, primaryImage->getFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                }
                else
                {
                    UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                }
                delete primaryImage;
            }
            else
            {
                UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
            }
            gtk_widget_set_size_request(GTK_WIDGET(gameImage), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);

            gtk_widget_set_size_request(GTK_WIDGET(gameGridItemBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
            gtk_box_pack_start(recentsGridBox, GTK_WIDGET(gameGridItemBox), 1, 1, 0);

            if(selectedGameId == game->getId())
            {
                gtk_widget_set_state_flags(GTK_WIDGET(gameGridItemBox), GTK_STATE_FLAG_SELECTED, 1);
            }
            gameGridItems->insert(pair<int64_t, GtkWidget *>(game->getId(), GTK_WIDGET(gameGridItemBox)));
        }
        else
        {
            GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            gtk_widget_set_size_request(GTK_WIDGET(dummyBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
            gtk_box_pack_start(recentsGridBox, GTK_WIDGET(dummyBox), 1, 1, 0);
        }
    }
    
    gtk_widget_show_all(GTK_WIDGET(recentsBox));
}

void HomePanel::launchGame(int64_t gameId)
{
    cout << "HomePanel::" << __FUNCTION__ << endl;
    
    launchDialog = new LaunchDialog(gameId);
    
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(this, callbackGameLauncher);
    GameLauncher::getInstance()->launch(gameId, uiThreadHandler, UiThreadHandler::callback);
    
    launchDialog->execute();
    delete launchDialog;
    
    launchDialog = NULL;
}

void HomePanel::updateGame(int64_t gameId)
{
    if(gameGridItems->find(gameId) != gameGridItems->end())
    {
        Game *game = new Game(gameId);            
        sqlite3 *sqlite = Database::getInstance()->acquire();
        game->load(sqlite);
        Database::getInstance()->release();
            
        GtkWidget *gameGridItemBox = gameGridItems->at(gameId);
        GtkImage *gameImage = (GtkImage *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gameGridItemBox), "gameImage");
        GtkLabel *nameLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gameGridItemBox), "nameLabel");

        gtk_widget_set_name(GTK_WIDGET(gameGridItemBox), to_string(game->getId()).c_str());
        gtk_label_set_text(nameLabel, game->getName().c_str());

        GameImage *primaryImage = GameImage::getPrimaryImage(sqlite, game->getId());
        Database::getInstance()->release();    
        if(primaryImage)
        {
            UiUtils::getInstance()->loadImage(gameImage, primaryImage->getThumbnailFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
        }
        gtk_widget_set_size_request(GTK_WIDGET(gameImage), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);

        gtk_widget_set_size_request(GTK_WIDGET(gameGridItemBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
    }
}

void HomePanel::selectGame(int64_t gameId)
{
    cout << "HomePanel::" << __FUNCTION__ << " A gameId: " << gameId << " selectedGameId: " << selectedGameId << endl;
    
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            cout << "HomePanel::" << __FUNCTION__ << " AA gameId: " << gameId << " selectedGameId: " << selectedGameId << endl;

            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        cout << "HomePanel::" << __FUNCTION__ << " B gameId: " << gameId << " selectedGameId: " << selectedGameId << endl;

        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            cout << "HomePanel::" << __FUNCTION__ << " BB gameId: " << gameId << " selectedGameId: " << selectedGameId << endl;

            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_SELECTED, 1);        
        }        
    }
}

void HomePanel::signalRecentsGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer pHomePanel)
{
    cout << "HomePanel::" << __FUNCTION__ << " allocation->width: " << allocation->width << endl;
    
    HomePanel *homePanel = (HomePanel *)pHomePanel;
    if(homePanel->panelWidth != allocation->width || homePanel->panelHeight != allocation->height)
    {        
        homePanel->panelWidth = allocation->width;
        homePanel->panelHeight = allocation->height;
        
        homePanel->loadRecentsGrid();
    }
}

void HomePanel::signalShow(GtkWidget* widget, gpointer pHomePanel)
{
    cout << "HomePanel::" << __FUNCTION__ << endl;
    
    HomePanel *homePanel = (HomePanel *)pHomePanel;
    if(!homePanel->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(50, callbackFirstShowHackyTimeout, homePanel);        
    }
}

gint HomePanel::callbackFirstShowHackyTimeout(gpointer pHomePanel)
{
    cout << "HomePanel::" << __FUNCTION__ << endl;
    
    HomePanel *homePanel = (HomePanel *)pHomePanel;
    homePanel->isShown = 1;
    homePanel->loadRecentsGrid();
    return 0;
}

gboolean HomePanel::signalGameItemBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer pHomePanel)
{
    HomePanel *homePanel = (HomePanel *)pHomePanel;
    
    // Mouse left button
    if(event->button.button == 1)
    {
        int64_t gameId = atol(gtk_widget_get_name(widget));
        time_t now = time(NULL);
        
        if(homePanel->selectedGameId == gameId)
        {
            if(homePanel->selectGameTimestamp == now && (now - (homePanel->launchGameTimestamp)) > 2)
            {
                homePanel->launchGameTimestamp = now;
                homePanel->launchGame(gameId);
            }
        }
        else
        {
            homePanel->launchGameTimestamp = 0;
            homePanel->selectGame(gameId);
        }
        homePanel->selectGameTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
    
    }
    
    return TRUE;
}

void HomePanel::callbackGameLauncher(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    HomePanel *homePanel = (HomePanel *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    GameLauncher::Status_t *gameLauncherStatus = (GameLauncher::Status_t *)uiThreadHandlerResult->data;
    
    string message = "";
    int activity = 0;
    if(gameLauncherStatus->error)
    {
        if(gameLauncherStatus->error == GameLauncher::ERROR_BUSY)
        {
            message = "There is another process running";
        }
        else if(gameLauncherStatus->error == GameLauncher::ERROR_FILE_NOT_FOUND)
        {
            message = "File not found";
        }
        else if(gameLauncherStatus->error == GameLauncher::ERROR_INFLATE)
        {
            message = "Decompressing/Unpacking failed";
        }
        else if(gameLauncherStatus->error == GameLauncher::ERROR_INFLATE_NOT_SUPPORTED)
        {
            message = "Compressed/Packed file format not supported";
        }
        else if(gameLauncherStatus->error == GameLauncher::ERROR_OTHER)
        {
            message = "An unknown error happened";
        }
        
        activity = 0;
    }
    else
    {
        if(gameLauncherStatus->state == GameLauncher::STATE_IDLE)
        {
            activity = 1;
            message = "Preparing...";
        }
        else if(gameLauncherStatus->state == GameLauncher::STATE_INFLATING)
        {
            activity = 1;
            message = "Decompressing/Unpacking...";
        }
        else if(gameLauncherStatus->state == GameLauncher::STATE_RUNNING)
        {
            activity = 1;
            message = "Running...";
        }
        else if(gameLauncherStatus->state == GameLauncher::STATE_FINISHED)
        {
            activity = 0;
            message = "Execution finished";
            homePanel->loadRecentsGrid();
        }
    }
    homePanel->launchDialog->setStatus(activity, message);
}

