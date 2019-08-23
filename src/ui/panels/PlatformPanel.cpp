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
#include "UiUtils.h"
#include "GameDialog.h"
#include "GameImage.h"
#include "MessageDialog.h"
#include "Database.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Platform.h"
#include "UiThreadBridge.h"
#include "GameLauncher.h"
#include "AddDirectoryDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Utils.h"

#include <iostream>

const int PlatformPanel::GAME_GRID_ITEM_WIDTH = 180;
const int PlatformPanel::GAME_GRID_ITEM_HEIGHT = 300;

const int PlatformPanel::GAME_GRID_ITEM_IMAGE_WIDTH = GAME_GRID_ITEM_WIDTH - 20;
const int PlatformPanel::GAME_GRID_ITEM_IMAGE_HEIGHT = GAME_GRID_ITEM_IMAGE_WIDTH * 1.443;
    
PlatformPanel::PlatformPanel(int platformId)  : Panel("PlatformPanel.ui", "platformBox")
{
    this->platformId = platformId;
    
    gameGridScrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "gameGridScrolledWindow");
    gameGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "gameGridListBox");
    gameDetailBox = (GtkBox *)gtk_builder_get_object (builder, "gameDetailBox");
    
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    gameGridItemIndex = 0;
    selectedGameId = 0;
    gameGridItems = new map<int64_t, GtkWidget *>;
    games = NULL;
    selectGameTimestamp = 0;
    launchGameTimestamp = 0;
    gameDetailWidget = NULL;
    searchQuery = "";
    launchDialog = NULL;
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);
    
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalGameGridSizeAllocate), this);
    
    g_signal_connect (gameGridScrolledWindow, "edge-reached", G_CALLBACK(signalGameGridScrolledWindowEdgeReached), this);
    
    launcherUiThreadBridge = UiThreadBridge::registerBridge(this, callbackGameLauncher);
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, callbackNotification, 1);
}

PlatformPanel::~PlatformPanel()
{    
    UiThreadBridge::unregisterBridge(launcherUiThreadBridge);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    
    gameGridItems->clear();
    delete gameGridItems;
    
    if(games)
    {
        Game::releaseItems(games);
    }
}

void PlatformPanel::showGameDialog(int64_t gameId)
{
    GameDialog *gameDialog = new GameDialog(platformId, gameId);   
    if(gameDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        // New game
        if(!gameId)
        {
            // Notifies of the new game to any object listening for platform changes
            CallbackResult *callbackResult = new CallbackResult(NULL);
            callbackResult->setType(NOTIFICATION_PLATFORM_UPDATED);
            callbackResult->setData(new Platform(platformId));
            NotificationManager::getInstance()->postNotification(callbackResult);                


            gameGridItemIndex = 0;
            loadGridPage();
        }
        // Update game
        else
        {
            updateGame(gameId);
            selectGame(gameId);
        }        
    }
    GameDialog::deleteWhenReady(gameDialog);
}

void PlatformPanel::updateGames(string searchQuery)
{
    this->searchQuery  = searchQuery;
    
    gameGridItemIndex = 0;
    loadGridPage();
}

void PlatformPanel::showAddDirectoryDialog()
{
    AddDirectoryDialog *addDirectoryDialog = new AddDirectoryDialog(platformId);   
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
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    games = Game::getItems(sqlite, platformId, searchQuery);
    Database::getInstance()->release();
}

