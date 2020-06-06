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
 * File:   PlatformPanel.cpp
 * Author: ram
 * 
 * Created on February 8, 2019, 10:04 PM
 */

#include "PlatformPanel.h"
#include "UiUtils.h"
#include "GameEditDialog.h"
#include "MessageDialog.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Platform.h"
#include "UiThreadBridge.h"
#include "AddDirectoryDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Utils.h"
#include "GameDialog.h"
#include "GameFavorite.h"
#include "Logger.h"
    
PlatformPanel::PlatformPanel(GtkWindow *parentWindow, int platformId)  : Panel(parentWindow, "PlatformPanel.ui", "platformBox")
{
    this->platformId = platformId;
    
    gameGridScrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "gameGridScrolledWindow");
    gameGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "gameGridListBox");
    
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    gameGridItemIndex = 0;
    selectedGameId = 0;
    gameGridItems = new map<int64_t, GameGridItemWidget *>;
    games = NULL;
    searchQuery = "";
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);    
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalGameGridSizeAllocate), this);    
    g_signal_connect (gameGridScrolledWindow, "edge-reached", G_CALLBACK(signalGameGridScrolledWindowEdgeReached), this);
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, callbackNotification, 1);
}

PlatformPanel::~PlatformPanel()
{
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    
    for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
    {
        delete item->second;
    }
    gameGridItems->clear();
    delete gameGridItems;
    
    if(games)
    {
        Game::releaseItems(games);
    }
}

void PlatformPanel::showGameEditDialog(int64_t gameId)
{
    GameEditDialog *gameEditDialog = new GameEditDialog(GTK_WINDOW(parentWindow), platformId, gameId);   
    if(gameEditDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        // New game
        if(!gameId)
        {
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
    GameEditDialog::deleteWhenReady(gameEditDialog);
}

void PlatformPanel::updateGames(string searchQuery)
{
    this->searchQuery  = searchQuery;
    
    gameGridItemIndex = 0;
    loadGridPage();
}

void PlatformPanel::showAddDirectoryDialog()
{
    AddDirectoryDialog *addDirectoryDialog = new AddDirectoryDialog(GTK_WINDOW(parentWindow), platformId);   
    if(addDirectoryDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        
    }
    delete addDirectoryDialog;
}

void PlatformPanel::loadGames()
{    
    if(games)
    {
        Game::releaseItems(games);
    }
    
    games = Game::getItems(platformId, searchQuery);
}

void PlatformPanel::loadGridPage()
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
        
        loadGames();
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
        if(gameGridItemIndex >= games->size())
        {
            break;
        }
        
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(gameGridItemIndex < games->size())
            {
                Game *game = Game::getItem(games, gameGridItemIndex);
                
                GameGridItemWidget *gameGridItemWidget = new GameGridItemWidget(this, game);
                gameGridItemWidget->setCallbackSelect(onGameGridItemWidgetSelect);
                gameGridItemWidget->setCallbackActivate(onGameGridItemWidgetActive);
                gameGridItemWidget->setCallbackContextMenuFavorite(onGameGridItemWidgetMenuFavoriteSelect);
                gameGridItemWidget->setCallbackContextMenuDetail(onGameGridItemWidgetMenuDetailSelect);
                gameGridItemWidget->setCallbackContextMenuEdit(onGameGridItemWidgetMenuEditSelect);
                gameGridItemWidget->setCallbackContextMenuRemove(onGameGridItemWidgetMenuRemoveSelect);
                                
                gtk_box_pack_start(rowBox, gameGridItemWidget->getWidget(), 1, 1, 0);
                
                if(selectedGameId == game->getId())
                {
                    gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameGridItems->insert(pair<int64_t, GameGridItemWidget *>(game->getId(), gameGridItemWidget));
                
                gameGridItemIndex++;
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

void PlatformPanel::updateGame(int64_t gameId)
{
    if(games == NULL)
    {
        return;
    }
    
    Game *game = NULL;
    for(unsigned int c = 0; c < games->size(); c++)
    {
        game = Game::getItem(games, c);                
        if(game->getId() == gameId)
        {
            game->load();

            break;
        }
    }

    if(gameGridItems->find(gameId) != gameGridItems->end())
    {
        GameGridItemWidget *gameGridItemWidget = gameGridItems->at(gameId);
        gameGridItemWidget->setGame(game);
    }            
}

void PlatformPanel::selectGame(int64_t gameId)
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

void PlatformPanel::removeGame(int64_t gameId)
{
    Game *game = new Game(gameId);
    game->load();
    
    MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(parentWindow), "Sure you want to remove \"" + game->getName() + "\"?", "Remove", "Cancel");   
    if(messageDialog->execute() == GTK_RESPONSE_YES)
    {
        game->remove();
        
        selectGame(0);
        gameGridItemIndex = 0;
        loadGridPage();
        
        // Notifies of the removed game to any object listening for platform changes
        NotificationManager::getInstance()->postNotification(NOTIFICATION_PLATFORM_UPDATED, new Platform(platformId));
        
        // Notifies of the removed game to favorites
        NotificationManager::getInstance()->postNotification(NOTIFICATION_FAVORITES_UPDATED, new Game(gameId));
    }
    delete messageDialog;
    delete game;
}

void PlatformPanel::launchGame(int64_t gameId)
{    
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), gameId);
    gameDialog->launch();
    delete gameDialog;    
}

void PlatformPanel::showGameDetailDialog(int64_t gameId)
{
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), gameId);   
    gameDialog->execute();
    delete gameDialog;    
}


