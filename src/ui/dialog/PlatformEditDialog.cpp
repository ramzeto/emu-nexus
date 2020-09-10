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
 * File:   PlatformEditDialog.cpp
 * Author: ram
 * 
 * Created on March 10, 2019, 7:14 PM
 */

#include "PlatformEditDialog.h"
#include "UiUtils.h"
#include "Preferences.h"
#include "Utils.h"
#include "MessageDialog.h"
#include "PlatformImage.h"
#include "HttpConnector.h"
#include "Asset.h"
#include "NotificationManager.h"
#include "Notifications.h"
#include "Directory.h"
#include "ThreadManager.h"

const int PlatformEditDialog::THUMBNAIL_IMAGE_WIDTH = 90;
const int PlatformEditDialog::THUMBNAIL_IMAGE_HEIGHT = 90;
const int PlatformEditDialog::IMAGE_WIDTH = 200;
const int PlatformEditDialog::IMAGE_HEIGHT = 200;

PlatformEditDialog::PlatformEditDialog(GtkWindow *parent, int64_t platformId) : Dialog(parent, "PlatformEditDialog.ui", "platformEditDialog")
{
    saved = 0;
    apiPlatform = NULL;
    apiPlatforms = NULL;
    
    platform = new Platform(platformId);
    platform->load();
    
    platformImages = PlatformImage::getItems(platform->getId());
    
    platformImageTypes = new list<int64_t>;    
    selectedPlatformImage = NULL;
    
    platformImagesToRemove = new list<PlatformImage *>;
    platformImageBoxes = new map<PlatformImage *, GtkWidget *>;    
    
    apiPlatformComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "apiPlatformComboBox");        
    nameEntry = (GtkEntry *)gtk_builder_get_object (builder, "nameEntry");        
    commandEntry = (GtkEntry *)gtk_builder_get_object (builder, "commandEntry");        
    deflateCheckButton = (GtkCheckButton *)gtk_builder_get_object (builder, "deflateCheckButton");        
    deflateFileExtensionsEntry = (GtkEntry *)gtk_builder_get_object (builder, "deflateFileExtensionsEntry");
    imageTypeComboBox = (GtkComboBox *)gtk_builder_get_object (builder, "imageTypeComboBox");            
    image = (GtkImage *)gtk_builder_get_object (builder, "image");    
    removeImageButton = (GtkButton *)gtk_builder_get_object (builder, "removeImageButton");
    imagesGridListBox = (GtkListBox *)gtk_builder_get_object (builder, "imagesGridListBox");    
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");    
    saveButton = (GtkButton *)gtk_builder_get_object(builder, "saveButton");
    
    
    gtk_entry_set_text(nameEntry, platform->getName().c_str());
    gtk_entry_set_text(commandEntry, platform->getCommand().c_str());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(deflateCheckButton), platform->getDeflate());
    gtk_entry_set_text(deflateFileExtensionsEntry, platform->getDeflateFileExtensions().c_str());
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    gtk_image_clear(image);
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
    toggleDeflate();
        
    g_signal_connect (apiPlatformComboBox, "changed", G_CALLBACK (+[](GtkComboBox *comboBox, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->updateApiPlatform();
    }), this);    
    
    g_signal_connect (deflateCheckButton, "toggled", G_CALLBACK (+[](GtkToggleButton *togglebutton, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->toggleDeflate(); 
    }), this); 
    
    g_signal_connect (imageTypeComboBox, "changed", G_CALLBACK (+[](GtkComboBox *comboBox, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->updateImageType();
    }), this);
    
    g_signal_connect (removeImageButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->removeImage();
    }), this);
    
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->cancel();
    }), this);
    
    g_signal_connect (saveButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer platformEditDialog) -> void {
        ((PlatformEditDialog *)platformEditDialog)->save();
    }), this);    
    
    loadApiPlatforms();
    loadPlatformImageTypes();
    updateImageGrid();        
}

PlatformEditDialog::~PlatformEditDialog()
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

void PlatformEditDialog::deleteWhenReady(PlatformEditDialog *platformEditDialog)
{
    // Checks if the PlatformEditDialog is waiting for PlatformImages to be downloaded
    int isDownloading = 0;
    pthread_mutex_lock(&downloadPlatformImageRefsMutex);
    for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
    {
        if((*aDownloadPlatformImageRef)->platformEditDialog == platformEditDialog)
        {
            isDownloading = 1;
            break;
        }
    }
    pthread_mutex_unlock(&downloadPlatformImageRefsMutex);
    
    platformEditDialog->dismiss();
    if(!isDownloading)
    {
        delete platformEditDialog;
    }
}

