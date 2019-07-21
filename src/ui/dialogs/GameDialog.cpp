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
 * File:   GameDialog.cpp
 * Author: ram
 * 
 * Created on March 21, 2019, 12:47 AM
 */

#include "GameDialog.h"
#include "Preferences.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Settings.h"
#include "Platform.h"
#include "MessageDialog.h"
#include "Database.h"
#include "DevelopersSelectDialog.h"
#include "PublishersSelectDialog.h"
#include "GenresSelectDialog.h"
#include "GameSearchDialog.h"
#include "Asset.h"
#include "HttpConnector.h"
#include "Notifications.h"
#include "NotificationManager.h"
#include "thegamesdb.h"

#include <iostream>

const int GameDialog::THUMBNAIL_IMAGE_WIDTH = 90;
const int GameDialog::THUMBNAIL_IMAGE_HEIGHT = 90;
const int GameDialog::IMAGE_WIDTH = 300;
const int GameDialog::IMAGE_HEIGHT = 500;

GameDialog::GameDialog(int64_t platformId, int64_t gameId) : Dialog("GameDialog.ui", "gameDialog")
{
    saved = 0;
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    
    game = new Game(gameId);    
    game->load(sqlite);
    game->setPlatformId(platformId);
    
    esrbRatings = EsrbRating::getItems(sqlite);
    gameImages = GameImage::getItems(sqlite, game->getId());
    gameDocuments = GameDocument::getItems(sqlite, game->getId());
    
    developers = new list<Developer *>;
    list<GameDeveloper *> *gameDevelopers = GameDeveloper::getItems(sqlite, game->getId());
    for(unsigned int c = 0; c < gameDevelopers->size(); c++)
    {
        GameDeveloper *gameDeveloper = GameDeveloper::getItem(gameDevelopers, c);
        
        Developer *developer = new Developer(gameDeveloper->getDeveloperId());
        developer->load(sqlite);
        developers->push_back(developer);
    }
    GameDeveloper::releaseItems(gameDevelopers);
    
    publishers = new list<Publisher *>;
    list<GamePublisher *> *gamePublishers = GamePublisher::getItems(sqlite, game->getId());
    for(unsigned int c = 0; c < gamePublishers->size(); c++)
    {
        GamePublisher *gamePublisher = GamePublisher::getItem(gamePublishers, c);
        
        Publisher *publisher = new Publisher(gamePublisher->getPublisherId());
        publisher->load(sqlite);
        publishers->push_back(publisher);
    }
    GamePublisher::releaseItems(gamePublishers);
    
    genres = new list<Genre *>;
    list<GameGenre *> *gameGenres = GameGenre::getItems(sqlite, game->getId());
    for(unsigned int c = 0; c < gameGenres->size(); c++)
    {
        GameGenre *gameGenre = GameGenre::getItem(gameGenres, c);
        
        Genre *genre = new Genre(gameGenre->getGenreId());
        genre->load(sqlite);
        genres->push_back(genre);
    }
    GameGenre::releaseItems(gameGenres);
      
    Database::getInstance()->release();
    
    
    
    
    if(game->getId() > 0)
    {
        gtk_window_set_title(GTK_WINDOW(dialog), game->getName().c_str());
    }
    else
    {
        gtk_window_set_title(GTK_WINDOW(dialog), "New game");
    }
    
    
    gameImageTypes = new list<int64_t>;
    selectedGameImage = NULL;    
    gameImagesToRemove = new list<GameImage *>;
    gameImageBoxes = new map<GameImage *, GtkWidget *>;
    
    gameDocumentTypes = new list<int64_t>;
    selectedGameDocument = NULL;    
    gameDocumentsToRemove = new list<GameDocument *>;
    gameDocumentBoxes = new map<GameDocument *, GtkWidget *>;
    
    
    
    nameEntry = (GtkEntry *)gtk_builder_get_object (builder, "nameEntry");
    gtk_entry_set_text(nameEntry, game->getName().c_str());
    
    searchButton = (GtkButton *)gtk_builder_get_object(builder, "searchButton");
    g_signal_connect (searchButton, "clicked", G_CALLBACK (signalSearchButtonClicked), this);

    fileNameEntry = (GtkEntry *)gtk_builder_get_object (builder, "fileNameEntry");
    gtk_entry_set_text(fileNameEntry, game->getFileName().c_str());
    
    fileNameButton = (GtkButton *)gtk_builder_get_object(builder, "fileNameButton");
    g_signal_connect (fileNameButton, "clicked", G_CALLBACK (signalFileNameButtonClicked), this);

    commandEntry = (GtkEntry *)gtk_builder_get_object (builder, "commandEntry");
    gtk_entry_set_text(commandEntry, game->getCommand().c_str());

    deflateCheckButton = (GtkCheckButton *)gtk_builder_get_object (builder, "deflateCheckButton");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(deflateCheckButton), game->getDeflate());
    
    deflateFileExtensionsEntry = (GtkEntry *)gtk_builder_get_object (builder, "deflateFileExtensionsEntry");
    gtk_entry_set_text(deflateFileExtensionsEntry, game->getDeflateFileExtensions().c_str());
          
    releaseDateEntry = (GtkEntry *)gtk_builder_get_object (builder, "releaseDateEntry");
    gtk_entry_set_text(releaseDateEntry, game->getReleaseDate().c_str());

    esrbComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "esrbComboBox");
    
    addDeveloperButton = (GtkButton *)gtk_builder_get_object(builder, "addDeveloperButton");
    g_signal_connect (addDeveloperButton, "clicked", G_CALLBACK (signalAddDeveloperButtonClicked), this);

    addPublisherButton = (GtkButton *)gtk_builder_get_object(builder, "addPublisherButton");
    g_signal_connect (addPublisherButton, "clicked", G_CALLBACK (signalAddPublisherButtonClicked), this);

    addGenreButton = (GtkButton *)gtk_builder_get_object(builder, "addGenreButton");
    g_signal_connect (addGenreButton, "clicked", G_CALLBACK (signalAddGenreButtonClicked), this);
        
    developersListBox = (GtkListBox *)gtk_builder_get_object (builder, "developersListBox");
    publishersListBox = (GtkListBox *)gtk_builder_get_object (builder, "publishersListBox");    
    genresListBox = (GtkListBox *)gtk_builder_get_object (builder, "genresListBox");
    
    
    
    descriptionTextView = (GtkTextView *)gtk_builder_get_object (builder, "descriptionTextView");
    GtkTextBuffer *descriptionTextBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(descriptionTextBuffer, game->getDescription().c_str(), game->getDescription().length());
    gtk_text_view_set_buffer(descriptionTextView, descriptionTextBuffer); 
    
    
    
    imagesGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "imagesGridListBox");
    
    imageTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "imageTypeComboBox");
    g_signal_connect (imageTypeComboBox, "changed", G_CALLBACK (signalImageTypeComboBoxChanged), this);
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    
    image = (GtkImage *)gtk_builder_get_object (builder, "image");
    gtk_image_clear(image);

    removeImageButton = (GtkButton *)gtk_builder_get_object (builder, "removeImageButton");
    g_signal_connect (removeImageButton, "clicked", G_CALLBACK (signalRemoveImageButtonClicked), this);
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
        
    
    
    documentGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "documentGridListBox");
    
    documentTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "documentTypeComboBox");
    g_signal_connect (documentTypeComboBox, "changed", G_CALLBACK (signalDocumentTypeComboBoxChanged), this);
    gtk_widget_hide(GTK_WIDGET(documentTypeComboBox));
    
    documentNameEntry = (GtkEntry *)gtk_builder_get_object (builder, "documentNameEntry");
    g_signal_connect (documentNameEntry, "key-release-event", G_CALLBACK (signalDocumentNameEntryKeyReleaseEvent), this);
    gtk_widget_hide(GTK_WIDGET(documentNameEntry));
    
    documentImage = (GtkImage *)gtk_builder_get_object (builder, "documentImage");
    gtk_image_clear(documentImage);
    
    removeDocumentButton = (GtkButton *)gtk_builder_get_object (builder, "removeDocumentButton");
    g_signal_connect (removeDocumentButton, "clicked", G_CALLBACK (signalRemoveDocumentButtonClicked), this);
    gtk_widget_hide(GTK_WIDGET(removeDocumentButton));
    
    
    notesTextView = (GtkTextView *)gtk_builder_get_object (builder, "notesTextView");
    GtkTextBuffer *notesTextBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(notesTextBuffer, game->getNotes().c_str(), game->getNotes().length());
    gtk_text_view_set_buffer(notesTextView, notesTextBuffer);   

    
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (signalCancelButtonClicked), this);

    saveButton = (GtkButton *)gtk_builder_get_object(builder, "saveButton");
    g_signal_connect (saveButton, "clicked", G_CALLBACK (signalSaveButtonClicked), this);
    
    loadDevelopers();    
    loadPublishers();    
    loadGenres();    
    loadEsrbRatings();
    
    loadGameImageTypes();
    updateGameImageGrid();
    
    loadGameDocumentTypes();
    updateGameDocumentGrid();
}

