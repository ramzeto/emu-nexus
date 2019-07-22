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
 * File:   HomePanel.h
 * Author: ram
 *
 * Created on May 6, 2019, 8:49 PM
 */

#ifndef HOMEPANEL_H
#define HOMEPANEL_H

#include "Panel.h"
#include "RecentGame.h"
#include "LaunchDialog.h"

#include <list>
#include <map>

using namespace std;

class HomePanel : public Panel
{
public:
    HomePanel();
    virtual ~HomePanel();
private:
    static const int GAME_GRID_ITEM_WIDTH;
    static const int GAME_GRID_ITEM_HEIGHT;
    
    static const int GAME_GRID_ITEM_IMAGE_WIDTH;
    static const int GAME_GRID_ITEM_IMAGE_HEIGHT;
    
    
    GtkBox *recentsBox;
    GtkBox *recentsGridBox;
    GtkLabel *informationLabel;
    
    int isShown;
    int panelWidth;
    int panelHeight;
    int64_t selectedGameId;
    time_t selectGameTimestamp;
    time_t launchGameTimestamp;
    map<int64_t, GtkWidget *> *gameGridItems;
    list<RecentGame *> *recentGames;
    LaunchDialog *launchDialog;
    
    void loadRecentsGrid();
    void launchGame(int64_t gameId);
    void updateGame(int64_t gameId);
    void selectGame(int64_t gameId);

    /**
     * Signal triggered when the recentsGridBox "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param pHomePanel
     */
    static void signalRecentsGridSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer pHomePanel);
    
    /**
     * 
     * @param widget
     * @param pHomePanel
     */
    static void signalShow(GtkWidget *widget, gpointer pHomePanel);

    
    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the first draw of the recents grid.
     * @param pHomePanel
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer pHomePanel);  
    
    /**
     * Signal triggered when the user presses a mouse button over a game in the recents grid.
     * @param widget
     * @param event
     * @param pHomePanel
     * @return 
     */
    static gboolean signalGameItemBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer pHomePanel);
    
    /**
     * 
     * @param pUiThreadHandlerResult
     * @return 
     */
    static int callbackGameLauncher(gpointer pUiThreadHandlerResult);
};

#endif /* HOMEPANEL_H */

