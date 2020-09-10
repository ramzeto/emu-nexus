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
 * File:   SetupBannerWidget.cpp
 * Author: ram
 * 
 * Created on September 6, 2020, 3:01 PM
 */

#include "SetupBannerOverlayWidget.h"
#include "Asset.h"
#include "UiUtils.h"
#include "SetupDatabaseProcess.h"
#include "SerialProcessExecutor.h"
#include "NotificationManager.h"
#include "Logger.h"
#include "Build.h"

SetupBannerOverlayWidget::SetupBannerOverlayWidget() : MainBannerOverlayWidget("SetupBannerOverlayWidget.ui", "setupBannerBox")
{
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    versionLabel = (GtkLabel *)gtk_builder_get_object (builder, "versionLabel");
    startButton = (GtkButton *)gtk_builder_get_object (builder, "startButton");
    activityBox = (GtkBox *)gtk_builder_get_object (builder, "activityBox");
    spinner = (GtkSpinner *)gtk_builder_get_object (builder, "spinner");
    activityLabel = (GtkLabel *)gtk_builder_get_object (builder, "activityLabel");
    
    started = 0;
    
    gtk_image_clear(logoImage);    
    gtk_label_set_text(versionLabel, string("Version " + string(BUILD_VERSION)).c_str());    
    
    g_signal_connect (startButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer setupBannerOverlayWidget) -> void {
        if(((SetupBannerOverlayWidget *)setupBannerOverlayWidget)->started)
        {
            return;
        }

        ((SetupBannerOverlayWidget *)setupBannerOverlayWidget)->started = 1;
        gtk_widget_hide(GTK_WIDGET(button));

        SetupDatabaseProcess *setupDatabaseProcess = new SetupDatabaseProcess();
        SerialProcessExecutor::getInstance()->schedule(setupDatabaseProcess);
    }), this);
    
    NotificationManager::getInstance()->registerToNotification(SetupDatabaseProcess::TYPE, this, onNotification, 1);
}

SetupBannerOverlayWidget::~SetupBannerOverlayWidget()
{
    NotificationManager::getInstance()->unregisterToNotification(SetupDatabaseProcess::TYPE, this);
}

void SetupBannerOverlayWidget::update()
{
    if(started)
    {
        gtk_widget_hide(GTK_WIDGET(startButton));
        gtk_widget_show_all(GTK_WIDGET(activityBox));
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(activityBox));
        gtk_widget_show(GTK_WIDGET(startButton));
    }
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(logoImage));
    int height = gtk_widget_get_allocated_width(GTK_WIDGET(logoImage));
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogo(), width, height);
}


void SetupBannerOverlayWidget::onNotification(Notification* notification)
{
    SetupBannerOverlayWidget *setupBannerOverlayWidget = (SetupBannerOverlayWidget *)notification->getListener();
    
    if(notification->getName().compare(SetupDatabaseProcess::TYPE) == 0)
    {
        if(notification->getStatus() == SerialProcess::STATUS_RUNNING)
        {
            gtk_widget_hide(GTK_WIDGET(setupBannerOverlayWidget->startButton));
            gtk_widget_show_all(GTK_WIDGET(setupBannerOverlayWidget->activityBox));
            gtk_spinner_start(setupBannerOverlayWidget->spinner);
            
            string message = notification->getMessage();
            if(notification->getProgress() > 0)
            {
                message += " (" + to_string(notification->getProgress()) + "%)";
            }            
            gtk_label_set_text(setupBannerOverlayWidget->activityLabel, message.c_str());
        }
        else
        {
            gtk_spinner_stop(setupBannerOverlayWidget->spinner);
            gtk_label_set_text(setupBannerOverlayWidget->activityLabel, "");
        }
    }
}

