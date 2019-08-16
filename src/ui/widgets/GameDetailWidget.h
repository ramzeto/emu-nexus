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
 * File:   GameDetailBox.h
 * Author: ram
 *
 * Created on April 9, 2019, 1:27 AM
 */

#ifndef GAMEDETAILWIDGET_H
#define GAMEDETAILWIDGET_H

#include "Widget.h"
#include "Game.h"
#include "GameImage.h"
#include "GameDocument.h"

#include <list>
#include <map>

using namespace std;

/**
 * A wrapper for the widget that shows the information of a game.
 */
class GameDetailWidget : public Widget
{
public:
    
    /**
     * 
     * @param gameId Id of the game.
     */
    GameDetailWidget(int64_t gameId);
    
    virtual ~GameDetailWidget();
    
private:
    static const int THUMBNAIL_IMAGE_WIDTH;
    static const int THUMBNAIL_IMAGE_HEIGHT;
    static const int IMAGE_HEIGHT;
    
    // @TODO This constant should not exists. The width should be gotten automatically.
    static const int WIDGET_WIDTH; 
    
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
    
    void updateInfo();
    
    void updateGameImageGrid();
    void selectGameImage(GameImage *gameImage);
    void viewGameImage(GameImage *gameImage);
    void saveGameImage(GameImage *gameImage);
    
    void updateGameDocumentGrid();
    void selectGameDocument(GameDocument *gameDocument);
    void viewGameDocument(GameDocument *gameDocument);
    void saveGameDocument(GameDocument *gameDocument);
    
    /**
     * Signal triggered when the gameGrid "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param gameDetailWidget
     */
    static void signalSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer gameDetailWidget);
    
    /**
     * 
     * @param widget
     * @param gameDetailWidget
     */
    static void signalShow(GtkWidget *widget, gpointer gameDetailWidget);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailWidget
     * @return 
     */
    static gboolean signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailWidget);
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailWidget
     * @return 
     */
    static gboolean signalImageButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailWidget
     */
    static void signalImageMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailWidget
     */
    static void signalImageMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget);
    
    
    /**
     * 
     * @param widget
     * @param event
     * @param gameDetailWidget
     * @return 
     */
    static gboolean signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameDetailWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailWidget
     */
    static void signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget);
    
    /**
     * 
     * @param menuitem
     * @param gameDetailWidget
     */
    static void signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameDetailWidget);
};

#endif /* GAMEDETAILBOX_H */

