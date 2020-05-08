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
 * File:   MainWindow.cpp
 * Author: ram
 * 
 * Created on February 5, 2019, 11:34 PM
 */

#include "MainWindow.h"
#include "UiUtils.h"
#include "PlatformPanel.h"
#include "HomePanel.h"
#include "MessageDialog.h"
#include "PlatformDialog.h"
#include "PlatformImage.h"
#include "FirstSetupPanel.h"
#include "Build.h"
#include "Preferences.h"
#include "Genre.h"
#include "Game.h"
#include "thegamesdb.h"
#include "ElasticsearchProcess.h"
#include "SerialProcessExecutor.h"
#include "Asset.h"
#include "Platform.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Directory.h"
#include "ParseDirectoryProcess.h"
#include "DownloadGameImagesProcess.h"
#include "Utils.h"
#include "UiThreadBridge.h"
#include "panels/FavoritePanel.h"


#include <string>
#include <iostream>
#include <typeinfo>

using namespace std;

const int MainWindow::PLATFORM_IMAGE_WIDTH = 100;
const int MainWindow::PLATFORM_IMAGE_HEIGHT = 100;

MainWindow::MainWindow()
{
    currentPanel = NULL;
    platforms = NULL;
    selectedPlatformId = 0;
    
    builder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "MainWindow.ui").c_str());

    mainWindow = (GtkApplicationWindow *)gtk_builder_get_object (builder, "mainWindow");    
    g_signal_connect(G_OBJECT(mainWindow), "configure-event", G_CALLBACK(signalMainWindowConfigureEvent), this);    
    g_signal_connect (mainWindow, "destroy", G_CALLBACK (gtk_main_quit), this);
    
    // Configures the main window depending on the saved preferences
    gtk_window_set_default_size(GTK_WINDOW(mainWindow), Preferences::getInstance()->getWindowWidth(), Preferences::getInstance()->getWindowHeight());
    
    if(Preferences::getInstance()->getMaximized())
    {
        gtk_window_maximize(GTK_WINDOW(mainWindow));
    }
    else
    {        
        gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);        
    }    

    addPlatformButton = (GtkButton *)gtk_builder_get_object (builder, "addPlatformButton");
    g_signal_connect (addPlatformButton, "clicked", G_CALLBACK (signalAddPlatformButtonClicked), this);
    
    gameSearchEntry = (GtkSearchEntry *)gtk_builder_get_object (builder, "gameSearchEntry");
    g_signal_connect (gameSearchEntry, "search-changed", G_CALLBACK (signalGameSearchEntrySearchChanged), this);
    
    addGameButton = (GtkButton *)gtk_builder_get_object (builder, "addGameButton");
    g_signal_connect (addGameButton, "clicked", G_CALLBACK (signalAddGameButtonClicked), this);    
    
    addDirectoryButton = (GtkButton *)gtk_builder_get_object (builder, "addDirectoryButton");
    g_signal_connect (addDirectoryButton, "clicked", G_CALLBACK (signalAddDirectoryButtonClicked), this);    
    
    platformListContainerBox = (GtkBox *)gtk_builder_get_object (builder, "platformListContainerBox");
    platformListBox = (GtkListBox *)gtk_builder_get_object (builder, "platformListBox");
    g_signal_connect (platformListBox, "row-selected", G_CALLBACK(signalPlatformListRowSelected), this);

    contentBox = (GtkBox *)gtk_builder_get_object (builder, "contentBox");
    
    processBox = (GtkBox *)gtk_builder_get_object (builder, "processBox");
    processSpinner = (GtkSpinner *)gtk_builder_get_object (builder, "processSpinner");;
    processTitleLabel = (GtkLabel *)gtk_builder_get_object (builder, "processTitleLabel");
    processMessageLabel = (GtkLabel *)gtk_builder_get_object (builder, "processMessageLabel");   
    processProgressBar = (GtkProgressBar *)gtk_builder_get_object (builder, "processProgressBar");
    
    gtk_widget_show_all(GTK_WIDGET(mainWindow));
    
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    gtk_spinner_stop(processSpinner);
    gtk_label_set_text(processTitleLabel, "");
    gtk_label_set_text(processMessageLabel, "");
    gtk_widget_hide(GTK_WIDGET(processProgressBar));
    
    
    processUiThreadBridge = UiThreadBridge::registerBridge(this, callbackSerialProcessStatus);
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_PLATFORM_UPDATED, this, callbackNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_ADD_DIRECTORY, this, callbackNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_FAVORITES_UPDATED, this, callbackNotification, 1);
    
    startGui();
}


