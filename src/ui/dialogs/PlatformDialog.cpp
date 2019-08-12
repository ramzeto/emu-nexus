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
 * File:   PlatformDialog.cpp
 * Author: ram
 * 
 * Created on March 10, 2019, 7:14 PM
 */

#include "PlatformDialog.h"
#include "UiUtils.h"
#include "Preferences.h"
#include "Utils.h"
#include "MessageDialog.h"
#include "Database.h"
#include "PlatformImage.h"
#include "UiThreadHandler.h"
#include "HttpConnector.h"
#include "Asset.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Directory.h"

#include <iostream>

const int PlatformDialog::THUMBNAIL_IMAGE_WIDTH = 90;
const int PlatformDialog::THUMBNAIL_IMAGE_HEIGHT = 90;
const int PlatformDialog::IMAGE_WIDTH = 300;
const int PlatformDialog::IMAGE_HEIGHT = 300;

PlatformDialog::PlatformDialog(int64_t platformId) : Dialog("PlatformDialog.ui", "platformDialog")
{
    saved = 0;
    apiPlatform = NULL;
    apiPlatforms = NULL;
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    platform = new Platform(platformId);
    platform->load(sqlite);    
    
    platformImages = PlatformImage::getItems(sqlite, platform->getId());
    Database::getInstance()->release();
    
    platformImageTypes = new list<int64_t>;    
    selectedPlatformImage = NULL;
    
    platformImagesToRemove = new list<PlatformImage *>;
    platformImageBoxes = new map<PlatformImage *, GtkWidget *>;    
    
    apiPlatformComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "apiPlatformComboBox");
    g_signal_connect (apiPlatformComboBox, "changed", G_CALLBACK (signalPlatformComboBoxChanged), this);
    
    nameEntry = (GtkEntry *)gtk_builder_get_object (builder, "nameEntry");
    gtk_entry_set_text(nameEntry, platform->getName().c_str());
    
    descriptionTextView = (GtkTextView *)gtk_builder_get_object (builder, "descriptionTextView");
    GtkTextBuffer *descriptionTextBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(descriptionTextBuffer, platform->getDescription().c_str(), platform->getDescription().length());
    gtk_text_view_set_buffer(descriptionTextView, descriptionTextBuffer);        
    
    commandEntry = (GtkEntry *)gtk_builder_get_object (builder, "commandEntry");
    gtk_entry_set_text(commandEntry, platform->getCommand().c_str());   
    
    deflateCheckButton = (GtkCheckButton *)gtk_builder_get_object (builder, "deflateCheckButton");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(deflateCheckButton), platform->getDeflate());
    
    deflateFileExtensionsEntry = (GtkEntry *)gtk_builder_get_object (builder, "deflateFileExtensionsEntry");
    gtk_entry_set_text(deflateFileExtensionsEntry, platform->getDeflateFileExtensions().c_str());
        
    imageTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "imageTypeComboBox");
    g_signal_connect (imageTypeComboBox, "changed", G_CALLBACK (signalImageTypeComboBoxChanged), this);
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    
    image = (GtkImage *)gtk_builder_get_object (builder, "image");
    gtk_image_clear(image);

    removeImageButton = (GtkButton *)gtk_builder_get_object (builder, "removeImageButton");
    g_signal_connect (removeImageButton, "clicked", G_CALLBACK (signalRemoveImageButtonClicked), this);
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
    
    imagesGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "imagesGridListBox");
    
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (signalCancelButtonClicked), this);

    saveButton = (GtkButton *)gtk_builder_get_object(builder, "saveButton");
    g_signal_connect (saveButton, "clicked", G_CALLBACK (signalSaveButtonClicked), this);
            
    loadApiPlatforms();
    loadPlatformImageTypes();
    updateImageGrid();
}

PlatformDialog::~PlatformDialog()
{
    PlatformImage::releaseItems(platformImages);    
    PlatformImage::releaseItems(platformImagesToRemove);
    
    if(apiPlatforms)
    {
        TheGamesDB::Platform::releaseItems(apiPlatforms);
    }
    
    platformImageBoxes->clear();
    delete platformImageBoxes;
    
    platformImageTypes->clear();
    delete platformImageTypes;
    
    delete platform;
}