void PlatformEditDialog::toggleDeflate()
{
    gtk_widget_set_sensitive(GTK_WIDGET(deflateFileExtensionsEntry), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(deflateCheckButton)));
}

void PlatformEditDialog::loadApiPlatforms()
{
    if(platform->getId() > 0)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(apiPlatformComboBox), 0);
    }
    
    TheGamesDB::Elasticsearch::getInstance()->getPlatforms([this](list<TheGamesDB::Platform *> *apiPlatforms) -> void {
        this->apiPlatforms = new list<TheGamesDB::Platform *>;
    
        for(unsigned int index = 0; index < apiPlatforms->size(); index++)
        {
            TheGamesDB::Platform *apiPlatform = TheGamesDB::Platform::getItem(apiPlatforms, index);
            this->apiPlatforms->push_back(new TheGamesDB::Platform(*apiPlatform));
        }

        GtkListStore *listStore = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(apiPlatformComboBox, GTK_TREE_MODEL(listStore));

        unsigned int selectedIndex = 0;
        for(unsigned int index = 0; index < this->apiPlatforms->size(); index++)
        {
            TheGamesDB::Platform *apiPlatform = TheGamesDB::Platform::getItem(this->apiPlatforms, index);
            gtk_list_store_insert_with_values (listStore, NULL, index, 0, apiPlatform->getName().c_str(), -1);

            if(platform->getId() > 0 && platform->getApiId() == TheGamesDB::API_ID && platform->getApiItemId() == apiPlatform->getId())
            {
                selectedIndex = index;
            }
        }

        GtkCellRenderer *cellRenderer = gtk_cell_renderer_text_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(apiPlatformComboBox), cellRenderer, 0);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(apiPlatformComboBox), cellRenderer, "text", 0, NULL);

        if(platform->getId() > 0)
        {
            gtk_widget_set_sensitive(GTK_WIDGET(apiPlatformComboBox), 0);
        }
        gtk_combo_box_set_active(apiPlatformComboBox, selectedIndex);
    });
}

void PlatformEditDialog::updateApiPlatform()
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
        pthread_mutex_lock(&downloadPlatformImageRefsMutex);        
        for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
        {
            if((*aDownloadPlatformImageRef)->platformEditDialog == this && (*aDownloadPlatformImageRef)->platformImage->getApiId() == TheGamesDB::API_ID && (*aDownloadPlatformImageRef)->platformImage->getApiItemId() == apiPlatformImage->getId())
            {
                platformImages->push_back((*aDownloadPlatformImageRef)->platformImage);
                isDownloading = 1;
                break;
            }
        }
        pthread_mutex_unlock(&downloadPlatformImageRefsMutex);
        //_________

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


void PlatformEditDialog::loadPlatformImageTypes()
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

void PlatformEditDialog::updateImageGrid()
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
    else if(platformImages->size() > 0)
    {
        selectImage(PlatformImage::getItem(platformImages, 0));
    }
}

void PlatformEditDialog::addImage()
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

void PlatformEditDialog::removeAllImages()
{
    selectedPlatformImage = NULL;
    
    // Checks if there are PlatformImages being downloaded, PlarformImages that are being downloaded cannot be deleted
    for(list<PlatformImage *>::iterator platformImage = platformImages->begin(); platformImage != platformImages->end(); platformImage++)
    {        
        pthread_mutex_lock(&downloadPlatformImageRefsMutex);
        int isDownloading = 0;
        for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
        {
            if((*aDownloadPlatformImageRef)->platformEditDialog == this && (*aDownloadPlatformImageRef)->platformImage == (*platformImage))
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {
            delete (*platformImage);
        }
        pthread_mutex_unlock(&downloadPlatformImageRefsMutex);    
    }
    platformImages->clear();
        
    updateImageGrid();
}

void PlatformEditDialog::removeImage()
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
        pthread_mutex_lock(&downloadPlatformImageRefsMutex);
        int isDownloading = 0;
        for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
        {
            if((*aDownloadPlatformImageRef)->platformEditDialog == this && (*aDownloadPlatformImageRef)->platformImage == selectedPlatformImage)
            {
                isDownloading = 1;
                break;
            }
        }
        if(!isDownloading)
        {
            delete selectedPlatformImage;
        }
        pthread_mutex_unlock(&downloadPlatformImageRefsMutex);
    }    
    selectedPlatformImage = NULL;
    
    gtk_widget_hide(GTK_WIDGET(imageTypeComboBox));
    gtk_widget_hide(GTK_WIDGET(removeImageButton));
    gtk_image_clear(image);
    updateImageGrid();
}

