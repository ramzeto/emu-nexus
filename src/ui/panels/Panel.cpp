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
 * File:   Panel.cpp
 * Author: ram
 * 
 * Created on August 2, 2016, 5:48 PM
 */

#include "Panel.h"
#include "Directory.h"

using namespace std;

Panel::Panel(GtkWindow *parentWindow, string panelFileName, string panelBoxId) 
{
    this->parentWindow = parentWindow;
    
    builder = gtk_builder_new_from_file(string(Directory::getInstance()->getUiTemplatesDirectory() + panelFileName).c_str());
    panelBox = (GtkBox *)gtk_builder_get_object (builder, panelBoxId.c_str());
    destroyed = 0;
    
    g_signal_connect (panelBox, "destroy", G_CALLBACK(signalDestroy), this);
}

Panel::~Panel() 
{
}

GtkBox *Panel::getPanelBox()
{
    return panelBox;
}

void Panel::show()
{    
    gtk_widget_show_all(GTK_WIDGET(panelBox));
}

void Panel::destroy()
{
    destroyed = 1;
    
    gtk_widget_destroy(GTK_WIDGET(panelBox));
}

int Panel::isDestroyed()
{
    return destroyed;
}

void Panel::signalDestroy(GtkWidget* widget, gpointer panel)
{
    //delete ((Panel *)panel);
    
    // @TODO .- Change this horrible solution to delete the Panel object. If deleting immediately, random crashes occur.
    g_timeout_add(1000, callbackDeleteTimeout, panel);
}

gint Panel::callbackDeleteTimeout(gpointer panel)
{    
    delete ((Panel *)panel);
    
    return 0;
}
