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
 * File:   GameEditDialog.cpp
 * Author: ram
 * 
 * Created on March 21, 2019, 12:47 AM
 */

#include "GameEditDialog.h"
#include "Preferences.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Platform.h"
#include "MessageDialog.h"
#include "DevelopersSelectDialog.h"
#include "PublishersSelectDialog.h"
#include "GenresSelectDialog.h"
#include "GameSearchDialog.h"
#include "Asset.h"
#include "HttpConnector.h"
#include "Notifications.h"
#include "NotificationManager.h"
#include "Directory.h"
#include "ThreadManager.h"
#include "thegamesdb.h"

#include <iostream>

const int GameEditDialog::THUMBNAIL_IMAGE_WIDTH = 90;
const int GameEditDialog::THUMBNAIL_IMAGE_HEIGHT = 90;
const int GameEditDialog::IMAGE_WIDTH = 300;
const int GameEditDialog::IMAGE_HEIGHT = 400;

GameEditDialog::GameEditDialog(GtkWindow *parent, int64_t gameId, int64_t platformId) : Dialog(parent, "GameEditDialog.ui", "gameEditDialog")
{
    saved = 0;
        
    game = new Game(gameId);    
    if(!game->load())
    {
        game->setPlatformId(platformId);
    }    
    
    esrbRatings = EsrbRating::getItems();
    gameImages = GameImage::getItems(game->getId());
    gameDocuments = GameDocument::getItems(game->getId());
    
    developers = new list<Developer *>;
    list<GameDeveloper *> *gameDevelopers = GameDeveloper::getItems(game->getId());
    for(unsigned int c = 0; c < gameDevelopers->size(); c++)
    {
        GameDeveloper *gameDeveloper = GameDeveloper::getItem(gameDevelopers, c);
        
        Developer *developer = new Developer(gameDeveloper->getDeveloperId());
        developer->load();
        developers->push_back(developer);
    }
    GameDeveloper::releaseItems(gameDevelopers);
    
    publishers = new list<Publisher *>;
    list<GamePublisher *> *gamePublishers = GamePublisher::getItems(game->getId());
    for(unsigned int c = 0; c < gamePublishers->size(); c++)
    {
        GamePublisher *gamePublisher = GamePublisher::getItem(gamePublishers, c);
        
        Publisher *publisher = new Publisher(gamePublisher->getPublisherId());
        publisher->load();
        publishers->push_back(publisher);
    }
    GamePublisher::releaseItems(gamePublishers);
    
    genres = new list<Genre *>;
    list<GameGenre *> *gameGenres = GameGenre::getItems(game->getId());
    for(unsigned int c = 0; c < gameGenres->size(); c++)
    {
        GameGenre *gameGenre = GameGenre::getItem(gameGenres, c);
        
        Genre *genre = new Genre(gameGenre->getGenreId());
        genre->load();
        genres->push_back(genre);
    }
    GameGenre::releaseItems(gameGenres);
    
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
    searchButton = (GtkButton *)gtk_builder_get_object(builder, "searchButton");    
    fileNameEntry = (GtkEntry *)gtk_builder_get_object (builder, "fileNameEntry");        
    fileNameButton = (GtkButton *)gtk_builder_get_object(builder, "fileNameButton");    
    commandEntry = (GtkEntry *)gtk_builder_get_object (builder, "commandEntry");    
    deflateCheckButton = (GtkCheckButton *)gtk_builder_get_object (builder, "deflateCheckButton");        
    deflateFileExtensionsEntry = (GtkEntry *)gtk_builder_get_object (builder, "deflateFileExtensionsEntry");
    releaseDateEntry = (GtkEntry *)gtk_builder_get_object (builder, "releaseDateEntry");    
    esrbComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "esrbComboBox");    
    addDeveloperButton = (GtkButton *)gtk_builder_get_object(builder, "addDeveloperButton");    
    addPublisherButton = (GtkButton *)gtk_builder_get_object(builder, "addPublisherButton");    
    addGenreButton = (GtkButton *)gtk_builder_get_object(builder, "addGenreButton");            
    developersListBox = (GtkListBox *)gtk_builder_get_object (builder, "developersListBox");
    publishersListBox = (GtkListBox *)gtk_builder_get_object (builder, "publishersListBox");    
    genresListBox = (GtkListBox *)gtk_builder_get_object (builder, "genresListBox");            
    descriptionTextView = (GtkTextView *)gtk_builder_get_object (builder, "descriptionTextView");                
    imagesGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "imagesGridListBox");    
    imageTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "imageTypeComboBox");            
    image = (GtkImage *)gtk_builder_get_object (builder, "image");    
    removeImageButton = (GtkButton *)gtk_builder_get_object (builder, "removeImageButton");
    documentGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "documentGridListBox");    
    documentTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "documentTypeComboBox");            
    documentNameEntry = (GtkEntry *)gtk_builder_get_object (builder, "documentNameEntry");            
    documentImage = (GtkImage *)gtk_builder_get_object (builder, "documentImage");        
    removeDocumentButton = (GtkButton *)gtk_builder_get_object (builder, "removeDocumentButton");                
    notesTextView = (GtkTextView *)gtk_builder_get_object (builder, "notesTextView");
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");    
    saveButton = (GtkButton *)gtk_builder_get_object(builder, "saveButton");        
    
    
    gtk_entry_set_text(nameEntry, game->getName().c_str());
    gtk_entry_set_text(fileNameEntry, game->getFileName().c_str());
    gtk_entry_set_text(commandEntry, game->getCommand().c_str());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(deflateCheckButton), game->getDeflate());
    gtk_entry_set_text(deflateFileExtensionsEntry, game->getDeflateFileExtensions().c_str());
    gtk_entry_set_text(releaseDateEntry, game->getReleaseDate().c_str());
    
    GtkTextBuffer *descriptionTextBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(descriptionTextBuffer, game->getDescription().c_str(), game->getDescription().length());
    gtk_text_view_set_buffer(descriptionTextView, descriptionTextBuffer); 
    
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    gtk_image_clear(image);
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
    gtk_widget_hide(GTK_WIDGET(documentTypeComboBox));
    gtk_widget_hide(GTK_WIDGET(documentNameEntry));
    gtk_image_clear(documentImage);
    gtk_widget_hide(GTK_WIDGET(removeDocumentButton));
    
    GtkTextBuffer *notesTextBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(notesTextBuffer, game->getNotes().c_str(), game->getNotes().length());
    gtk_text_view_set_buffer(notesTextView, notesTextBuffer);   
    
    
    
    g_signal_connect (searchButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->search();
    }), this);
        
    g_signal_connect (fileNameButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->selectFileName();
    }), this);
        
    g_signal_connect (addDeveloperButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->showDevelopersDialog();
    }), this);
            
    g_signal_connect (addPublisherButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->showPublishersDialog();
    }), this);
        
    g_signal_connect (addGenreButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->showGenresDialog();
    }), this);        
        
    g_signal_connect (imageTypeComboBox, "changed", G_CALLBACK (+[](GtkComboBox *comboBox, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->updateGameImageType();
    }), this);
            
    g_signal_connect (removeImageButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->removeGameImage();
    }), this);
    
    g_signal_connect (documentTypeComboBox, "changed", G_CALLBACK (+[](GtkComboBox *comboBox, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->updateGameDocumentType();
    }), this);    
    
    g_signal_connect (documentNameEntry, "key-release-event", G_CALLBACK (+[](GtkEntry* entry, GdkEvent *event, gpointer gameEditDialog) -> gboolean {
        if(((GameEditDialog *)gameEditDialog)->selectedGameDocument)
        {
            ((GameEditDialog *)gameEditDialog)->selectedGameDocument->setName(string(gtk_entry_get_text(entry)));
        }
        return FALSE;
    }), this);
    
    g_signal_connect (removeDocumentButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->removeGameDocument();
    }), this);
    
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->cancel();
    }), this);
        
    g_signal_connect (saveButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer gameEditDialog) -> void {
        ((GameEditDialog *)gameEditDialog)->save();
    }), this);
            
    
    
    loadDevelopers();    
    loadPublishers();    
    loadGenres();    
    loadEsrbRatings();
    
    loadGameImageTypes();
    updateGameImageGrid();
    
    loadGameDocumentTypes();
    updateGameDocumentGrid();
}

