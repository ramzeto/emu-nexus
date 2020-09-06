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
 * File:   MainBannerWidget.h
 * Author: ram
 *
 * Created on September 3, 2020, 11:53 PM
 */

#ifndef MAINBANNERWIDGET_H
#define MAINBANNERWIDGET_H

#include "Widget.h"
#include "GameBannerWidget.h"

class MainBannerWidget : public Widget
{
public:
    static const int TYPE_HOME;
    static const int TYPE_RECENTS;
    static const int TYPE_FAVORITES;
    static const int TYPE_PLATFORM;
    
    MainBannerWidget();
    virtual ~MainBannerWidget();
    
    int getBannerType();
    int getPlatformId();
    
    void setBannerType(int bannerType, int64_t platformId = 0);
    void setGameId(int64_t gameId);   
        
    static MainBannerWidget *getInstance();
    
private:   
    GtkImage *backgroundImage;
    GtkImage *bannerImage;
    GtkImage *overlayImage;
    GtkBox *contentBox;
    
    int bannerWidth;
    int bannerHeight;
    int bannerType;
    int64_t gameId;
    int64_t platformId;
    gulong signalSizeAllocateHandlerId;
    GameBannerWidget *gameBannerWidget;
    
    void setBackgroundImage();
    void setOverlayVisibility(int visible);
    
    static MainBannerWidget *instance;
    
    /**
     * Signal triggered when the "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param mainBannerWidget
     */
    static void signalSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer mainBannerWidget);

    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the draw of the widget.
     * @param mainBannerWidget
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer mainBannerWidget);    
};

#endif /* MAINBANNERWIDGET_H */

