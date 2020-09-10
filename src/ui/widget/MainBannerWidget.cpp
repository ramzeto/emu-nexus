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
 * File:   MainBannerWidget.cpp
 * Author: ram
 * 
 * Created on September 3, 2020, 11:53 PM
 */

#include "MainBannerWidget.h"
#include "Platform.h"
#include "Game.h"
#include "PlatformImage.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Asset.h"
#include "Logger.h"
#include "GameLauncher.h"
#include "Database.h"

const int MainBannerWidget::TYPE_HOME = 1;
const int MainBannerWidget::TYPE_RECENTS = 2;
const int MainBannerWidget::TYPE_FAVORITES = 3;
const int MainBannerWidget::TYPE_PLATFORM = 4;

MainBannerWidget *MainBannerWidget::instance = NULL;

MainBannerWidget::MainBannerWidget() : Widget("MainBannerWidget.ui", "mainBannerBox")
{
    backgroundImage = (GtkImage *)gtk_builder_get_object (builder, "backgroundImage");
    bannerImage = (GtkImage *)gtk_builder_get_object (builder, "bannerImage");
    overlayBackgroundImage = (GtkImage *)gtk_builder_get_object (builder, "overlayBackgroundImage");
    contentBox = (GtkBox *)gtk_builder_get_object (builder, "contentBox");
    
    bannerType = 0;
    platformId = 0;
    gameId = 0;
    setupBannerOverlayWidget = NULL;
    welcomeBannerOverlayWidget = NULL;
    gameBannerOverlayWidget = NULL;    
    bannerWidth = 0;
    bannerHeight = 0;
    
    signalSizeAllocateHandlerId = g_signal_connect (widget, "size-allocate", G_CALLBACK(+[](GtkWidget *widget, GtkAllocation *allocation, gpointer mainBannerWidget) -> void {
        if(((MainBannerWidget *)mainBannerWidget)->bannerWidth != allocation->width || ((MainBannerWidget *)mainBannerWidget)->bannerHeight != allocation->height)
        {        
            ((MainBannerWidget *)mainBannerWidget)->bannerWidth = allocation->width;
            ((MainBannerWidget *)mainBannerWidget)->bannerHeight = allocation->height;

            // Hacky reload
            g_timeout_add(10, [](gpointer mainBannerWidget) -> gint{
                if(((MainBannerWidget *)mainBannerWidget)->gameId)
                {
                    ((MainBannerWidget *)mainBannerWidget)->setGameId(((MainBannerWidget *)mainBannerWidget)->gameId);
                }
                else
                {
                    ((MainBannerWidget *)mainBannerWidget)->setBannerType(((MainBannerWidget *)mainBannerWidget)->bannerType, ((MainBannerWidget *)mainBannerWidget)->platformId);
                }

                return 0;
            }, mainBannerWidget);
        }
    }), this);
}

MainBannerWidget::~MainBannerWidget()
{
    g_signal_handler_disconnect(widget, signalSizeAllocateHandlerId);
}

int MainBannerWidget::getBannerType()
{
    return bannerType;
}

int MainBannerWidget::getPlatformId()
{
    return platformId;
}