GameEditDialog::~GameEditDialog()
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

void GameEditDialog::deleteWhenReady(GameEditDialog *gameEditDialog)
{
    // Checks if the GameEditDialog is waiting for GameImages to be downloaded
    int isDownloading = 0;
    pthread_mutex_lock(&downloadGameImageRefsMutex);    
    for(list<DownloadGameImageRef_t *>::iterator downloadGameImageRef = downloadGameImageRefs->begin(); downloadGameImageRef != downloadGameImageRefs->end(); downloadGameImageRef++)
    {
        if((*downloadGameImageRef)->gameEditDialog == gameEditDialog)
        {
            isDownloading = 1;
            break;
        }
    }
    pthread_mutex_unlock(&downloadGameImageRefsMutex);
    
    gameEditDialog->dismiss();
    if(!isDownloading)
    {
        delete gameEditDialog;
    }
}

void GameEditDialog::loadDevelopers()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(developersListBox), 1);
    
    for(unsigned int c = 0; c < developers->size(); c++)
    {
        Developer *developer = Developer::getItem(developers, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
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


void GameEditDialog::removeDeveloper(int64_t developerId)
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

void GameEditDialog::showDevelopersDialog()
{
    DevelopersSelectDialog *developersSelectDialog = new DevelopersSelectDialog(GTK_WINDOW(dialog), developers);
    if(developersSelectDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Developer::releaseItems(developers);
        developers = new list<Developer *>;
        
        for(unsigned int c = 0; c < developersSelectDialog->getSelectedItems()->size(); c++)
        {
            Developer *developer = new Developer(*Developer::getItem(developersSelectDialog->getSelectedItems(), c));
            developers->push_back(developer);
        }
        
        loadDevelopers();
    }
    delete developersSelectDialog;
}

void GameEditDialog::loadPublishers()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(publishersListBox), 1);
    
    for(unsigned int c = 0; c < publishers->size(); c++)
    {
        Publisher *publisher = Publisher::getItem(publishers, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
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

void GameEditDialog::removePublisher(int64_t publisherId)
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

void GameEditDialog::showPublishersDialog()
{
    PublishersSelectDialog *publishersSelectDialog = new PublishersSelectDialog(GTK_WINDOW(dialog), publishers);
    if(publishersSelectDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Publisher::releaseItems(publishers);
        publishers = new list<Publisher *>;
        
        for(unsigned int c = 0; c < publishersSelectDialog->getSelectedItems()->size(); c++)
        {
            Publisher *publisher = new Publisher(*Publisher::getItem(publishersSelectDialog->getSelectedItems(), c));
            publishers->push_back(publisher);
        }
        
        loadPublishers();
    }
    delete publishersSelectDialog;
}

void GameEditDialog::loadGenres()
{
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(genresListBox), 1);
    
    for(unsigned int c = 0; c < genres->size(); c++)
    {
        Genre *genre = Genre::getItem(genres, c);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "OptionRowBox.ui").c_str());
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

void GameEditDialog::removeGenre(int64_t genreId)
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

void GameEditDialog::showGenresDialog()
{
    GenresSelectDialog *genresDialog = new GenresSelectDialog(GTK_WINDOW(dialog), genres);
    if(genresDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        Genre::releaseItems(genres);
        genres = new list<Genre *>;
        
        for(unsigned int c = 0; c < genresDialog->getSelectedItems()->size(); c++)
        {
            Genre *genre = new Genre(*Genre::getItem(genresDialog->getSelectedItems(), c));
            genres->push_back(genre);
        }
        
        loadGenres();
    }
    delete genresDialog;
}

void GameEditDialog::loadEsrbRatings()
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

void GameEditDialog::selectFileName()
{
    int fileSelected = 0;    
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select filename", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        string fileName = string(cFileName);
        g_free(cFileName);
        
        gtk_entry_set_text(fileNameEntry, fileName.c_str());

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(fileName));
        Preferences::getInstance()->save();
       
        string gameName = sanitizeFileName(fileName);
        
        gtk_entry_set_text(nameEntry, gameName.c_str());
        fileSelected = 1;
    }
    gtk_widget_destroy(fileChooserDialog);
    
    if(fileSelected)
    {        
        int yes = 0;
        MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(dialog), "Search \"" + string(gtk_entry_get_text(nameEntry)) + "\" in the database?", "Yes", "No");
        yes = messageDialog->execute() == GTK_RESPONSE_YES;
        delete messageDialog;
        
        if(yes)
        {
            search();
        }
    }
}