void PlatformDialog::deleteWhenReady(PlatformDialog *platformDialog)
{
    // Checks if the PlatformDialog is waiting for PlatformImages to be downloaded
    int isDownloading = 0;
    pthread_mutex_lock(&downloadPlatformImagesMutex);    
    for(list<UiThreadHandler *>::iterator downloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); downloadPlatformImageHandler != downloadPlatformImageHandlers->end(); downloadPlatformImageHandler++)
    {
        DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*downloadPlatformImageHandler)->getRequesterInUiThread();
        if(downloadPlatformImage->platformDialog == platformDialog)
        {
            isDownloading = 1;
            break;
        }
    }
    pthread_mutex_unlock(&downloadPlatformImagesMutex);
    
    platformDialog->dismiss();
    if(!isDownloading)
    {
        delete platformDialog;
    }
}

void PlatformDialog::loadApiPlatforms()
{
    if(platform->getId() > 0)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(apiPlatformComboBox), 0);
    }
    
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(this, callbackElasticsearchPlatforms);
    TheGamesDB::Elasticsearch::getInstance()->getPlatforms(uiThreadHandler, UiThreadHandler::callback);    
}

void PlatformDialog::updateApiPlatform()
{
    if(platform->getId() > 0)
    {
        return;
    }
    
    int position = gtk_combo_box_get_active(apiPlatformComboBox);
    apiPlatform = TheGamesDB::Platform::getItem(apiPlatforms, position);
    
    gtk_entry_set_text(nameEntry, apiPlatform->getName().c_str());
    
    removeAllImages();
    for(unsigned int c = 0; c < apiPlatform->getPlatformImages()->size(); c++)
    {
        TheGamesDB::PlatformImage *apiPlatformImage = TheGamesDB::PlatformImage::getItem(apiPlatform->getPlatformImages(), c);
        
        int64_t type = PlatformImage::TYPE_BOXART;
        if(apiPlatformImage->getType().compare(TheGamesDB::PlatformImage::TYPE_BANNER) == 0)
        {
            type = PlatformImage::TYPE_BANNER;
        }
        else if(apiPlatformImage->getType().compare(TheGamesDB::PlatformImage::TYPE_BOXART) == 0)
        {
            type = PlatformImage::TYPE_BOXART;
        }
        else if(apiPlatformImage->getType().compare(TheGamesDB::PlatformImage::TYPE_FANART) == 0)
        {
            type = PlatformImage::TYPE_FANART;
        }
        else if(apiPlatformImage->getType().compare(TheGamesDB::PlatformImage::TYPE_ICON) == 0)
        {
            type = PlatformImage::TYPE_ICON;
        }
        
        // Checks if there is PlatformImage for this TheGamesDB::PlatformImage being downloaded, if it is, then appends it to the list
        int isDownloading = 0;
        pthread_mutex_lock(&downloadPlatformImagesMutex);        
        for(list<UiThreadHandler *>::iterator downloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); downloadPlatformImageHandler != downloadPlatformImageHandlers->end(); downloadPlatformImageHandler++)
        {
            DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*downloadPlatformImageHandler)->getRequesterInUiThread();
            if(downloadPlatformImage->platformDialog == this && downloadPlatformImage->platformImage->getApiId() == TheGamesDB::API_ID && downloadPlatformImage->platformImage->getApiItemId() == apiPlatformImage->getId())
            {
                platformImages->push_back(downloadPlatformImage->platformImage);
                isDownloading = 1;
                break;
            }
        }
        pthread_mutex_unlock(&downloadPlatformImagesMutex);

        if(!isDownloading)
        {
            PlatformImage *platformImage = new PlatformImage((int64_t)0);
            platformImage->setApiId(TheGamesDB::API_ID);
            platformImage->setApiItemId(apiPlatformImage->getId());
            platformImage->setType(type);
            platformImage->setFileName(apiPlatformImage->getFileName());
            platformImage->setExternal(0);
            platformImage->setDownloaded(0);
            platformImage->setUrl(apiPlatformImage->getOriginal());

            platformImages->push_back(platformImage);

            if(!Utils::getInstance()->fileExists(apiPlatformImage->getFileName()))
            {
                downloadPlatformImage(platformImage);
            }            
        }
    }
    
    updateImageGrid();    
    if(platformImages->size() > 0)
    {
        selectImage(PlatformImage::getItem(platformImages, 0));
    }
}