void PlatformPanel::loadGridPage()
{
    if(!isShown)
    {
        return;
    }

    if(gameGridItemIndex == 0)
    {
        if(gameGridItems->size() > 0)
        {
            gameGridItems->clear();
            UiUtils::getInstance()->clearContainer(GTK_CONTAINER(gameGridListBox), 1);
        }
        
        loadGames();
    }
     
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(gameGridScrolledWindow));
    int columns = width / GAME_GRID_ITEM_WIDTH;
    if(!columns)
    {
        return;
    }

    int height = gtk_widget_get_allocated_height(GTK_WIDGET(gameGridScrolledWindow));
    int rows = height / GAME_GRID_ITEM_HEIGHT;    
    if(!rows)
    {
        return;
    }
    rows *= 2;
        
    for(int row = 0; row < rows; row++)
    {
        if(gameGridItemIndex >= games->size())
        {
            break;
        }
        
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(gameGridItemIndex < games->size())
            {
                Game *game = Game::getItem(games, gameGridItemIndex);
                
                GtkBuilder *gameGridItemBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "GameGridItemBox.ui").c_str());
                GtkEventBox *gameGridItemBox = (GtkEventBox *)gtk_builder_get_object (gameGridItemBuilder, "gameGridItemBox");
                GtkImage *gameImage = (GtkImage *)gtk_builder_get_object (gameGridItemBuilder, "gameImage");
                GtkLabel *nameLabel = (GtkLabel *)gtk_builder_get_object (gameGridItemBuilder, "nameLabel");

                gtk_widget_set_name(GTK_WIDGET(gameGridItemBox), to_string(game->getId()).c_str());                                
                g_signal_connect (gameGridItemBox, "button-press-event", G_CALLBACK(signalGameItemBoxButtonPressedEvent), this);


                gtk_label_set_text(nameLabel, game->getName().c_str());
                
                sqlite3 *sqlite = Database::getInstance()->acquire();
                GameImage *primaryImage = GameImage::getPrimaryImage(sqlite, game->getId());
                Database::getInstance()->release();
                
                if(primaryImage)
                {
                    if(Utils::getInstance()->fileExists(primaryImage->getThumbnailFileName()))
                    {
                        UiUtils::getInstance()->loadImage(gameImage, primaryImage->getThumbnailFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                    }
                    else if(Utils::getInstance()->fileExists(primaryImage->getFileName()))
                    {
                        UiUtils::getInstance()->loadImage(gameImage, primaryImage->getFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                    }
                    else
                    {
                        UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                    }
                    delete primaryImage;
                }
                else
                {
                    UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
                }
                gtk_widget_set_size_request(GTK_WIDGET(gameImage), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);

                gtk_widget_set_size_request(GTK_WIDGET(gameGridItemBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(gameGridItemBox), 1, 1, 0);
                
                if(selectedGameId == game->getId())
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(gameGridItemBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameGridItems->insert(pair<int64_t, GtkWidget *>(game->getId(), GTK_WIDGET(gameGridItemBox)));
                
                gameGridItemIndex++;
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
        }        
        
        gtk_container_add (GTK_CONTAINER(gameGridListBox), GTK_WIDGET(rowBox));
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }

    
    // Force redraw the entire widget
    gtk_widget_hide(GTK_WIDGET(getPanelBox()));
    gtk_widget_show_all(GTK_WIDGET(getPanelBox()));
}

void PlatformPanel::updateGame(int64_t gameId)
{
    if(games == NULL)
    {
        return;
    }
    
    Game *game = NULL;
    for(unsigned int c = 0; c < games->size(); c++)
    {
        game = Game::getItem(games, c);                
        if(game->getId() == gameId)
        {
            sqlite3 *sqlite = Database::getInstance()->acquire();
            game->load(sqlite);
            Database::getInstance()->release();

            break;
        }
    }

    if(gameGridItems->find(gameId) != gameGridItems->end())
    {
        GtkWidget *gameGridItemBox = gameGridItems->at(gameId);
        GtkImage *gameImage = (GtkImage *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gameGridItemBox), "gameImage");
        GtkLabel *nameLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gameGridItemBox), "nameLabel");

        gtk_widget_set_name(GTK_WIDGET(gameGridItemBox), to_string(game->getId()).c_str());
        gtk_label_set_text(nameLabel, game->getName().c_str());

        sqlite3 *sqlite = Database::getInstance()->acquire();
        GameImage *primaryImage = GameImage::getPrimaryImage(sqlite, game->getId());
        Database::getInstance()->release();    
        if(primaryImage && Utils::getInstance()->fileExists(primaryImage->getFileName()))
        {
            if(Utils::getInstance()->fileExists(primaryImage->getThumbnailFileName()))
            {
                UiUtils::getInstance()->loadImage(gameImage, primaryImage->getThumbnailFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
            }
            else if(Utils::getInstance()->fileExists(primaryImage->getFileName()))
            {
                UiUtils::getInstance()->loadImage(gameImage, primaryImage->getFileName(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
            }
            else
            {
                UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
            }
            delete primaryImage;
        }
        else
        {
            UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageLogo(), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);
        }
        gtk_widget_set_size_request(GTK_WIDGET(gameImage), GAME_GRID_ITEM_IMAGE_WIDTH, GAME_GRID_ITEM_IMAGE_HEIGHT);

        gtk_widget_set_size_request(GTK_WIDGET(gameGridItemBox), GAME_GRID_ITEM_WIDTH, GAME_GRID_ITEM_HEIGHT);
    }            
}

