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
 * File:   GameDialog.cpp
 * Author: ram
 * 
 * Created on May 1, 2020, 9:29 PM
 */

#include "GameDialog.h"
#include "Platform.h"
#include "UiUtils.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameGenre.h"
#include "Developer.h"
#include "Publisher.h"
#include "Genre.h"
#include "Utils.h"
#include "Preferences.h"
#include "Asset.h"
#include "Directory.h"
#include "EsrbRating.h"
#include "SelectFromListDialog.h"
#include "GameLauncher.h"
#include "GameActivity.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>


const int GameDialog::THUMBNAIL_IMAGE_WIDTH = 100;
const int GameDialog::THUMBNAIL_IMAGE_HEIGHT = 100;
const int GameDialog::IMAGE_WIDTH = 400;
const int GameDialog::IMAGE_HEIGHT = 400;
const int GameDialog::INFORMATION_WIDTH = 600;

GameDialog::GameDialog(GtkWindow *parent, int64_t gameId) : Dialog(parent, "GameDialog.ui", "gameDialog")
{
    nameLabel = (GtkLabel *)gtk_builder_get_object (builder, "nameLabel");
    fileNameLabel = (GtkLabel *)gtk_builder_get_object (builder, "fileNameLabel");
    platformLabel = (GtkLabel *)gtk_builder_get_object (builder, "platformLabel");
    imageBox = (GtkEventBox *)gtk_builder_get_object (builder, "imageBox");
    image = (GtkImage *)gtk_builder_get_object (builder, "image");
    imageTypeLabel = (GtkLabel *)gtk_builder_get_object (builder, "imageTypeLabel");    
    imagesBox = (GtkBox *)gtk_builder_get_object (builder, "imagesBox");
    g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalImageButtonPressedEvent), this);
    documentsLabel = (GtkLabel *)gtk_builder_get_object (builder, "documentsLabel");
    documentsBox = (GtkBox *)gtk_builder_get_object (builder, "documentsBox");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
 
    launchBox = (GtkBox *)gtk_builder_get_object(builder, "launchBox");
    spinner = (GtkSpinner *)gtk_builder_get_object(builder, "spinner");
    messageLabel = (GtkLabel *)gtk_builder_get_object(builder, "messageLabel");
    progressBar = (GtkProgressBar *)gtk_builder_get_object(builder, "progressBar");
    gtk_widget_hide(GTK_WIDGET(launchBox));
    
    game = new Game(gameId);
    game->load();
    
    gameImages = GameImage::getItems(game->getId());
    gameDocuments = GameDocument::getItems(game->getId());
    
    Platform *platform = new Platform(game->getPlatformId());        
    platform->load();
    
    gameImageBoxes = new map<GameImage *, GtkWidget *>;
    selectedGameImage = NULL;
    
    gameDocumentBoxes = new map<GameDocument *, GtkWidget *>;
    selectedGameDocument = NULL;
    
    running = 0;
    selectGameImageTimestamp = 0;
    viewGameImageTimestamp = 0;
    selectGameImageBoxTimestamp = 0;
    viewGameImageBoxTimestamp = 0;
    selectGameDocumentBoxTimestamp = 0;
    viewGameDocumentBoxTimestamp = 0;
    
    gtk_window_set_title(GTK_WINDOW(dialog), game->getName().c_str());
    gtk_label_set_text(nameLabel, game->getName().c_str());
    gtk_label_set_text(fileNameLabel, Utils::getInstance()->getFileBasename(game->getFileName()).c_str());
    gtk_label_set_text(imageTypeLabel, "");
    
    gtk_label_set_text(platformLabel, platform->getName().c_str());
    delete platform;
    
    gtk_image_clear(image);
    
    updateInformation();
    updateGameImagesGrid();
    updateGameDocumentsGrid();
    
    launcherUiThreadBridge = UiThreadBridge::registerBridge(this, callbackGameLauncher);
    
    g_signal_connect(dialog, "delete-event", G_CALLBACK(signalDeleteEvent), this);
    g_signal_connect(dialog, "key-press-event", G_CALLBACK (signalKeyPressedEvent), this);        
}

