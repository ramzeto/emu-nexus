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
#include "PlatformEditDialog.h"
#include "PlatformImage.h"
#include "Build.h"
#include "Preferences.h"
#include "Genre.h"
#include "Game.h"
#include "GameFavorite.h"
#include "SerialProcessExecutor.h"
#include "Asset.h"
#include "Platform.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Directory.h"
#include "ParseDirectoryProcess.h"
#include "DownloadGameImagesProcess.h"
#include "GameActivity.h"
#include "FavoritePanel.h"
#include "RecentsPanel.h"
#include "Logger.h"
#include "MainBannerWidget.h"
#include "Utils.h"
#include "SelectFromListDialog.h"
#include "GameLauncher.h"
#include "Logger.h"
#include "Database.h"
#include "SetupDatabaseProcess.h"


#include <string>
#include <typeinfo>

using namespace std;

const int MainWindow::PLATFORM_IMAGE_WIDTH = 75;
const int MainWindow::PLATFORM_IMAGE_HEIGHT = 75;

const int MainWindow::ITEM_HOME_INDEX = 0;
const int MainWindow::ITEM_RECENTS_INDEX = 1;
const int MainWindow::ITEM_FAVORITES_INDEX = 2;
const int MainWindow::ITEM_PLATFORMS_OFFSET = 3;
    
MainWindow::MainWindow()
{
    currentPanel = NULL;
    platforms = NULL;
    selectedPlatformId = 0;
    
    builder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "MainWindow.ui").c_str());

    mainWindow = (GtkApplicationWindow *)gtk_builder_get_object (builder, "mainWindow");
        
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
    gameSearchEntry = (GtkSearchEntry *)gtk_builder_get_object (builder, "gameSearchEntry");
    addGameButton = (GtkButton *)gtk_builder_get_object (builder, "addGameButton");        
    addDirectoryButton = (GtkButton *)gtk_builder_get_object (builder, "addDirectoryButton");        
    platformBox = (GtkBox *)gtk_builder_get_object (builder, "platformBox");
    platformListBox = (GtkListBox *)gtk_builder_get_object (builder, "platformListBox");
    bannerBox = (GtkBox *)gtk_builder_get_object (builder, "bannerBox");
    contentBox = (GtkBox *)gtk_builder_get_object (builder, "contentBox");    
    processBox = (GtkBox *)gtk_builder_get_object (builder, "processBox");
    processSpinner = (GtkSpinner *)gtk_builder_get_object (builder, "processSpinner");;
    processTitleLabel = (GtkLabel *)gtk_builder_get_object (builder, "processTitleLabel");
    processMessageLabel = (GtkLabel *)gtk_builder_get_object (builder, "processMessageLabel");   
    processProgressBar = (GtkProgressBar *)gtk_builder_get_object (builder, "processProgressBar");
    
    
    // Loads the CSS
    GError *error = NULL;
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, (Asset::getInstance()->getCss()).c_str(), &error);
    if(error)
    {
        Logger::getInstance()->error("MainWindow", __FUNCTION__, string(error->message));
    }
    else
    {
        GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(mainWindow));
        gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    //______________
    
    g_signal_connect(G_OBJECT(mainWindow), "configure-event", G_CALLBACK(+[](GtkWindow* window, GdkEvent* event, gpointer mainWindow) -> void {
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
    }), this);
    
    g_signal_connect (mainWindow, "destroy", G_CALLBACK (gtk_main_quit), this);
    
    g_signal_connect (platformListBox, "row-selected", G_CALLBACK(+[](GtkListBox* listBox, GtkWidget* row, gpointer mainWindow) -> void {
        if(!mainWindow || !row || !GTK_IS_LIST_BOX_ROW(row))
        {
            return;
        }

        int position = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW(row));

        // Home
        if(position == ITEM_HOME_INDEX)
        {
            ((MainWindow *)mainWindow)->showHome();
            return;
        }
        // Recents
        else if(position == ITEM_RECENTS_INDEX)
        {
            ((MainWindow *)mainWindow)->showRecents();
            return;
        }
        // Favorites
        else if(position == ITEM_FAVORITES_INDEX)
        {
            ((MainWindow *)mainWindow)->showFavorites();
            return;
        }

        Platform *platform = Platform::getItem(((MainWindow *)mainWindow)->platforms, position - ITEM_PLATFORMS_OFFSET);
        ((MainWindow *)mainWindow)->selectPlatform(platform->getId());
    }), this);
    
    g_signal_connect (addPlatformButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer mainWindow) -> void {
        ((MainWindow *)mainWindow)->showPlatformEditDialog(0);
    }), this);
    
    g_signal_connect (gameSearchEntry, "search-changed", G_CALLBACK (+[](GtkSearchEntry* searchEntry, gpointer mainWindow) -> void {
        ((PlatformPanel *) ((MainWindow *)mainWindow)->currentPanel)->updateGames(string(gtk_entry_get_text(GTK_ENTRY(searchEntry))));
    }), this);
    
    g_signal_connect (addGameButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer mainWindow) -> void {
        PlatformPanel *platformPanel = (PlatformPanel *)((MainWindow *)mainWindow)->currentPanel;
        platformPanel->showGameEditDialog(0, platformPanel->getPlatformId());
    }), this);
    
    g_signal_connect (addDirectoryButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer mainWindow) -> void {
        ((PlatformPanel *) ((MainWindow *)mainWindow)->currentPanel)->showAddDirectoryDialog();
    }), this);    
    
    NotificationManager::getInstance()->registerToNotification(SetupDatabaseProcess::TYPE, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(ParseDirectoryProcess::TYPE, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(DownloadGameImagesProcess::TYPE, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_PLATFORM_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_DIRECTORY_ADDED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_ACTIVITY_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED, this, onNotification, 1);
    
    gtk_container_add(GTK_CONTAINER(bannerBox), GTK_WIDGET(MainBannerWidget::getInstance()->getWidget()));        
    gtk_widget_show_all(GTK_WIDGET(mainWindow));
    
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    gtk_spinner_stop(processSpinner);
    gtk_label_set_text(processTitleLabel, "");
    gtk_label_set_text(processMessageLabel, "");
    gtk_widget_hide(GTK_WIDGET(processProgressBar));           
    
    if(Database::getInstance()->isSetup())
    {
        gtk_widget_hide(GTK_WIDGET(addPlatformButton));
        loadPlatformList();
        
        SetupDatabaseProcess *setupDatabaseProcess = new SetupDatabaseProcess();
        SerialProcessExecutor::getInstance()->schedule(setupDatabaseProcess);
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(addPlatformButton));
        gtk_widget_hide(GTK_WIDGET(platformBox));
    }
    
    showPanel(new HomePanel(GTK_WINDOW(mainWindow)));
}


