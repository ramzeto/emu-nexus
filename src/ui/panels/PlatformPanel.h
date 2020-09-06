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

#include "GamesPanel.h"

using namespace std;

/**
 * Panel that handles the games of a platform.
 */
class PlatformPanel : public GamesPanel
{
public:
    
    /**
     * 
     * @param parentWindow Parent GtkWindow.
     * @param platformId Id of the platform.
     */
    PlatformPanel(GtkWindow *parentWindow, int platformId);
    
    /**
     * 
     * @return platformId.
     */
    int64_t getPlatformId();    
    
    /**
     * Updates the game grid.
     * @param searchQuery Search query.
     */
    void updateGames(string searchQuery);
    
    
    /**
     * Shows the add directory dialog.
     */
    void showAddDirectoryDialog();        
    
protected:
    int64_t platformId;
    string searchQuery;
    
    /**
     * Loads the game list
     */
    void loadGames();            
};

#endif /* PLATFORMPANEL_H */

