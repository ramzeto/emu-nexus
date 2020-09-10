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
 * File:   GamesPanel.cpp
 * Author: ram
 * 
 * Created on September 5, 2020, 12:58 PM
 */

#include "GamesPanel.h"
#include "UiUtils.h"
#include "GameEditDialog.h"
#include "MessageDialog.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Logger.h"
#include "Platform.h"
#include "MainBannerWidget.h"


GamesPanel::GamesPanel(GtkWindow *parentWindow)  : Panel(parentWindow, "GamesPanel.ui", "gamesBox")
{        
    gameGridScrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "gameGridScrolledWindow");
    gameGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "gameGridListBox");
 
    panelWidth = 0;
    panelHeight = 0;
    gameGridItemIndex = 0;
    selectedGameId = 0;
    gameGridItems = new map<int64_t, GameGridItemWidget *>;
    games = NULL;    
    
    signalSizeAllocateHandlerId = g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(+[](GtkWidget* widget, GtkAllocation* allocation, gpointer gamesPanel) -> void {
        if(((GamesPanel *)gamesPanel)->panelWidth != allocation->width || ((GamesPanel *)gamesPanel)->panelHeight != allocation->height)
        {        
            ((GamesPanel *)gamesPanel)->panelWidth = allocation->width;
            ((GamesPanel *)gamesPanel)->panelHeight = allocation->height;

            // @TODO .- Hacky load
            g_timeout_add(10, [](gpointer gamesPanel) -> gint {
                ((GamesPanel *)gamesPanel)->gameGridItemIndex = 0;
                ((GamesPanel *)gamesPanel)->loadGridPage();    

                return 0;
            }, gamesPanel);
        }
    }), this);

    
    signalSizeEdgeReachedHandlerId = g_signal_connect (gameGridScrolledWindow, "edge-reached", G_CALLBACK(+[](GtkScrolledWindow* scrolledWindow, GtkPositionType positionType, gpointer gamesPanel) -> void {
        if(positionType == GTK_POS_TOP)
        {

        }
        else if(positionType == GTK_POS_BOTTOM)
        {
            ((GamesPanel *)gamesPanel)->loadGridPage();
        }
    }), this);
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_EDIT_REQUIRED, this, onNotification, 1);
}

GamesPanel::~GamesPanel()
{
    g_signal_handler_disconnect(getPanelBox(), signalSizeAllocateHandlerId);
    g_signal_handler_disconnect(gameGridScrolledWindow, signalSizeEdgeReachedHandlerId);
    
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_EDIT_REQUIRED, this);
    
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

void GamesPanel::showGameEditDialog(int64_t gameId, int64_t platformId)
{
    GameEditDialog *gameEditDialog = new GameEditDialog(GTK_WINDOW(parentWindow), gameId, platformId);
    if(gameEditDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        if(!gameId)
        {
            gameGridItemIndex = 0;
            loadGridPage();
        }
        else
        {
            updateGame(gameId);
            selectGame(gameId);
        }        
    }
    GameEditDialog::deleteWhenReady(gameEditDialog);
}

void GamesPanel::loadGames()
{
    if(games)
    {
        Game::releaseItems(games);
    }
    
    games = new list<Game *>();
}

void GamesPanel::loadGridPage()
{
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
    rows += 2;

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
                gameGridItemWidget->setCallbackContextMenuFavorite(onGameGridItemWidgetMenuFavoriteSelect);
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

void GamesPanel::updateGame(int64_t gameId)
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

void GamesPanel::selectGame(int64_t gameId)
{
    // @TODO.- Change the background color when selected
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            
            GtkStyleContext *styleContext = gtk_widget_get_style_context(GTK_WIDGET(gameGridItemWidget->getWidget()));            
            gtk_style_context_set_state (styleContext, GTK_STATE_FLAG_NORMAL);
            
            //gtk_style_context_get (styleContext, GTK_STATE_FLAG_PRELIGHT,"background-color", &color1, NULL);
            
            //gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            
            GtkStyleContext *styleContext = gtk_widget_get_style_context(GTK_WIDGET(gameGridItemWidget->getWidget()));
            gtk_style_context_set_state (styleContext, GTK_STATE_FLAG_SELECTED);
            //gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);            
        }        
    }
}

void GamesPanel::removeGame(int64_t gameId)
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
        Platform *platfom = new Platform(game->getPlatformId());
        platfom->load();
        NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0, 0, platfom);
        
        // Notifies of the removed game to favorites
        NotificationManager::getInstance()->notify(NOTIFICATION_GAME_FAVORITE_UPDATED, "", 0, 0, new Game(*game));
        
        // Updates the banner
        MainBannerWidget::getInstance()->setBannerType(MainBannerWidget::getInstance()->getBannerType(), MainBannerWidget::getInstance()->getPlatformId());
    }
    delete messageDialog;
    delete game;
}

void GamesPanel::onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((GamesPanel *)gameGridItemWidget->getOwner())->selectGame(gameGridItemWidget->getGame()->getId());
}

void GamesPanel::onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget)
{
    
}

void GamesPanel::onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((GamesPanel *)gameGridItemWidget->getOwner())->showGameEditDialog(gameGridItemWidget->getGame()->getId());
}

void GamesPanel::onGameGridItemWidgetMenuRemoveSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((GamesPanel *)gameGridItemWidget->getOwner())->removeGame(gameGridItemWidget->getGame()->getId());
}


void GamesPanel::onNotification(Notification *notification)
{
    GamesPanel *gamesPanel = (GamesPanel *)notification->getListener();        

    if(notification->getName().compare(NOTIFICATION_GAME_UPDATED) == 0)
    {
        Game *game = (Game *)notification->getData();
        gamesPanel->updateGame(game->getId());
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_FAVORITE_UPDATED) == 0)
    {
        Game *game = (Game *)notification->getData();
        gamesPanel->updateGame(game->getId());
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_EDIT_REQUIRED) == 0)
    {
        Game *game = (Game *)notification->getData();
        gamesPanel->showGameEditDialog(game->getId());
    }
}
