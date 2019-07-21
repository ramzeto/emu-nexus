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
 * File:   GameDetailBox.cpp
 * Author: ram
 * 
 * Created on April 9, 2019, 1:27 AM
 */

#include "GameDetailWidget.h"
#include "Platform.h"
#include "Settings.h"
#include "UiUtils.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameGenre.h"
#include "Developer.h"
#include "Publisher.h"
#include "Genre.h"
#include "Utils.h"
#include "Preferences.h"
#include "Database.h"
#include "Asset.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

const int GameDetailWidget::THUMBNAIL_IMAGE_WIDTH = 90;
const int GameDetailWidget::THUMBNAIL_IMAGE_HEIGHT = 90;
const int GameDetailWidget::IMAGE_HEIGHT = 400;
const int GameDetailWidget::WIDGET_WIDTH = 400;

GameDetailWidget::GameDetailWidget(int64_t gameId) : Widget("GameDetailWidget.ui", "gameDetailBox")
{
    nameLabel = (GtkLabel *)gtk_builder_get_object (builder, "nameLabel");
    platformLabel = (GtkLabel *)gtk_builder_get_object (builder, "platformLabel");
    imageBox = (GtkEventBox *)gtk_builder_get_object (builder, "imageBox");
    image = (GtkImage *)gtk_builder_get_object (builder, "image");
    imageTypeLabel = (GtkLabel *)gtk_builder_get_object (builder, "imageTypeLabel");    
    imagesBox = (GtkBox *)gtk_builder_get_object (builder, "imagesBox");
    g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalImageButtonPressedEvent), this);      
    documentsLabel = (GtkLabel *)gtk_builder_get_object (builder, "documentsLabel");
    documentsBox = (GtkBox *)gtk_builder_get_object (builder, "documentsBox");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    game = new Game(gameId);
    game->load(sqlite);
    
    gameImages = GameImage::getItems(sqlite, game->getId());
    gameDocuments = GameDocument::getItems(sqlite, game->getId());
    
    Platform *platform = new Platform(game->getPlatformId());        
    platform->load(sqlite);
    Database::getInstance()->release();
    
    gameImageBoxes = new map<GameImage *, GtkWidget *>;
    selectedGameImage = NULL;
    
    gameDocumentBoxes = new map<GameDocument *, GtkWidget *>;
    selectedGameDocument = NULL;
    
    selectGameImageTimestamp = 0;
    viewGameImageTimestamp = 0;
    selectGameImageBoxTimestamp = 0;
    viewGameImageBoxTimestamp = 0;
    selectGameDocumentBoxTimestamp = 0;
    viewGameDocumentBoxTimestamp = 0;
    
    
    gtk_label_set_text(nameLabel, game->getName().c_str());
    
    gtk_label_set_text(platformLabel, platform->getName().c_str());
    delete platform;        
    
    //g_signal_connect (rootWidget, "show", G_CALLBACK(signalShow), this);    
    //g_signal_connect (rootWidget, "size-allocate", G_CALLBACK(signalSizeAllocate), this);
    
    updateInfo();
    updateGameImageGrid();
    updateGameDocumentGrid();
}

GameDetailWidget::~GameDetailWidget()
{
    delete game;
    
    GameImage::releaseItems(gameImages);
    delete gameImageBoxes;
    
    GameDocument::releaseItems(gameDocuments);
    delete gameDocumentBoxes;
}

void GameDetailWidget::updateInfo()
{
    string html = "";
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    list<GameDeveloper *> *gameDevelopers = GameDeveloper::getItems(sqlite, game->getId());
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
            developer->load(sqlite);
            
            html += Utils::getInstance()->htmlEntities(developer->getName());
            
            delete developer;
        }
        html += "\n\n";
    }
    GameDeveloper::releaseItems(gameDevelopers);
    
    list<GamePublisher *> *gamePublishers = GamePublisher::getItems(sqlite, game->getId());
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
            publisher->load(sqlite);
            
            html += Utils::getInstance()->htmlEntities(publisher->getName());
            
            delete publisher;
        }
        html += "\n\n";
    }
    GamePublisher::releaseItems(gamePublishers);
    
    
    list<GameGenre *> *gameGenres = GameGenre::getItems(sqlite, game->getId());
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
            genre->load(sqlite);
            
            html += Utils::getInstance()->htmlEntities(genre->getName());
            
            delete genre;
        }
        html += "\n\n";
    }
    GameGenre::releaseItems(gameGenres);
    
    
    Database::getInstance()->release();
    
    if(game->getDescription().length() > 0)
    {
        html += "\n\n";
        html += "<b>Description</b>\n\n";
        html += Utils::getInstance()->htmlEntities(game->getDescription());
    }
    
    if(game->getNotes().length() > 0)
    {
        html += "\n\n\n";
        html += "<b>Notes</b>\n\n";
        html += Utils::getInstance()->htmlEntities(game->getNotes());
    }
    
    gtk_label_set_markup(informationLabel, html.c_str());
}

void GameDetailWidget::updateGameImageGrid()
{
    gameImageBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(imagesBox), 1);
    
    int width = WIDGET_WIDTH - 20;//gtk_widget_get_allocated_width(rootWidget);
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
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                UiUtils::getInstance()->loadImage(image, gameImage->getThumbnailFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                
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

void GameDetailWidget::selectGameImage(GameImage *gameImage)
{
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

    int width = WIDGET_WIDTH - 20;//gtk_widget_get_allocated_width(rootWidget);
    UiUtils::getInstance()->loadImage(image, selectedGameImage->getFileName(), width, IMAGE_HEIGHT);
}

void GameDetailWidget::viewGameImage(GameImage* gameImage)
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

void GameDetailWidget::saveGameImage(GameImage* gameImage)
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

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();

        Utils::getInstance()->copyFile(gameImage->getFileName(), string(cFileName));

        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void GameDetailWidget::updateGameDocumentGrid()
{
    if(gameDocuments->size() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(documentsLabel));
        gtk_widget_hide(GTK_WIDGET(documentsBox));
        
        return;
    }
    
    gameDocumentBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(documentsBox), 1);
    
    int width = WIDGET_WIDTH - 20;//gtk_widget_get_allocated_width(rootWidget);
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
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
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