GameDialog::~GameDialog()
{
    UiThreadBridge::unregisterBridge(launcherUiThreadBridge);
    
    delete game;
    
    GameImage::releaseItems(gameImages);
    delete gameImageBoxes;
    
    GameDocument::releaseItems(gameDocuments);
    delete gameDocumentBoxes;
}

void GameDialog::launch()
{
    //GameLauncher::getInstance()->launch(game->getId(), launcherUiThreadBridge, UiThreadBridge::callback);
    //execute();
}

void GameDialog::setLaunchStatus(int running, string message, int progress)
{
    this->running = running;
    gtk_widget_show_all(GTK_WIDGET(launchBox));
    
    if(running)
    {        
        gtk_spinner_start(spinner);
        
        if(progress >= 0)
        {
            gtk_widget_show(GTK_WIDGET(progressBar));
            gtk_progress_bar_set_fraction(progressBar, ((double)progress) / 100.0);
        }
        else
        {
            gtk_widget_hide(GTK_WIDGET(progressBar));
        }
    }
    else
    {
        gtk_spinner_stop(spinner);
        gtk_widget_hide(GTK_WIDGET(progressBar));
        updateInformation();
    }
    
    gtk_window_set_title(GTK_WINDOW(dialog), message.c_str());
    gtk_label_set_text(messageLabel, message.c_str());    
}

string GameDialog::selectLaunchFileName(list<string> fileNames)
{
    string fileName = "";
    list<string> baseFileNames;
    for(list<string>::iterator item = fileNames.begin(); item != fileNames.end(); item++)
    {
        baseFileNames.push_back(Utils::getInstance()->getFileBasename(*item));
    }
    
    SelectFromListDialog *selectFromListDialog = new SelectFromListDialog(GTK_WINDOW(dialog), "Multiple files where found", baseFileNames);
    if(selectFromListDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        list<string>::iterator item = fileNames.begin();
        advance(item, selectFromListDialog->getSelectedIndex());
        fileName = *item;
    }
    delete selectFromListDialog;
    
    return fileName;
}


