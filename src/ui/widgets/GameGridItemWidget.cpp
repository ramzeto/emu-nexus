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
 * File:   GameGridItemWidget.cpp
 * Author: ram
 * 
 * Created on May 7, 2020, 9:37 PM
 */

#include "GameGridItemWidget.h"
#include "GameImage.h"
#include "GameFavorite.h"
#include "UiUtils.h"
#include "Asset.h"
#include "Utils.h"
#include "CallbackResult.h"
#include "NotificationManager.h"
#include "Notifications.h"

const int GameGridItemWidget::GAME_GRID_ITEM_WIDTH = 180;
const int GameGridItemWidget::GAME_GRID_ITEM_HEIGHT = 300;
const int GameGridItemWidget::GAME_GRID_ITEM_IMAGE_WIDTH = GAME_GRID_ITEM_WIDTH - 20;
const int GameGridItemWidget::GAME_GRID_ITEM_IMAGE_HEIGHT = GAME_GRID_ITEM_IMAGE_WIDTH * 1.443;
const int GameGridItemWidget::FAVORITE_IMAGE_WIDTH = 30;
const int GameGridItemWidget::FAVORITE_IMAGE_HEIGHT = 30;

GameGridItemWidget::GameGridItemWidget(void *owner, Game *game) : Widget("GameGridItemBox.ui", "gameGridItemBox")
{
    this->owner = owner;
    this->game = NULL;
    
    gameImage = (GtkImage *)gtk_builder_get_object (builder, "gameImage");
    favoriteImage = (GtkImage *)gtk_builder_get_object (builder, "favoriteImage");
    nameLabel = (GtkLabel *)gtk_builder_get_object (builder, "nameLabel");

    g_signal_connect (widget, "button-press-event", G_CALLBACK(signalBoxButtonPressedEvent), this);
    
    selectedTimestamp = 0;
    activatedTimestamp = 0;
    
    callbackSelect = NULL;
    callbackActivate = NULL;
    callbackContextMenuFavorite = NULL;
    callbackContextMenuDetail = NULL;
    callbackContextMenuEdit = NULL;
    callbackContextMenuRemove = NULL;
    
    setGame(game);
}

GameGridItemWidget::~GameGridItemWidget()
{
    if(game != NULL)
    {
        delete game;        
    }
    game = NULL;
}

void* GameGridItemWidget::getOwner()
{
    return owner;
}