void GameEditDialog::loadGameImageTypes()
{
    gameImageTypes->push_back(GameImage::TYPE_BOX_FRONT);
    gameImageTypes->push_back(GameImage::TYPE_BOX_BACK);
    gameImageTypes->push_back(GameImage::TYPE_SCREENSHOT);
    gameImageTypes->push_back(GameImage::TYPE_CLEAR_LOGO);
    gameImageTypes->push_back(GameImage::TYPE_BANNER);
    gameImageTypes->push_back(GameImage::TYPE_FANART);
    
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
        else if(*type == GameImage::TYPE_FANART)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Fanart", -1);
        }        
    }
    gtk_combo_box_set_model(imageTypeComboBox, GTK_TREE_MODEL(listStore));
    
    GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, "text", 0, NULL);
}

void GameEditDialog::updateGameImageGrid()
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
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                if(!gameImage->getId())
                {
                    if(Utils::getInstance()->fileExists(gameImage->getFileName()))
                    {
                        UiUtils::getInstance()->loadImage(image, gameImage->getFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                    }
                    else
                    {
                        gtk_image_set_from_icon_name(image, "emblem-downloads", GTK_ICON_SIZE_DIALOG);
                    }
                }
                else
                {
                    UiUtils::getInstance()->loadImage(image, gameImage->getThumbnailFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
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
    
    if(selectedGameImage)
    {
        selectGameImage(selectedGameImage);
    }
    else if(gameImages->size() > 0)
    {
        selectGameImage(GameImage::getItem(gameImages, 0));
    }
}

void GameEditDialog::addGameImage()
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

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(imageFileName));
        Preferences::getInstance()->save();
        
        g_free (cFileName);
                
        GameImage *gameImage = new GameImage((int64_t)0);
        gameImage->setApiId(0);
        gameImage->setType(GameImage::TYPE_BOX_FRONT);
        gameImage->setFileName(imageFileName);
        gameImage->setExternal(0);
                
        gameImages->push_back(gameImage);
        
        updateGameImageGrid();
        selectGameImage(gameImage);
    }
    gtk_widget_destroy (fileChooserDialog);
}

