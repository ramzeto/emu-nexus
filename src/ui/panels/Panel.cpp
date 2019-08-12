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
#include <iostream>

using namespace std;

Panel::Panel(string panelFileName, string panelBoxId) 
{
    builder = gtk_builder_new_from_file(string(Directory::getInstance()->getUiTemplatesDirectory() + panelFileName).c_str());
    panelBox = (GtkBox *)gtk_builder_get_object (builder, panelBoxId.c_str());
    closed = 0;
}

Panel::~Panel() 
{
}

GtkBox *Panel::getPanelBox()
{
    return panelBox;
}

string Panel::getTitle()
{
    return title;
}

void Panel::show()
{    
    gtk_widget_show_all(GTK_WIDGET(panelBox));
}

void Panel::close()
{    
    gtk_widget_hide_on_delete(GTK_WIDGET(panelBox));
    closed = 1;
}

