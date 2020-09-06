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
 * File:   RecentsPanel.cpp
 * Author: ram
 * 
 * Created on June 5, 2020, 10:20 PM
 */

#include "RecentsPanel.h"
#include "GameActivity.h"
#include "MainBannerWidget.h"

RecentsPanel::RecentsPanel(GtkWindow *parentWindow) : GamesPanel(parentWindow)
{
    MainBannerWidget::getInstance()->setBannerType(MainBannerWidget::TYPE_RECENTS);
}

void RecentsPanel::loadGames()
{    
    if(games)
    {
        Game::releaseItems(games);
    }
    
    games = new list<Game *>();
    list<GameActivity *> *gameActivities = GameActivity::getRecentItems();
    for(unsigned int c = 0; c < gameActivities->size(); c++)
    {
        GameActivity *gameActivity = GameActivity::getItem(gameActivities, c);
        
        Game *game = new Game(gameActivity->getGameId());
        game->load();
        games->push_back(game);
    }
    GameActivity::releaseItems(gameActivities);
}