void PlatformDialog::loadPlatformImageTypes()
{
    platformImageTypes->push_back(PlatformImage::TYPE_BANNER);
    platformImageTypes->push_back(PlatformImage::TYPE_BOXART);
    platformImageTypes->push_back(PlatformImage::TYPE_FANART);
    platformImageTypes->push_back(PlatformImage::TYPE_ICON);
    
    int position = 0;
    GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);

    for(list<int64_t>::iterator type = platformImageTypes->begin(); type != platformImageTypes->end(); type++)
    {
        if(*type == PlatformImage::TYPE_BANNER)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Banner", -1);
        }
        else if(*type == PlatformImage::TYPE_BOXART)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Boxart", -1);
        }
        else if(*type == PlatformImage::TYPE_FANART)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Fanart", -1);
        }
        else if(*type == PlatformImage::TYPE_ICON)
        {
            gtk_list_store_insert_with_values (listStore, NULL, position++, 0, "Icon", -1);
        }
    }
    gtk_combo_box_set_model(imageTypeComboBox, GTK_TREE_MODEL(listStore));
    
    GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(imageTypeComboBox), cellRenderer, "text", 0, NULL);
}

void PlatformDialog::updateImageGrid()
{
    platformImageBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(imagesGridListBox), 1);
 
    int columns = 3;
    int rows = platformImages->size() / columns;
    if(platformImages->size() % columns)
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
            if(index < platformImages->size())
            {
                PlatformImage *platformImage = PlatformImage::getItem(platformImages, index);
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                string fileName;
                if(!platformImage->getId())
                {
                    if(Utils::getInstance()->fileExists(platformImage->getFileName()))
                    {
                        fileName = platformImage->getFileName();
                    }
                    else
                    {
                        fileName = Asset::getInstance()->getImageDownloading();
                    }
                }
                else
                {
                    fileName = platformImage->getThumbnailFileName();
                }
                UiUtils::getInstance()->loadImage(image, fileName, THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                
                gtk_widget_set_name(GTK_WIDGET(imageBox), to_string(index).c_str());                                
                g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalImageBoxButtonPressedEvent), this);                                
                
                                
                if(selectedPlatformImage == platformImage)
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                platformImageBoxes->insert(pair<PlatformImage*, GtkWidget *>(platformImage, GTK_WIDGET(imageBox)));
                
                gtk_widget_set_size_request(GTK_WIDGET(imageBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(imageBox), 1, 1, 0);
            }
            else if(index == platformImages->size())
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
    
    if(selectedPlatformImage)
    {
        selectImage(selectedPlatformImage);
    }
}

void PlatformDialog::addImage()
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
        
        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();        
        
        g_free (cFileName);
        
        
        PlatformImage *platformImage = new PlatformImage((int64_t)0);
        platformImage->setApiId(0);
        platformImage->setApiItemId(0);
        platformImage->setType(PlatformImage::TYPE_BOXART);
        platformImage->setFileName(imageFileName);
        platformImage->setExternal(0);
                
        platformImages->push_back(platformImage);
        
        updateImageGrid();
        selectImage(platformImage);
    }
    gtk_widget_destroy (fileChooserDialog);
}