void GameDetailWidget::selectGameDocument(GameDocument *gameDocument)
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

void GameDetailWidget::viewGameDocument(GameDocument *gameDocument)
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

void GameDetailWidget::saveGameDocument(GameDocument *gameDocument)
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
        
        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();

        Utils::getInstance()->copyFile(gameDocument->getFileName(), string(cFileName));
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}



void GameDetailWidget::signalSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer gameDetailWidget)
{
    cout << __FUNCTION__ << " width: " << allocation->width << " height: " << allocation->height << endl;
    ((GameDetailWidget *)gameDetailWidget)->updateGameImageGrid();
}

void GameDetailWidget::signalShow(GtkWidget* widget, gpointer gameDetailWidget)
{
    cout << __FUNCTION__ << endl;
}

gboolean GameDetailWidget::signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailWidget)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameImage *gameImage = GameImage::getItem(((GameDetailWidget *)gameDetailWidget)->gameImages, atoi(gtk_widget_get_name(widget)));
        
        if(gameImage == ((GameDetailWidget *)gameDetailWidget)->selectedGameImage)
        {
            time_t now = time(NULL);
            if(now == ((GameDetailWidget *)gameDetailWidget)->selectGameImageBoxTimestamp && (now - ((GameDetailWidget *)gameDetailWidget)->viewGameImageBoxTimestamp) > 2)
            {
                 ((GameDetailWidget *)gameDetailWidget)->viewGameImageBoxTimestamp = now;
                 ((GameDetailWidget *)gameDetailWidget)->viewGameImage(((GameDetailWidget *)gameDetailWidget)->selectedGameImage);
            }
            ((GameDetailWidget *)gameDetailWidget)->selectGameImageBoxTimestamp = now;
        }
        else
        {
            ((GameDetailWidget *)gameDetailWidget)->selectGameImage(gameImage);
        }                
    }    
    
    return TRUE;
}

gboolean GameDetailWidget::signalImageButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer gameDetailWidget)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        time_t now = time(NULL);        
        if(now == ((GameDetailWidget *)gameDetailWidget)->selectGameImageTimestamp && (now - ((GameDetailWidget *)gameDetailWidget)->viewGameImageTimestamp) > 2)
        {
             ((GameDetailWidget *)gameDetailWidget)->viewGameImageTimestamp = now;
             ((GameDetailWidget *)gameDetailWidget)->viewGameImage(((GameDetailWidget *)gameDetailWidget)->selectedGameImage);
        }
        ((GameDetailWidget *)gameDetailWidget)->selectGameImageTimestamp = now;
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
        GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
        g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalImageMenuViewActivate), gameDetailWidget);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
        g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalImageMenuSaveActivate), gameDetailWidget);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    
    return TRUE;
}

void GameDetailWidget::signalImageMenuViewActivate(GtkMenuItem* menuitem, gpointer gameDetailWidget)
{
    ((GameDetailWidget *)gameDetailWidget)->viewGameImage(((GameDetailWidget *)gameDetailWidget)->selectedGameImage);    
}

void GameDetailWidget::signalImageMenuSaveActivate(GtkMenuItem* menuitem, gpointer gameDetailWidget)
{
    ((GameDetailWidget *)gameDetailWidget)->saveGameImage(((GameDetailWidget *)gameDetailWidget)->selectedGameImage);
}

gboolean GameDetailWidget::signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailWidget)
{        
    // Mouse left button
    if(event->button.button == 1)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameDetailWidget *)gameDetailWidget)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        if(gameDocument == ((GameDetailWidget *)gameDetailWidget)->selectedGameDocument)
        {
            time_t now = time(NULL);
            if(now == ((GameDetailWidget *)gameDetailWidget)->selectGameDocumentBoxTimestamp && (now - ((GameDetailWidget *)gameDetailWidget)->viewGameDocumentBoxTimestamp) > 2)
            {
                 ((GameDetailWidget *)gameDetailWidget)->viewGameDocumentBoxTimestamp = now;
                 ((GameDetailWidget *)gameDetailWidget)->viewGameDocument(((GameDetailWidget *)gameDetailWidget)->selectedGameDocument);
            }
            ((GameDetailWidget *)gameDetailWidget)->selectGameDocumentBoxTimestamp = now;
        }
        else
        {
            ((GameDetailWidget *)gameDetailWidget)->selectGameDocument(gameDocument);
        }
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameDetailWidget *)gameDetailWidget)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        ((GameDetailWidget *)gameDetailWidget)->selectGameDocument(gameDocument);
    
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
        GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
        g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalDocumentMenuViewActivate), gameDetailWidget);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
        g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalDocumentMenuSaveActivate), gameDetailWidget);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    
    return TRUE;
}

void GameDetailWidget::signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget)
{
    ((GameDetailWidget *)gameDetailWidget)->viewGameDocument(((GameDetailWidget *)gameDetailWidget)->selectedGameDocument);
}

void GameDetailWidget::signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget)
{
    ((GameDetailWidget *)gameDetailWidget)->saveGameDocument(((GameDetailWidget *)gameDetailWidget)->selectedGameDocument);
}