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
 * File:   HomePanel.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 8:49 PM
 */

#include "HomePanel.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Game.h"
#include "GameDetailDialog.h"
#include "GameDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Build.h"
#include "Logger.h"


HomePanel::HomePanel(GtkWindow *parentWindow) : Panel(parentWindow, "HomePanel.ui", "homeBox")
{
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    selectedGameId = 0;
    gameActivities = NULL;
    gameGridItems = new map<int64_t, GameGridItemWidget *>;
    
    recentsBox = (GtkBox *)gtk_builder_get_object (builder, "recentsBox");
    recentsGridBox = (GtkBox *)gtk_builder_get_object (builder, "recentsGridBox");
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    versionLabel = (GtkLabel *)gtk_builder_get_object (builder, "versionLabel");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    
    gtk_label_set_text(versionLabel, (string("Version ") + string(BUILD_VERSION)).c_str());
    gtk_label_set_markup(informationLabel, Utils::getInstance()->getFileContents(Asset::getInstance()->getHomePml()).c_str());
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalRecentsGridSizeAllocate), this);
    
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogo(), 300, 300);    
}

HomePanel::~HomePanel()
{    
    if(gameActivities)
    {
        GameActivity::releaseItems(gameActivities);
    }
    
    for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
    {
        delete item->second;
    }    
    gameGridItems->clear();
    delete gameGridItems;
}

void HomePanel::loadRecentsGrid()
{
    if(!isShown)
    {
        return;
    }

    for (map<int64_t, GameGridItemWidget *>::iterator item = gameGridItems->begin(); item != gameGridItems->end(); item++)
    {
        delete item->second;
    }
    gameGridItems->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(recentsGridBox), 1);
    
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(getPanelBox()));
    unsigned int columns = width / GameGridItemWidget::GAME_GRID_ITEM_WIDTH;
    if(!columns)
    {
        gtk_widget_hide(GTK_WIDGET(recentsBox));
        return;
    }    

    if(gameActivities)
    {
        GameActivity::releaseItems(gameActivities);
    }    
    gameActivities = GameActivity::getRecentItems();
    
    if(gameActivities->size() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(recentsBox));
        return;
    }        
    
    for(unsigned int column = 0; column < columns; column++)
    {
        if(column < gameActivities->size())
        {
            GameActivity *gameActivity = GameActivity::getItem(gameActivities, column);
            Game *game = new Game(gameActivity->getGameId());            
            game->load();
            
            GameGridItemWidget *gameGridItemWidget = new GameGridItemWidget(this, game);
            gameGridItemWidget->setCallbackSelect(onGameGridItemWidgetSelect);
            gameGridItemWidget->setCallbackActivate(onGameGridItemWidgetActive);
            gameGridItemWidget->setCallbackContextMenuFavorite(onGameGridItemWidgetMenuFavoriteSelect);
            gameGridItemWidget->setCallbackContextMenuDetail(onGameGridItemWidgetMenuDetailSelect);
            gameGridItemWidget->setCallbackContextMenuEdit(onGameGridItemWidgetMenuEditSelect);
            
            gtk_box_pack_start(recentsGridBox, gameGridItemWidget->getWidget(), 1, 1, 0);

            if(selectedGameId == game->getId())
            {
                gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);
            }
            gameGridItems->insert(pair<int64_t, GameGridItemWidget *>(game->getId(), gameGridItemWidget));
            
            delete game;
        }
        else
        {
            GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            gtk_widget_set_size_request(GTK_WIDGET(dummyBox), GameGridItemWidget::GAME_GRID_ITEM_WIDTH, GameGridItemWidget::GAME_GRID_ITEM_HEIGHT);
            gtk_box_pack_start(recentsGridBox, GTK_WIDGET(dummyBox), 1, 1, 0);
        }
    }
    
    gtk_widget_show_all(GTK_WIDGET(recentsBox));
}

void HomePanel::launchGame(int64_t gameId)
{
    Logger::getInstance()->debug("HomePanel", __FUNCTION__, to_string(gameId));
    GameDetailDialog *gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);
    gameDetailDialog->launch();
    delete gameDetailDialog;
}

void HomePanel::updateGame(int64_t gameId)
{
    if(gameGridItems->find(gameId) != gameGridItems->end())
    {
        Game *game = new Game(gameId);            
        game->load();
            
        GameGridItemWidget *gameGridItemWidget = gameGridItems->at(gameId);
        gameGridItemWidget->setGame(game);
        
        delete game;
    }
}

void HomePanel::selectGame(int64_t gameId)
{    
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GameGridItemWidget *gameGridItemWidget = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemWidget->getWidget(), GTK_STATE_FLAG_SELECTED, 1);     
        }        
    }
}

void HomePanel::showGameDialog(int64_t gameId)
{
    Game *game = new Game(gameId);            
    game->load();
    
    GameDialog *gameDialog = new GameDialog(GTK_WINDOW(parentWindow), game->getPlatformId(), game->getId());   
    if(gameDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        updateGame(gameId);
        selectGame(gameId);        
    }
    GameDialog::deleteWhenReady(gameDialog);
    
    delete game;
}


void HomePanel::showGameDetail(int64_t gameId)
{
    GameDetailDialog *gameDetailDialog = new GameDetailDialog(GTK_WINDOW(parentWindow), gameId);
    gameDetailDialog->execute();
    delete gameDetailDialog;
}

void HomePanel::onGameGridItemWidgetSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((HomePanel *)gameGridItemWidget->getOwner())->selectGame(gameGridItemWidget->getGame()->getId());
}

void HomePanel::onGameGridItemWidgetActive(GameGridItemWidget *gameGridItemWidget)
{
    ((HomePanel *)gameGridItemWidget->getOwner())->launchGame(gameGridItemWidget->getGame()->getId());
}

void HomePanel::onGameGridItemWidgetMenuFavoriteSelect(GameGridItemWidget *gameGridItemWidget)
{
    
}

void HomePanel::onGameGridItemWidgetMenuDetailSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((HomePanel *)gameGridItemWidget->getOwner())->showGameDetail(gameGridItemWidget->getGame()->getId());
}

void HomePanel::onGameGridItemWidgetMenuEditSelect(GameGridItemWidget *gameGridItemWidget)
{
    ((HomePanel *)gameGridItemWidget->getOwner())->showGameDialog(gameGridItemWidget->getGame()->getId());
}



void HomePanel::signalRecentsGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer homePanel)
{    
    if(((HomePanel *)homePanel)->panelWidth != allocation->width || ((HomePanel *)homePanel)->panelHeight != allocation->height)
    {        
        ((HomePanel *)homePanel)->panelWidth = allocation->width;
        ((HomePanel *)homePanel)->panelHeight = allocation->height;
        
        ((HomePanel *)homePanel)->loadRecentsGrid();
    }
}

void HomePanel::signalShow(GtkWidget* widget, gpointer homePanel)
{    
    if(!((HomePanel *)homePanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(50, callbackFirstShowHackyTimeout, ((HomePanel *)homePanel));        
    }
}

gint HomePanel::callbackFirstShowHackyTimeout(gpointer homePanel)
{    
    ((HomePanel *)homePanel)->isShown = 1;
    ((HomePanel *)homePanel)->loadRecentsGrid();
    return 0;
}