void PlatformDialog::removeAllImages()
{
    selectedPlatformImage = NULL;
    
    // Checks if there are PlatformImages being downloaded, PlarformImages that are being downloaded cannot be deleted
    for(list<PlatformImage *>::iterator platformImage = platformImages->begin(); platformImage != platformImages->end(); platformImage++)
    {        
        pthread_mutex_lock(&downloadPlatformImagesMutex);
        int isDownloading = 0;
        for(list<UiThreadHandler *>::iterator downloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); downloadPlatformImageHandler != downloadPlatformImageHandlers->end(); downloadPlatformImageHandler++)
        {
            DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*downloadPlatformImageHandler)->getRequesterInUiThread();
            if(downloadPlatformImage->platformDialog == this && downloadPlatformImage->platformImage == (*platformImage))
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {
            delete (*platformImage);
        }
        pthread_mutex_unlock(&downloadPlatformImagesMutex);    
    }
    platformImages->clear();
        
    updateImageGrid();
}

void PlatformDialog::removeImage()
{
    if(!selectedPlatformImage)
    {
        return;
    }
    
    platformImages->remove(selectedPlatformImage);
    
    if(selectedPlatformImage->getId())
    {
        platformImagesToRemove->push_back(selectedPlatformImage);
    }
    else
    {
        // Checks if the PlatformImage is being downloaded, PlarformImages that are being downloaded cannot be deleted
        pthread_mutex_lock(&downloadPlatformImagesMutex);
        int isDownloading = 0;
        for(list<UiThreadHandler *>::iterator downloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); downloadPlatformImageHandler != downloadPlatformImageHandlers->end(); downloadPlatformImageHandler++)
        {
            DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*downloadPlatformImageHandler)->getRequesterInUiThread();
            if(downloadPlatformImage->platformDialog == this && downloadPlatformImage->platformImage == selectedPlatformImage)
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {
            delete selectedPlatformImage;
        }
        pthread_mutex_unlock(&downloadPlatformImagesMutex);
    }    
    selectedPlatformImage = NULL;
    
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
    gtk_image_clear(image);
    updateImageGrid();
}

void PlatformDialog::updateImageType()
{
    if(!selectedPlatformImage)
    {
        return;
    }
    
    int position = gtk_combo_box_get_active(imageTypeComboBox);
    if(position >= 0 && position < (int)platformImageTypes->size())
    {
        list<int64_t>::iterator type = platformImageTypes->begin();
	advance(type, position);
        
        selectedPlatformImage->setType(*type);
    }
}

void PlatformDialog::selectImage(PlatformImage* platformImage)
{
    if(selectedPlatformImage)
    {
        if(platformImageBoxes->find(selectedPlatformImage) != platformImageBoxes->end())
        {
            GtkWidget *imageBox = platformImageBoxes->at(selectedPlatformImage);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_NORMAL, 1);
        }        
    }
    
    selectedPlatformImage = platformImage;
    if(platformImageBoxes->find(selectedPlatformImage) != platformImageBoxes->end())
    {
        GtkWidget *imageBox = platformImageBoxes->at(selectedPlatformImage);
        gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
    }
    
    UiUtils::getInstance()->loadImage(image, selectedPlatformImage->getFileName(), IMAGE_WIDTH, IMAGE_HEIGHT);    
    
    int position = 0;
    for(list<int64_t>::iterator type = platformImageTypes->begin(); type != platformImageTypes->end(); type++)
    {
        if(*type == selectedPlatformImage->getType())
        {
            gtk_combo_box_set_active(imageTypeComboBox, position);
        }
        position++;
    }
    
    gtk_widget_show(GTK_WIDGET(imageTypeComboBox));
    gtk_widget_show(GTK_WIDGET(removeImageButton));
}

void PlatformDialog::downloadPlatformImage(PlatformImage *platformImage)
{
    downloadPlatformImage(this, platformImage);
}