void GameDialog::updateInformation()
{
    string html = "";
    
    list<GameActivity *> *gameActivities = GameActivity::getItems(-1, game->getId());
    if(gameActivities->size() > 0)
    {
        html += "<b>Last played</b>\n";
        html += gameActivities->back()->getTimestamp();        
        html += "\n\n";
        
        int64_t duration = 0; 
        for(unsigned int c = 0; c < gameActivities->size(); c++)
        {
            GameActivity *gameActivity = GameActivity::getItem(gameActivities, c);
            duration += gameActivity->getDuration();
        }
        
        int64_t hours = (duration / 3600);
        int64_t minutes = (duration % 3600) / 60;
        int64_t seconds = (duration % 3600) % 60;
        
        html += "<b>Play time</b>\n";
        if(hours > 0)
        {
            html += to_string(hours);
            if(hours == 1)
            {
                html += " hour ";
            }
            else
            {
                html += " hours ";
            }
        }
        if(minutes > 0)
        {
            html += to_string(minutes);
            if(minutes == 1)
            {
                html += " minute ";
            }
            else
            {
                html += " minutes ";
            }
        }
        if(seconds > 0)
        {
            html += to_string(seconds) + " seconds";
        }                
        html += "\n\n";
    }
    GameActivity::releaseItems(gameActivities);
    
    list<GameDeveloper *> *gameDevelopers = GameDeveloper::getItems(game->getId());
    if(gameDevelopers->size() > 0)
    {
        html += "<b>Developed by</b>\n";
        for(unsigned int c = 0; c < gameDevelopers->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GameDeveloper *gameDeveloper = GameDeveloper::getItem(gameDevelopers, c);
            
            Developer *developer = new Developer(gameDeveloper->getDeveloperId());
            developer->load();
            
            html += Utils::getInstance()->htmlEntities(developer->getName());
            
            delete developer;
        }
        html += "\n\n";
    }
    GameDeveloper::releaseItems(gameDevelopers);
    
    list<GamePublisher *> *gamePublishers = GamePublisher::getItems(game->getId());
    if(gamePublishers->size() > 0)
    {
        html += "<b>Published by</b>\n";
        for(unsigned int c = 0; c < gamePublishers->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GamePublisher *gamePublisher = GamePublisher::getItem(gamePublishers, c);
            
            Publisher *publisher = new Publisher(gamePublisher->getPublisherId());
            publisher->load();
            
            html += Utils::getInstance()->htmlEntities(publisher->getName());
            
            delete publisher;
        }
        html += "\n\n";
    }
    GamePublisher::releaseItems(gamePublishers);
    
    
    list<GameGenre *> *gameGenres = GameGenre::getItems(game->getId());
    if(gameGenres->size() > 0)
    {
        if(gameGenres->size() == 1)
        {
            html += "<b>Genre</b>\n";
        }
        else
        {
            html += "<b>Genres</b>\n";
        }
        for(unsigned int c = 0; c < gameGenres->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GameGenre *gameGenre = GameGenre::getItem(gameGenres, c);
            
            Genre *genre = new Genre(gameGenre->getGenreId());
            genre->load();
            
            html += Utils::getInstance()->htmlEntities(genre->getName());
            
            delete genre;
        }
        html += "\n\n";
    }
    GameGenre::releaseItems(gameGenres);
    
    if(game->getReleaseDate().length() > 0)
    {
        html += "<b>Released</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getReleaseDate());
        html += "\n\n";
    }
    
    if(game->getEsrbRatingId() > 0)
    {
        EsrbRating *esrbRating = new EsrbRating(game->getEsrbRatingId());
        if(esrbRating->load())
        {
            html += "<b>ESRB rating</b>\n";
            html += Utils::getInstance()->htmlEntities(esrbRating->getName());
            html += "\n\n";
        }
        delete esrbRating;
    }        
    
    if(game->getDescription().length() > 0)
    {
        html += "<b>Description</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getDescription());
    }
    
    if(game->getNotes().length() > 0)
    {
        html += "\n\n";
        html += "<b>Notes</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getNotes());
    }
    
    gtk_label_set_markup(informationLabel, html.c_str());
}