void GameEditDialog::removeGameImage()
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

void GameEditDialog::removeNewGameImages()
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
                
        pthread_mutex_lock(&downloadGameImageRefsMutex);
        int isDownloading = 0;
        for(list<DownloadGameImageRef_t *>::iterator downloadGameImageRef = downloadGameImageRefs->begin(); downloadGameImageRef != downloadGameImageRefs->end(); downloadGameImageRef++)
        {
            if((*downloadGameImageRef)->gameEditDialog == this && (*downloadGameImageRef)->gameImage == (*gameImage))
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {            
            delete (*gameImage);
        }
        pthread_mutex_unlock(&downloadGameImageRefsMutex);                
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

void GameEditDialog::updateGameImageType()
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

void GameEditDialog::selectGameImage(GameImage *gameImage)
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

void GameEditDialog::downloadGameImage(GameImage* gameImage)
{
    downloadGameImage(this, gameImage);
}

void GameEditDialog::saveNewGameImage(GameImage* gameImage)
{             
    // When the image is new, copies the file to the media directory and creates the thumbnail.
    if(Utils::getInstance()->fileExists(gameImage->getFileName()))
    {        
        gameImage->setGameId(game->getId());
        
        // Saves before to generate the id
        gameImage->save();

        string imageFileName = game->getMediaDirectory() + GameImage::FILE_PREFIX + to_string(gameImage->getId());
        if(!Utils::getInstance()->copyFile(gameImage->getFileName(), imageFileName))
        {
            gameImage->setFileName(imageFileName);

            Utils::getInstance()->scaleImage(gameImage->getFileName(), GameImage::THUMBNAIL_WIDTH, GameImage::THUMBNAIL_HEIGHT, gameImage->getThumbnailFileName());            
            gameImage->save();
        }
    }    
}

void GameEditDialog::loadGameDocumentTypes()
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

void GameEditDialog::updateGameDocumentGrid()
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
    
    if(selectedGameDocument)
    {
        selectGameDocument(selectedGameDocument);
    }
    else if(gameDocuments->size() > 0)
    {
        selectGameDocument(GameDocument::getItem(gameDocuments, 0));
    }
}