GameDialog::~GameDialog()
{
    delete game;    
    Developer::releaseItems(developers);        
    Publisher::releaseItems(publishers);    
    Genre::releaseItems(genres);
    EsrbRating::releaseItems(esrbRatings);
    
    GameImage::releaseItems(gameImages);
    GameImage::releaseItems(gameImagesToRemove);
    
    GameDocument::releaseItems(gameDocuments);
    GameDocument::releaseItems(gameDocumentsToRemove);
    
    
    gameImageBoxes->clear();
    delete gameImageBoxes;
    
    gameImageTypes->clear();
    delete gameImageTypes;
}

void GameDialog::deleteWhenReady(GameDialog *gameDialog)
{
    // Checks if the GameDialog is waiting for GameImages to be downloaded
    int isDownloading = 0;
    pthread_mutex_lock(&downloadGameImagesMutex);    
    for(list<UiThreadHandler *>::iterator downloadGameImageHandler = downloadGameImageHandlers->begin(); downloadGameImageHandler != downloadGameImageHandlers->end(); downloadGameImageHandler++)
    {
        DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)(*downloadGameImageHandler)->getRequesterInUiThread();
        if(downloadGameImage->gameDialog == gameDialog)
        {
            isDownloading = 1;
            break;
        }
    }
    pthread_mutex_unlock(&downloadGameImagesMutex);
    
    gameDialog->dismiss();
    if(!isDownloading)
    {
        delete gameDialog;
    }
}

void GameDialog::loadDevelopers()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(developersListBox), 1);
    
    for(unsigned int c = 0; c < developers->size(); c++)
    {
        Developer *developer = Developer::getItem(developers, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
        GtkWidget *optionRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "optionRowBox");
        GtkButton *removeButton = (GtkButton *)gtk_builder_get_object (rowBuilder, "removeButton");
        GtkLabel *label = (GtkLabel *)gtk_builder_get_object (rowBuilder, "label");
        
        gtk_label_set_text(label, developer->getName().c_str());
        
        gtk_widget_set_name(GTK_WIDGET(removeButton), to_string(developer->getId()).c_str());
        g_signal_connect (removeButton, "clicked", G_CALLBACK (signalRemoveDeveloperButtonClicked), this);
        
        gtk_container_add(GTK_CONTAINER(developersListBox), GTK_WIDGET(optionRowBox));
        gtk_widget_show_all(GTK_WIDGET(optionRowBox));
    }
}


void GameDialog::removeDeveloper(int64_t developerId)
{
    for(unsigned int c = 0; c < developers->size(); c++)
    {
        Developer *developer = Developer::getItem(developers, c);
        if(developer->getId() == developerId)
        {
            developers->remove(developer);
            delete developer;
            break;
        }
    }
    
    loadDevelopers();
}

void GameDialog::showDevelopersDialog()
{
    DevelopersSelectDialog *dialog = new DevelopersSelectDialog(developers);
    if(dialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Developer::releaseItems(developers);
        developers = new list<Developer *>;
        
        for(unsigned int c = 0; c < dialog->getSelectedItems()->size(); c++)
        {
            Developer *developer = new Developer(*Developer::getItem(dialog->getSelectedItems(), c));
            developers->push_back(developer);
        }
        
        loadDevelopers();
    }
    delete dialog;
}

