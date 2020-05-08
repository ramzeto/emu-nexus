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
 * File:   GameGridItemWidget.h
 * Author: ram
 *
 * Created on May 7, 2020, 9:37 PM
 */

#ifndef GAMEGRIDITEMWIDGET_H
#define GAMEGRIDITEMWIDGET_H

#include "Widget.h"
#include "Game.h"

class GameGridItemWidget : public Widget
{
public:
    static const int GAME_GRID_ITEM_WIDTH;
    static const int GAME_GRID_ITEM_HEIGHT;    
    static const int GAME_GRID_ITEM_IMAGE_WIDTH;
    static const int GAME_GRID_ITEM_IMAGE_HEIGHT;    
    
    /**
     * 
     * @param owner Pointer to the object that owns the GameGridItemWidget.
     * @param game Game that will be displayed. The object is copied.
     */
    GameGridItemWidget(void *owner, Game *game);
    
    virtual ~GameGridItemWidget();
    
    /**
     * 
     * @return 
     */
    void *getOwner();
    
    /**
     * 
     * @param game Game that will be displayed. The object is copied.
     */
    void setGame(Game *game);
    
    /**
     * 
     * @return 
     */
    Game *getGame();
    
    /**
     * 
     * @param callbackSelect
     */
    void setCallbackSelect(void (*callbackSelect)(GameGridItemWidget *));
    
    /**
     * 
     * @param callbackActivate
     */
    void setCallbackActivate(void (*callbackActivate)(GameGridItemWidget *));
    
    /**
     * 
     * @param callbackContextMenuFavorite
     */
    void setCallbackContextMenuFavorite(void (*callbackContextMenuFavorite)(GameGridItemWidget *));
    
    /**
     * 
     * @param callbackContextMenuDetail
     */
    void setCallbackContextMenuDetail(void (*callbackContextMenuDetail)(GameGridItemWidget *));
    
    /**
     * 
     * @param callbackContextMenuEdit
     */
    void setCallbackContextMenuEdit(void (*callbackContextMenuEdit)(GameGridItemWidget *));
    
    /**
     * 
     * @param callbackContextMenuRemove
     */
    void setCallbackContextMenuRemove(void (*callbackContextMenuRemove)(GameGridItemWidget *));
    
private:
    static const int FAVORITE_IMAGE_WIDTH;
    static const int FAVORITE_IMAGE_HEIGHT;
    
    void *owner;
    GtkImage *gameImage;
    GtkImage *favoriteImage;
    GtkLabel *nameLabel;
    
    Game *game;
    time_t selectedTimestamp;
    time_t activatedTimestamp;
    
    void (*callbackSelect)(GameGridItemWidget *);
    void (*callbackActivate)(GameGridItemWidget *);
    void (*callbackContextMenuFavorite)(GameGridItemWidget *);
    void (*callbackContextMenuDetail)(GameGridItemWidget *);
    void (*callbackContextMenuEdit)(GameGridItemWidget *);
    void (*callbackContextMenuRemove)(GameGridItemWidget *);
    
    /**
     * Signal triggered when the user presses a mouse button over a game in the grid.
     * @param widget
     * @param event
     * @param gameGridItemWidget
     * @return 
     */
    static gboolean signalBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameGridItemWidget);

    /**
     * Signal triggered when the user selects the favorite menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalMenuFavoriteActivate(GtkMenuItem *menuitem, gpointer gameGridItemWidget);
    
    /**
     * Signal triggered when the user selects the information menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalMenuDetailActivate(GtkMenuItem *menuitem, gpointer gameGridItemWidget);
    
    /**
     * Signal triggered when the user selects the edit menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalMenuEditActivate(GtkMenuItem *menuitem, gpointer gameGridItemWidget);
    
    /**
     * Signal triggered when the user selects the remove menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalMenuRemoveActivate(GtkMenuItem *menuitem, gpointer gameGridItemWidget);
};

#endif /* GAMEGRIDITEMWIDGET_H */

