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
 * File:   GamesPanel.h
 * Author: ram
 *
 * Created on September 5, 2020, 12:58 PM
 */

#ifndef GAMESPANEL_H
#define GAMESPANEL_H

#include "Panel.h"
#include "Game.h"
#include "Notification.h"
#include "GameGridItemWidget.h"

#include <map>
#include <list>

using namespace std;

class GamesPanel : public Panel
{
public:
    
    /**
     * 
     * @param parentWindow Parent GtkWindow.
     */
    GamesPanel(GtkWindow *parentWindow);        
    
    /**
     * 
     */
    virtual ~GamesPanel();
    
    /**
     * Shows the game dialog for adding or editing.
     * @param gameId 0 is for a new game.
     * @param platformId A valid platformId should be passed for a new game, otherwise is ignored.
     */
    void showGameEditDialog(int64_t gameId, int64_t platformId = 0);
        
    
protected:
    GtkScrolledWindow *gameGridScrolledWindow;
    GtkListBox *gameGridListBox;
    
    list<Game *> *games;
    map<int64_t, GameGridItemWidget *> *gameGridItems;
    int panelWidth;
    int panelHeight;
    unsigned int gameGridItemIndex;
    int64_t selectedGameId;
    gulong signalSizeAllocateHandlerId;
    gulong signalSizeEdgeReachedHandlerId;
    gulong loadGridHandlerId;
    gulong gameSelectHandlerId;
    
    /**
     * Loads the game list
     */
    virtual void loadGames();
    
    /**
     * Loads the game grid
     */
    void loadGridPage();
    
    /**
     * Updates the game visual representation in the grid
     * @param gameId
     */
    void updateGame(int64_t gameId);
            
    /**
     * Selects a game
     * @param gameId
     */
    void selectGame(int64_t gameId);
    
    /**
     * Removes a game
     * @param gameId
     */
    void removeGame(int64_t gameId);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuRemoveSelect(GameGridItemWidget *gameGridItemWidget);    
        
    /**
     * 
     * @param notification
     */
    static void onNotification(Notification *notification);   
};

#endif /* GAMESPANEL_H */

