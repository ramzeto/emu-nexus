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
 * File:   FavoritePanel.cpp
 * Author: ram
 * 
 * Created on May 8, 2020, 2:05 PM
 */

#include "FavoritePanel.h"
#include "GameFavorite.h"
#include "MainBannerWidget.h"

FavoritePanel::FavoritePanel(GtkWindow *parentWindow) : GamesPanel(parentWindow)
{
    MainBannerWidget::getInstance()->setBannerType(MainBannerWidget::TYPE_FAVORITES);
}

void FavoritePanel::loadGames()
{    
    if(games)
    {
        Game::releaseItems(games);
    }
    
    games = new list<Game *>();
    list<GameFavorite *> *gameFavorites = GameFavorite::getItems();
    for(unsigned int c = 0; c < gameFavorites->size(); c++)
    {
        GameFavorite *gameFavorite = GameFavorite::getItem(gameFavorites, c);
        
        Game *game = new Game(gameFavorite->getGameId());
        game->load();
        games->push_back(game);
    }
    GameFavorite::releaseItems(gameFavorites);
}