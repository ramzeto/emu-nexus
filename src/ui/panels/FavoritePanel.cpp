/*
 * Copyright (C) 2020 ram
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
 * File:   FavoritePanel.cpp
 * Author: ram
 * 
 * Created on May 8, 2020, 2:05 PM
 */

#include "FavoritePanel.h"
#include "Notifications.h"
#include "UiUtils.h"
#include "GameDialog.h"
#include "MessageDialog.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Platform.h"
#include "UiThreadBridge.h"
#include "AddDirectoryDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Utils.h"
#include "GameDetailDialog.h"

FavoritePanel::FavoritePanel(GtkWindow *parentWindow)  : Panel(parentWindow, "FavoritePanel.ui", "favoriteBox")
{
    gameGridScrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "gameGridScrolledWindow");
    gameGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "gameGridListBox");

    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    gameGridItemIndex = 0;
    selectedGameId = 0;
    gameGridItems = new map<int64_t, GameGridItemWidget *>;
    gameFavorites = NULL;
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);    
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalGameGridSizeAllocate), this);    
    g_signal_connect (gameGridScrolledWindow, "edge-reached", G_CALLBACK(signalGameGridScrolledWindowEdgeReached), this);
        
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, callbackNotification, 1);
}

FavoritePanel::~FavoritePanel()
{
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    
    for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
    {
        delete item->second;
    }
    gameGridItems->clear();
    delete gameGridItems;
    
    if(gameFavorites)
    {
        GameFavorite::releaseItems(gameFavorites);
    }
}


void FavoritePanel::loadGridPage()
{
    if(!isShown)
    {
        return;
    }

    if(gameGridItemIndex == 0)
    {
        if(gameGridItems->size() > 0)
        {
            for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
            {
                delete item->second;
            }
            gameGridItems->clear();
            UiUtils::getInstance()->clearContainer(GTK_CONTAINER(gameGridListBox), 1);
        }
        
        if(gameFavorites)
        {
            GameFavorite::releaseItems(gameFavorites);
        }
        gameFavorites = GameFavorite::getItems();
    }

    int width = gtk_widget_get_allocated_width(GTK_WIDGET(gameGridScrolledWindow));
    int columns = width / GameGridItemWidget::GAME_GRID_ITEM_WIDTH;
    if(!columns)
    {
        return;
    }

    int height = gtk_widget_get_allocated_height(GTK_WIDGET(gameGridScrolledWindow));
    int rows = height / GameGridItemWidget::GAME_GRID_ITEM_HEIGHT;    
    if(!rows)
    {
        return;
    }
    rows *= 2;

    for(int row = 0; row < rows; row++)
    {
        if(gameGridItemIndex >= gameFavorites->size())
        {
            break;
        }
        
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(gameGridItemIndex < gameFavorites->size())
            {
                GameFavorite *gameFavorite = GameFavorite::getItem(gameFavorites, gameGridItemIndex);
                Game *game = new Game(gameFavorite->getGameId());            
                game->load();
                                
                GameGridItemWidget *gameGridItemWidget = new GameGridItemWidget(this, game);
                gameGridItemWidget->setCallbackSelect(onGameGridItemWidgetSelect);
                gameGridItemWidget->setCallbackActivate(onGameGridItemWidgetActive);
                gameGridItemWidget->setCallbackContextMenuFavorite(onGameGridItemWidgetMenuFavoriteSelect);
                gameGridItemWidget->setCallbackContextMenuDetail(onGameGridItemWidgetMenuDetailSelect);
                gameGridItemWidget->setCallbackContextMenuEdit(onGameGridItemWidgetMenuEditSelect);
                
                gtk_box_pack_start(rowBox, gameGridItemWidget->getWidget(), 1, 1, 0);
                
                if(selectedGameId == game->getId())
                {
                    gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameGridItems->insert(pair<int64_t, GameGridItemWidget *>(game->getId(), gameGridItemWidget));
                
                gameGridItemIndex++;
                
                delete game;
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), GameGridItemWidget::GAME_GRID_ITEM_WIDTH, GameGridItemWidget::GAME_GRID_ITEM_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
        }        
        
        gtk_container_add (GTK_CONTAINER(gameGridListBox), GTK_WIDGET(rowBox));
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }

    
    // Force redraw the entire widget
    gtk_widget_hide(GTK_WIDGET(getPanelBox()));
    gtk_widget_show_all(GTK_WIDGET(getPanelBox()));
}

