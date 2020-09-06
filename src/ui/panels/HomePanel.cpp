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
 * File:   HomePanel.cpp
 * Author: ram
 * 
 * Created on May 6, 2019, 8:49 PM
 */

#include "HomePanel.h"
#include "Utils.h"
#include "UiUtils.h"
#include "Game.h"
#include "GameEditDialog.h"
#include "Directory.h"
#include "Asset.h"
#include "Build.h"
#include "Logger.h"
#include "MainBannerWidget.h"


HomePanel::HomePanel(GtkWindow *parentWindow) : Panel(parentWindow, "HomePanel.ui", "homeBox")
{
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    
    isShown = 0;
    panelWidth = 0;
    panelHeight = 0;
    
    //gtk_label_set_text(versionLabel, (string("Version ") + string(BUILD_VERSION)).c_str());
    gtk_label_set_markup(informationLabel, Utils::getInstance()->getFileContents(Asset::getInstance()->getHomePml()).c_str());
    
    g_signal_connect (getPanelBox(), "show", G_CALLBACK(signalShow), this);    
    g_signal_connect (getPanelBox(), "size-allocate", G_CALLBACK(signalSizeAllocate), this);  
}

HomePanel::~HomePanel()
{
}

void HomePanel::signalSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, gpointer homePanel)
{    
    if(((HomePanel *)homePanel)->isDestroyed())
    {
        return;
    }

    if(((HomePanel *)homePanel)->panelWidth != allocation->width || ((HomePanel *)homePanel)->panelHeight != allocation->height)
    {        
        ((HomePanel *)homePanel)->panelWidth = allocation->width;
        ((HomePanel *)homePanel)->panelHeight = allocation->height;
       
        ((HomePanel *)homePanel)->isShown = 0;
        signalShow(widget, homePanel);
    }
}

void HomePanel::signalShow(GtkWidget* widget, gpointer homePanel)
{
    if(!((HomePanel *)homePanel)->isShown)
    {
        // @TODO .- Change this horrible solution to force the list to show the first time.
        g_timeout_add(10, callbackFirstShowHackyTimeout, homePanel);
    }
}

gint HomePanel::callbackFirstShowHackyTimeout(gpointer homePanel)
{    
    ((HomePanel *)homePanel)->isShown = 1;
    MainBannerWidget::getInstance()->setBannerType(MainBannerWidget::TYPE_HOME);
    
    return 0;
}