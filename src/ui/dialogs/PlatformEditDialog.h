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
 * File:   PlatformEditDialog.h
 * Author: ram
 *
 * Created on March 10, 2019, 7:14 PM
 */

#ifndef PLATFORMEDITDIALOG_H
#define PLATFORMEDITDIALOG_H

#include "Dialog.h"
#include "Platform.h"
#include "PlatformImage.h"
#include "thegamesdb.h"
#include "CallbackResult.h"
#include "UiThreadBridge.h"

#include <pthread.h>

#include <list>
#include <map>

using namespace std;

/**
 * A dialog to configure a Platform. The dialog will call gtk_dialog_response(..., GTK_RESPONSE_ACCEPT) if accepted.
 */
class PlatformEditDialog : public Dialog
{
public:
    
    /**
     * 
     * @param parent Parent GtkWindow.
     * @param platformId Id of the platform to configure. If is 0, a new platform will be created.
     */
    PlatformEditDialog(GtkWindow *parent, int64_t platformId);
    
    /**
     * This method should be called to dismiss the dialog. Explicitely deleting the dialog is forbidden. The dialog downloads the platform images when required and the download process may continue after the dialog is dismissed.
     * @param platformEditDialog PlatformEditDialog to dismiss.
     */
    static void deleteWhenReady(PlatformEditDialog *platformEditDialog);

private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    static const int IMAGE_WIDTH;
    static const int IMAGE_HEIGHT;
    
    list<TheGamesDB::Platform *> *apiPlatforms;
    TheGamesDB::Platform *apiPlatform;
    Platform *platform;
    list<int64_t> *platformImageTypes;
    list<PlatformImage *> *platformImages;
    list<PlatformImage *> *platformImagesToRemove;
    map<PlatformImage *, GtkWidget *> *platformImageBoxes;
    PlatformImage *selectedPlatformImage;
    int saved;
    
    GtkComboBox *apiPlatformComboBox;
    GtkEntry *nameEntry;
    GtkEntry *commandEntry;
    GtkCheckButton *deflateCheckButton;
    GtkEntry *deflateFileExtensionsEntry;
    GtkComboBox *imageTypeComboBox;
    GtkImage *image;
    GtkButton *removeImageButton;
    GtkListBox *imagesGridListBox;
    GtkButton *cancelButton;
    GtkButton *saveButton;
    
    UiThreadBridge *dataUiThreadBridge;
    
    virtual ~PlatformEditDialog();
    
    void loadApiPlatforms();
    void updateApiPlatform();
    void loadPlatformImageTypes();
    void updateImageGrid();
    void addImage();
    void removeAllImages();
    void removeImage();
    void updateImageType();
    void selectImage(PlatformImage *platformImage);
    void downloadPlatformImage(PlatformImage *platformImage);
    void saveNewImage(PlatformImage *platformImage);
    void cancel();
    void save();
    
    static void signalPlatformComboBoxChanged(GtkComboBox *comboBox, gpointer platformEditDialog);
    static void signalImageTypeComboBoxChanged(GtkComboBox *comboBox, gpointer platformEditDialog);
    static void signalRemoveImageButtonClicked(GtkButton *button, gpointer platformEditDialog);
    static void signalAddImageButtonClicked(GtkButton *button, gpointer platformEditDialog);
    static void signalCancelButtonClicked(GtkButton *button, gpointer platformEditDialog);
    static void signalSaveButtonClicked(GtkButton *button, gpointer platformEditDialog);
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer platformEditDialog);
    static void callbackElasticsearch(CallbackResult *callbackResult);
    
    

    typedef struct
    {
        PlatformEditDialog *platformEditDialog;
        PlatformImage *platformImage;
    }DownloadPlatformImageRef_t;
    
    static list<DownloadPlatformImageRef_t *> *downloadPlatformImageRefs;
    static pthread_t downloadPlatformImagesThread;
    static pthread_mutex_t downloadPlatformImageRefsMutex;    
    static int downloadingPlatformImages;
    static void downloadPlatformImage(PlatformEditDialog *platformEditDialog, PlatformImage *platformImage);
    static void *downloadPlatformImagesWorker(void *);
    static void callbackDownloadPlatformImage(void *pDownloadPlatformImageRef);
    
    
};

#endif /* PLATFORMEDITDIALOG_H */

