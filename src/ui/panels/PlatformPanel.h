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
#include "GameDetailDialog.h"
#include "UiThreadBridge.h"
#include "CallbackResult.h"

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
    void showGameDialog(int64_t gameId);
    
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
    static const int GAME_GRID_ITEM_WIDTH;
    static const int GAME_GRID_ITEM_HEIGHT;
    
    static const int GAME_GRID_ITEM_IMAGE_WIDTH;
    static const int GAME_GRID_ITEM_IMAGE_HEIGHT;
    
    GtkScrolledWindow *gameGridScrolledWindow;
    GtkListBox *gameGridListBox;
        
    int64_t platformId;
    list<Game *> *games;
    map<int64_t, GtkWidget *> *gameGridItems;
    int isShown;
    int panelWidth;
    int panelHeight;
    unsigned int gameGridItemIndex;
    int64_t selectedGameId;
    time_t selectGameTimestamp;
    time_t launchGameTimestamp;
    string searchQuery;
    GameDetailDialog *gameDetailDialog;
    UiThreadBridge *launcherUiThreadBridge;
    
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
     * Shows a game detail.
     * @param gameId
     */
    void showGameDetail(int64_t gameId);    
    
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
     * Signal triggered when the user presses a mouse button over a game in the grid.
     * @param widget
     * @param event
     * @param platformPanel
     * @return 
     */
    static gboolean signalGameItemBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer platformPanel);

    /**
     * Signal triggered when the user selects the detail menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalGameMenuDetailActivate(GtkMenuItem *menuitem, gpointer platformPanel);
    
    /**
     * Signal triggered when the user selects the edit menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalGameMenuEditActivate(GtkMenuItem *menuitem, gpointer platformPanel);
    
    /**
     * Signal triggered when the user selects the remove menu option on a game in the grid.
     * @param menuitem
     * @param mainWindow
     */
    static void signalGameMenuRemoveActivate(GtkMenuItem *menuitem, gpointer platformPanel);
    
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
    
    /**
     * 
     * @param callbackResult
     */
    static void callbackGameLauncher(CallbackResult *callbackResult);
};

#endif /* PLATFORMPANEL_H */