MainWindow::~MainWindow()
{
    Platform::releaseItems(platforms);
    
    if (mainWindow)
    {
        gtk_widget_destroy(GTK_WIDGET(mainWindow));
        mainWindow = NULL;
    }

    if (builder)
    {
        builder = NULL;
    }
    
    UiThreadBridge::unregisterBridge(processUiThreadBridge);
}

void MainWindow::startGui()
{
    list<Genre *> *genres = Genre::getItems("");    
            
    // If genres exists, means that TheGamesDB preloaded data is present.
    if(genres->size() > 0)
    {
        if(TheGamesDB::Elasticsearch::getInstance()->getStatus() != TheGamesDB::Elasticsearch::STATUS_OK)
        {
            ElasticsearchProcess *elasticsearchProcess = new ElasticsearchProcess(processUiThreadBridge, UiThreadBridge::callback);
            SerialProcessExecutor::getInstance()->schedule(elasticsearchProcess);
            
            gtk_widget_hide(GTK_WIDGET(addPlatformButton));
        }
        else
        {
            gtk_widget_show_all(GTK_WIDGET(addPlatformButton));
        }
        
        gtk_widget_show_all(GTK_WIDGET(platformListContainerBox));
        
        loadPlatformList();
        showPanel(new HomePanel(GTK_WINDOW(mainWindow)));
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(addPlatformButton));
        gtk_widget_hide(GTK_WIDGET(platformListContainerBox));
        
        FirstSetupPanel *firstSetupPanel = new FirstSetupPanel(GTK_WINDOW(mainWindow));
        firstSetupPanel->setOnSetupReadyCallback(onSetupReadyCallback, this);
        showPanel(firstSetupPanel);
    }
    Genre::releaseItems(genres);
}

void MainWindow::showPanel(Panel* panel)
{
    if(currentPanel)
    {
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 0);
        currentPanel->destroy();
        
        // @TODO Fix this memory leak. For some reason the application crashes randomly when the panel is freed.
        //delete currentPanel;
    }
    
    currentPanel = panel;
    gtk_container_add(GTK_CONTAINER(contentBox), GTK_WIDGET(currentPanel->getPanelBox()));
    
    currentPanel->show();
}

void MainWindow::showHome()
{
    selectedPlatformId = 0;
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    showPanel(new HomePanel(GTK_WINDOW(mainWindow)));    
}

void MainWindow::showFavorites()
{
    selectedPlatformId = 0;
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    showPanel(new FavoritePanel(GTK_WINDOW(mainWindow)));    
}

void MainWindow::showPlatformDialog(int64_t platformId)
{
    PlatformDialog *platformDialog = new PlatformDialog(GTK_WINDOW(mainWindow), platformId);
    if(platformDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        if(platformId)
        {
            updatePlatform(platformId);            
        }   
        else
        {
            loadPlatformList();
        }
    }
    PlatformDialog::deleteWhenReady(platformDialog);
}