void GameDialog::updateGameImagesGrid()
{
    gameImageBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(imagesBox), 1);
    
    int width = IMAGE_WIDTH;//gtk_widget_get_allocated_width(rootWidget);
    int columns = width / THUMBNAIL_IMAGE_WIDTH;
    
    int rows = gameImages->size() / columns;
    if(gameImages->size() % columns)
    {
        rows++;
    }

    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(index < gameImages->size())
            {
                GameImage *gameImage = GameImage::getItem(gameImages, index);                               
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                if(Utils::getInstance()->fileExists(gameImage->getThumbnailFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, gameImage->getThumbnailFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                }
                else if(Utils::getInstance()->fileExists(gameImage->getFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, gameImage->getFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                }
                else
                {
                    UiUtils::getInstance()->loadImage(image, Asset::getInstance()->getImageDownloading(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                }
                
                gtk_widget_set_name(GTK_WIDGET(imageBox), to_string(index).c_str());                                
                g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalImageBoxButtonPressedEvent), this);                                
                
                                
                if(selectedGameImage == gameImage)
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameImageBoxes->insert(pair<GameImage*, GtkWidget *>(gameImage, GTK_WIDGET(imageBox)));
                
                gtk_widget_set_size_request(GTK_WIDGET(imageBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(imageBox), 1, 1, 0);
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_box_pack_start(imagesBox, GTK_WIDGET(rowBox), 1, 1, 0);
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }
    
    if(!selectedGameImage && gameImages->size() > 0)
    {
        selectGameImage(GameImage::getItem(gameImages, 0));
    }
}

void GameDialog::selectGameImage(GameImage *gameImage)
{
    if(!Utils::getInstance()->fileExists(gameImage->getFileName()))
    {
        return;
    }
    
    if(selectedGameImage)
    {
        if(gameImageBoxes->find(selectedGameImage) != gameImageBoxes->end())
        {
            GtkWidget *imageBox = gameImageBoxes->at(selectedGameImage);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_NORMAL, 1);
        }        
    }
    
    selectedGameImage = gameImage;
    if(gameImageBoxes->find(selectedGameImage) != gameImageBoxes->end())
    {
        GtkWidget *imageBox = gameImageBoxes->at(selectedGameImage);
        gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
    }
    
    
    if(selectedGameImage->getType() == GameImage::TYPE_BOX_FRONT)
    {
        gtk_label_set_text(imageTypeLabel, "Box front");
    }
    else if(selectedGameImage->getType() == GameImage::TYPE_BOX_BACK)
    {
        gtk_label_set_text(imageTypeLabel, "Box back");
    }
    else if(selectedGameImage->getType() == GameImage::TYPE_SCREENSHOT)
    {
        gtk_label_set_text(imageTypeLabel, "Screenshot");
    }
    else if(selectedGameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
    {
        gtk_label_set_text(imageTypeLabel, "Logo");
    }
    else if(selectedGameImage->getType() == GameImage::TYPE_BANNER)
    {
        gtk_label_set_text(imageTypeLabel, "Banner");
    }

    UiUtils::getInstance()->loadImage(image, selectedGameImage->getFileName(), IMAGE_WIDTH, IMAGE_HEIGHT);
}

void GameDialog::viewGameImage(GameImage* gameImage)
{
    string tempDirectoryName = Utils::getInstance()->getTempFileName();
    Utils::getInstance()->makeDirectory(tempDirectoryName);
    
    string imageName = game->getName();
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
    imageName += " (";        
    if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
    {
        imageName += "Box front";
    }
    else if(gameImage->getType() == GameImage::TYPE_BOX_BACK)
    {
        imageName += "Box back";
    }
    else if(gameImage->getType() == GameImage::TYPE_SCREENSHOT)
    {
        imageName += "Screenshot";
    }
    else if(gameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
    {
        imageName += "Logo";
    }
    else if(gameImage->getType() == GameImage::TYPE_BANNER)
    {
        imageName += "Banner";
    }
    imageName += ")";
    imageName = tempDirectoryName + "/" + imageName;    
    
    if(!Utils::getInstance()->copyFile(gameImage->getFileName(), imageName))
    {
        Utils::getInstance()->openFileWithDefaultApplication(imageName);
    }
}

void GameDialog::saveGameImage(GameImage* gameImage)
{
    string imageName = game->getName();
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
    imageName += " (";        
    if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
    {
        imageName += "Box front";
    }
    else if(gameImage->getType() == GameImage::TYPE_BOX_BACK)
    {
        imageName += "Box back";
    }
    else if(gameImage->getType() == GameImage::TYPE_SCREENSHOT)
    {
        imageName += "Screenshot";
    }
    else if(gameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
    {
        imageName += "Logo";
    }
    else if(gameImage->getType() == GameImage::TYPE_BANNER)
    {
        imageName += "Banner";
    }
    imageName += ")";

    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Save ", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fileChooserDialog), imageName.c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save();

        Utils::getInstance()->copyFile(gameImage->getFileName(), string(cFileName));

        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void GameDialog::updateGameDocumentsGrid()
{
    if(gameDocuments->size() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(documentsLabel));
        gtk_widget_hide(GTK_WIDGET(documentsBox));
        
        return;
    }
    
    gameDocumentBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(documentsBox), 1);
    
    int width = INFORMATION_WIDTH;//gtk_widget_get_allocated_width(rootWidget);
    int columns = width / THUMBNAIL_IMAGE_WIDTH;
    
    int rows = gameDocuments->size() / columns;
    if(gameDocuments->size() % columns)
    {
        rows++;
    }

    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(index < gameDocuments->size())
            {                
                GameDocument *gameDocument = GameDocument::getItem(gameDocuments, index);
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                UiUtils::getInstance()->loadImage(image, gameDocument->getPreviewImageFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                
                gtk_widget_set_name(GTK_WIDGET(imageBox), to_string(index).c_str());                                
                g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalDocumentBoxButtonPressedEvent), this);                                
                
                                
                if(selectedGameDocument == gameDocument)
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameDocumentBoxes->insert(pair<GameDocument*, GtkWidget *>(gameDocument, GTK_WIDGET(imageBox)));
                
                gtk_widget_set_size_request(GTK_WIDGET(imageBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(imageBox), 1, 1, 0);
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_box_pack_start(documentsBox, GTK_WIDGET(rowBox), 1, 1, 0);
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }
}

void GameDialog::selectGameDocument(GameDocument *gameDocument)
{
    if(selectedGameDocument)
    {
        if(gameDocumentBoxes->find(selectedGameDocument) != gameDocumentBoxes->end())
        {
            GtkWidget *imageBox = gameDocumentBoxes->at(selectedGameDocument);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_NORMAL, 1);
        }        
    }
    
    selectedGameDocument = gameDocument;
    if(gameDocumentBoxes->find(selectedGameDocument) != gameDocumentBoxes->end())
    {
        GtkWidget *imageBox = gameDocumentBoxes->at(selectedGameDocument);
        gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
    }
}

void GameDialog::viewGameDocument(GameDocument *gameDocument)
{    
    string tempDirectoryName = Utils::getInstance()->getTempFileName();
    Utils::getInstance()->makeDirectory(tempDirectoryName);
    
    string imageName = game->getName() + " (" + gameDocument->getName() + ")";
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
    imageName = tempDirectoryName + "/" + imageName;    
    
    if(!Utils::getInstance()->copyFile(gameDocument->getFileName(), imageName))
    {
        Utils::getInstance()->openFileWithDefaultApplication(imageName);
    }
}

void GameDialog::saveGameDocument(GameDocument *gameDocument)
{
    string imageName = game->getName() + " (" + gameDocument->getName() + ")";
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
        
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Save ", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fileChooserDialog), imageName.c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));        
        Preferences::getInstance()->save();

        Utils::getInstance()->copyFile(gameDocument->getFileName(), string(cFileName));
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}




gboolean GameDialog::signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameImage *gameImage = GameImage::getItem(((GameDialog *)gameDialog)->gameImages, atoi(gtk_widget_get_name(widget)));
        
        if(gameImage == ((GameDialog *)gameDialog)->selectedGameImage)
        {
            time_t now = time(NULL);
            if(now == ((GameDialog *)gameDialog)->selectGameImageBoxTimestamp && (now - ((GameDialog *)gameDialog)->viewGameImageBoxTimestamp) > 2)
            {
                 ((GameDialog *)gameDialog)->viewGameImageBoxTimestamp = now;
                 ((GameDialog *)gameDialog)->viewGameImage(((GameDialog *)gameDialog)->selectedGameImage);
            }
            ((GameDialog *)gameDialog)->selectGameImageBoxTimestamp = now;
        }
        else
        {
            ((GameDialog *)gameDialog)->selectGameImage(gameImage);
        }                
    }    
    
    return TRUE;
}

