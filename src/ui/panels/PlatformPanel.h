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
 * File:   PlatformPanel.h
 * Author: ram
 *
 * Created on February 8, 2019, 10:04 PM
 */

#ifndef PLATFORMPANEL_H
#define PLATFORMPANEL_H

#include "Panel.h"
#include "Game.h"
#include "CallbackResult.h"
#include "GameGridItemWidget.h"

#include <map>
#include <list>

using namespace std;

/**
 * Panel that handles the games of a platform.
 */
class PlatformPanel : public Panel
{
public:
    
    /**
     * 
     * @param parentWindow Parent GtkWindow.
     * @param platformId Id of the platform.
     */
    PlatformPanel(GtkWindow *parentWindow, int platformId);        
    
    /**
     * Shows the game dialog for adding or editing.
     * @param gameId 0 is for a new game.
     */
    void showGameEditDialog(int64_t gameId);
    
    /**
     * Updates the game grid
     * @param searchQuery Search query
     */
    void updateGames(string searchQuery);
    
    
    /**
     * Shows the add directory dialog
     */
    void showAddDirectoryDialog();
    
private:
    GtkScrolledWindow *gameGridScrolledWindow;
    GtkListBox *gameGridListBox;
        
    int64_t platformId;
    list<Game *> *games;
    map<int64_t, GameGridItemWidget *> *gameGridItems;
    int isShown;
    int panelWidth;
    int panelHeight;
    unsigned int gameGridItemIndex;
    int64_t selectedGameId;
    string searchQuery;
    
    
    virtual ~PlatformPanel();
    
    /**
     * Loads the game list
     */
    void loadGames();
    
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
     * 
     * @param gameGridItemWidget
     */
    static void onGameGridItemWidgetMenuRemoveSelect(GameGridItemWidget *gameGridItemWidget);
    
    /**
     * Signal triggered when the gameGridListBox "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param platformPanel
     */
    static void signalGameGridSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer platformPanel);
        
    /**
     * Signal triggered when the gameGridScrolledWindow "edge-reached" event happens.
     * @param scrolledWindow
     * @param positionType
     * @param data
     */
    static void signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow *scrolledWindow, GtkPositionType positionType, gpointer platformPanel);        
    
    /**
     * 
     * @param widget
     * @param platformPanel
     */
    static void signalShow(GtkWidget *widget, gpointer platformPanel);
    
    
    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the first draw of the grid.
     * @param platformPanel
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer platformPanel);
        
    /**
     * 
     * @param callbackResult
     */
    static void callbackNotification(CallbackResult *callbackResult);
    
};

#endif /* PLATFORMPANEL_H */

