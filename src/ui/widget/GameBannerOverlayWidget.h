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
 * File:   GameBannerOverlayWidget.h
 * Author: ram
 *
 * Created on September 4, 2020, 5:13 PM
 */

#ifndef GAMEBANNERWIDGET_H
#define GAMEBANNERWIDGET_H

#include "MainBannerOverlayWidget.h"
#include "Game.h"
#include "GameImage.h"
#include "GameDocument.h"
#include "Notification.h"

#include <list>
#include <map>

class GameBannerOverlayWidget : public MainBannerOverlayWidget
{
public:
    GameBannerOverlayWidget(int64_t gameId);
    virtual ~GameBannerOverlayWidget();
    
    int64_t getGameId() const;
    
private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    
    GtkEventBox *gameImageEventBox;
    GtkImage *gameImage;
    GtkLabel *nameLabel;
    GtkLabel *fileNameLabel;
    GtkBox *imagesBox;
    GtkBox *documentsBox;
    GtkLabel *informationLabel;
    GtkLabel *playTimeLabel;
    GtkLabel *lastPlayedLabel;
    GtkButton *launchButton;
    GtkBox *activityBox;
    GtkSpinner *spinner;
    GtkLabel *activityLabel;
    GtkEventBox *favoriteEventBox;
    GtkImage *favoriteImage;
    GtkEventBox *editEventBox;
    GtkImage *editImage;
    
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
    
    void update() override;
    
    void updateGameImagesGrid();
    void selectGameImage(GameImage *gameImage);
    void viewGameImage(GameImage *gameImage);
    void saveGameImage(GameImage *gameImage);
    
    void updateGameDocumentsGrid();
    void selectGameDocument(GameDocument *gameDocument);
    void viewGameDocument(GameDocument *gameDocument);
    void saveGameDocument(GameDocument *gameDocument);    
    
    void updateLaunchStatus(int status, int error, int progress = -1);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerOverlayWidget
     * @return 
     */
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget);    
    
    /**
     * 
     * @param menuitem
     * @param gameBannerOverlayWidget
     */
    static void signalImageMenuViewActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerOverlayWidget
     */
    static void signalImageMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget);
        
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerOverlayWidget
     * @return 
     */
    static gboolean signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerOverlayWidget
     */
    static void signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerOverlayWidget
     */
    static void signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget);
    
    /**
     * 
     * @param notification
     */
    static void onNotification(Notification *notification);  
};

#endif /* GAMEBANNERWIDGET_H */