void GameDialog::loadPublishers()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(publishersListBox), 1);
    
    for(unsigned int c = 0; c < publishers->size(); c++)
    {
        Publisher *publisher = Publisher::getItem(publishers, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
        GtkWidget *optionRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "optionRowBox");
        GtkButton *removeButton = (GtkButton *)gtk_builder_get_object (rowBuilder, "removeButton");
        GtkLabel *label = (GtkLabel *)gtk_builder_get_object (rowBuilder, "label");
        
        gtk_label_set_text(label, publisher->getName().c_str());
        
        gtk_widget_set_name(GTK_WIDGET(removeButton), to_string(publisher->getId()).c_str());
        g_signal_connect (removeButton, "clicked", G_CALLBACK (signalRemovePublisherButtonClicked), this);
        
        gtk_container_add(GTK_CONTAINER(publishersListBox), GTK_WIDGET(optionRowBox));
        gtk_widget_show_all(GTK_WIDGET(optionRowBox));
    }
}

void GameDialog::removePublisher(int64_t publisherId)
{
    for(unsigned int c = 0; c < publishers->size(); c++)
    {
        Publisher *publisher = Publisher::getItem(publishers, c);
        if(publisher->getId() == publisherId)
        {
            publishers->remove(publisher);
            delete publisher;
            break;
        }
    }
    
    loadPublishers();
}

void GameDialog::showPublishersDialog()
{
    PublishersSelectDialog *dialog = new PublishersSelectDialog(publishers);
    if(dialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Publisher::releaseItems(publishers);
        publishers = new list<Publisher *>;
        
        for(unsigned int c = 0; c < dialog->getSelectedItems()->size(); c++)
        {
            Publisher *publisher = new Publisher(*Publisher::getItem(dialog->getSelectedItems(), c));
            publishers->push_back(publisher);
        }
        
        loadPublishers();
    }
    delete dialog;
}

void GameDialog::loadGenres()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(genresListBox), 1);
    
    for(unsigned int c = 0; c < genres->size(); c++)
    {
        Genre *genre = Genre::getItem(genres, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
        GtkWidget *optionRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "optionRowBox");
        GtkButton *removeButton = (GtkButton *)gtk_builder_get_object (rowBuilder, "removeButton");
        GtkLabel *label = (GtkLabel *)gtk_builder_get_object (rowBuilder, "label");
        
        gtk_label_set_text(label, genre->getName().c_str());
        
        gtk_widget_set_name(GTK_WIDGET(removeButton), to_string(genre->getId()).c_str());
        g_signal_connect (removeButton, "clicked", G_CALLBACK (signalRemoveGenreButtonClicked), this);
        
        gtk_container_add(GTK_CONTAINER(genresListBox), GTK_WIDGET(optionRowBox));
        gtk_widget_show_all(GTK_WIDGET(optionRowBox));
    }
}

void GameDialog::removeGenre(int64_t genreId)
{
    for(unsigned int c = 0; c < genres->size(); c++)
    {
        Genre *genre = Genre::getItem(genres, c);
        if(genre->getId() == genreId)
        {
            genres->remove(genre);
            delete genre;
            break;
        }
    }
    
    loadGenres();
}

void GameDialog::showGenresDialog()
{
    GenresSelectDialog *dialog = new GenresSelectDialog(genres);
    if(dialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Genre::releaseItems(genres);
        genres = new list<Genre *>;
        
        for(unsigned int c = 0; c < dialog->getSelectedItems()->size(); c++)
        {
            Genre *genre = new Genre(*Genre::getItem(dialog->getSelectedItems(), c));
            genres->push_back(genre);
        }
        
        loadGenres();
    }
    delete dialog;
}

void GameDialog::loadEsrbRatings()
{
    GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);

    for(unsigned int c = 0; c < esrbRatings->size(); c++)
    {
        EsrbRating *esrbRating = EsrbRating::getItem(esrbRatings, c);
        gtk_list_store_insert_with_values (listStore, NULL, c, 0, esrbRating->getName().c_str(), -1);
    }
    gtk_combo_box_set_model(esrbComboBox, GTK_TREE_MODEL(listStore));
    
    GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(esrbComboBox), cellRenderer, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(esrbComboBox), cellRenderer, "text", 0, NULL);
    
    if(game->getEsrbRatingId() > 0)
    {
        for(unsigned int c = 0; c < esrbRatings->size(); c++)
        {
            EsrbRating *esrbRating = EsrbRating::getItem(esrbRatings, c);
            
            if(esrbRating->getId() == game->getEsrbRatingId())
            {
                gtk_combo_box_set_active(esrbComboBox, c);
                break;
            }
        }
    }
    else
    {
        gtk_combo_box_set_active(esrbComboBox, 0);
    }
}

void GameDialog::selectFileName()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select filename", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(fileNameEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void GameDialog::loadGameImageTypes()
{
    gameImageTypes->push_back(GameImage::TYPE_BOX_FRONT);
    gameImageTypes->push_back(GameImage::TYPE_BOX_BACK);
    gameImageTypes->push_back(GameImage::TYPE_SCREENSHOT);
    gameImageTypes->push_back(GameImage::TYPE_CLEAR_LOGO);
    gameImageTypes->push_back(GameImage::TYPE_BANNER);
    
    int position = 0;
    GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);

    for(list<int64_t>::iterator type = gameImageTypes->begin(); type != gameImageTypes->end(); type++)
    {
        if(*type == GameImage::TYPE_BOX_FRONT)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Box front", -1);
        }
        else if(*type == GameImage::TYPE_BOX_BACK)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Box back", -1);
        }
        else if(*type == GameImage::TYPE_SCREENSHOT)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Screenshot", -1);
        }
        else if(*type == GameImage::TYPE_CLEAR_LOGO)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Logo", -1);
        }
        else if(*type == GameImage::TYPE_BANNER)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Banner", -1);
        }
    }
    gtk_combo_box_set_model(imageTypeComboBox, GTK_TREE_MODEL(listStore));
    
    GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, "text", 0, NULL);
}

