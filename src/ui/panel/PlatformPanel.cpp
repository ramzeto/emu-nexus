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
 * File:   PlatformPanel.cpp
 * Author: ram
 * 
 * Created on February 8, 2019, 10:04 PM
 */

#include "PlatformPanel.h"
#include "AddDirectoryDialog.h"
#include "MainBannerWidget.h"

PlatformPanel::PlatformPanel(GtkWindow *parentWindow, int platformId)  : GamesPanel(parentWindow)
{
    this->platformId = platformId;
    searchQuery = "";
    MainBannerWidget::getInstance()->setBannerType(MainBannerWidget::TYPE_PLATFORM, platformId);
}

int64_t PlatformPanel::getPlatformId()
{
    return platformId;
}

void PlatformPanel::updateGames(string searchQuery)
{
    this->searchQuery  = searchQuery;
    
    gameGridItemIndex = 0;
    loadGridPage();
}

void PlatformPanel::showAddDirectoryDialog()
{
    AddDirectoryDialog *addDirectoryDialog = new AddDirectoryDialog(GTK_WINDOW(parentWindow), platformId);   
    if(addDirectoryDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        
    }
    delete addDirectoryDialog;
}

void PlatformPanel::loadGames()
{
    if(games)
    {
        Game::releaseItems(games);
    }
    
    games = Game::getItems(platformId, searchQuery);
}