void PlatformEditDialog::updateImageType()
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

void PlatformEditDialog::selectImage(PlatformImage* platformImage)
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

void PlatformEditDialog::downloadPlatformImage(PlatformImage *platformImage)
{
    downloadPlatformImage(this, platformImage);
}

void PlatformEditDialog::saveNewImage(PlatformImage* platformImage)
{
    // When the image is new, copies the file to the media directory and creates the thumbnail.        
    if(Utils::getInstance()->fileExists(platformImage->getFileName()))
    {
        // Saves before to generate the id
        platformImage->setPlatformId(platform->getId());
        platformImage->save();
        
        string imageFileName = platform->getMediaDirectory() + PlatformImage::FILE_PREFIX + to_string(platformImage->getId());
        if(!Utils::getInstance()->copyFile(platformImage->getFileName(), imageFileName))
        {
            platformImage->setFileName(imageFileName);

            Utils::getInstance()->scaleImage(platformImage->getFileName(), PlatformImage::THUMBNAIL_WIDTH, PlatformImage::THUMBNAIL_HEIGHT, platformImage->getThumbnailFileName());            
            platformImage->save();
        }        
    }    
}

void PlatformEditDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void PlatformEditDialog::save()
{    
    string name = Utils::getInstance()->trim(string(gtk_entry_get_text(nameEntry)));
    if(name.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog(GTK_WINDOW(dialog), "Name cannot be empty", "Ok", "");   
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
    platform->setDescription("");

    platform->setCommand(string(gtk_entry_get_text(commandEntry)));
    platform->setDeflate(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(deflateCheckButton)));
    platform->setDeflateFileExtensions(string(gtk_entry_get_text(deflateFileExtensionsEntry)));    
    platform->save();
    
    for(unsigned int c = 0; c < platformImagesToRemove->size(); c++)
    {
        PlatformImage *platformImage = PlatformImage::getItem(platformImagesToRemove, c);
        platformImage->remove();
    }
        
    for(unsigned int c = 0; c < platformImages->size(); c++)
    {
        PlatformImage *platformImage = PlatformImage::getItem(platformImages, c);                
        if(!platformImage->getId())
        {
            // Checks if the image is still downloading, in this case, the image should be saved after it is downloaded
            int isDownloading = 0;
            pthread_mutex_lock(&downloadPlatformImageRefsMutex);            
            for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
            {
                if((*aDownloadPlatformImageRef)->platformEditDialog == this && (*aDownloadPlatformImageRef)->platformImage == platformImage)
                {
                    isDownloading = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&downloadPlatformImageRefsMutex);

            if(!isDownloading)
            {
                saveNewImage(platformImage);
            }
        }
        else
        {
            platformImage->save();
        }
    }

    saved = 1;
    
    NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0 , 0, new Platform(*platform));
    
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}



void PlatformEditDialog::signalAddImageButtonClicked(GtkButton* button, gpointer platformEditDialog)
{
    ((PlatformEditDialog *)platformEditDialog)->addImage();
}

gboolean PlatformEditDialog::signalImageBoxButtonPressedEvent(GtkWidget* widget, GdkEvent* event, gpointer platformEditDialog)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        PlatformImage *platformImage = PlatformImage::getItem(((PlatformEditDialog *)platformEditDialog)->platformImages, atoi(gtk_widget_get_name(widget)));
        ((PlatformEditDialog *)platformEditDialog)->selectImage(platformImage);
    }
    
    return TRUE;
}





list<PlatformEditDialog::DownloadPlatformImageRef_t *> *PlatformEditDialog::downloadPlatformImageRefs = new list<PlatformEditDialog::DownloadPlatformImageRef_t *>;
pthread_t PlatformEditDialog::downloadPlatformImagesThread;
pthread_mutex_t PlatformEditDialog::downloadPlatformImageRefsMutex = PTHREAD_MUTEX_INITIALIZER;
int PlatformEditDialog::downloadingPlatformImages = 0;