void GameDialog::updateGameImageGrid()
{    
    gameImageBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(imagesGridListBox), 1);
 
    int columns = 3;
    int rows = gameImages->size() / columns;
    if(gameImages->size() % columns)
    {
        rows++;
    }
    
    // Adds an extra row for the add button (just in case)
    rows++;
    
    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand(GTK_WIDGET(rowBox), 1);
        gtk_widget_set_vexpand(GTK_WIDGET(rowBox), 1);
        
        for(int column = 0; column < columns; column++)
        {
            if(index < gameImages->size())
            {
                GameImage *gameImage = GameImage::getItem(gameImages, index);
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Settings::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                string fileName;
                if(!gameImage->getId())
                {
                    if(Utils::getInstance()->fileExists(gameImage->getFileName()))
                    {
                        fileName = gameImage->getFileName();
                    }
                    else
                    {
                        fileName = Asset::getInstance()->getImageDownloading();
                    }
                }
                else
                {
                    fileName = gameImage->getThumbnailFileName();
                }
                UiUtils::getInstance()->loadImage(image, fileName, THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                
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
            else if(index == gameImages->size())
            {
                GtkWidget *addImageButton = gtk_button_new_with_label("+");
                g_signal_connect (addImageButton, "clicked", G_CALLBACK (signalAddImageButtonClicked), this);
                gtk_widget_set_size_request(GTK_WIDGET(addImageButton), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                
                gtk_box_pack_start(rowBox, GTK_WIDGET(addImageButton), 1, 1, 0);                
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                gtk_widget_set_hexpand(GTK_WIDGET(dummyBox), 1);
                gtk_widget_set_vexpand(GTK_WIDGET(dummyBox), 1);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_container_add (GTK_CONTAINER(imagesGridListBox), GTK_WIDGET(rowBox));
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }    
}

void GameDialog::addGameImage()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select image", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    
    GtkFileFilter *fileFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern (fileFilter, "*.png");
    gtk_file_filter_add_pattern (fileFilter, "*.jpg");
    gtk_file_filter_add_pattern (fileFilter, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fileChooserDialog), fileFilter);

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        string imageFileName = string(cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(imageFileName));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free (cFileName);
                
        GameImage *gameImage = new GameImage((int64_t)0);
        gameImage->setApiId(0);
        gameImage->setApiItemId(0);
        gameImage->setType(GameImage::TYPE_BOX_FRONT);
        gameImage->setFileName(imageFileName);
        gameImage->setExternal(0);
                
        gameImages->push_back(gameImage);
        
        updateGameImageGrid();
        selectGameImage(gameImage);
    }
    gtk_widget_destroy (fileChooserDialog);
}

void GameDialog::removeGameImage()
{
    if(!selectedGameImage)
    {
        return;
    }
    
    gameImages->remove(selectedGameImage);
    
    if(selectedGameImage->getId())
    {
        gameImagesToRemove->push_back(selectedGameImage);
    }
    else
    {
        delete selectedGameImage;
    }    
        
    selectGameImage(NULL);
    updateGameImageGrid();
}

void GameDialog::removeNewGameImages()
{
    selectGameImage(NULL);
    
    list<GameImage *> *notNewGameImages = new list<GameImage *>;
    
    // Checks if there are GameImages being downloaded, GameImages that are being downloaded cannot be deleted
    for(list<GameImage *>::iterator gameImage = gameImages->begin(); gameImage != gameImages->end(); gameImage++)
    {
        if((*gameImage)->getId())
        {
            notNewGameImages->push_back((*gameImage));
            continue;
        }
                
        pthread_mutex_lock(&downloadGameImagesMutex);
        int isDownloading = 0;
        for(list<UiThreadHandler *>::iterator downloadGameImageHandler = downloadGameImageHandlers->begin(); downloadGameImageHandler != downloadGameImageHandlers->end(); downloadGameImageHandler++)
        {
            DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)(*downloadGameImageHandler)->getRequesterInUiThread();
            if(downloadGameImage->gameDialog == this && downloadGameImage->gameImage == (*gameImage))
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {            
            delete (*gameImage);
        }
        pthread_mutex_unlock(&downloadGameImagesMutex);                
    }
    
    gameImages->clear();
    for(list<GameImage *>::iterator gameImage = notNewGameImages->begin(); gameImage != notNewGameImages->end(); gameImage++)
    {
        gameImages->push_back((*gameImage));        
    }
    notNewGameImages->clear();
    delete notNewGameImages;
    
    updateGameImageGrid();
}

void GameDialog::updateGameImageType()
{
    if(!selectedGameImage)
    {
        return;
    }
    
    int position = gtk_combo_box_get_active(imageTypeComboBox);
    if(position >= 0 && position < (int)gameImageTypes->size())
    {
        list<int64_t>::iterator type = gameImageTypes->begin();
	advance(type, position);
        
        selectedGameImage->setType(*type);
    }
}

void GameDialog::selectGameImage(GameImage *gameImage)
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
    if(selectedGameImage)
    {
        if(gameImageBoxes->find(selectedGameImage) != gameImageBoxes->end())
        {
            GtkWidget *imageBox = gameImageBoxes->at(selectedGameImage);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
        }

        UiUtils::getInstance()->loadImage(image, selectedGameImage->getFileName(), IMAGE_WIDTH, IMAGE_HEIGHT);


        int position = 0;
        for(list<int64_t>::iterator type = gameImageTypes->begin(); type != gameImageTypes->end(); type++)
        {
            if(*type == selectedGameImage->getType())
            {
                gtk_combo_box_set_active(imageTypeComboBox, position);
            }
            position++;
        }

        gtk_widget_show(GTK_WIDGET(imageTypeComboBox));
        gtk_widget_show(GTK_WIDGET(removeImageButton));        
    }
    else
    {
        gtk_image_clear(image);
        gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
        gtk_widget_hide(GTK_WIDGET(removeImageButton));
    }
}

void GameDialog::downloadGameImage(GameImage* gameImage)
{
    downloadGameImage(this, gameImage);
}

void GameDialog::saveNewGameImage(GameImage* gameImage)
{
    // When the image is new, copies the file to the media directory and creates the thumbnail.
    sqlite3 *sqlite = Database::getInstance()->acquire();
    gameImage->setGameId(game->getId());
    gameImage->save(sqlite);
    
    if(Utils::getInstance()->fileExists(gameImage->getFileName()))
    {
        string imageFileName = game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId());
        if(!Utils::getInstance()->copyFile(gameImage->getFileName(), imageFileName))
        {
            gameImage->setFileName(imageFileName);

            Utils::getInstance()->scaleImage(gameImage->getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, gameImage->getThumbnailFileName());
            gameImage->save(sqlite);
        }        
    }
    Database::getInstance()->release();
}

