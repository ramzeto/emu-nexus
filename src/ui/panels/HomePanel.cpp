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
#include "GameDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Build.h"

#include <iostream>

const int HomePanel::GAME_GRID_ITEM_WIDTH = 180;
const int HomePanel::GAME_GRID_ITEM_HEIGHT = 300;

const int HomePanel::GAME_GRID_ITEM_IMAGE_WIDTH = GAME_GRID_ITEM_WIDTH - 20;
const int HomePanel::GAME_GRID_ITEM_IMAGE_HEIGHT = GAME_GRID_ITEM_IMAGE_WIDTH * 1.443;


HomePanel::HomePanel(GtkWindow *parentWindow) : Panel(parentWindow, "HomePanel.ui", "homeBox")
{
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    selectedGameId = 0;
    recentGames = NULL;
    gameGridItems = new map<int64_t, GtkWidget *>;
    gameDetailDialog = NULL;
    
    recentsBox = (GtkBox *)gtk_builder_get_object (builder, "recentsBox");
    recentsGridBox = (GtkBox *)gtk_builder_get_object (builder, "recentsGridBox");
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    versionLabel = (GtkLabel *)gtk_builder_get_object (builder, "versionLabel");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    
    gtk_label_set_text(versionLabel, (string("Version ") + string(BUILD_VERSION)).c_str());
    gtk_label_set_markup(informationLabel, Utils::getInstance()->getFileContents(Asset::getInstance()->getHomePml()).c_str());
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalRecentsGridSizeAllocate), this);
    
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogo(), 300, 300);
    
    launcherUiThreadBridge = UiThreadBridge::registerBridge(this, callbackGameLauncher);
}

HomePanel::~HomePanel()
{    
    UiThreadBridge::unregisterBridge(launcherUiThreadBridge);
    
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
    gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);
    
    GameLauncher::getInstance()->launch(gameId, launcherUiThreadBridge, UiThreadBridge::callback);
    
    gameDetailDialog->execute();
    delete gameDetailDialog;
    
    gameDetailDialog = NULL;
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
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_SELECTED, 1);        
        }        
    }
}

void HomePanel::showGameDialog(int64_t gameId)
{
    Game *game = new Game(gameId);            
    sqlite3 *sqlite = Database::getInstance()->acquire();
    game->load(sqlite);
    Database::getInstance()->release();
    
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), game->getPlatformId(), game->getId());   
    if(gameDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        updateGame(gameId);
        selectGame(gameId);        
    }
    GameDialog::deleteWhenReady(gameDialog);
    
    delete game;
}

void HomePanel::removeGame(int64_t gameId)
{
    RecentGame *recentGame = new RecentGame(gameId);
    sqlite3 *sqlite = Database::getInstance()->acquire();    
    recentGame->load(sqlite);
    recentGame->remove(sqlite);
    Database::getInstance()->release();
    delete recentGame;

    loadRecentsGrid();
}

void HomePanel::showGameDetail(int64_t gameId)
{
    gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);
    gameDetailDialog->execute();
    delete gameDetailDialog;
    
    gameDetailDialog = NULL;
}



void HomePanel::signalRecentsGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer homePanel)
{    
    if(((HomePanel *)homePanel)->panelWidth != allocation->width || ((HomePanel *)homePanel)->panelHeight != allocation->height)
    {        
        ((HomePanel *)homePanel)->panelWidth = allocation->width;
        ((HomePanel *)homePanel)->panelHeight = allocation->height;
        
        ((HomePanel *)homePanel)->loadRecentsGrid();
    }
}

void HomePanel::signalShow(GtkWidget* widget, gpointer homePanel)
{    
    if(!((HomePanel *)homePanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(50, callbackFirstShowHackyTimeout, ((HomePanel *)homePanel));        
    }
}

gint HomePanel::callbackFirstShowHackyTimeout(gpointer homePanel)
{    
    ((HomePanel *)homePanel)->isShown = 1;
    ((HomePanel *)homePanel)->loadRecentsGrid();
    return 0;
}

gboolean HomePanel::signalGameItemBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer homePanel)
{    
    // Mouse left button
    if(event->button.button == 1)
    {
        int64_t gameId = atol(gtk_widget_get_name(widget));
        time_t now = time(NULL);
        
        if(((HomePanel *)homePanel)->selectedGameId == gameId)
        {
            if(((HomePanel *)homePanel)->selectGameTimestamp == now && (now - (((HomePanel *)homePanel)->launchGameTimestamp)) > 2)
            {
                ((HomePanel *)homePanel)->launchGameTimestamp = now;
                ((HomePanel *)homePanel)->launchGame(gameId);
            }
        }
        else
        {
            ((HomePanel *)homePanel)->launchGameTimestamp = 0;
            ((HomePanel *)homePanel)->selectGame(gameId);
        }
        ((HomePanel *)homePanel)->selectGameTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        ((HomePanel *)homePanel)->selectGame(atoi(gtk_widget_get_name(widget)));

        GtkWidget *menu = gtk_menu_new();
        GtkWidget *detailMenuitem = gtk_menu_item_new_with_label("Information");
        GtkWidget *editMenuitem = gtk_menu_item_new_with_label("Edit");
        GtkWidget *removeMenuitem = gtk_menu_item_new_with_label("Remove from recents");
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), detailMenuitem);
        g_signal_connect(detailMenuitem, "activate", G_CALLBACK(signalGameMenuDetailActivate), homePanel);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), editMenuitem);
        g_signal_connect(editMenuitem, "activate", G_CALLBACK(signalGameMenuEditActivate), homePanel);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), removeMenuitem);
        g_signal_connect(removeMenuitem, "activate", G_CALLBACK(signalGameMenuRemoveActivate), homePanel);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);  
    }
    
    return TRUE;
}

void HomePanel::signalGameMenuDetailActivate(GtkMenuItem* menuitem, gpointer homePanel)
{
    ((HomePanel *)homePanel)->showGameDetail(((HomePanel *)homePanel)->selectedGameId);
}

void HomePanel::signalGameMenuEditActivate(GtkMenuItem* menuitem, gpointer homePanel)
{
    ((HomePanel *)homePanel)->showGameDialog(((HomePanel *)homePanel)->selectedGameId);
}

void HomePanel::signalGameMenuRemoveActivate(GtkMenuItem* menuitem, gpointer homePanel)
{
    ((HomePanel *)homePanel)->removeGame(((HomePanel *)homePanel)->selectedGameId);
}

void HomePanel::callbackGameLauncher(CallbackResult *callbackResult)
{
    HomePanel *homePanel = (HomePanel *)callbackResult->getRequester();
    
    string message = "";
    int running = 0;
    if(callbackResult->getError())
    {
        if(callbackResult->getError() == GameLauncher::ERROR_BUSY)
        {
            message = "There is another process running";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_FILE_NOT_FOUND)
        {
            message = "File not found";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE)
        {
            message = "Decompressing/Unpacking failed";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE_NOT_SUPPORTED)
        {
            message = "Compressed/Packed file format not supported";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_OTHER)
        {
            message = "An unknown error happened";
        }
        
        running = 0;
    }
    else
    {
        if(callbackResult->getStatus() == GameLauncher::STATE_IDLE)
        {
            running = 1;
            message = "Preparing...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_INFLATING)
        {
            running = 1;
            message = "Decompressing/Unpacking...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_RUNNING)
        {
            running = 1;
            message = "Running...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_FINISHED)
        {
            running = 0;
            message = "Execution finished";
            homePanel->loadRecentsGrid();
        }
    }
    homePanel->gameDetailDialog->setStatus(running, message, callbackResult->getProgress());
}

