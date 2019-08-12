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
 * File:   Widget.cpp
 * Author: ram
 * 
 * Created on April 9, 2019, 1:32 AM
 */

#include "Widget.h"
#include "Directory.h"

Widget::Widget(string uiFileName, string rootId)
{
    builder = gtk_builder_new_from_file(string(Directory::getInstance()->getUiTemplatesDirectory() + uiFileName).c_str());
    rootWidget = (GtkWidget *)gtk_builder_get_object(builder, rootId.c_str());
}

Widget::~Widget()
{
    if(rootWidget)
    {
        gtk_widget_destroy (rootWidget);
        rootWidget = NULL;
    }
    
    if(builder)
    {
        builder = NULL;
    }
}

GtkWidget* Widget::getRootWidget()
{
    return rootWidget;
}