void GameDialog::loadGameDocumentTypes()
{
    gameDocumentTypes->push_back(GameDocument::TYPE_MANUAL);
    gameDocumentTypes->push_back(GameDocument::TYPE_MAGAZINE);
    gameDocumentTypes->push_back(GameDocument::TYPE_GUIDE);
    gameDocumentTypes->push_back(GameDocument::TYPE_BOOK);
    gameDocumentTypes->push_back(GameDocument::TYPE_OTHER);
    
    int position = 0;
    GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);

    for(list<int64_t>::iterator type = gameDocumentTypes->begin(); type != gameDocumentTypes->end(); type++)
    {
        if(*type == GameDocument::TYPE_MANUAL)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Manual", -1);
        }
        else if(*type == GameDocument::TYPE_MAGAZINE)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Magazine", -1);
        }
        else if(*type == GameDocument::TYPE_GUIDE)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Guide", -1);
        }
        else if(*type == GameDocument::TYPE_BOOK)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Book", -1);
        }
        else if(*type == GameDocument::TYPE_OTHER)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Other", -1);
        }
    }
    gtk_combo_box_set_model(documentTypeComboBox, GTK_TREE_MODEL(listStore));
    
    GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(documentTypeComboBox), cellRenderer, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(documentTypeComboBox), cellRenderer, "text", 0, NULL);
}

void GameDialog::updateGameDocumentGrid()
{
    gameDocumentBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(documentGridListBox), 1);
 
    int columns = 3;
    int rows = gameDocuments->size() / columns;
    if(gameDocuments->size() % columns)
    {
        rows++;
    }
    
    // Adds an extra row for the add button (just in case)
    rows++;
    
    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand(GTK_WIDGET(rowBox), 1);
        gtk_widget_set_vexpand(GTK_WIDGET(rowBox), 1);
        
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
            else if(index == gameDocuments->size())
            {
                GtkWidget *addDocumentButton = gtk_button_new_with_label("+");
                g_signal_connect (addDocumentButton, "clicked", G_CALLBACK (signalAddDocumentButtonClicked), this);
                gtk_widget_set_size_request(GTK_WIDGET(addDocumentButton), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                
                gtk_box_pack_start(rowBox, GTK_WIDGET(addDocumentButton), 1, 1, 0);                
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                gtk_widget_set_hexpand(GTK_WIDGET(dummyBox), 1);
                gtk_widget_set_vexpand(GTK_WIDGET(dummyBox), 1);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_container_add (GTK_CONTAINER(documentGridListBox), GTK_WIDGET(rowBox));
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }
}

void GameDialog::addGameDocument()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select document", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    
    GtkFileFilter *fileFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern (fileFilter, "*.pdf");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fileChooserDialog), fileFilter);

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        string fileName = string(cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(fileName));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free (cFileName);
                
        GameDocument *gameDocument = new GameDocument((int64_t)0);
        gameDocument->setApiId(0);
        gameDocument->setApiItemId(0);
        gameDocument->setType(GameDocument::TYPE_MANUAL);
        gameDocument->setName("");
        gameDocument->setFileName(fileName);
        
        string tempPreviewImageFileName = Utils::getInstance()->getTempFileName();
        Utils::getInstance()->createPreviewImageFromPdf(fileName, tempPreviewImageFileName);
        gameDocument->setPreviewImageFileName(tempPreviewImageFileName);
                
        gameDocuments->push_back(gameDocument);
        
        updateGameDocumentGrid();
        selectGameDocument(gameDocument);
    }
    gtk_widget_destroy (fileChooserDialog);    
}

void GameDialog::removeGameDocument()
{
    if(!selectedGameDocument)
    {
        return;
    }
    
    gameDocuments->remove(selectedGameDocument);
    
    if(selectedGameDocument->getId())
    {
        gameDocumentsToRemove->push_back(selectedGameDocument);
    }
    else
    {
        delete selectedGameDocument;
    }    
    selectedGameDocument = NULL;
    
    gtk_widget_hide(GTK_WIDGET(documentTypeComboBox));
    gtk_widget_hide(GTK_WIDGET(removeDocumentButton));
    gtk_widget_hide(GTK_WIDGET(documentNameEntry));
    gtk_entry_set_text(documentNameEntry, "");
    gtk_image_clear(documentImage);
    updateGameDocumentGrid();
}