void GameGridItemWidget::setGame(Game *game)
{
    if(this->game != NULL)
    {
        delete this->game;        
    }
    
    this->game = new Game(*game);
    gtk_label_set_text(nameLabel, game->getName().c_str());

 
    GameImage *primaryImage = GameImage::getPrimaryImage(game->getId());
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

    
    GameFavorite *gameFavorite = new GameFavorite(game->getId());    
    if(gameFavorite->load())
    {
        UiUtils::getInstance()->loadImage(favoriteImage, Asset::getInstance()->getImageFavorite(), FAVORITE_IMAGE_WIDTH, FAVORITE_IMAGE_HEIGHT);
    }
    else
    {
        gtk_image_clear(favoriteImage);
        gtk_widget_set_size_request(GTK_WIDGET(favoriteImage), 0, 0);
    }
    delete gameFavorite;

    gtk_widget_set_size_request(widget, GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
}

Game* GameGridItemWidget::getGame()
{
    return game;
}

void GameGridItemWidget::setCallbackSelect(void (*callbackSelect)(GameGridItemWidget *))
{
    this->callbackSelect = callbackSelect;
}

void GameGridItemWidget::setCallbackActivate(void (*callbackActivate)(GameGridItemWidget *))
{
    this->callbackActivate = callbackActivate;
}

void GameGridItemWidget::setCallbackContextMenuFavorite(void (*callbackContextMenuFavorite)(GameGridItemWidget *))
{
    this->callbackContextMenuFavorite = callbackContextMenuFavorite;
}

void GameGridItemWidget::setCallbackContextMenuDetail(void (*callbackContextMenuDetail)(GameGridItemWidget *))
{
    this->callbackContextMenuDetail = callbackContextMenuDetail;
}

void GameGridItemWidget::setCallbackContextMenuEdit(void (*callbackContextMenuEdit)(GameGridItemWidget *))
{
    this->callbackContextMenuEdit = callbackContextMenuEdit;
}

void GameGridItemWidget::setCallbackContextMenuRemove(void (*callbackContextMenuRemove)(GameGridItemWidget *))
{
    this->callbackContextMenuRemove = callbackContextMenuRemove;
}

gboolean GameGridItemWidget::signalBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer gameGridItemWidget)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        time_t now = time(NULL);
        
                
        if(now - ((GameGridItemWidget *)gameGridItemWidget)->selectedTimestamp == 0 && now - ((GameGridItemWidget *)gameGridItemWidget)->activatedTimestamp > 2)
        {
            ((GameGridItemWidget *)gameGridItemWidget)->activatedTimestamp = now;
            if(((GameGridItemWidget *)gameGridItemWidget)->callbackActivate)
            {
                ((GameGridItemWidget *)gameGridItemWidget)->callbackActivate((GameGridItemWidget *)gameGridItemWidget);
            }
        }
        else
        {
            if(((GameGridItemWidget *)gameGridItemWidget)->callbackSelect)
            {
                ((GameGridItemWidget *)gameGridItemWidget)->callbackSelect((GameGridItemWidget *)gameGridItemWidget);
            }            
        }
        
        ((GameGridItemWidget *)gameGridItemWidget)->selectedTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackSelect)
        {
            ((GameGridItemWidget *)gameGridItemWidget)->callbackSelect((GameGridItemWidget *)gameGridItemWidget);
        }            

        GtkWidget *menu = gtk_menu_new();
        
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuFavorite)
        {
            GtkWidget *menuitem = NULL;
            
            GameFavorite *gameFavorite = new GameFavorite(((GameGridItemWidget *)gameGridItemWidget)->game->getId());
            if(gameFavorite->load())
            {
                menuitem = gtk_menu_item_new_with_label("Remove from favorites");
            }
            else
            {
                menuitem = gtk_menu_item_new_with_label("Add to favorites");
            }
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_signal_connect(menuitem, "activate", G_CALLBACK(signalMenuFavoriteActivate), gameGridItemWidget);
            
            delete gameFavorite;
        }
        
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuDetail)
        {
            GtkWidget *menuitem = gtk_menu_item_new_with_label("Information");
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_signal_connect(menuitem, "activate", G_CALLBACK(signalMenuDetailActivate), gameGridItemWidget);
        }
        
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuEdit)
        {
            GtkWidget *menuitem = gtk_menu_item_new_with_label("Edit");
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_signal_connect(menuitem, "activate", G_CALLBACK(signalMenuEditActivate), gameGridItemWidget);
        }
        
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuRemove)
        {
            GtkWidget *menuitem = gtk_menu_item_new_with_label("Remove");
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_signal_connect(menuitem, "activate", G_CALLBACK(signalMenuRemoveActivate), gameGridItemWidget);
        }
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    return TRUE;
}

void GameGridItemWidget::signalMenuFavoriteActivate(GtkMenuItem *menuitem, gpointer gameGridItemWidget)
{
    GameFavorite *gameFavorite = new GameFavorite(((GameGridItemWidget *)gameGridItemWidget)->game->getId());
    if(gameFavorite->load())
    {
        gameFavorite->remove();
    }
    else
    {
        gameFavorite->setTimestamp(Utils::getInstance()->nowIsoDateTime());
        gameFavorite->save();
    }
    delete gameFavorite;
    
    Game *game = new Game(*(((GameGridItemWidget *)gameGridItemWidget)->game));
    ((GameGridItemWidget *)gameGridItemWidget)->setGame(game);    
    delete game;
    
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuFavorite((GameGridItemWidget *)gameGridItemWidget);
    
    NotificationManager::getInstance()->postNotification(NOTIFICATION_FAVORITES_UPDATED, new Game(((GameGridItemWidget *)gameGridItemWidget)->game->getId()));        
}

void GameGridItemWidget::signalMenuDetailActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuDetail((GameGridItemWidget *)gameGridItemWidget);
}

void GameGridItemWidget::signalMenuEditActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuEdit((GameGridItemWidget *)gameGridItemWidget);
}

void GameGridItemWidget::signalMenuRemoveActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuRemove((GameGridItemWidget *)gameGridItemWidget);
}