MainWindow::~MainWindow()
{
    NotificationManager::getInstance()->unregisterToNotification(SetupDatabaseProcess::TYPE, this);
    NotificationManager::getInstance()->unregisterToNotification(ParseDirectoryProcess::TYPE, this);
    NotificationManager::getInstance()->unregisterToNotification(DownloadGameImagesProcess::TYPE, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_PLATFORM_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_DIRECTORY_ADDED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_ACTIVITY_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED, this);
    
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
}

void MainWindow::showPanel(Panel* panel)
{
    if(currentPanel)
    {        
        delete currentPanel;
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
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

void MainWindow::showRecents()
{
    selectedPlatformId = 0;
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    showPanel(new RecentsPanel(GTK_WINDOW(mainWindow)));
}

void MainWindow::showFavorites()
{
    selectedPlatformId = 0;
    gtk_widget_hide(GTK_WIDGET(addGameButton));
    gtk_widget_hide(GTK_WIDGET(addDirectoryButton));
    gtk_widget_hide(GTK_WIDGET(gameSearchEntry));
    
    showPanel(new FavoritePanel(GTK_WINDOW(mainWindow)));    
}

void MainWindow::showPlatformEditDialog(int64_t platformId)
{
    PlatformEditDialog *platformEditDialog = new PlatformEditDialog(GTK_WINDOW(mainWindow), platformId);
    if(platformEditDialog->execute() == GTK_RESPONSE_ACCEPT)
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
    PlatformEditDialog::deleteWhenReady(platformEditDialog);
}


void MainWindow::loadPlatformList()
{
    if(platforms)
    {
        Platform::releaseItems(platforms);
    }

    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(platformListBox), 1);

    platforms = Platform::getItems();
    for(unsigned int index = 0; index < platforms->size() + ITEM_PLATFORMS_OFFSET; index++)
    {
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "PlatformRowBox.ui").c_str());
        GtkWidget *platformRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "platformRowBox");
        GtkImage *image = (GtkImage *)gtk_builder_get_object (rowBuilder, "image");                                        
        GtkLabel *nameLabel = (GtkLabel *)gtk_builder_get_object (rowBuilder, "nameLabel");
        GtkLabel *gamesLabel = (GtkLabel *)gtk_builder_get_object (rowBuilder, "gamesLabel");
        
        // Home
        if(index == ITEM_HOME_INDEX)
        {
            gtk_label_set_text(nameLabel, "EMU-nexus");
            gtk_label_set_text(gamesLabel, "Home");
            
            //https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
            gtk_image_set_from_icon_name(image, "go-home", GTK_ICON_SIZE_DIALOG);
            
            gtk_widget_set_name(platformRowBox, to_string(0).c_str());
        }
        // Recents
        else if(index == ITEM_RECENTS_INDEX)
        {
            gtk_label_set_text(nameLabel, "Recently played");
            
            gtk_image_set_from_icon_name(image, "document-open-recent", GTK_ICON_SIZE_DIALOG);
            
            list<GameActivity *> *recentItems = GameActivity::getRecentItems();
            gtk_label_set_text(gamesLabel, string("(" + to_string(recentItems->size()) + ")").c_str());            
            GameActivity::releaseItems(recentItems);
            
            gtk_widget_set_name(platformRowBox, to_string(0).c_str());
        }        
        // Favorites
        else if(index == ITEM_FAVORITES_INDEX)
        {
            gtk_label_set_text(nameLabel, "Favorites");
            
            gtk_image_set_from_icon_name(image, "emblem-favorite", GTK_ICON_SIZE_DIALOG);
            
            list<GameFavorite *> *gameFavorites = GameFavorite::getItems();
            gtk_label_set_text(gamesLabel, string("(" + to_string(gameFavorites->size()) + ")").c_str());            
            GameFavorite::releaseItems(gameFavorites);
            
            gtk_widget_set_name(platformRowBox, to_string(0).c_str());
        }
        // Platform
        else
        {
            Platform *platform = Platform::getItem(platforms, index - ITEM_PLATFORMS_OFFSET);            
            list<Game *> *games = Game::getItems(platform->getId(), "");
            
            gtk_label_set_text(nameLabel, platform->getName().c_str());
            gtk_label_set_text(gamesLabel, string("(" + to_string(games->size()) + ")").c_str());
            
            Game::releaseItems(games);

            PlatformImage *platformImage = PlatformImage::getPlatformImage(platform->getId(), PlatformImage::TYPE_BOXART);
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
            
            g_signal_connect (platformRowBox, "button-press-event", G_CALLBACK(signalPlatformListRowPressedEvent), this);
        }
        
        gtk_widget_set_size_request(GTK_WIDGET(image), PLATFORM_IMAGE_WIDTH, PLATFORM_IMAGE_HEIGHT);                

        gtk_container_add (GTK_CONTAINER(platformListBox), platformRowBox);
        gtk_widget_show_all(platformRowBox);
    }    
    
    if(selectedPlatformId)
    {
        selectPlatform(selectedPlatformId);
    }
}