void GameDialog::updateGameDocumentType()
{
    if(!selectedGameDocument)
    {
        return;
    }
    
    int position = gtk_combo_box_get_active(documentTypeComboBox);
    if(position >= 0 && position < (int)gameDocumentTypes->size())
    {
        list<int64_t>::iterator type = gameDocumentTypes->begin();
	advance(type, position);
        
        selectedGameDocument->setType(*type);
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
    
    UiUtils::getInstance()->loadImage(documentImage, gameDocument->getPreviewImageFileName(), IMAGE_WIDTH, IMAGE_HEIGHT);
    
    
    int position = 0;
    for(list<int64_t>::iterator type = gameDocumentTypes->begin(); type != gameDocumentTypes->end(); type++)
    {
        if(*type == selectedGameDocument->getType())
        {
            gtk_combo_box_set_active(documentTypeComboBox, position);
        }
        position++;
    }
    
    gtk_entry_set_text(documentNameEntry, selectedGameDocument->getName().c_str());
    
    gtk_widget_show(GTK_WIDGET(documentTypeComboBox));
    gtk_widget_show(GTK_WIDGET(removeDocumentButton));
    gtk_widget_show(GTK_WIDGET(documentNameEntry));
}

void GameDialog::search()
{
    string query = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(query.length() == 0)
    {
        return;
    }
    
    if(TheGamesDB::Elasticsearch::getInstance()->getStatus() != TheGamesDB::Elasticsearch::STATUS_OK)
    {
        MessageDialog *messageDialog = new MessageDialog("Database is not ready yet. Please wait a moment.", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    Platform *platform = new Platform(game->getPlatformId());
    sqlite3 *sqlite = Database::getInstance()->acquire();
    platform->load(sqlite);
    Database::getInstance()->release();
    
    GameSearchDialog *dialog = new GameSearchDialog(platform->getApiItemId(), query);
    if(dialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        TheGamesDB::Game *apiGame = dialog->getSelectedApiGame();
        if(apiGame)
        {
            game->setApiId(TheGamesDB::API_ID);
            game->setApiItemId(apiGame->getId());
            
            gtk_entry_set_text(nameEntry, apiGame->getName().c_str());
            gtk_entry_set_text(releaseDateEntry, apiGame->getReleaseDate().c_str());
            
            // Assigns the ESRB rating.
            for(unsigned int c = 0; c < esrbRatings->size(); c++)
            {
                EsrbRating *esrbRating = EsrbRating::getItem(esrbRatings, c);

                if(esrbRating->getApiItemId() == apiGame->getEsrbRatingId())
                {
                    gtk_combo_box_set_active(esrbComboBox, c);
                    break;
                }
            }
            
            // Assigns developers.
            Developer::releaseItems(developers);
            developers = new list<Developer *>;
            for(unsigned int c = 0; c < apiGame->getGameDevelopers()->size(); c++)
            {                
                TheGamesDB::GameDeveloper *apiGameDeveloper = TheGamesDB::GameDeveloper::getItem(apiGame->getGameDevelopers(), c);
                sqlite3 *sqlite = Database::getInstance()->acquire();
                Developer *developer = Developer::getDeveloper(sqlite, TheGamesDB::API_ID, apiGameDeveloper->getDeveloperId());
                if(developer)
                {
                    developers->push_back(developer);
                }
                Database::getInstance()->release();                    
            }
            loadDevelopers();
            
            // Assigns publishers.
            Publisher::releaseItems(publishers);
            publishers = new list<Publisher *>;
            for(unsigned int c = 0; c < apiGame->getGamePublishers()->size(); c++)
            {                
                TheGamesDB::GamePublisher *apiGamePublisher = TheGamesDB::GamePublisher::getItem(apiGame->getGamePublishers(), c);
                sqlite3 *sqlite = Database::getInstance()->acquire();
                Publisher *publisher = Publisher::getPublisher(sqlite, TheGamesDB::API_ID, apiGamePublisher->getPublisherId());
                if(publisher)
                {
                    publishers->push_back(publisher);
                }
                Database::getInstance()->release();                    
            }
            loadPublishers();
            
            // Assigns genres.
            Genre::releaseItems(genres);
            genres = new list<Genre *>;
            for(unsigned int c = 0; c < apiGame->getGameGenres()->size(); c++)
            {                
                TheGamesDB::GameGenre *apiGameGenre = TheGamesDB::GameGenre::getItem(apiGame->getGameGenres(), c);
                sqlite3 *sqlite = Database::getInstance()->acquire();
                Genre *genre = Genre::getGenre(sqlite, TheGamesDB::API_ID, apiGameGenre->getGenreId());
                if(genre)
                {
                    genres->push_back(genre);
                }
                Database::getInstance()->release();                    
            }
            loadGenres();
            
            
            // Updates description
            GtkTextBuffer *descriptionTextBuffer = gtk_text_buffer_new(NULL);
            gtk_text_buffer_set_text(descriptionTextBuffer, apiGame->getDescription().c_str(), apiGame->getDescription().length());
            gtk_text_view_set_buffer(descriptionTextView, descriptionTextBuffer);
            
            
            // Updates images            
            removeNewGameImages();
            for(unsigned int c = 0; c < apiGame->getGameImages()->size(); c++)
            {
                TheGamesDB::GameImage *apiGameImage = TheGamesDB::GameImage::getItem(apiGame->getGameImages(), c);

                int64_t type = 0;
                if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_BOXART) == 0)
                {
                    if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_FRONT) == 0)
                    {
                        type = GameImage::TYPE_BOX_FRONT;
                    }
                    else if(apiGameImage->getSide().compare(TheGamesDB::GameImage::SIDE_BACK) == 0)
                    {
                        type = GameImage::TYPE_BOX_BACK;
                    }
                }

                // Checks if there is GameImage for this TheGamesDB::GameImage being downloaded, if it is, then appends it to the list
                int isDownloading = 0;
                pthread_mutex_lock(&downloadGameImagesMutex);        
                for(list<UiThreadHandler *>::iterator downloadGameImageHandler = downloadGameImageHandlers->begin(); downloadGameImageHandler != downloadGameImageHandlers->end(); downloadGameImageHandler++)
                {
                    DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)(*downloadGameImageHandler)->getRequesterInUiThread();
                    if(downloadGameImage->gameDialog == this && downloadGameImage->gameImage->getApiId() == TheGamesDB::API_ID && downloadGameImage->gameImage->getApiItemId() == apiGameImage->getId())
                    {
                        gameImages->push_back(downloadGameImage->gameImage);
                        isDownloading = 1;
                        break;
                    }
                }
                pthread_mutex_unlock(&downloadGameImagesMutex);

                if(!isDownloading)
                {
                    GameImage *gameImage = new GameImage((int64_t)0);
                    gameImage->setApiId(TheGamesDB::API_ID);
                    gameImage->setApiItemId(apiGameImage->getId());
                    gameImage->setType(type);
                    gameImage->setFileName(apiGameImage->getFileName());
                    gameImage->setExternal(0);
                    gameImage->setDownloaded(0);
                    gameImage->setUrl(apiGameImage->getOriginal());

                    gameImages->push_back(gameImage);

                    if(!Utils::getInstance()->fileExists(apiGameImage->getFileName()))
                    {
                        downloadGameImage(gameImage);
                    }            
                }
            }

            updateGameImageGrid();    
            if(gameImages->size() > 0)
            {
                selectGameImage(GameImage::getItem(gameImages, 0));
            }
            
            
        }
    }
    delete dialog;
    delete platform;
}

void GameDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void GameDialog::save()
{
    string name = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(name.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("Name cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    string fileName = Utils::getInstance()->trim(string(gtk_entry_get_text(fileNameEntry)));
    if(fileName.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("Filename cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    game->setName(name);
    game->setFileName(fileName);
    game->setCommand(string(gtk_entry_get_text(commandEntry)));
    game->setDeflate(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(deflateCheckButton)));
    game->setDeflateFileExtensions(string(gtk_entry_get_text(deflateFileExtensionsEntry)));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(gtk_text_view_get_buffer(descriptionTextView), &start, &end);    
    game->setDescription(string(gtk_text_buffer_get_text (gtk_text_view_get_buffer(descriptionTextView), &start, &end, FALSE)));
    
    game->setReleaseDate(string(gtk_entry_get_text(releaseDateEntry)));
    
    if(esrbRatings->size() > 0)
    {
        EsrbRating *esrbRating = EsrbRating::getItem(esrbRatings, gtk_combo_box_get_active(esrbComboBox));
        game->setEsrbRatingId(esrbRating->getId());
    }
    else
    {
        game->setEsrbRatingId(0);
    }
    
    gtk_text_buffer_get_bounds(gtk_text_view_get_buffer(notesTextView), &start, &end);    
    game->setNotes(string(gtk_text_buffer_get_text (gtk_text_view_get_buffer(notesTextView), &start, &end, FALSE)));
    
    game->setTimestamp(Utils::getInstance()->nowIsoDateTime());
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    game->save(sqlite);
    
    
    GameDeveloper::remove(sqlite, game->getId());
    for(unsigned int c = 0; c < developers->size(); c++)
    {
        Developer *developer = Developer::getItem(developers, c);
        
        GameDeveloper *gameDeveloper = new GameDeveloper(game->getId(), developer->getId());
        gameDeveloper->save(sqlite);
        delete gameDeveloper;
    }
    
    GamePublisher::remove(sqlite, game->getId());
    for(unsigned int c = 0; c < publishers->size(); c++)
    {
        Publisher *publisher = Publisher::getItem(publishers, c);
        
        GamePublisher *gamePublisher = new GamePublisher(game->getId(), publisher->getId());
        gamePublisher->save(sqlite);
        delete gamePublisher;
    }
    
    GameGenre::remove(sqlite, game->getId());
    for(unsigned int c = 0; c < genres->size(); c++)
    {
        Genre *genre = Genre::getItem(genres, c);
        
        GameGenre *gameGenre = new GameGenre(game->getId(), genre->getId());
        gameGenre->save(sqlite);
        delete gameGenre;
    }
    

    for(unsigned int c = 0; c < gameDocumentsToRemove->size(); c++)
    {
        GameDocument *gameDocument = GameDocument::getItem(gameDocumentsToRemove, c);
        gameDocument->remove(sqlite);
    }
    for(unsigned int c = 0; c < gameDocuments->size(); c++)
    {
        GameDocument *gameDocument = GameDocument::getItem(gameDocuments, c);
        
        // When the document is new, copies the file to the media directory
        if(!gameDocument->getId())
        {
            gameDocument->setGameId(game->getId());
            gameDocument->save(sqlite);

            string imageFileName = game->getMediaDirectory() + GameDocument::FILE_PREFIX + to_string(gameDocument->getId());
            if(!Utils::getInstance()->copyFile(gameDocument->getFileName(), imageFileName))
            {
                gameDocument->setFileName(imageFileName);
                
                string tempPreviewImageFileName = gameDocument->getPreviewImageFileName();
                gameDocument->setPreviewImageFileName("");
                Utils::getInstance()->copyFile(tempPreviewImageFileName, gameDocument->getPreviewImageFileName());
                
                gameDocument->save(sqlite);
            }
        }
        else
        {
            gameDocument->save(sqlite);
        }
    }


    for(unsigned int c = 0; c < gameImagesToRemove->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gameImagesToRemove, c);
        gameImage->remove(sqlite);
    }    
    Database::getInstance()->release();
    
        
    for(unsigned int c = 0; c < gameImages->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gameImages, c);
        if(!gameImage->getId())
        {
            // Checks if the image is still downloading, in this case, the image should be saved after it is downloaded
            int isDownloading = 0;
            pthread_mutex_lock(&downloadGameImagesMutex);            
            for(list<UiThreadHandler *>::iterator downloadGameImageHandler = downloadGameImageHandlers->begin(); downloadGameImageHandler != downloadGameImageHandlers->end(); downloadGameImageHandler++)
            {
                DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)(*downloadGameImageHandler)->getRequesterInUiThread();
                if(downloadGameImage->gameDialog == this && downloadGameImage->gameImage == gameImage)
                {
                    isDownloading = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&downloadGameImagesMutex);

            if(!isDownloading)
            {
                saveNewGameImage(gameImage);
            }
        }
        else
        {
            sqlite3 *sqlite = Database::getInstance()->acquire();
            gameImage->save(sqlite);
            Database::getInstance()->release();
        }
    }
            
    saved = 1;
    NotificationManager::getInstance()->postNotification(NOTIFICATION_GAME_UPDATED, game);
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);        
}




void GameDialog::signalSearchButtonClicked(GtkButton *button, gpointer gameDialog)
{    
    ((GameDialog *)gameDialog)->search();
}

void GameDialog::signalFileNameButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->selectFileName();
}

void GameDialog::signalAddDeveloperButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->showDevelopersDialog();
}

void GameDialog::signalRemoveDeveloperButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->removeDeveloper(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameDialog::signalAddPublisherButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->showPublishersDialog();
}

void GameDialog::signalRemovePublisherButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->removePublisher(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameDialog::signalAddGenreButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->showGenresDialog();
}

void GameDialog::signalRemoveGenreButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->removeGenre(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameDialog::signalImageTypeComboBoxChanged(GtkComboBox *comboBox, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->updateGameImageType();
}

void GameDialog::signalRemoveImageButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->removeGameImage();
}

void GameDialog::signalAddImageButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->addGameImage();
}

gboolean GameDialog::signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameImage *gameImage = GameImage::getItem(((GameDialog *)gameDialog)->gameImages, atoi(gtk_widget_get_name(widget)));
        ((GameDialog *)gameDialog)->selectGameImage(gameImage);
    }
    
    return TRUE;
}

void GameDialog::signalDocumentTypeComboBoxChanged(GtkComboBox *comboBox, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->updateGameDocumentType();
}

void GameDialog::signalRemoveDocumentButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->removeGameDocument();
}

void GameDialog::signalAddDocumentButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->addGameDocument();
}

gboolean GameDialog::signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameDialog *)gameDialog)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        ((GameDialog *)gameDialog)->selectGameDocument(gameDocument);
    }
    
    return TRUE;
}