void PlatformDialog::saveNewImage(PlatformImage* platformImage)
{
    // When the image is new, copies the file to the media directory and creates the thumbnail.        
    if(Utils::getInstance()->fileExists(platformImage->getFileName()))
    {
        // Saves before to generate the id
        sqlite3 *sqlite = Database::getInstance()->acquire();
        platformImage->setPlatformId(platform->getId());
        platformImage->save(sqlite);
        Database::getInstance()->release();
        
        string imageFileName = platform->getMediaDirectory() + PlatformImage::FILE_PREFIX + to_string(platformImage->getId());
        if(!Utils::getInstance()->copyFile(platformImage->getFileName(), imageFileName))
        {
            platformImage->setFileName(imageFileName);

            Utils::getInstance()->scaleImage(platformImage->getFileName(), PlatformImage::THUMBNAIL_WIDTH, PlatformImage::THUMBNAIL_HEIGHT, platformImage->getThumbnailFileName());
            
            sqlite = Database::getInstance()->acquire();
            platformImage->save(sqlite);
            Database::getInstance()->release();
        }        
    }    
}

void PlatformDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void PlatformDialog::save()
{    
    string name = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(name.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("Name cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }    

    // Only assign the API platform if the platform is new
    if(!platform->getId())
    {
        if(apiPlatform)
        {
            platform->setApiId(TheGamesDB::API_ID);
            platform->setApiItemId(apiPlatform->getId());        
        }
        else
        {
            platform->setApiId(0);
            platform->setApiItemId(0);
        }        
    }
    
    platform->setName(name);
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(gtk_text_view_get_buffer(descriptionTextView), &start, &end);    
    platform->setDescription(string(gtk_text_buffer_get_text (gtk_text_view_get_buffer(descriptionTextView), &start, &end, FALSE)));

    platform->setCommand(string(gtk_entry_get_text(commandEntry)));
    platform->setDeflate(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(deflateCheckButton)));
    platform->setDeflateFileExtensions(string(gtk_entry_get_text(deflateFileExtensionsEntry)));
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    platform->save(sqlite);    
    
    for(unsigned int c = 0; c < platformImagesToRemove->size(); c++)
    {
        PlatformImage *platformImage = PlatformImage::getItem(platformImagesToRemove, c);
        platformImage->remove(sqlite);
    }
    Database::getInstance()->release();
        
    for(unsigned int c = 0; c < platformImages->size(); c++)
    {
        PlatformImage *platformImage = PlatformImage::getItem(platformImages, c);                
        if(!platformImage->getId())
        {
            // Checks if the image is still downloading, in this case, the image should be saved after it is downloaded
            int isDownloading = 0;
            pthread_mutex_lock(&downloadPlatformImagesMutex);            
            for(list<UiThreadHandler *>::iterator downloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); downloadPlatformImageHandler != downloadPlatformImageHandlers->end(); downloadPlatformImageHandler++)
            {
                DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*downloadPlatformImageHandler)->getRequesterInUiThread();
                if(downloadPlatformImage->platformDialog == this && downloadPlatformImage->platformImage == platformImage)
                {
                    isDownloading = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&downloadPlatformImagesMutex);

            if(!isDownloading)
            {
                saveNewImage(platformImage);
            }
        }
        else
        {
            sqlite3 *sqlite = Database::getInstance()->acquire();
            platformImage->save(sqlite);
            Database::getInstance()->release();
        }
    }

    saved = 1;
    NotificationManager::getInstance()->postNotification(NOTIFICATION_PLATFORM_UPDATED, platform);
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}



void PlatformDialog::signalPlatformComboBoxChanged(GtkComboBox *comboBox, gpointer platformDialog)
{    
    ((PlatformDialog *)platformDialog)->updateApiPlatform();
}

void PlatformDialog::signalImageTypeComboBoxChanged(GtkComboBox* comboBox, gpointer platformDialog)
{
    ((PlatformDialog *)platformDialog)->updateImageType();
}

void PlatformDialog::signalRemoveImageButtonClicked(GtkButton* button, gpointer platformDialog)
{
    ((PlatformDialog *)platformDialog)->removeImage();
}

void PlatformDialog::signalAddImageButtonClicked(GtkButton* button, gpointer platformDialog)
{
    ((PlatformDialog *)platformDialog)->addImage();
}

void PlatformDialog::signalCancelButtonClicked(GtkButton *button, gpointer platformDialog)
{
    ((PlatformDialog *)platformDialog)->cancel();
}

