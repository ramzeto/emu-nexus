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
 * File:   GameBannerWidget.h
 * Author: ram
 *
 * Created on September 4, 2020, 5:13 PM
 */

#ifndef GAMEBANNERWIDGET_H
#define GAMEBANNERWIDGET_H

#include "Widget.h"
#include "Game.h"
#include "GameImage.h"
#include "GameDocument.h"
#include "CallbackResult.h"

#include <list>
#include <map>

class GameBannerWidget : public Widget
{
public:
    GameBannerWidget(int64_t gameId);
    virtual ~GameBannerWidget();
    
    int64_t getGameId() const;
    
private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    static const int IMAGE_WIDTH;
    static const int IMAGE_HEIGHT;
    
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
    
    int bannerWidth;
    int bannerHeight;    
    gulong signalSizeAllocateHandlerId;
    
    void updateInformation();
    
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
     * @param gameBannerWidget
     * @return 
     */
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerWidget);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerWidget
     * @return 
     */
    static gboolean signalImageButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerWidget
     */
    static void signalImageMenuViewActivate(GtkMenuItem *menuitem, gpointer gameBannerWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerWidget
     */
    static void signalImageMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameBannerWidget);
        
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerWidget
     * @return 
     */
    static gboolean signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerWidget
     */
    static void signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameBannerWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameBannerWidget
     */
    static void signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameBannerWidget);
        
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerWidget
     * @return 
     */
    static gboolean signalFavoriteButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerWidget);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameBannerWidget
     * @return 
     */
    static gboolean signalEditButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerWidget);    
    
    /**
     * 
     * @param button
     * @param gameBannerWidget
     */    
    static void signalLaunchButtonClicked(GtkButton *button, gpointer gameBannerWidget);

    /**
     * Signal triggered when the "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param gameBannerWidget
     */
    static void signalSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer gameBannerWidget);    
    
    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the draw of the widget.
     * @param gameBannerWidget
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer gameBannerWidget);
    
    /**
     * 
     * @param callbackResult
     */
    static void callbackNotification(CallbackResult *callbackResult);
};

#endif /* GAMEBANNERWIDGET_H */