void PlatformEditDialog::downloadPlatformImage(PlatformEditDialog* platformEditDialog, PlatformImage* platformImage)
{
    pthread_mutex_lock(&downloadPlatformImageRefsMutex);
    
    DownloadPlatformImageRef_t *downloadPlatformImageRef = new DownloadPlatformImageRef_t;
    downloadPlatformImageRef->platformEditDialog = platformEditDialog;
    downloadPlatformImageRef->platformImage = platformImage;
    downloadPlatformImageRefs->push_back(downloadPlatformImageRef);
        
    if(!downloadingPlatformImages)
    {
        downloadingPlatformImages = 1;
        ThreadManager::getInstance()->execute(0, []() -> void {
            DownloadPlatformImageRef_t *downloadPlatformImageRef = NULL;
            do
            {
                downloadPlatformImageRef = NULL;

                pthread_mutex_lock(&downloadPlatformImageRefsMutex);
                for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
                {
                    if(!(*aDownloadPlatformImageRef)->platformImage->getDownloaded())
                    {
                        downloadPlatformImageRef = (*aDownloadPlatformImageRef);
                        break;
                    }
                }
                pthread_mutex_unlock(&downloadPlatformImageRefsMutex);

                if(downloadPlatformImageRef)
                {
                    HttpConnector *httpConnector = new HttpConnector(downloadPlatformImageRef->platformImage->getUrl());
                    httpConnector->get();            
                    if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
                    {
                        Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), downloadPlatformImageRef->platformImage->getFileName());
                        downloadPlatformImageRef->platformImage->setDownloaded(1);
                    }
                    delete httpConnector;

                    ThreadManager::getInstance()->execute(1, [downloadPlatformImageRef]() -> void {
    
                        pthread_mutex_lock(&downloadPlatformImageRefsMutex);
                        downloadPlatformImageRefs->remove(downloadPlatformImageRef);
                        pthread_mutex_unlock(&downloadPlatformImageRefsMutex);

                        // Checks if the PlatformImage is still present in the list of the PlatformEditDialog
                        int isImage = 0;
                        for(list<PlatformImage *>::iterator platformImage = downloadPlatformImageRef->platformEditDialog->platformImages->begin(); platformImage != downloadPlatformImageRef->platformEditDialog->platformImages->end(); platformImage++)
                        {
                            if(downloadPlatformImageRef->platformImage == (*platformImage))
                            {
                                isImage = 1;
                                break;
                            }
                        }

                        if(isImage)
                        {
                            // Checks if the PlarformDialog is still been shown
                            if(!downloadPlatformImageRef->platformEditDialog->dismissed)
                            {
                                // Checks if the PlatformImage has been downloaded
                                if(downloadPlatformImageRef->platformImage->getDownloaded())
                                {
                                    downloadPlatformImageRef->platformEditDialog->updateImageGrid();
                                }            
                            }
                            // Checks if the PlatformEditDialog was dismissed with the save instruction
                            else
                            {
                                if(downloadPlatformImageRef->platformEditDialog->saved)
                                {
                                   downloadPlatformImageRef->platformEditDialog->saveNewImage(downloadPlatformImageRef->platformImage); 
                                }
                            }
                        }
                        else
                        {
                            delete downloadPlatformImageRef->platformImage;
                        }

                        // Checks if the PlatformEditDialog was dismissed and if all its images has finished downloading
                        if(downloadPlatformImageRef->platformEditDialog->dismissed)
                        {
                            pthread_mutex_lock(&downloadPlatformImageRefsMutex);
                            int isDialog = 0;
                            for(list<DownloadPlatformImageRef_t *>::iterator aDownloadPlatformImageRef = downloadPlatformImageRefs->begin(); aDownloadPlatformImageRef != downloadPlatformImageRefs->end(); aDownloadPlatformImageRef++)
                            {
                                if((*aDownloadPlatformImageRef)->platformEditDialog == downloadPlatformImageRef->platformEditDialog)
                                {
                                    isDialog = 1;
                                    break;
                                }
                            }
                            if(!isDialog)
                            {
                                if(downloadPlatformImageRef->platformEditDialog->saved)
                                {                                    
                                    NotificationManager::getInstance()->notify(NOTIFICATION_PLATFORM_UPDATED, "", 0 , 0, new Platform(*(downloadPlatformImageRef->platformEditDialog->platform)));
                                }
                                delete downloadPlatformImageRef->platformEditDialog;                
                            }
                            pthread_mutex_unlock(&downloadPlatformImageRefsMutex);        
                        }
                    });
                }

            }while(downloadPlatformImageRef);

            downloadingPlatformImages = 0;
        });
    }
    pthread_mutex_unlock(&downloadPlatformImageRefsMutex);
}