void GameEditDialog::addGameDocument()
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

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(fileName));
        Preferences::getInstance()->save();
        
        g_free (cFileName);
                
        GameDocument *gameDocument = new GameDocument((int64_t)0);
        gameDocument->setApiId(0);
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

void GameEditDialog::removeGameDocument()
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

void GameEditDialog::updateGameDocumentType()
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

void GameEditDialog::selectGameDocument(GameDocument *gameDocument)
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

string GameEditDialog::sanitizeFileName(string fileName)
{
    fileName = Utils::getInstance()->getFileBasename(fileName);    
    size_t periodPosition = fileName.find_last_of(".");        
    if(periodPosition != string::npos)
    {
        fileName.erase(periodPosition, fileName.length() - periodPosition);
    }
    
    string sanitizedName = "";
    int open = 0;
    for(unsigned int c = 0; c < fileName.length(); c++)
    {
        char letter = fileName.c_str()[c];
        
        if(letter == '(' || letter == '[' || letter == '{')
        {
            open++;
        }
        else if(open > 0 && (letter == ')' || letter == ']' || letter == '}'))
        {
            open--;
        }                
        else if(!open)
        {
            sanitizedName += letter;
        }
    }
    
    return Utils::getInstance()->trim(sanitizedName);
}


void GameEditDialog::search()
{
    string query = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(query.length() == 0)
    {
        return;
    }
    
    if(TheGamesDB::Elasticsearch::getInstance()->getStatus() != TheGamesDB::Elasticsearch::STATUS_OK)
    {
        MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(dialog), "Database is not ready yet. Please wait a moment.", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    Platform *platform = new Platform(game->getPlatformId());
    platform->load();
    
    GameSearchDialog *gameSearchDialog = new GameSearchDialog(GTK_WINDOW(dialog), platform->getApiId(), query);
    if(gameSearchDialog->execute() == GTK_RESPONSE_ACCEPT)
    {
        TheGamesDB::Game *apiGame = gameSearchDialog->getSelectedApiGame();
        if(apiGame)
        {
            game->setApiId(apiGame->getId());
            
            gtk_entry_set_text(nameEntry, apiGame->getName().c_str());
            gtk_entry_set_text(releaseDateEntry, apiGame->getReleaseDate().c_str());
            
            // Assigns the ESRB rating.
            for(unsigned int c = 0; c < esrbRatings->size(); c++)
            {
                EsrbRating *esrbRating = EsrbRating::getItem(esrbRatings, c);

                if(esrbRating->getApiId() == apiGame->getEsrbRatingId())
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
                Developer *developer = Developer::getDeveloper(apiGameDeveloper->getDeveloperId());
                if(developer)
                {
                    developers->push_back(developer);
                }
            }
            loadDevelopers();
            
            // Assigns publishers.
            Publisher::releaseItems(publishers);
            publishers = new list<Publisher *>;
            for(unsigned int c = 0; c < apiGame->getGamePublishers()->size(); c++)
            {                
                TheGamesDB::GamePublisher *apiGamePublisher = TheGamesDB::GamePublisher::getItem(apiGame->getGamePublishers(), c);
                Publisher *publisher = Publisher::getPublisher(apiGamePublisher->getPublisherId());
                if(publisher)
                {
                    publishers->push_back(publisher);
                }
            }
            loadPublishers();
            
            // Assigns genres.
            Genre::releaseItems(genres);
            genres = new list<Genre *>;
            for(unsigned int c = 0; c < apiGame->getGameGenres()->size(); c++)
            {                
                TheGamesDB::GameGenre *apiGameGenre = TheGamesDB::GameGenre::getItem(apiGame->getGameGenres(), c);
                Genre *genre = Genre::getGenre(apiGameGenre->getGenreId());
                if(genre)
                {
                    genres->push_back(genre);
                }
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
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_BANNER) == 0)
                {
                    type = GameImage::TYPE_BANNER;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_FANART) == 0)
                {
                    type = GameImage::TYPE_FANART;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_SCREENSHOT) == 0)
                {
                    type = GameImage::TYPE_SCREENSHOT;
                }
                else if(apiGameImage->getType().compare(TheGamesDB::GameImage::TYPE_CLEAR_LOGO) == 0)
                {
                    type = GameImage::TYPE_CLEAR_LOGO;
                }
                

                // Checks if there is GameImage for this TheGamesDB::GameImage being downloaded, if it is, then appends it to the list
                int isDownloading = 0;
                pthread_mutex_lock(&downloadGameImageRefsMutex);        
                for(list<DownloadGameImageRef_t *>::iterator downloadGameImageRef = downloadGameImageRefs->begin(); downloadGameImageRef != downloadGameImageRefs->end(); downloadGameImageRef++)
                {                    
                    if((*downloadGameImageRef)->gameEditDialog == this && (*downloadGameImageRef)->gameImage->getApiId() == apiGameImage->getId())
                    {
                        gameImages->push_back((*downloadGameImageRef)->gameImage);
                        isDownloading = 1;
                        break;
                    }
                }
                pthread_mutex_unlock(&downloadGameImageRefsMutex);

                if(!isDownloading)
                {
                    GameImage *gameImage = new GameImage((int64_t)0);
                    gameImage->setApiId(apiGameImage->getId());
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
    delete gameSearchDialog;
    delete platform;
}

void GameEditDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void GameEditDialog::save()
{
    int isNew = game->getId() == 0;
    
    string name = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(name.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(dialog), "Name cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    string fileName = Utils::getInstance()->trim(string(gtk_entry_get_text(fileNameEntry)));
    if(fileName.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(dialog), "Filename cannot be empty", "Ok", "");   
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
    game->save();
    
    
    GameDeveloper::remove(game->getId());
    for(unsigned int c = 0; c < developers->size(); c++)
    {
        Developer *developer = Developer::getItem(developers, c);
        
        GameDeveloper *gameDeveloper = new GameDeveloper(game->getId(), developer->getId());
        gameDeveloper->save();
        delete gameDeveloper;
    }
    
    GamePublisher::remove(game->getId());
    for(unsigned int c = 0; c < publishers->size(); c++)
    {
        Publisher *publisher = Publisher::getItem(publishers, c);
        
        GamePublisher *gamePublisher = new GamePublisher(game->getId(), publisher->getId());
        gamePublisher->save();
        delete gamePublisher;
    }
    
    GameGenre::remove(game->getId());
    for(unsigned int c = 0; c < genres->size(); c++)
    {
        Genre *genre = Genre::getItem(genres, c);
        
        GameGenre *gameGenre = new GameGenre(game->getId(), genre->getId());
        gameGenre->save();
        delete gameGenre;
    }
    

    for(unsigned int c = 0; c < gameDocumentsToRemove->size(); c++)
    {
        GameDocument *gameDocument = GameDocument::getItem(gameDocumentsToRemove, c);
        gameDocument->remove();
    }
    for(unsigned int c = 0; c < gameDocuments->size(); c++)
    {
        GameDocument *gameDocument = GameDocument::getItem(gameDocuments, c);
        
        // When the document is new, copies the file to the media directory
        if(!gameDocument->getId())
        {
            gameDocument->setGameId(game->getId());
            gameDocument->save();

            string imageFileName = game->getMediaDirectory() + GameDocument::FILE_PREFIX + to_string(gameDocument->getId());
            if(!Utils::getInstance()->copyFile(gameDocument->getFileName(), imageFileName))
            {
                gameDocument->setFileName(imageFileName);
                
                string tempPreviewImageFileName = gameDocument->getPreviewImageFileName();
                gameDocument->setPreviewImageFileName("");
                Utils::getInstance()->copyFile(tempPreviewImageFileName, gameDocument->getPreviewImageFileName());
                
                gameDocument->save();
            }
        }
        else
        {
            gameDocument->save();
        }
    }


    for(unsigned int c = 0; c < gameImagesToRemove->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gameImagesToRemove, c);
        gameImage->remove();
    }    
    
        
    for(unsigned int c = 0; c < gameImages->size(); c++)
    {
        GameImage *gameImage = GameImage::getItem(gameImages, c);
        if(!gameImage->getId())
        {
            // Checks if the image is still downloading, in this case, the image should be saved after it is downloaded
            int isDownloading = 0;
            pthread_mutex_lock(&downloadGameImageRefsMutex);            
            for(list<DownloadGameImageRef_t *>::iterator downloadGameImageRef = downloadGameImageRefs->begin(); downloadGameImageRef != downloadGameImageRefs->end(); downloadGameImageRef++)
            {
                if((*downloadGameImageRef)->gameEditDialog == this && (*downloadGameImageRef)->gameImage == gameImage)
                {
                    isDownloading = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&downloadGameImageRefsMutex);

            if(!isDownloading)
            {
                saveNewGameImage(gameImage);
            }
        }
        else
        {
            gameImage->save();
        }
    }
            
    saved = 1;
    
    NotificationManager::getInstance()->notify(NOTIFICATION_GAME_UPDATED, "", 0, 0, new Game(*game));
    
    if(isNew)
    {
        Platform *platfom = new Platform(game->getPlatformId());
        platfom->load();
        NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0, 0, platfom); 
    }
        
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);        
}




