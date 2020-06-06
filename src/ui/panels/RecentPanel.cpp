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
 * File:   RecentPanel.cpp
 * Author: ram
 * 
 * Created on June 5, 2020, 10:20 PM
 */

#include "RecentPanel.h"
#include "Notifications.h"
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
#include "Logger.h"

RecentPanel::RecentPanel(GtkWindow *parentWindow)  : Panel(parentWindow, "RecentPanel.ui", "recentBox")
{
    gameGridScrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "gameGridScrolledWindow");
    gameGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "gameGridListBox");

    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    gameGridItemIndex = 0;
    selectedGameId = 0;
    gameGridItems = new map<int64_t, GameGridItemWidget *>;
    gameActivities = NULL;
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);    
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalGameGridSizeAllocate), this);    
    g_signal_connect (gameGridScrolledWindow, "edge-reached", G_CALLBACK(signalGameGridScrolledWindowEdgeReached), this);
        
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, callbackNotification, 1);
}

RecentPanel::~RecentPanel()
{
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    
    for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
    {
        delete item->second;
    }
    gameGridItems->clear();
    delete gameGridItems;
    
    if(gameActivities)
    {
        GameActivity::releaseItems(gameActivities);
    }
}


void RecentPanel::loadGridPage()
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
        
        if(gameActivities)
        {
            GameActivity::releaseItems(gameActivities);
        }
        gameActivities = GameActivity::getRecentItems();
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
        if(gameGridItemIndex >= gameActivities->size())
        {
            break;
        }
        
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(gameGridItemIndex < gameActivities->size())
            {
                GameActivity *gameActivity = GameActivity::getItem(gameActivities, gameGridItemIndex);
                Game *game = new Game(gameActivity->getGameId());            
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

void RecentPanel::showGameEditDialog(int64_t gameId)
{
    Game *game = new Game(gameId);
    game->load();
    
    GameEditDialog *gameEditDialog = new GameEditDialog(GTK_WINDOW(parentWindow), game->getPlatformId(), game->getId());   
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
    
    delete game;
}

void RecentPanel::updateGame(int64_t gameId)
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

void RecentPanel::selectGame(int64_t gameId)
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

void RecentPanel::launchGame(int64_t gameId)
{    
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), gameId);
    gameDialog->launch();
    delete gameDialog;    
}

void RecentPanel::showGameDetailDialog(int64_t gameId)
{
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), gameId);   
    gameDialog->execute();
    delete gameDialog;    
}


void RecentPanel::onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((RecentPanel *)gameGridItemWidget->getOwner())->selectGame(gameGridItemWidget->getGame()->getId());
}

void RecentPanel::onGameGridItemWidgetActive(GameGridItemWidget *gameGridItemWidget)
{
    ((RecentPanel *)gameGridItemWidget->getOwner())->launchGame(gameGridItemWidget->getGame()->getId());
}

void RecentPanel::onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget)
{
    
}

void RecentPanel::onGameGridItemWidgetMenuDetailSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((RecentPanel *)gameGridItemWidget->getOwner())->showGameDetailDialog(gameGridItemWidget->getGame()->getId());
}

void RecentPanel::onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((RecentPanel *)gameGridItemWidget->getOwner())->showGameEditDialog(gameGridItemWidget->getGame()->getId());
}



void RecentPanel::signalGameGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer recentPanel)
{    
    if(((RecentPanel *)recentPanel)->isDestroyed())
    {
        return;
    }

    if(((RecentPanel *)recentPanel)->panelWidth != allocation->width || ((RecentPanel *)recentPanel)->panelHeight != allocation->height)
    {        
        ((RecentPanel *)recentPanel)->panelWidth = allocation->width;
        ((RecentPanel *)recentPanel)->panelHeight = allocation->height;
       
        ((RecentPanel *)recentPanel)->isShown = 0;
        signalShow(widget, recentPanel);
        //((RecentPanel *)recentPanel)->gameGridItemIndex = 0;
        //((RecentPanel *)recentPanel)->loadGridPage();
    }
}

void RecentPanel::signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow* scrolledWindow, GtkPositionType positionType, gpointer recentPanel)
{        
    if(positionType == GTK_POS_TOP)
    {
        
    }
    else if(positionType == GTK_POS_BOTTOM)
    {
        ((RecentPanel *)recentPanel)->loadGridPage();
    }
}

void RecentPanel::signalShow(GtkWidget* widget, gpointer recentPanel)
{
    if(!((RecentPanel *)recentPanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(10, callbackFirstShowHackyTimeout, recentPanel);
    }
}

gint RecentPanel::callbackFirstShowHackyTimeout(gpointer recentPanel)
{    
    ((RecentPanel *)recentPanel)->isShown = 1;
    ((RecentPanel *)recentPanel)->gameGridItemIndex = 0;
    ((RecentPanel *)recentPanel)->loadGridPage();
    return 0;
}


void RecentPanel::callbackNotification(CallbackResult *callbackResult)
{
    try
    {
        RecentPanel *recentPanel = (RecentPanel *)callbackResult->getRequester();        
        if(recentPanel->isDestroyed())
        {
            return;
        }

        if(callbackResult->getType().compare(NOTIFICATION_GAME_UPDATED) == 0)
        {
            Game *game = (Game *)callbackResult->getData();
            ((RecentPanel *)recentPanel)->updateGame(game->getId());
        }
    }
    catch(exception ex)
    {
        
    }
}