void PlatformDialog::signalSaveButtonClicked(GtkButton *button, gpointer platformDialog)
{
    ((PlatformDialog *)platformDialog)->save();
}

gboolean PlatformDialog::signalImageBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer platformDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        PlatformImage *platformImage = PlatformImage::getItem(((PlatformDialog *)platformDialog)->platformImages, atoi(gtk_widget_get_name(widget)));
        ((PlatformDialog *)platformDialog)->selectImage(platformImage);
    }
    
    return TRUE;
}

void PlatformDialog::callbackElasticsearchPlatforms(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;    
    PlatformDialog *platformDialog = (PlatformDialog *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;
    
    platformDialog->apiPlatforms = (list<TheGamesDB::Platform *> *)dbResult->data;
    if(!dbResult->error)
    {
        GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(platformDialog->apiPlatformComboBox, GTK_TREE_MODEL(listStore));

        unsigned int selectedIndex = 0;
        for(unsigned int index = 0; index < platformDialog->apiPlatforms->size(); index++)
        {
            TheGamesDB::Platform *apiPlatform = TheGamesDB::Platform::getItem(platformDialog->apiPlatforms, index);
            gtk_list_store_insert_with_values (listStore, NULL, index, 0, apiPlatform->getName().c_str(), -1);
            
            if(platformDialog->platform->getId() > 0 && platformDialog->platform->getApiId() == TheGamesDB::API_ID && platformDialog->platform->getApiItemId() == apiPlatform->getId())
            {
                selectedIndex = index;
            }
        }
        
        GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(platformDialog->apiPlatformComboBox), cellRenderer, 0);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(platformDialog->apiPlatformComboBox), cellRenderer, "text", 0, NULL);

        if(platformDialog->platform->getId() > 0)
        {
            gtk_widget_set_sensitive(GTK_WIDGET(platformDialog->apiPlatformComboBox), 0);
        }
        gtk_combo_box_set_active(platformDialog->apiPlatformComboBox, selectedIndex);
    }
        
    UiThreadHandler::releaseResult(uiThreadHandlerResult);
}





list<UiThreadHandler *> *PlatformDialog::downloadPlatformImageHandlers = new list<UiThreadHandler *>;
pthread_t PlatformDialog::downloadPlatformImagesThread;
pthread_mutex_t PlatformDialog::downloadPlatformImagesMutex = PTHREAD_MUTEX_INITIALIZER;
int PlatformDialog::downloadingPlatformImages = 0;

void PlatformDialog::downloadPlatformImage(PlatformDialog* platformDialog, PlatformImage* platformImage)
{
    pthread_mutex_lock(&downloadPlatformImagesMutex);
    
    DownloadPlatformImage_t *downloadPlatformImage = new DownloadPlatformImage_t;
    downloadPlatformImage->platformDialog = platformDialog;
    downloadPlatformImage->platformImage = platformImage;
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(downloadPlatformImage, callbackDownloadPlatformImage);
    downloadPlatformImageHandlers->push_back(uiThreadHandler);
        
    if(!downloadingPlatformImages)
    {
        downloadingPlatformImages = 1;
        if(pthread_create(&downloadPlatformImagesThread, NULL, downloadPlatformImagesWorker, NULL) != 0) 
        {
            cerr << "PlatformDialog::" << __FUNCTION__ << endl;
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&downloadPlatformImagesMutex);
}

void *PlatformDialog::downloadPlatformImagesWorker(void *)
{    
    UiThreadHandler *downloadPlatformImageHandler = NULL;
    do
    {
        downloadPlatformImageHandler = NULL;
        
        pthread_mutex_lock(&downloadPlatformImagesMutex);
        for(list<UiThreadHandler *>::iterator aDownloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); aDownloadPlatformImageHandler != downloadPlatformImageHandlers->end(); aDownloadPlatformImageHandler++)
        {
            DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)(*aDownloadPlatformImageHandler)->getRequesterInUiThread();
            if(!downloadPlatformImage->platformImage->getDownloaded())
            {
                downloadPlatformImageHandler = (*aDownloadPlatformImageHandler);
                break;
            }
        }
        pthread_mutex_unlock(&downloadPlatformImagesMutex);
        
        if(downloadPlatformImageHandler)
        {
            DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)downloadPlatformImageHandler->getRequesterInUiThread();
            
            HttpConnector *httpConnector = new HttpConnector(downloadPlatformImage->platformImage->getUrl());
            httpConnector->get();
            cout  << "PlatformDialog::" << __FUNCTION__ << " fileName: " << downloadPlatformImage->platformImage->getFileName() << " url: " << downloadPlatformImage->platformImage->getUrl() << " httpStatus: " << httpConnector->getHttpStatus() << endl;
            if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
            {
                Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), downloadPlatformImage->platformImage->getFileName());
                downloadPlatformImage->platformImage->setDownloaded(1);
            }
            delete httpConnector;
            
            UiThreadHandler::callback(downloadPlatformImageHandler, new char[1]); //Sends dummy data to avoid NULL
        }
        
    }while(downloadPlatformImageHandler);
    
    downloadingPlatformImages = 0;    
    return NULL;
}