gboolean GameDialog::signalDocumentNameEntryKeyReleaseEvent(GtkEntry* entry, GdkEvent *event, gpointer gameDialog)
{    
    if(((GameDialog *)gameDialog)->selectedGameDocument)
    {
        //cout << __FUNCTION__ << " signalDocumentNameEntryKeyPressedEvent: " << gtk_entry_get_text(entry) << endl;
        ((GameDialog *)gameDialog)->selectedGameDocument->setName(string(gtk_entry_get_text(entry)));
    }
    return 0;
}

void GameDialog::signalCancelButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->cancel();
}

void GameDialog::signalSaveButtonClicked(GtkButton *button, gpointer gameDialog)
{
    ((GameDialog *)gameDialog)->save();
}




list<UiThreadHandler *> *GameDialog::downloadGameImageHandlers = new list<UiThreadHandler *>;
pthread_t GameDialog::downloadGameImagesThread;
pthread_mutex_t GameDialog::downloadGameImagesMutex = PTHREAD_MUTEX_INITIALIZER;
int GameDialog::downloadingGameImages = 0;

void GameDialog::downloadGameImage(GameDialog *gameDialog, GameImage *gameImage)
{
    pthread_mutex_lock(&downloadGameImagesMutex);
    
    DownloadGameImage_t *downloadGameImage = new DownloadGameImage_t;
    downloadGameImage->gameDialog = gameDialog;
    downloadGameImage->gameImage = gameImage;
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(downloadGameImage, callbackDownloadGameImage);
    downloadGameImageHandlers->push_back(uiThreadHandler);
        
    if(!downloadingGameImages)
    {
        downloadingGameImages = 1;
        if(pthread_create(&downloadGameImagesThread, NULL, downloadGameImagesWorker, NULL) != 0) 
        {
            cerr << "GameDialog::" << __FUNCTION__ << endl;
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&downloadGameImagesMutex);
}

void *GameDialog::downloadGameImagesWorker(void *)
{
    UiThreadHandler *downloadGameImageHandler = NULL;
    do
    {
        downloadGameImageHandler = NULL;
        
        pthread_mutex_lock(&downloadGameImagesMutex);
        for(list<UiThreadHandler *>::iterator aDownloadGameImageHandler = downloadGameImageHandlers->begin(); aDownloadGameImageHandler != downloadGameImageHandlers->end(); aDownloadGameImageHandler++)
        {
            DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)(*aDownloadGameImageHandler)->getRequesterInUiThread();
            if(!downloadGameImage->gameImage->getDownloaded())
            {
                downloadGameImageHandler = (*aDownloadGameImageHandler);
                break;
            }
        }
        pthread_mutex_unlock(&downloadGameImagesMutex);
        
        if(downloadGameImageHandler)
        {
            DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)downloadGameImageHandler->getRequesterInUiThread();
            
            HttpConnector *httpConnector = new HttpConnector(downloadGameImage->gameImage->getUrl());
            httpConnector->get();
            cout  << "GameDialog::" << __FUNCTION__ << " fileName: " << downloadGameImage->gameImage->getFileName() << " url: " << downloadGameImage->gameImage->getUrl() << " httpStatus: " << httpConnector->getHttpStatus() << endl;
            if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
            {
                Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), downloadGameImage->gameImage->getFileName());
                downloadGameImage->gameImage->setDownloaded(1);
            }
            delete httpConnector;
            
            UiThreadHandler::callback(downloadGameImageHandler, new char[1]); //Sends dummy data to avoid NULL
        }
        
    }while(downloadGameImageHandler);
    
    downloadingGameImages = 0;    
    return NULL;
}

int GameDialog::callbackDownloadGameImage(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;
    DownloadGameImage_t *downloadGameImage = (DownloadGameImage_t *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();    
    
    pthread_mutex_lock(&downloadGameImagesMutex);
    downloadGameImageHandlers->remove(uiThreadHandlerResult->uiThreadHandler);
    pthread_mutex_unlock(&downloadGameImagesMutex);
    
    // Checks if the GameImage is still present in the list of the GameDialog
    int isImage = 0;
    for(list<GameImage *>::iterator gameImage = downloadGameImage->gameDialog->gameImages->begin(); gameImage != downloadGameImage->gameDialog->gameImages->end(); gameImage++)
    {
        if(downloadGameImage->gameImage == (*gameImage))
        {
            isImage = 1;
            break;
        }
    }
    
    if(isImage)
    {
        // Checks if the GameDialog is still been shown
        if(!downloadGameImage->gameDialog->dismissed)
        {
            // Checks if the GameImage has been downloaded
            if(downloadGameImage->gameImage->getDownloaded())
            {
                downloadGameImage->gameDialog->updateGameImageGrid();
            }            
        }
        // Checks if the GameDialog was dismissed with the save instruction
        else
        {
            if(downloadGameImage->gameDialog->saved)
            {
               downloadGameImage->gameDialog->saveNewGameImage(downloadGameImage->gameImage); 
            }
        }
    }
    else
    {
        delete downloadGameImage->gameImage;
    }
    
    // Checks if the GameDialog was dismissed and if all its images has finished downloading
    if(downloadGameImage->gameDialog->dismissed)
    {
        pthread_mutex_lock(&downloadGameImagesMutex);
        int isDialog = 0;
        for(list<UiThreadHandler *>::iterator aDownloadGameImageHandler = downloadGameImageHandlers->begin(); aDownloadGameImageHandler != downloadGameImageHandlers->end(); aDownloadGameImageHandler++)
        {
            DownloadGameImage_t *aDownloadGameImage = (DownloadGameImage_t *)(*aDownloadGameImageHandler)->getRequesterInUiThread();
            if(aDownloadGameImage->gameDialog == downloadGameImage->gameDialog)
            {
                isDialog = 1;
                break;
            }
        }
        if(!isDialog)
        {
            if(downloadGameImage->gameDialog->saved)
            {
                NotificationManager::getInstance()->postNotification(NOTIFICATION_GAME_UPDATED, downloadGameImage->gameDialog->game);
            }
            delete downloadGameImage->gameDialog;                
        }
        pthread_mutex_unlock(&downloadGameImagesMutex);        
    }

    
    UiThreadHandler::releaseResult(uiThreadHandlerResult);
    
    return G_SOURCE_REMOVE;
}