void MainWindow::updateRecents()
{
    list<GameActivity *> *recentItems = GameActivity::getRecentItems();
    GtkLabel *gamesLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, ITEM_RECENTS_INDEX)), "gamesLabel");
    gtk_label_set_text(gamesLabel, string("(" + to_string(recentItems->size()) + ")").c_str());            
    GameActivity::releaseItems(recentItems);
}

void MainWindow::updateFavorites()
{
    list<GameFavorite *> *gameFavorites = GameFavorite::getItems();            
    GtkLabel *gamesLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, ITEM_FAVORITES_INDEX)), "gamesLabel");
    gtk_label_set_text(gamesLabel, string("(" + to_string(gameFavorites->size()) + ")").c_str());            
    GameFavorite::releaseItems(gameFavorites);            
}

void MainWindow::updatePlatform(int64_t platformId)
{
    for(unsigned int index = ITEM_PLATFORMS_OFFSET; index < platforms->size() + ITEM_PLATFORMS_OFFSET; index++)
    {
        Platform *platform = Platform::getItem(platforms, index - ITEM_PLATFORMS_OFFSET);

        if(platform->getId() == platformId)
        {
            platform->load();

            GtkImage *image = (GtkImage *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "image");
            GtkLabel *nameLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "nameLabel");
            GtkLabel *gamesLabel = (GtkLabel *)UiUtils::getInstance()->getWidget(GTK_CONTAINER(gtk_list_box_get_row_at_index(platformListBox, index)), "gamesLabel");

            list<Game *> *games = Game::getItems(platform->getId(), "");

            gtk_label_set_text(nameLabel, platform->getName().c_str());
            gtk_label_set_text(gamesLabel, string("(" + to_string(games->size()) + ")").c_str());

            Game::releaseItems(games);

            PlatformImage *platformImage = PlatformImage::getPlatformImage(platform->getId(), PlatformImage::TYPE_BOXART);
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
    for(unsigned int index = ITEM_PLATFORMS_OFFSET; index < platforms->size() + ITEM_PLATFORMS_OFFSET; index++)
    {
        Platform *platform = Platform::getItem(platforms, index - ITEM_PLATFORMS_OFFSET);
        
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

gboolean MainWindow::signalPlatformListRowPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer mainWindow)
{    
    // Right click
    if(event->button.button == 3)
    {
        int64_t platformId = atol(gtk_widget_get_name(widget));
        
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
    ((MainWindow *)mainWindow)->showPlatformEditDialog(((MainWindow *)mainWindow)->selectedPlatformId);
}

void MainWindow::signalPlatformMenuRemoveActivate(GtkMenuItem* menuitem, gpointer mainWindow)
{
    ((MainWindow *)mainWindow)->removePlatform(((MainWindow *)mainWindow)->selectedPlatformId);
}

void MainWindow::onNotification(Notification* notification)
{
    MainWindow *mainWindow = (MainWindow *)notification->getListener();
    
    if(notification->getName().compare(SetupDatabaseProcess::TYPE) == 0)
    {
        if(notification->getStatus() == SerialProcess::STATUS_RUNNING)
        {
            gtk_spinner_start(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "Starting");
            gtk_label_set_text(mainWindow->processMessageLabel, notification->getMessage().c_str());

            if(notification->getProgress() < 0)
            {
                gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            }
            else
            {
                gtk_widget_show(GTK_WIDGET(mainWindow->processProgressBar));
                gtk_progress_bar_set_fraction(mainWindow->processProgressBar, ((double)notification->getProgress()) / 100.0);
            }
        }
        else
        {
            gtk_spinner_stop(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "");
            gtk_label_set_text(mainWindow->processMessageLabel, "");
            gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            
            if(notification->getStatus() == SerialProcess::STATUS_SUCCESS)
            {
                gtk_widget_show(GTK_WIDGET(mainWindow->addPlatformButton));
                gtk_widget_show_all(GTK_WIDGET(mainWindow->platformBox));
                mainWindow->loadPlatformList();
                
                ParseDirectoryProcess *parseDirectoryProcess = new ParseDirectoryProcess();
                SerialProcessExecutor::getInstance()->schedule(parseDirectoryProcess);
            }
            else
            {
                MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow), "Database failed to start. Please restart EMU-nexus", "Ok", "");   
                messageDialog->execute();
                delete messageDialog;
            }
        }
    }
    else if(notification->getName().compare(ParseDirectoryProcess::TYPE) == 0)
    {
        if(notification->getStatus() == SerialProcess::STATUS_RUNNING)
        {
            gtk_spinner_start(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "Processing files");
            gtk_label_set_text(mainWindow->processMessageLabel, notification->getMessage().c_str());

            if(notification->getProgress() < 0)
            {
                gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            }
            else
            {
                gtk_widget_show(GTK_WIDGET(mainWindow->processProgressBar));
                gtk_progress_bar_set_fraction(mainWindow->processProgressBar, ((double)notification->getProgress()) / 100.0);
            }
        }
        else
        {
            gtk_spinner_stop(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "");
            gtk_label_set_text(mainWindow->processMessageLabel, "");
            gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            
            if(notification->getStatus() == SerialProcess::STATUS_SUCCESS)
            {
                DownloadGameImagesProcess *downloadGameImagesProcess = new DownloadGameImagesProcess();
                SerialProcessExecutor::getInstance()->schedule(downloadGameImagesProcess);
            }
            else
            {
                MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow), "An error happened while processing the directory", "Ok", "");   
                messageDialog->execute();
                delete messageDialog;
            }
        }
    }
    else if(notification->getName().compare(DownloadGameImagesProcess::TYPE) == 0)
    {
        if(notification->getStatus() == SerialProcess::STATUS_RUNNING)
        {
            gtk_spinner_start(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "Downloading images");
            gtk_label_set_text(mainWindow->processMessageLabel, notification->getMessage().c_str());

            if(notification->getProgress() < 0)
            {
                gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            }
            else
            {
                gtk_widget_show(GTK_WIDGET(mainWindow->processProgressBar));
                gtk_progress_bar_set_fraction(mainWindow->processProgressBar, ((double)notification->getProgress()) / 100.0);
            }
        }
        else
        {
            gtk_spinner_stop(mainWindow->processSpinner);
            gtk_label_set_text(mainWindow->processTitleLabel, "");
            gtk_label_set_text(mainWindow->processMessageLabel, "");
            gtk_widget_hide(GTK_WIDGET(mainWindow->processProgressBar));
            
            if(notification->getStatus() == SerialProcess::STATUS_SUCCESS)
            {
            }
            else
            {
                MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow), "An error happened while downloading the images", "Ok", "");   
                messageDialog->execute();
                delete messageDialog;
            }
        }
    }
    
    
    else if(notification->getName().compare(NOTIFICATION_PLATFORM_UPDATED) == 0)
    {
        Platform *platform = (Platform *)notification->getData();        
        mainWindow->updatePlatform(platform->getId());
        
        if(platform->getId() == mainWindow->selectedPlatformId)
        {
            ((PlatformPanel *)mainWindow->currentPanel)->updateGames(string(gtk_entry_get_text(GTK_ENTRY(mainWindow->gameSearchEntry))));
        }
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_ACTIVITY_UPDATED) == 0)
    {
        mainWindow->updateRecents();
    }    
    else if(notification->getName().compare(NOTIFICATION_GAME_FAVORITE_UPDATED) == 0)
    {
        mainWindow->updateFavorites();
    }    
    else if(notification->getName().compare(NOTIFICATION_DIRECTORY_ADDED) == 0)
    {
        ParseDirectoryProcess *parseDirectoryProcess = new ParseDirectoryProcess();
        SerialProcessExecutor::getInstance()->schedule(parseDirectoryProcess);
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED) == 0)
    {
        if(notification->getError())
        {
            string message = "";
            if(notification->getError() == GameLauncher::ERROR_BUSY)
            {
                message = "Launcher busy";
            }
            else if(notification->getError() == GameLauncher::ERROR_FILE_NOT_FOUND)
            {
                message = "File not found";
            }
            else if(notification->getError() == GameLauncher::ERROR_INFLATE)
            {
                message = "Inflating failed";
            }
            else if(notification->getError() == GameLauncher::ERROR_INFLATE_NOT_SUPPORTED)
            {
                message = "Compressed format unsupported";
            }
            else if(notification->getError() == GameLauncher::ERROR_OTHER)
            {
                message = "Other error";
            }
            
            MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(mainWindow->mainWindow), message, "Close", "");
            if(messageDialog->execute() == GTK_RESPONSE_YES)
            {
            }
            delete messageDialog;
        }
        else if(notification->getStatus() == GameLauncher::STATUS_FOUND_MULTIPLE_FILES)
        {
            list<string> fileNames = GameLauncher::getInstance()->getFileNames();
            string fileName = "";
            list<string> baseFileNames;
            for(list<string>::iterator item = fileNames.begin(); item != fileNames.end(); item++)
            {
                baseFileNames.push_back(Utils::getInstance()->getFileBasename(*item));
            }

            SelectFromListDialog *selectFromListDialog = new SelectFromListDialog(GTK_WINDOW(mainWindow->mainWindow), "Multiple files where found", baseFileNames);
            if(selectFromListDialog->execute() == GTK_RESPONSE_ACCEPT)
            {
                list<string>::iterator item = fileNames.begin();
                advance(item, selectFromListDialog->getSelectedIndex());
                fileName = *item;
            }
            delete selectFromListDialog;

            if(fileName.length() > 0)
            {
                GameLauncher::getInstance()->selectFileName(fileName);
            }
            else
            {
                GameLauncher::getInstance()->cancel();
            }
        }
    }
    
    
    
    
}