void PlatformPanel::onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((PlatformPanel *)gameGridItemWidget->getOwner())->selectGame(gameGridItemWidget->getGame()->getId());
}

void PlatformPanel::onGameGridItemWidgetActive(GameGridItemWidget *gameGridItemWidget)
{
    ((PlatformPanel *)gameGridItemWidget->getOwner())->launchGame(gameGridItemWidget->getGame()->getId());
}

void PlatformPanel::onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget)
{
    
}

void PlatformPanel::onGameGridItemWidgetMenuDetailSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((PlatformPanel *)gameGridItemWidget->getOwner())->showGameDetailDialog(gameGridItemWidget->getGame()->getId());
}

void PlatformPanel::onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((PlatformPanel *)gameGridItemWidget->getOwner())->showGameEditDialog(gameGridItemWidget->getGame()->getId());
}

void PlatformPanel::onGameGridItemWidgetMenuRemoveSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((PlatformPanel *)gameGridItemWidget->getOwner())->removeGame(gameGridItemWidget->getGame()->getId());
}



void PlatformPanel::signalGameGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer platformPanel)
{    
    if(((PlatformPanel *)platformPanel)->isDestroyed())
    {
        return;
    }

    if(((PlatformPanel *)platformPanel)->panelWidth != allocation->width || ((PlatformPanel *)platformPanel)->panelHeight != allocation->height)
    {        
        ((PlatformPanel *)platformPanel)->panelWidth = allocation->width;
        ((PlatformPanel *)platformPanel)->panelHeight = allocation->height;
        
        ((PlatformPanel *)platformPanel)->isShown = 0;
        signalShow(widget, platformPanel);
        //((PlatformPanel *)platformPanel)->gameGridItemIndex = 0;
        //((PlatformPanel *)platformPanel)->loadGridPage();
    }
}

void PlatformPanel::signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow* scrolledWindow, GtkPositionType positionType, gpointer platformPanel)
{        
    if(positionType == GTK_POS_TOP)
    {
        
    }
    else if(positionType == GTK_POS_BOTTOM)
    {
        ((PlatformPanel *)platformPanel)->loadGridPage();
    }
}

void PlatformPanel::signalShow(GtkWidget* widget, gpointer platformPanel)
{
    if(!((PlatformPanel *)platformPanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(10, callbackFirstShowHackyTimeout, platformPanel);
    }
}

gint PlatformPanel::callbackFirstShowHackyTimeout(gpointer platformPanel)
{    
    ((PlatformPanel *)platformPanel)->isShown = 1;
    ((PlatformPanel *)platformPanel)->gameGridItemIndex = 0;
    ((PlatformPanel *)platformPanel)->loadGridPage();
    return 0;
}

void PlatformPanel::callbackNotification(CallbackResult *callbackResult)
{
    try
    {
        PlatformPanel *platformPanel = (PlatformPanel *)callbackResult->getRequester();        
        if(platformPanel->isDestroyed())
        {
            return;
        }

        if(callbackResult->getType().compare(NOTIFICATION_GAME_UPDATED) == 0)
        {
            Game *game = (Game *)callbackResult->getData();
            ((PlatformPanel *)platformPanel)->updateGame(game->getId());
        }
    }
    catch(exception ex)
    {
        
    }
}