void FavoritePanel::showGameDialog(int64_t gameId)
{
    Game *game = new Game(gameId);
    game->load();
    
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), game->getPlatformId(), game->getId());   
    if(gameDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        // New game
        if(!gameId)
        {
            // Notifies of the new game to any object listening for platform changes
            CallbackResult *callbackResult = new CallbackResult(NULL);
            callbackResult->setType(NOTIFICATION_PLATFORM_UPDATED);
            callbackResult->setData(new Platform(game->getPlatformId()));
            NotificationManager::getInstance()->postNotification(callbackResult);                


            gameGridItemIndex = 0;
            loadGridPage();
        }
        // Update game
        else
        {
            updateGame(gameId);
            selectGame(gameId);
        }        
    }
    GameDialog::deleteWhenReady(gameDialog);
    
    delete game;
}

void FavoritePanel::updateGame(int64_t gameId)
{
    if(gameGridItems->find(gameId) != gameGridItems->end())
    {
        Game *game = new Game(gameId);            
        game->load();
            
        GameGridItemWidget *gameGridItemWidget = gameGridItems->at(gameId);
        gameGridItemWidget->setGame(game);
        
        delete game;
    }          
}

void FavoritePanel::selectGame(int64_t gameId)
{    
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);
        }        
    }
}

void FavoritePanel::launchGame(int64_t gameId)
{    
    GameDetailDialog *gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);
    gameDetailDialog->launch();
    delete gameDetailDialog;    
}

void FavoritePanel::showGameDetail(int64_t gameId)
{
    GameDetailDialog *gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);   
    gameDetailDialog->execute();
    delete gameDetailDialog;    
}


void FavoritePanel::onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((FavoritePanel *)gameGridItemWidget->getOwner())->selectGame(gameGridItemWidget->getGame()->getId());
}

void FavoritePanel::onGameGridItemWidgetActive(GameGridItemWidget *gameGridItemWidget)
{
    ((FavoritePanel *)gameGridItemWidget->getOwner())->launchGame(gameGridItemWidget->getGame()->getId());
}

void FavoritePanel::onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget)
{
    
}

void FavoritePanel::onGameGridItemWidgetMenuDetailSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((FavoritePanel *)gameGridItemWidget->getOwner())->showGameDetail(gameGridItemWidget->getGame()->getId());
}

void FavoritePanel::onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((FavoritePanel *)gameGridItemWidget->getOwner())->showGameDialog(gameGridItemWidget->getGame()->getId());
}



void FavoritePanel::signalGameGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer favoritePanel)
{    
    if(((FavoritePanel *)favoritePanel)->isDestroyed())
    {
        return;
    }
    
    if(((FavoritePanel *)favoritePanel)->panelWidth != allocation->width || ((FavoritePanel *)favoritePanel)->panelHeight != allocation->height)
    {        
        ((FavoritePanel *)favoritePanel)->panelWidth = allocation->width;
        ((FavoritePanel *)favoritePanel)->panelHeight = allocation->height;
        
        ((FavoritePanel *)favoritePanel)->gameGridItemIndex = 0;
        ((FavoritePanel *)favoritePanel)->loadGridPage();
    }
}

void FavoritePanel::signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow* scrolledWindow, GtkPositionType positionType, gpointer favoritePanel)
{        
    if(positionType == GTK_POS_TOP)
    {
        
    }
    else if(positionType == GTK_POS_BOTTOM)
    {
        ((FavoritePanel *)favoritePanel)->loadGridPage();
    }
}

void FavoritePanel::signalShow(GtkWidget* widget, gpointer favoritePanel)
{
    if(!((FavoritePanel *)favoritePanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(50, callbackFirstShowHackyTimeout, favoritePanel);
    }
}

gint FavoritePanel::callbackFirstShowHackyTimeout(gpointer favoritePanel)
{    
    ((FavoritePanel *)favoritePanel)->isShown = 1;
    ((FavoritePanel *)favoritePanel)->loadGridPage();
    return 0;
}


void FavoritePanel::callbackNotification(CallbackResult *callbackResult)
{
    try
    {
        FavoritePanel *favoritePanel = (FavoritePanel *)callbackResult->getRequester();        
        if(favoritePanel->isDestroyed())
        {
            return;
        }

        if(callbackResult->getType().compare(NOTIFICATION_GAME_UPDATED) == 0)
        {
            Game *game = (Game *)callbackResult->getData();
            ((FavoritePanel *)favoritePanel)->updateGame(game->getId());
        }
    }
    catch(exception ex)
    {
        
    }
}