void GameEditDialog::signalRemoveDeveloperButtonClicked(GtkButton *button, gpointer gameEditDialog)
{
    ((GameEditDialog *)gameEditDialog)->removeDeveloper(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameEditDialog::signalRemovePublisherButtonClicked(GtkButton *button, gpointer gameEditDialog)
{
    ((GameEditDialog *)gameEditDialog)->removePublisher(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameEditDialog::signalRemoveGenreButtonClicked(GtkButton *button, gpointer gameEditDialog)
{
    ((GameEditDialog *)gameEditDialog)->removeGenre(atol(gtk_widget_get_name(GTK_WIDGET(button))));
}

void GameEditDialog::signalAddImageButtonClicked(GtkButton *button, gpointer gameEditDialog)
{
    ((GameEditDialog *)gameEditDialog)->addGameImage();
}

gboolean GameEditDialog::signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameEditDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameImage *gameImage = GameImage::getItem(((GameEditDialog *)gameEditDialog)->gameImages, atoi(gtk_widget_get_name(widget)));
        ((GameEditDialog *)gameEditDialog)->selectGameImage(gameImage);
    }
    
    return TRUE;
}

void GameEditDialog::signalAddDocumentButtonClicked(GtkButton *button, gpointer gameEditDialog)
{
    ((GameEditDialog *)gameEditDialog)->addGameDocument();
}

gboolean GameEditDialog::signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameEditDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameEditDialog *)gameEditDialog)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        ((GameEditDialog *)gameEditDialog)->selectGameDocument(gameDocument);
    }
    
    return TRUE;
}





