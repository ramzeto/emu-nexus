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
#include "NotificationManager.h"
#include "Notifications.h"
#include "MainBannerWidget.h"
#include "GameLauncher.h"
#include "Logger.h"

const int GameGridItemWidget::GAME_GRID_ITEM_WIDTH = 250;
const int GameGridItemWidget::GAME_GRID_ITEM_HEIGHT = 250;
const int GameGridItemWidget::GAME_GRID_ITEM_IMAGE_WIDTH = GAME_GRID_ITEM_WIDTH - 20;
const int GameGridItemWidget::GAME_GRID_ITEM_IMAGE_HEIGHT = GAME_GRID_ITEM_IMAGE_WIDTH;
const int GameGridItemWidget::FAVORITE_IMAGE_WIDTH = 30;
const int GameGridItemWidget::FAVORITE_IMAGE_HEIGHT = 30;

GdkPixbuf *GameGridItemWidget::selectedPixbuf = NULL;
GameGridItemWidget *GameGridItemWidget::selectedGameGridItemWidget = NULL;

GameGridItemWidget::GameGridItemWidget(void *owner, Game *game) : Widget("GameGridItemWidget.ui", "gameGridItemBox")
{
    this->owner = owner;
    this->game = NULL;
    
    selectedImage = (GtkImage *)gtk_builder_get_object (builder, "selectedImage");
    gameImage = (GtkImage *)gtk_builder_get_object (builder, "gameImage");
    favoriteImage = (GtkImage *)gtk_builder_get_object (builder, "favoriteImage");
    nameLabel = (GtkLabel *)gtk_builder_get_object (builder, "nameLabel");

    g_signal_connect (widget, "button-press-event", G_CALLBACK(signalBoxButtonPressedEvent), this);
    
    selectedTimestamp = 0;
    activatedTimestamp = 0;
    
    callbackSelect = NULL;
    callbackContextMenuFavorite = NULL;
    callbackContextMenuEdit = NULL;
    callbackContextMenuRemove = NULL;
    
    gtk_image_clear(selectedImage);
    setGame(game);
}