void PlatformPanel::selectGame(int64_t gameId)
{    
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_NORMAL, 1);
        }
    }
        
    selectedGameId = gameId;
    if(selectedGameId)
    {
        if(gameGridItems->find(selectedGameId) != gameGridItems->end())
        {
            GtkWidget *gameGridItemBox = gameGridItems->at(selectedGameId);
            gtk_widget_set_state_flags(gameGridItemBox, GTK_STATE_FLAG_SELECTED, 1);        
        }        
    }
    
    if(gameDetailWidget)
    {
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(gameDetailBox), 0);
        delete gameDetailWidget;
        gameDetailWidget = NULL;
    }
    
    if(selectedGameId)
    {
        gameDetailWidget = new GameDetailWidget(selectedGameId);
        gtk_container_add(GTK_CONTAINER(gameDetailBox), gameDetailWidget->getRootWidget());
    }
}

void PlatformPanel::removeGame(int64_t gameId)
{
    sqlite3 *sqlite = Database::getInstance()->acquire();
    Game *game = new Game(gameId);
    game->load(sqlite);
    Database::getInstance()->release();
    
    MessageDialog *messageDialog = new MessageDialog("Sure you want to remove \"" + game->getName() + "\"?", "Remove", "Cancel");   
    if(messageDialog->execute() == GTK_RESPONSE_YES)
    {
        sqlite = Database::getInstance()->acquire();
        game->remove(sqlite);
        Database::getInstance()->release();
                
        
        selectGame(0);
        gameGridItemIndex = 0;
        loadGridPage();
        
        // Notifies of the removed game to any object listening for platform changes
        CallbackResult *callbackResult = new CallbackResult(NULL);
        callbackResult->setType(NOTIFICATION_PLATFORM_UPDATED);
        callbackResult->setData(new Platform(platformId));
        NotificationManager::getInstance()->postNotification(callbackResult); 

    }
    delete messageDialog;
    delete game;
}

void PlatformPanel::launchGame(int64_t gameId)
{
    cout << __FUNCTION__ << endl;
    
    launchDialog = new LaunchDialog(gameId);
    
    GameLauncher::getInstance()->launch(gameId, launcherUiThreadBridge, UiThreadBridge::callback);
    
    launchDialog->execute();
    delete launchDialog;
    
    launchDialog = NULL;
}

void PlatformPanel::signalGameGridSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer platformPanel)
{    
    if(((PlatformPanel *)platformPanel)->isDestroyed())
    {
        return;
    }
    
    if(((PlatformPanel *)platformPanel)->panelWidth != allocation->width || ((PlatformPanel *)platformPanel)->panelHeight != allocation->height)
    {        
        ((PlatformPanel *)platformPanel)->panelWidth = allocation->width;
        ((PlatformPanel *)platformPanel)->panelHeight = allocation->height;
        
        ((PlatformPanel *)platformPanel)->gameGridItemIndex = 0;
        ((PlatformPanel *)platformPanel)->loadGridPage();
    }
}

void PlatformPanel::signalGameGridScrolledWindowEdgeReached(GtkScrolledWindow* scrolledWindow, GtkPositionType positionType, gpointer platformPanel)
{        
    if(positionType == GTK_POS_TOP)
    {
        
    }
    else if(positionType == GTK_POS_BOTTOM)
    {
        ((PlatformPanel *)platformPanel)->loadGridPage();
    }
}

