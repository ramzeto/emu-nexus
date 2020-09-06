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
 * File:   FavoritePanel.h
 * Author: ram
 *
 * Created on May 8, 2020, 2:05 PM
 */

#ifndef FAVORITEPANEL_H
#define FAVORITEPANEL_H

#include "GamesPanel.h"


class FavoritePanel : public GamesPanel
{
public:
    FavoritePanel(GtkWindow *parentWindow);
    
protected:

    /**
     * Loads the game list
     */
    void loadGames();
};

#endif /* FAVORITEPANEL_H */

