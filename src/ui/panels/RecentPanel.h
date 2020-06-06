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
 * File:   RecentPanel.h
 * Author: ram
 *
 * Created on June 5, 2020, 10:20 PM
 */

#ifndef RECENTPANEL_H
#define RECENTPANEL_H

#include "Panel.h"
#include "GameGridItemWidget.h"
#include "GameActivity.h"
#include "CallbackResult.h"

#include <map>
#include <list>

class RecentPanel : public Panel
{
public:
    RecentPanel(GtkWindow *parentWindow); 

private:
    GtkScrolledWindow *gameGridScrolledWindow;
    GtkListBox *gameGridListBox;

    list<GameActivity *> *gameActivities;
    map<int64_t, GameGridItemWidget *> *gameGridItems;
    int isShown;
    int panelWidth;
    int panelHeight;
    unsigned int gameGridItemIndex;
    int64_t selectedGameId;

    virtual ~RecentPanel();
    
    
    /**
     * Loads the game grid
     */
    void loadGridPage();
    
    /**
     * Shows the game dialog for editing.
     * @param gameId 0 is for a new game.
     */
    void showGameEditDialog(int64_t gameId);
    
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
     * Launches a game
     * @param gameId
     */
    void launchGame(int64_t gameId);
    
    /**
     * Shows the game detail dialog.
     * @param gameId
     */
    void showGameDetailDialog(int64_t gameId);    
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetActive(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuDetailSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * Signal triggered when the gameGridListBox "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param recentPanel
     */
    static void signalGameGridSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer recentPanel);
        
    /**
     * Signal triggered when the gameGridScrolledWindow "edge-reached" event happens.
     * @param scrolledWindow
     * @param positionType
     * @param recentPanel
     */
    static void signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow *scrolledWindow, GtkPositionType positionType, gpointer recentPanel);        
    
    /**
     * 
     * @param widget
     * @param recentPanel
     */
    static void signalShow(GtkWidget *widget, gpointer recentPanel);
    
    
    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the first draw of the grid.
     * @param recentPanel
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer recentPanel);
    
    /**
     * 
     * @param callbackResult
     */
    static void callbackNotification(CallbackResult *callbackResult);
};

#endif /* RECENTPANEL_H */