gboolean PlatformPanel::signalGameItemBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer platformPanel)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        int64_t gameId = atol(gtk_widget_get_name(widget));
        time_t now = time(NULL);
        
        if(((PlatformPanel *)platformPanel)->selectedGameId == gameId)
        {
            if(((PlatformPanel *)platformPanel)->selectGameTimestamp == now && (now - ((PlatformPanel *)platformPanel)->launchGameTimestamp) > 2)
            {
                ((PlatformPanel *)platformPanel)->launchGameTimestamp = now;
                ((PlatformPanel *)platformPanel)->launchGame(gameId);
            }
        }
        else
        {
            ((PlatformPanel *)platformPanel)->launchGameTimestamp = 0;
            ((PlatformPanel *)platformPanel)->selectGame(gameId);
        }
        ((PlatformPanel *)platformPanel)->selectGameTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        ((PlatformPanel *)platformPanel)->selectGame(atoi(gtk_widget_get_name(widget)));

        GtkWidget *menu = gtk_menu_new();
        GtkWidget *editMenuitem = gtk_menu_item_new_with_label("Edit");
        GtkWidget *removeMenuitem = gtk_menu_item_new_with_label("Remove");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), editMenuitem);
        g_signal_connect(editMenuitem, "activate", G_CALLBACK(signalGameMenuEditActivate), platformPanel);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), removeMenuitem);
        g_signal_connect(removeMenuitem, "activate", G_CALLBACK(signalGameMenuRemoveActivate), platformPanel);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    return TRUE;
}

void PlatformPanel::signalGameMenuEditActivate(GtkMenuItem* menuitem, gpointer platformPanel)
{
    ((PlatformPanel *)platformPanel)->showGameDialog(((PlatformPanel *)platformPanel)->selectedGameId);
}

void PlatformPanel::signalGameMenuRemoveActivate(GtkMenuItem* menuitem, gpointer platformPanel)
{
    ((PlatformPanel *)platformPanel)->removeGame(((PlatformPanel *)platformPanel)->selectedGameId);
}

void PlatformPanel::signalShow(GtkWidget* widget, gpointer platformPanel)
{
    if(!((PlatformPanel *)platformPanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(50, callbackFirstShowHackyTimeout, platformPanel);        
    }
}

gint PlatformPanel::callbackFirstShowHackyTimeout(gpointer platformPanel)
{
    ((PlatformPanel *)platformPanel)->isShown = 1;
    ((PlatformPanel *)platformPanel)->loadGridPage();
    return 0;
}

void PlatformPanel::callbackNotification(CallbackResult *callbackResult)
{
    try
    {
        PlatformPanel *platformPanel = (PlatformPanel *)callbackResult->getRequester();        
        if(platformPanel->isDestroyed())
        {
            return;
        }

        if(callbackResult->getType().compare(NOTIFICATION_GAME_UPDATED) == 0)
        {
            Game *game = (Game *)callbackResult->getData();
            ((PlatformPanel *)platformPanel)->updateGame(game->getId());
        }
    }
    catch(exception ex)
    {
        
    }
}

void PlatformPanel::callbackGameLauncher(CallbackResult *callbackResult)
{
    PlatformPanel *platformPanel = (PlatformPanel *)callbackResult->getRequester();
    
    string message = "";
    int activity = 0;
    if(callbackResult->getError())
    {
        if(callbackResult->getError() == GameLauncher::ERROR_BUSY)
        {
            message = "There is another process running";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_FILE_NOT_FOUND)
        {
            message = "File not found";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE)
        {
            message = "Decompressing/Unpacking failed";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE_NOT_SUPPORTED)
        {
            message = "Compressed/Packed file format not supported";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_OTHER)
        {
            message = "An unknown error happened";
        }
        
        activity = 0;
    }
    else
    {
        if(callbackResult->getStatus() == GameLauncher::STATE_IDLE)
        {
            activity = 1;
            message = "Preparing...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_INFLATING)
        {
            activity = 1;
            message = "Decompressing/Unpacking...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_RUNNING)
        {
            activity = 1;
            message = "Running...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATE_FINISHED)
        {
            activity = 0;
            message = "Execution finished";
        }
    }
    platformPanel->launchDialog->setStatus(activity, message, callbackResult->getProgress());
}