void PlatformDialog::callbackDownloadPlatformImage(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;
    DownloadPlatformImage_t *downloadPlatformImage = (DownloadPlatformImage_t *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();    
    
    pthread_mutex_lock(&downloadPlatformImagesMutex);
    downloadPlatformImageHandlers->remove(uiThreadHandlerResult->uiThreadHandler);
    pthread_mutex_unlock(&downloadPlatformImagesMutex);
    
    // Checks if the PlatformImage is still present in the list of the PlatformDialog
    int isImage = 0;
    for(list<PlatformImage *>::iterator platformImage = downloadPlatformImage->platformDialog->platformImages->begin(); platformImage != downloadPlatformImage->platformDialog->platformImages->end(); platformImage++)
    {
        if(downloadPlatformImage->platformImage == (*platformImage))
        {
            isImage = 1;
            break;
        }
    }
    
    if(isImage)
    {
        // Checks if the PlarformDialog is still been shown
        if(!downloadPlatformImage->platformDialog->dismissed)
        {
            // Checks if the PlatformImage has been downloaded
            if(downloadPlatformImage->platformImage->getDownloaded())
            {
                downloadPlatformImage->platformDialog->updateImageGrid();
            }            
        }
        // Checks if the PlatformDialog was dismissed with the save instruction
        else
        {
            if(downloadPlatformImage->platformDialog->saved)
            {
               downloadPlatformImage->platformDialog->saveNewImage(downloadPlatformImage->platformImage); 
            }
        }
    }
    else
    {
        delete downloadPlatformImage->platformImage;
    }
    
    // Checks if the PlatformDialog was dismissed and if all its images has finished downloading
    if(downloadPlatformImage->platformDialog->dismissed)
    {
        pthread_mutex_lock(&downloadPlatformImagesMutex);
        int isDialog = 0;
        for(list<UiThreadHandler *>::iterator aDownloadPlatformImageHandler = downloadPlatformImageHandlers->begin(); aDownloadPlatformImageHandler != downloadPlatformImageHandlers->end(); aDownloadPlatformImageHandler++)
        {
            DownloadPlatformImage_t *aDownloadPlatformImage = (DownloadPlatformImage_t *)(*aDownloadPlatformImageHandler)->getRequesterInUiThread();
            if(aDownloadPlatformImage->platformDialog == downloadPlatformImage->platformDialog)
            {
                isDialog = 1;
                break;
            }
        }
        if(!isDialog)
        {
            if(downloadPlatformImage->platformDialog->saved)
            {
                NotificationManager::getInstance()->postNotification(NOTIFICATION_PLATFORM_UPDATED, downloadPlatformImage->platformDialog->platform);
            }
            delete downloadPlatformImage->platformDialog;                
        }
        pthread_mutex_unlock(&downloadPlatformImagesMutex);        
    }

    
    UiThreadHandler::releaseResult(uiThreadHandlerResult);
}

