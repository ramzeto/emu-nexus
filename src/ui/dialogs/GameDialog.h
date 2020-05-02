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
 * File:   GameDialog.h
 * Author: ram
 *
 * Created on March 21, 2019, 12:47 AM
 */

#ifndef GAMEDIALOG_H
#define GAMEDIALOG_H

#include "Dialog.h"
#include "Game.h"
#include "Developer.h"
#include "Publisher.h"
#include "Genre.h"
#include "EsrbRating.h"
#include "GameImage.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameGenre.h"
#include "GameDocument.h"

#include <pthread.h>
#include <list>
#include <map>

using namespace std;

/**
 * A dialog to configure a game. The dialog will call gtk_dialog_response(..., GTK_RESPONSE_ACCEPT) if accepted.
 */
class GameDialog : public Dialog
{
public:
    
    /**
     * 
     * @param parent Parent GtkWindow.
     * @param platformId Id of the platform of the game.
     * @param gameId Id of the game to configure. If is 0, a new game will be created.
     */
    GameDialog(GtkWindow *parent, int64_t platformId, int64_t gameId);    
    
    /**
     * This method should be called to dismiss the dialog. Explicitely deleting the dialog is forbidden. The dialog downloads the game images when required and the download process may continue after the dialog is dismissed.
     * @param platformDialog PlatformDialog to dismiss.
     */
    static void deleteWhenReady(GameDialog *gameDialog);
    
private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    static const int IMAGE_WIDTH;
    static const int IMAGE_HEIGHT;        
    
    Game *game;
    list<Developer *> *developers;
    list<Publisher *> *publishers;
    list<Genre *> *genres;
    list<EsrbRating *> *esrbRatings;    
    
    list<int64_t> *gameImageTypes;
    list<GameImage *> *gameImages;
    list<GameImage *> *gameImagesToRemove;
    map<GameImage *, GtkWidget *> *gameImageBoxes;
    GameImage *selectedGameImage;
    
    list<int64_t> *gameDocumentTypes;
    list<GameDocument *> *gameDocuments;
    list<GameDocument *> *gameDocumentsToRemove;
    map<GameDocument *, GtkWidget *> *gameDocumentBoxes;
    GameDocument *selectedGameDocument;
    int saved;
    
    GtkEntry *nameEntry;
    GtkButton *searchButton;
    GtkEntry *fileNameEntry;
    GtkButton *fileNameButton;
    GtkEntry *commandEntry;
    GtkCheckButton *deflateCheckButton;
    GtkEntry *deflateFileExtensionsEntry;    
    GtkEntry *releaseDateEntry;
    GtkComboBox *esrbComboBox;
    GtkButton *addDeveloperButton;
    GtkButton *addPublisherButton;
    GtkButton *addGenreButton;
    GtkListBox *developersListBox;
    GtkListBox *publishersListBox;
    GtkListBox *genresListBox;
    
    GtkTextView *descriptionTextView;
    
    GtkComboBox *imageTypeComboBox;
    GtkImage *image;
    GtkButton *removeImageButton;
    GtkListBox *imagesGridListBox;   
    
    GtkListBox *documentGridListBox;
    GtkComboBox *documentTypeComboBox;
    GtkEntry *documentNameEntry;
    GtkImage *documentImage;
    GtkButton *removeDocumentButton;
    
    GtkTextView *notesTextView;
    
    GtkButton *cancelButton;
    GtkButton *saveButton;
    
    virtual ~GameDialog();
    
    void loadDevelopers();
    void removeDeveloper(int64_t developerId);
    void showDevelopersDialog();
    
    void loadPublishers();
    void removePublisher(int64_t publisherId);
    void showPublishersDialog();    
    
    void loadGenres();
    void removeGenre(int64_t genreId);
    void showGenresDialog();
    
    
    void loadEsrbRatings();
    void selectFileName();
    
    void loadGameImageTypes();
    void updateGameImageGrid();
    void addGameImage();
    void removeGameImage();
    void removeNewGameImages();
    void updateGameImageType();
    void selectGameImage(GameImage *gameImage);
    void downloadGameImage(GameImage *gameImage);
    void saveNewGameImage(GameImage *gameImage);
    
    void loadGameDocumentTypes();
    void updateGameDocumentGrid();
    void addGameDocument();
    void removeGameDocument();
    void updateGameDocumentType();
    void selectGameDocument(GameDocument *gameDocument);
    
    void search();
    
    void cancel();
    void save();
    
    
    
    static void signalSearchButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalFileNameButtonClicked(GtkButton *button, gpointer gameDialog);

    static void signalAddDeveloperButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalRemoveDeveloperButtonClicked(GtkButton *button, gpointer gameDialog);
    
    static void signalAddPublisherButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalRemovePublisherButtonClicked(GtkButton *button, gpointer gameDialog);
    
    static void signalAddGenreButtonClicked(GtkButton *button, gpointer gameDialog);    
    static void signalRemoveGenreButtonClicked(GtkButton *button, gpointer gameDialog);
    
    static void signalImageTypeComboBoxChanged(GtkComboBox *comboBox, gpointer gameDialog);
    static void signalRemoveImageButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalAddImageButtonClicked(GtkButton *button, gpointer gameDialog);
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog);

    static void signalDocumentTypeComboBoxChanged(GtkComboBox *comboBox, gpointer gameDialog);
    static void signalRemoveDocumentButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalAddDocumentButtonClicked(GtkButton *button, gpointer gameDialog);
    static gboolean signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDialog);
    static gboolean signalDocumentNameEntryKeyReleaseEvent(GtkEntry *entry, GdkEvent *event, gpointer gameDialog);
    
    static void signalCancelButtonClicked(GtkButton *button, gpointer gameDialog);
    static void signalSaveButtonClicked(GtkButton *button, gpointer gameDialog);
    
    
    
    typedef struct
    {
        GameDialog *gameDialog;
        GameImage *gameImage;
    }DownloadGameImageRef_t;
    
    static list<DownloadGameImageRef_t *> *downloadGameImageRefs;
    static pthread_t downloadGameImagesThread;
    static pthread_mutex_t downloadGameImageRefsMutex;    
    static int downloadingGameImages;
    static void downloadGameImage(GameDialog *gameDialog, GameImage *gameImage);
    static void *downloadGameImagesWorker(void *);
    static void callbackDownloadGameImage(void *pDownloadGameImageRef);
};

#endif /* GAMEDIALOG_H */