void MainWindow::loadPlatformList()
{
    if(platforms)
    {
        Platform::releaseItems(platforms);
    }

    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(platformListBox), 0);

    platforms = Platform::getItems();
    for(unsigned int index = 0; index < platforms->size() + 2; index++)
    {
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "PlatformRowBox.ui").c_str());
        GtkWidget *platformRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "platformRowBox");
        GtkImage *image = (GtkImage *)gtk_builder_get_object (rowBuilder, "image");                                        
        GtkLabel *nameLabel = (GtkLabel *)gtk_builder_get_object (rowBuilder, "nameLabel");
        GtkLabel *gamesLabel = (GtkLabel *)gtk_builder_get_object (rowBuilder, "gamesLabel");
        
        // Home
        if(index == 0)
        {
            gtk_label_set_text(nameLabel, "EMU-nexus");
            gtk_label_set_text(gamesLabel, "Home");
            UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageHome(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
            
            gtk_widget_set_name(platformRowBox, to_string(0).c_str());
        }
        // Favorites
        else if(index == 1)
        {
            list<GameFavorite *> *gameFavorites = GameFavorite::getItems();
            
            gtk_label_set_text(nameLabel, "Favorites");
            UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageFavorite(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
            
            gtk_label_set_text(gamesLabel, string(to_string(gameFavorites->size()) + " items").c_str());
            
            GameFavorite::releaseItems(gameFavorites);
        }
        // Platform
        else
        {
            Platform *platform = Platform::getItem(platforms, index - 2);            
            list<Game *> *games = Game::getItems(platform->getId(), "");
            
            gtk_label_set_text(nameLabel, platform->getName().c_str());
            gtk_label_set_text(gamesLabel, string(to_string(games->size()) + " items").c_str());
            
            Game::releaseItems(games);

            PlatformImage *platformImage = PlatformImage::getPrimaryImage(platform->getId());        
            if(platformImage)
            {
                if(Utils::getInstance()->fileExists(platformImage->getThumbnailFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, platformImage->getThumbnailFileName(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                else if(Utils::getInstance()->fileExists(platformImage->getFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, platformImage->getFileName(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                else
                {
                    UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageLogo(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                delete platformImage;
            }
            else
            {
                UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageLogo(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
            }
            gtk_widget_set_name(platformRowBox, to_string(platform->getId()).c_str());
        }
        gtk_widget_set_size_request(GTK_WIDGET(image), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
        
        g_signal_connect (platformRowBox, "button-press-event", G_CALLBACK(signalPlatformListRowPressedEvent), this);

        gtk_container_add (GTK_CONTAINER(platformListBox), platformRowBox);
        gtk_widget_show_all(platformRowBox);
    }    
    
    if(selectedPlatformId)
    {
        selectPlatform(selectedPlatformId);
    }
}

void MainWindow::updateFavorites()
{
    list<GameFavorite *> *gameFavorites = GameFavorite::getItems();            
    GtkLabel *gamesLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, 1)), "gamesLabel");
    gtk_label_set_text(gamesLabel, string(to_string(gameFavorites->size()) + " items").c_str());            
    GameFavorite::releaseItems(gameFavorites);            
}

void MainWindow::updatePlatform(int64_t platformId)
{
    for(unsigned int index = 2; index < platforms->size() + 2; index++)
    {
        Platform *platform = Platform::getItem(platforms, index - 2);
        
        if(platform->getId() == platformId)
        {
            platform->load();
            
            GtkImage *image = (GtkImage *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "image");
            GtkLabel *nameLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "nameLabel");
            GtkLabel *gamesLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "gamesLabel");
            
            list<Game *> *games = Game::getItems(platform->getId(), "");
            
            gtk_label_set_text(nameLabel, platform->getName().c_str());
            gtk_label_set_text(gamesLabel, string(to_string(games->size()) + " items").c_str());
            
            Game::releaseItems(games);

            PlatformImage *platformImage = PlatformImage::getPrimaryImage(platform->getId());        
            if(platformImage)
            {
                if(Utils::getInstance()->fileExists(platformImage->getThumbnailFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, platformImage->getThumbnailFileName(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                else if(Utils::getInstance()->fileExists(platformImage->getFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, platformImage->getFileName(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                else
                {
                    UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageLogo(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
                }
                delete platformImage;
            }
            else
            {
                UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageLogo(), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);
            }                        
            break;
        }
    }        
}

void MainWindow::selectPlatform(int64_t platformId)
{
    for(unsigned int index = 2; index < platforms->size() + 2; index++)
    {
        Platform *platform = Platform::getItem(platforms, index - 2);
        
        if(platform->getId() == platformId)
        {
            // Prevents recursion
            if(gtk_list_box_get_selected_row(platformListBox) != gtk_list_box_get_row_at_index(platformListBox, index))
            {
                gtk_list_box_select_row(platformListBox, gtk_list_box_get_row_at_index(platformListBox, index));
            }
            break;
        }
    }
    selectedPlatformId = platformId;
    
    gtk_widget_show(GTK_WIDGET(addGameButton));
    gtk_widget_show(GTK_WIDGET(addDirectoryButton));
    gtk_widget_show(GTK_WIDGET(gameSearchEntry));
    
    showPanel(new PlatformPanel(GTK_WINDOW(mainWindow), selectedPlatformId));
    ((PlatformPanel *)currentPanel)->updateGames(string(gtk_entry_get_text(GTK_ENTRY(gameSearchEntry))));
}

void MainWindow::removePlatform(int64_t platformId)
{
    Platform *platform = new Platform(platformId);
    platform->load();
    
    MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow), "Sure you want to remove \"" + platform->getName() + "\"?", "Remove", "Cancel");   
    if(messageDialog->execute() == GTK_RESPONSE_YES)
    {
        showHome();
        
        platform->remove();
        
        loadPlatformList();                
    }
    delete messageDialog;
    delete platform;
}


void MainWindow::onSetupReadyCallback(void* mainWindow)
{
    ((MainWindow *)mainWindow)->startGui();
}


void MainWindow::signalMainWindowConfigureEvent(GtkWindow* window, GdkEvent* event, gpointer mainWindow)
{
    if(gtk_window_is_maximized(window))
    {
        Preferences::getInstance()->setMaximized(1);
    }
    else
    {
        Preferences::getInstance()->setMaximized(0);
        Preferences::getInstance()->setWindowWidth(event->configure.width);
        Preferences::getInstance()->setWindowHeight(event->configure.height);
    }
            
    Preferences::getInstance()->save();
}

void MainWindow::signalPlatformListRowSelected(GtkListBox* listBox, GtkWidget* row, gpointer mainWindow)
{
    if(!mainWindow || !row || !GTK_IS_LIST_BOX_ROW(row))
    {
        return;
    }
        
    int position = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW(row));
    
    // Home
    if(position == 0)
    {
        ((MainWindow *)mainWindow)->showHome();
        return;
    }
    // Favorite
    if(position == 1)
    {
        ((MainWindow *)mainWindow)->showFavorites();
        return;
    }
    
    Platform *platform = Platform::getItem(((MainWindow *)mainWindow)->platforms, position - 2);
    ((MainWindow *)mainWindow)->selectPlatform(platform->getId());
}

gboolean MainWindow::signalPlatformListRowPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer mainWindow)
{    
    // Right click
    if(event->button.button == 3)
    {
        int64_t platformId = atol(gtk_widget_get_name(widget));
        
        // Home of Favorites
        if(!platformId)
        {
            return FALSE;
        }
        
        ((MainWindow *)mainWindow)->selectPlatform(platformId);
        
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *editMenuitem = gtk_menu_item_new_with_label("Edit");
        GtkWidget *removeMenuitem = gtk_menu_item_new_with_label("Remove");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), editMenuitem);
        g_signal_connect(editMenuitem, "activate", G_CALLBACK(signalPlatformMenuEditActivate), mainWindow);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), removeMenuitem);
        g_signal_connect(removeMenuitem, "activate", G_CALLBACK(signalPlatformMenuRemoveActivate), mainWindow);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    return FALSE;
}

void MainWindow::signalPlatformMenuEditActivate(GtkMenuItem* menuitem, gpointer mainWindow)
{
    ((MainWindow *)mainWindow)->showPlatformDialog(((MainWindow *)mainWindow)->selectedPlatformId);
}

void MainWindow::signalPlatformMenuRemoveActivate(GtkMenuItem* menuitem, gpointer mainWindow)
{
    ((MainWindow *)mainWindow)->removePlatform(((MainWindow *)mainWindow)->selectedPlatformId);
}

void MainWindow::signalAddPlatformButtonClicked(GtkButton* button, gpointer mainWindow)
{
    ((MainWindow *)mainWindow)->showPlatformDialog(0);
}

void MainWindow::signalAddGameButtonClicked(GtkButton* button, gpointer mainWindow)
{
    ((PlatformPanel *) ((MainWindow *)mainWindow)->currentPanel)->showGameDialog(0);
}

void MainWindow::signalAddDirectoryButtonClicked(GtkButton* button, gpointer mainWindow)
{
    ((PlatformPanel *) ((MainWindow *)mainWindow)->currentPanel)->showAddDirectoryDialog();
}

void MainWindow::signalGameSearchEntrySearchChanged(GtkSearchEntry* searchEntry, gpointer mainWindow)
{
    ((PlatformPanel *) ((MainWindow *)mainWindow)->currentPanel)->updateGames(string(gtk_entry_get_text(GTK_ENTRY(searchEntry))));
}

void MainWindow::callbackSerialProcessStatus(CallbackResult *callbackResult)
{    
    MainWindow *mainWindow = (MainWindow *)callbackResult->getRequester();
    
    if(callbackResult->getStatus() == SerialProcess::STATUS_RUNNING)
    {
        gtk_spinner_start(mainWindow->processSpinner);
        gtk_label_set_text(mainWindow->processTitleLabel, callbackResult->getTitle().c_str());
        gtk_label_set_text(mainWindow->processMessageLabel, callbackResult->getMessage().c_str());
        
        if(callbackResult->getProgress() < 0)
        {
            gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
        }
        else
        {
            gtk_widget_show(GTK_WIDGET(mainWindow->processProgressBar));
            gtk_progress_bar_set_fraction(mainWindow->processProgressBar, ((double)callbackResult->getProgress()) / 100.0);
        }
    }
    else
    {
        gtk_spinner_stop(mainWindow->processSpinner);
        gtk_label_set_text(mainWindow->processTitleLabel, "");
        gtk_label_set_text(mainWindow->processMessageLabel, "");
        gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
        
        if(callbackResult->getType().compare(ElasticsearchProcess::TYPE) == 0)
        {
            gtk_widget_show_all(GTK_WIDGET(mainWindow->addPlatformButton));
            
            if(callbackResult->getStatus() == SerialProcess::STATUS_SUCCESS)
            {
                // Runs a parseDirectoryProcess in case there are pending
                ParseDirectoryProcess *parseDirectoryProcess = new ParseDirectoryProcess(((MainWindow *)mainWindow)->processUiThreadBridge, UiThreadBridge::callback);
                SerialProcessExecutor::getInstance()->schedule(parseDirectoryProcess);
            }
            else
            {
                MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow), "Database failed to start. Please restart EMU-nexus", "Ok", "");   
                messageDialog->execute();
                delete messageDialog;
            }
        }        
        else if(callbackResult->getType().compare(ParseDirectoryProcess::TYPE) == 0)
        {
            // Schedules a process for downloading images
            DownloadGameImagesProcess *downloadGameImagesProcess = new DownloadGameImagesProcess(((MainWindow *)mainWindow)->processUiThreadBridge, UiThreadBridge::callback);
            SerialProcessExecutor::getInstance()->schedule(downloadGameImagesProcess);
        }
    }
}

void MainWindow::callbackNotification(CallbackResult *callbackResult)
{
    MainWindow *mainWindow = (MainWindow *)callbackResult->getRequester();
    if(callbackResult->getType().compare(NOTIFICATION_PLATFORM_UPDATED) == 0)
    {
        Platform *platform = (Platform *)callbackResult->getData();        
        mainWindow->updatePlatform(platform->getId());
        
        if(platform->getId() == mainWindow->selectedPlatformId)
        {
            ((PlatformPanel *)mainWindow->currentPanel)->updateGames(string(gtk_entry_get_text(GTK_ENTRY(mainWindow->gameSearchEntry))));
        }
    }
    else if(callbackResult->getType().compare(NOTIFICATION_FAVORITES_UPDATED) == 0)
    {
        mainWindow->updateFavorites();
    }
    else if(callbackResult->getType().compare(NOTIFICATION_ADD_DIRECTORY) == 0)
    {
        ParseDirectoryProcess *parseDirectoryProcess = new ParseDirectoryProcess(mainWindow->processUiThreadBridge, UiThreadBridge::callback);
        SerialProcessExecutor::getInstance()->schedule(parseDirectoryProcess);
    }
}