gboolean GameDialog::signalImageButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer gameDialog)
{    
    if(!((GameDialog *)gameDialog)->selectedGameImage)
    {
        return TRUE;
    }
    
    // Mouse left button
    if(event->button.button == 1)
    {
        time_t now = time(NULL);        
        if(now == ((GameDialog *)gameDialog)->selectGameImageTimestamp && (now - ((GameDialog *)gameDialog)->viewGameImageTimestamp) > 2)
        {
             ((GameDialog *)gameDialog)->viewGameImageTimestamp = now;
             ((GameDialog *)gameDialog)->viewGameImage(((GameDialog *)gameDialog)->selectedGameImage);
        }
        ((GameDialog *)gameDialog)->selectGameImageTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
        GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
        g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalImageMenuViewActivate), gameDialog);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
        g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalImageMenuSaveActivate), gameDialog);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    
    return TRUE;
}

void GameDialog::signalImageMenuViewActivate(GtkMenuItem* menuitem, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->viewGameImage(((GameDialog *)gameDialog)->selectedGameImage);    
}

void GameDialog::signalImageMenuSaveActivate(GtkMenuItem* menuitem, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->saveGameImage(((GameDialog *)gameDialog)->selectedGameImage);
}

gboolean GameDialog::signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog)
{        
    // Mouse left button
    if(event->button.button == 1)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameDialog *)gameDialog)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        if(gameDocument == ((GameDialog *)gameDialog)->selectedGameDocument)
        {
            time_t now = time(NULL);
            if(now == ((GameDialog *)gameDialog)->selectGameDocumentBoxTimestamp && (now - ((GameDialog *)gameDialog)->viewGameDocumentBoxTimestamp) > 2)
            {
                 ((GameDialog *)gameDialog)->viewGameDocumentBoxTimestamp = now;
                 ((GameDialog *)gameDialog)->viewGameDocument(((GameDialog *)gameDialog)->selectedGameDocument);
            }
            ((GameDialog *)gameDialog)->selectGameDocumentBoxTimestamp = now;
        }
        else
        {
            ((GameDialog *)gameDialog)->selectGameDocument(gameDocument);
        }
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameDialog *)gameDialog)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        ((GameDialog *)gameDialog)->selectGameDocument(gameDocument);
    
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
        GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
        g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalDocumentMenuViewActivate), gameDialog);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
        g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalDocumentMenuSaveActivate), gameDialog);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    
    return TRUE;
}

