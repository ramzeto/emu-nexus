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
 * File:   GameDetailDialog.h
 * Author: ram
 *
 * Created on May 1, 2020, 9:29 PM
 */

#ifndef GAMEDETAILDIALOG_H
#define GAMEDETAILDIALOG_H

#include "Dialog.h"
#include "Game.h"
#include "GameImage.h"
#include "GameDocument.h"
#include "UiThreadBridge.h"

#include <list>
#include <map>

using namespace std;


class GameDetailDialog : public Dialog
{
public:
    GameDetailDialog(GtkWindow *parent, int64_t gameId);
    virtual ~GameDetailDialog();

    void launch();    
private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    static const int IMAGE_WIDTH;
    static const int IMAGE_HEIGHT;
    static const int INFORMATION_WIDTH;


    Game *game;    
    list<GameImage *> *gameImages;    
    GameImage *selectedGameImage;
    map<GameImage *, GtkWidget *> *gameImageBoxes;
    
    list<GameDocument *> *gameDocuments;
    GameDocument *selectedGameDocument;
    map<GameDocument *, GtkWidget *> *gameDocumentBoxes;
    
    time_t selectGameImageTimestamp;
    time_t viewGameImageTimestamp;
    
    time_t selectGameImageBoxTimestamp;
    time_t viewGameImageBoxTimestamp;

    time_t selectGameDocumentBoxTimestamp;
    time_t viewGameDocumentBoxTimestamp;
    
    int running;

    GtkLabel *nameLabel;
    GtkLabel *fileNameLabel;
    GtkLabel *platformLabel;
    GtkEventBox *imageBox;
    GtkImage *image;
    GtkLabel *imageTypeLabel;
    GtkBox *imagesBox;
    GtkLabel *documentsLabel;
    GtkBox *documentsBox;
    GtkLabel *informationLabel;
    GtkBox *launchBox;
    GtkSpinner *spinner;
    GtkLabel *messageLabel;
    GtkProgressBar *progressBar;
    
    UiThreadBridge *launcherUiThreadBridge;

    
    void updateInformation();
    
    void updateGameImagesGrid();
    void selectGameImage(GameImage *gameImage);
    void viewGameImage(GameImage *gameImage);
    void saveGameImage(GameImage *gameImage);
    
    void updateGameDocumentsGrid();
    void selectGameDocument(GameDocument *gameDocument);
    void viewGameDocument(GameDocument *gameDocument);
    void saveGameDocument(GameDocument *gameDocument);
    
    void setLaunchStatus(int running, string message, int progress);
    string selectLaunchFileName(list<string> fileNames);

    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailDialog
     * @return 
     */
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailDialog);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailDialog
     * @return 
     */
    static gboolean signalImageButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailDialog);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailDialog
     */
    static void signalImageMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDetailDialog);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailDialog
     */
    static void signalImageMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDetailDialog);
    
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailDialog
     * @return 
     */
    static gboolean signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailDialog);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailDialog
     */
    static void signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDetailDialog);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailDialog
     */
    static void signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDetailDialog);
    
    /**
     * 
     * @param window
     * @param event
     * @param gameDetailDialog
     * @return 
     */
    static gboolean signalDeleteEvent(GtkWidget *window, GdkEvent *event, gpointer gameDetailDialog);
    
    /**
     * 
     * @param entry
     * @param event
     * @param gameDetailDialog
     * @return 
     */
    static gboolean signalKeyPressedEvent(GtkEntry *entry, GdkEvent *event, gpointer gameDetailDialog);
    
    /**
     * 
     * @param callbackResult
     */
    static void callbackGameLauncher(CallbackResult *callbackResult);
};

#endif /* GAMEDETAILDIALOG_H */