list<GameEditDialog::DownloadGameImageRef_t *> *GameEditDialog::downloadGameImageRefs = new list<GameEditDialog::DownloadGameImageRef_t *>;
pthread_t GameEditDialog::downloadGameImagesThread;
pthread_mutex_t GameEditDialog::downloadGameImageRefsMutex = PTHREAD_MUTEX_INITIALIZER;
int GameEditDialog::downloadingGameImages = 0;

void GameEditDialog::downloadGameImage(GameEditDialog *gameEditDialog, GameImage *gameImage)
{
    pthread_mutex_lock(&downloadGameImageRefsMutex);
    
    DownloadGameImageRef_t *downloadGameImageRef = new DownloadGameImageRef_t;
    downloadGameImageRef->gameEditDialog = gameEditDialog;
    downloadGameImageRef->gameImage = gameImage;
    downloadGameImageRefs->push_back(downloadGameImageRef);
        
    if(!downloadingGameImages)
    {
        downloadingGameImages = 1;
        ThreadManager::getInstance()->execute(0, []() -> void {
            DownloadGameImageRef_t *downloadGameImageRef = NULL;
            do
            {
                downloadGameImageRef = NULL;

                pthread_mutex_lock(&downloadGameImageRefsMutex);
                for(list<DownloadGameImageRef_t *>::iterator aDownloadGameImageRef = downloadGameImageRefs->begin(); aDownloadGameImageRef != downloadGameImageRefs->end(); aDownloadGameImageRef++)
                {
                    if(!(*aDownloadGameImageRef)->gameImage->getDownloaded())
                    {
                        downloadGameImageRef = (*aDownloadGameImageRef);
                        break;
                    }
                }
                pthread_mutex_unlock(&downloadGameImageRefsMutex);

                if(downloadGameImageRef)
                {            
                    HttpConnector *httpConnector = new HttpConnector(downloadGameImageRef->gameImage->getUrl());
                    httpConnector->get();

                    if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
                    {
                        Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), downloadGameImageRef->gameImage->getFileName());
                        downloadGameImageRef->gameImage->setDownloaded(1);
                    }
                    delete httpConnector;
                    
                    ThreadManager::getInstance()->execute(1, [downloadGameImageRef]() -> void {
                        pthread_mutex_lock(&downloadGameImageRefsMutex);
                        downloadGameImageRefs->remove(downloadGameImageRef);
                        pthread_mutex_unlock(&downloadGameImageRefsMutex);

                        // Checks if the GameImage is still present in the list of the GameEditDialog
                        int isImage = 0;
                        for(list<GameImage *>::iterator gameImage = downloadGameImageRef->gameEditDialog->gameImages->begin(); gameImage != downloadGameImageRef->gameEditDialog->gameImages->end(); gameImage++)
                        {
                            if(downloadGameImageRef->gameImage == (*gameImage))
                            {
                                isImage = 1;
                                break;
                            }
                        }

                        if(isImage)
                        {
                            // Checks if the GameEditDialog is still been shown
                            if(!downloadGameImageRef->gameEditDialog->dismissed)
                            {
                                // Checks if the GameImage has been downloaded
                                if(downloadGameImageRef->gameImage->getDownloaded())
                                {
                                    downloadGameImageRef->gameEditDialog->updateGameImageGrid();
                                }            
                            }
                            // Checks if the GameEditDialog was dismissed with the save instruction
                            else
                            {
                                if(downloadGameImageRef->gameEditDialog->saved)
                                {
                                   downloadGameImageRef->gameEditDialog->saveNewGameImage(downloadGameImageRef->gameImage); 
                                }
                            }
                        }
                        else
                        {
                            delete downloadGameImageRef->gameImage;
                        }

                        // Checks if the GameEditDialog was dismissed and if all its images has finished downloading
                        if(downloadGameImageRef->gameEditDialog->dismissed)
                        {
                            pthread_mutex_lock(&downloadGameImageRefsMutex);
                            int isDialog = 0;
                            for(list<DownloadGameImageRef_t *>::iterator aDownloadGameImageRef = downloadGameImageRefs->begin(); aDownloadGameImageRef != downloadGameImageRefs->end(); aDownloadGameImageRef++)
                            {
                                if((*aDownloadGameImageRef)->gameEditDialog == downloadGameImageRef->gameEditDialog)
                                {
                                    isDialog = 1;
                                    break;
                                }
                            }
                            if(!isDialog)
                            {
                                if(downloadGameImageRef->gameEditDialog->saved)
                                {
                                    NotificationManager::getInstance()->notify(NOTIFICATION_GAME_UPDATED, "", 0, 0, new Game(*(downloadGameImageRef->gameEditDialog->game)));    
                                }
                                delete downloadGameImageRef->gameEditDialog;                
                            }
                            pthread_mutex_unlock(&downloadGameImageRefsMutex);        
                        }
                    });
                }

            }while(downloadGameImageRef);

            downloadingGameImages = 0;
        });        
    }
    pthread_mutex_unlock(&downloadGameImageRefsMutex);
}