GameGridItemWidget::~GameGridItemWidget()
{
    if(game != NULL)
    {
        delete game;        
    }
    game = NULL;
    
    if(selectedGameGridItemWidget == this)
    {
        selectedGameGridItemWidget = NULL;
    }
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
    gtk_widget_set_tooltip_text(widget, game->getName().c_str());

 
    GameImage *primaryImage = GameImage::getPrimaryImage(game->getId());
    if(primaryImage)
    {
        if(Utils::getInstance()->fileExists(primaryImage->getThumbnailFileName()))
        {
            gtk_label_set_text(nameLabel, "");
            UiUtils::getInstance()->loadImage(gameImage, primaryImage->getThumbnailFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
        }
        else if(Utils::getInstance()->fileExists(primaryImage->getFileName()))
        {
            gtk_label_set_text(nameLabel, "");
            UiUtils::getInstance()->loadImage(gameImage, primaryImage->getFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
        }
        else
        {
            gtk_label_set_text(nameLabel, game->getName().c_str());
            UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageNoGameImage(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
        }
        delete primaryImage;
    }
    else
    {
        gtk_label_set_text(nameLabel, game->getName().c_str());
        UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageNoGameImage(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
    }
    gtk_widget_set_size_request(GTK_WIDGET(gameImage), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);

    
    GameFavorite *gameFavorite = new GameFavorite(game->getId());    
    if(gameFavorite->load())
    {
        gtk_image_set_from_icon_name(favoriteImage, "emblem-favorite", GTK_ICON_SIZE_LARGE_TOOLBAR);
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

void GameGridItemWidget::setSelected(int selected)
{
    if(selectedGameGridItemWidget)
    {
        GameGridItemWidget *gameGridItemWidget = selectedGameGridItemWidget;
        selectedGameGridItemWidget = NULL;
        gameGridItemWidget->setSelected(0);
    }

    // Hacky solution to draw the theme background color on the widget
    if(selected)
    {
        int widgetWidth = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
        int widgetHeight = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
                
        if(!selectedPixbuf)
        {
            // Gets the selected background color from a GtkListBoxRow widget, because it has a background color en every theme.
            GdkRGBA *selectedColor;
            GtkListBoxRow *rowBox = (GtkListBoxRow *)gtk_list_box_row_new();
            GtkStyleContext *styleContext = gtk_widget_get_style_context(GTK_WIDGET(rowBox));            
            gtk_style_context_get (styleContext, GTK_STATE_FLAG_SELECTED, "background-color", &selectedColor, NULL);            
                        
            GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, widgetWidth, widgetHeight);
            
            int width = gdk_pixbuf_get_width (pixbuf);
            int height = gdk_pixbuf_get_height (pixbuf);
            int channels = gdk_pixbuf_get_n_channels(pixbuf);
            int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
            guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
                       
            for(int x = 0; x < width; x++)
            {
                for(int y = 0; y < height; y++)
                {
                    guchar *pixel = pixels + y * rowstride + x * channels;
                    pixel[0] = selectedColor->red * 255;
                    pixel[1] = selectedColor->green * 255;
                    pixel[2] = selectedColor->blue * 255;
                    pixel[3] = selectedColor->alpha * 255;
                }
            }
            
            gdk_rgba_free (selectedColor);
            
            selectedPixbuf = pixbuf;
        }
        
        GdkPixbuf *scaledPixbuf = gdk_pixbuf_scale_simple(selectedPixbuf, widgetWidth, widgetHeight, GDK_INTERP_NEAREST);
        gtk_image_set_from_pixbuf(selectedImage, scaledPixbuf);
        g_object_unref(scaledPixbuf);
        
        gtk_widget_set_size_request(GTK_WIDGET(selectedImage), widgetWidth, widgetHeight);
        
        selectedGameGridItemWidget = this;
    }
    else
    {
        gtk_image_clear(selectedImage);
    }
}


void GameGridItemWidget::setCallbackSelect(void (*callbackSelect)(GameGridItemWidget *))
{
    this->callbackSelect = callbackSelect;
}

void GameGridItemWidget::setCallbackContextMenuFavorite(void (*callbackContextMenuFavorite)(GameGridItemWidget *))
{
    this->callbackContextMenuFavorite = callbackContextMenuFavorite;
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
            
            GameLauncher::getInstance()->launch(((GameGridItemWidget *)gameGridItemWidget)->game->getId());
        }
        else
        {
            ((GameGridItemWidget *)gameGridItemWidget)->setSelected(1);
            
            if(((GameGridItemWidget *)gameGridItemWidget)->callbackSelect)
            {
                ((GameGridItemWidget *)gameGridItemWidget)->callbackSelect((GameGridItemWidget *)gameGridItemWidget);
            }
        }
        
        ((GameGridItemWidget *)gameGridItemWidget)->selectedTimestamp = now;
        MainBannerWidget::getInstance()->setGameId(((GameGridItemWidget *)gameGridItemWidget)->game->getId());
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        ((GameGridItemWidget *)gameGridItemWidget)->setSelected(1);       
        if(((GameGridItemWidget *)gameGridItemWidget)->callbackSelect)
        {
            ((GameGridItemWidget *)gameGridItemWidget)->callbackSelect((GameGridItemWidget *)gameGridItemWidget);
        }
        MainBannerWidget::getInstance()->setGameId(((GameGridItemWidget *)gameGridItemWidget)->game->getId());

        GtkWidget *menu = gtk_menu_new();        
        
        // Favorites
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
        
        
        // Show in explorer
        GtkWidget *showInExplorerMenuitem = gtk_menu_item_new_with_label("Show in explorer");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), showInExplorerMenuitem);
        g_signal_connect(showInExplorerMenuitem, "activate", G_CALLBACK(signalMenuShowInExplorerActivate), gameGridItemWidget);
        
        
        // Edit and remove
        int allowEditing = 1;
        if(GameLauncher::getInstance()->getStatus() != GameLauncher::STATUS_IDLE)
        {
            if(GameLauncher::getInstance()->getGameId() == ((GameGridItemWidget *)gameGridItemWidget)->game->getId())
            {
                allowEditing = 0;
            }
        }
        if(allowEditing)
        {
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
    
    NotificationManager::getInstance()->notify(NOTIFICATION_GAME_FAVORITE_UPDATED, "", 0, 0, new Game(*((GameGridItemWidget *)gameGridItemWidget)->game));
}

void GameGridItemWidget::signalMenuShowInExplorerActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    Utils::getInstance()->showFileInFileManager(((GameGridItemWidget *)gameGridItemWidget)->game->getFileName());
}

void GameGridItemWidget::signalMenuEditActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuEdit((GameGridItemWidget *)gameGridItemWidget);
}

void GameGridItemWidget::signalMenuRemoveActivate(GtkMenuItem* menuitem, gpointer gameGridItemWidget)
{
    ((GameGridItemWidget *)gameGridItemWidget)->callbackContextMenuRemove((GameGridItemWidget *)gameGridItemWidget);
}