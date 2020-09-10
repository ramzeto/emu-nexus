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
 * File:   WelcomeBannerOverlayWidget.cpp
 * Author: ram
 * 
 * Created on September 9, 2020, 8:17 PM
 */

#include "WelcomeBannerOverlayWidget.h"
#include "Asset.h"
#include "UiUtils.h"
#include "Build.h"

WelcomeBannerOverlayWidget::WelcomeBannerOverlayWidget() : MainBannerOverlayWidget("WelcomeBannerOverlayWidget.ui", "welcomeBannerBox")
{    
    logoImage = (GtkImage *)gtk_builder_get_object (builder, "logoImage");
    versionLabel = (GtkLabel *)gtk_builder_get_object (builder, "versionLabel");
    
    gtk_image_clear(logoImage);
    gtk_label_set_text(versionLabel, string("Version " + string(BUILD_VERSION)).c_str());     
}

WelcomeBannerOverlayWidget::~WelcomeBannerOverlayWidget()
{
}

void WelcomeBannerOverlayWidget::update()
{
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(logoImage));
    int height = gtk_widget_get_allocated_width(GTK_WIDGET(logoImage));
    UiUtils::getInstance()->loadImage(logoImage, Asset::getInstance()->getImageLogo(), width, height);
}