void MainBannerWidget::setBannerType(int bannerType, int64_t platformId)
{
    this->platformId = platformId;
    this->bannerType = bannerType;
    
    if(GameLauncher::getInstance()->getStatus() != GameLauncher::STATUS_IDLE)
    {
        setGameId(GameLauncher::getInstance()->getGameId());
        return;
    }
    this->gameId = 0;
    
    gtk_image_clear(bannerImage);
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
        

    if(bannerType == TYPE_HOME)
    {                
        if(Database::getInstance()->isSetup())
        {                                    
            if(!welcomeBannerOverlayWidget)
            {
                if(setupBannerOverlayWidget)
                {
                    delete setupBannerOverlayWidget;
                    setupBannerOverlayWidget = NULL;
                }              
                if(gameBannerOverlayWidget)
                {
                    delete welcomeBannerOverlayWidget;
                    welcomeBannerOverlayWidget = NULL;
                }
                UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
                
                welcomeBannerOverlayWidget = new WelcomeBannerOverlayWidget();
                gtk_box_pack_start(contentBox, welcomeBannerOverlayWidget->getWidget(), 1, 1, 0);
                gtk_widget_show_all(GTK_WIDGET(contentBox));                
            }
        }
        else
        {
            if(!setupBannerOverlayWidget)
            {
                if(welcomeBannerOverlayWidget)
                {
                    delete welcomeBannerOverlayWidget;
                    welcomeBannerOverlayWidget = NULL;
                }
                if(gameBannerOverlayWidget)
                {
                    delete welcomeBannerOverlayWidget;
                    welcomeBannerOverlayWidget = NULL;
                }
                UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
                
                setupBannerOverlayWidget = new SetupBannerOverlayWidget();
                gtk_box_pack_start(contentBox, setupBannerOverlayWidget->getWidget(), 1, 1, 0);
                gtk_widget_show_all(GTK_WIDGET(contentBox));
            }
        }
        
        UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageBaseBanner(), width, height, 1);
        setBackgroundImage();
        setOverlayBackgroundVisibility(1);
    }
    else if(bannerType == TYPE_RECENTS)
    {
        if(setupBannerOverlayWidget)
        {
            delete setupBannerOverlayWidget;
            setupBannerOverlayWidget = NULL;
        }            
        if(welcomeBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }            
        if(gameBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
        
        UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageBaseBanner(), width, height, 1);    
        setBackgroundImage();
        setOverlayBackgroundVisibility(0);        
    }
    else if(bannerType == TYPE_FAVORITES)
    {
        if(setupBannerOverlayWidget)
        {
            delete setupBannerOverlayWidget;
            setupBannerOverlayWidget = NULL;
        }            
        if(welcomeBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }            
        if(gameBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
        
        UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageBaseBanner(), width, height, 1);    
        setBackgroundImage();
        setOverlayBackgroundVisibility(0);        
    }
    else if(bannerType == TYPE_PLATFORM)
    {
        if(setupBannerOverlayWidget)
        {
            delete setupBannerOverlayWidget;
            setupBannerOverlayWidget = NULL;
        }            
        if(welcomeBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }            
        if(gameBannerOverlayWidget)
        {
            delete welcomeBannerOverlayWidget;
            welcomeBannerOverlayWidget = NULL;
        }
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
        
        PlatformImage *platformImage = PlatformImage::getPlatformImage(this->platformId, PlatformImage::TYPE_BANNER);
        if(platformImage)
        {
            if(Utils::getInstance()->fileExists(platformImage->getFileName()))
            {
                UiUtils::getInstance()->loadImage(bannerImage, platformImage->getFileName(), width, height);
            }
            else
            {
                UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageLogo(), width, height);
            }
            delete platformImage;
        }
        else
        {
            UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageLogo(), width, height);
        }

        setBackgroundImage();
        setOverlayBackgroundVisibility(0);
    }
}