void GameDialog::signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->viewGameDocument(((GameDialog *)gameDialog)->selectedGameDocument);
}

void GameDialog::signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->saveGameDocument(((GameDialog *)gameDialog)->selectedGameDocument);
}

gboolean GameDialog::signalDeleteEvent(GtkWidget* window, GdkEvent* event, gpointer gameDialog)
{
    return ((GameDialog *)gameDialog)->running;
}

gboolean GameDialog::signalKeyPressedEvent(GtkEntry* entry, GdkEvent* event, gpointer gameDialog)
{
    return ((GameDialog *)gameDialog)->running;
}

void GameDialog::callbackGameLauncher(CallbackResult *callbackResult)
{
    GameDialog *gameDialog = (GameDialog *)callbackResult->getRequester();
    
    string message = "";
    int running = 0;
    if(callbackResult->getError())
    {
        if(callbackResult->getError() == GameLauncher::ERROR_BUSY)
        {
            message = "Launcher busy";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_FILE_NOT_FOUND)
        {
            message = "File not found";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE)
        {
            message = "Inflating failed";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_INFLATE_NOT_SUPPORTED)
        {
            message = "Format unsupported";
        }
        else if(callbackResult->getError() == GameLauncher::ERROR_OTHER)
        {
            message = "Other error";
        }
        
        running = 0;
    }
    else
    {
        if(callbackResult->getStatus() == GameLauncher::STATUS_IDLE)
        {
            running = 1;
            message = "Preparing...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATUS_INFLATING)
        {
            running = 1;
            message = "Decompressing/Unpacking...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATUS_SELECTING_FILE)
        {
            running = 1;
            message = "Selecting file...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATUS_FOUND_MULTIPLE_FILES)
        {            
            string fileName = gameDialog->selectLaunchFileName(GameLauncher::getInstance()->getFileNames());
            if(fileName.length() > 0)
            {
                running = 1;
                GameLauncher::getInstance()->selectFileName(fileName);
            }
            else
            {
                running = 0;
                message = "Execution canceled";
                GameLauncher::getInstance()->cancel();
            }
        }        
        else if(callbackResult->getStatus() == GameLauncher::STATUS_RUNNING)
        {
            running = 1;
            message = "Running...";
        }
        else if(callbackResult->getStatus() == GameLauncher::STATUS_FINISHED)
        {
            running = 0;
            message = "Execution finished";
        }
    }
    gameDialog->setLaunchStatus(running, message, callbackResult->getProgress());
}