void MainBannerWidget::setGameId(int64_t gameId)
{
    if(setupBannerOverlayWidget)
    {
        delete setupBannerOverlayWidget;
        setupBannerOverlayWidget = NULL;
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
    }            
    if(welcomeBannerOverlayWidget)
    {
        delete welcomeBannerOverlayWidget;
        welcomeBannerOverlayWidget = NULL;
        UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
    }            
  

    if(GameLauncher::getInstance()->getStatus() != GameLauncher::STATUS_IDLE)
    {
        this->gameId = GameLauncher::getInstance()->getGameId();
    }
    else
    {
        this->gameId = gameId;
    }
        
    Game *game = new Game(this->gameId);    
    game->load();
    platformId = game->getPlatformId();
    
    gtk_image_clear(bannerImage);
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
    
    PlatformImage *platformImage = PlatformImage::getPlatformImage(platformId, PlatformImage::TYPE_BANNER);
    if(platformImage)
    {
        if(Utils::getInstance()->fileExists(platformImage->getFileName()))
        {
            UiUtils::getInstance()->loadImage(bannerImage, platformImage->getFileName(), width, height);
        }
        else
        {
            UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageLogo(), width, height);
        }
        delete platformImage;
    }
    else
    {
        UiUtils::getInstance()->loadImage(bannerImage, Asset::getInstance()->getImageLogo(), width, height);
    }
        
    setBackgroundImage();
    setOverlayBackgroundVisibility(1);
    
    if(gameBannerOverlayWidget != NULL)
    {
        if(gameBannerOverlayWidget->getGameId() != this->gameId)
        {
            delete gameBannerOverlayWidget;
            UiUtils::getInstance()->clearContainer(GTK_CONTAINER(contentBox), 1);
            
            gameBannerOverlayWidget = NULL;
        }
    }
    
    if(gameBannerOverlayWidget == NULL)
    {
       gameBannerOverlayWidget = new GameBannerOverlayWidget(this->gameId);
       gtk_box_pack_start(contentBox, gameBannerOverlayWidget->getWidget(), 1, 1, 0);
       gtk_widget_show_all(GTK_WIDGET(contentBox));
    }
    
    delete game;
}

void MainBannerWidget::setBackgroundImage()
{
    gtk_image_clear(backgroundImage);
    
    //https://developer.gnome.org/gdk-pixbuf/2.28/gdk-pixbuf-The-GdkPixbuf-Structure.html#image-data
    
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(bannerImage);
    
    int width = gdk_pixbuf_get_width (pixbuf);
    int height = gdk_pixbuf_get_height (pixbuf);
    int channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int channelsAverage[channels];
    
    for(int c = 0; c < channels; c++)
    {
        channelsAverage[c] = 0;
    }
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            guchar *pixel = pixels + y * rowstride + x * channels;
            //guchar red = pixel[0];
            //guchar green = pixel[1];
            //guchar blue = pixel[2];

            for(int c = 0; c < channels; c++)
            {
                channelsAverage[c] += pixel[c];
            }
        }
    }
    
    int numberOfPixels = height * width;
    for(int c = 0; c < channels; c++)
    {
        channelsAverage[c] /= numberOfPixels;
    }
    int rawDataArrayLength = height * rowstride + width * channels;
    guchar *averagePixels = new guchar[rawDataArrayLength];
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            guchar *pixel = averagePixels + y * rowstride + x * channels;
            for(int c = 0; c < channels; c++)
            {
                pixel[c] = channelsAverage[c];
            }
        }
    }
    
    GBytes *averagePixelsBytes = g_bytes_new_take(averagePixels, rawDataArrayLength);
    GdkPixbuf *averagePixbuf = gdk_pixbuf_new_from_bytes(averagePixelsBytes, gdk_pixbuf_get_colorspace(pixbuf), gdk_pixbuf_get_has_alpha(pixbuf), 8, width, height, rowstride);
    
    int bannerWidth = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    int bannerHeight = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
    GdkPixbuf *scaledPixbuf = gdk_pixbuf_scale_simple(averagePixbuf, bannerWidth, bannerHeight, GDK_INTERP_NEAREST);
    gtk_image_set_from_pixbuf(backgroundImage, scaledPixbuf);
    
    g_object_unref(averagePixbuf);
}

void MainBannerWidget::setOverlayBackgroundVisibility(int visible)
{
    gtk_image_clear(overlayBackgroundImage);
    
    if(visible)
    {
        gtk_widget_show(GTK_WIDGET(overlayBackgroundImage));
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(overlayBackgroundImage));
    }
}



MainBannerWidget* MainBannerWidget::getInstance()
{
    if(!instance)
    {
        instance = new MainBannerWidget();
    }
    
    return instance;
}
