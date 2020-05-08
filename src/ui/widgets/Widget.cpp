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
 * File:   Widget.cpp
 * Author: ram
 * 
 * Created on May 7, 2020, 9:12 PM
 */

#include "Widget.h"
#include "Directory.h"

Widget::Widget(string widgetFileName, string widgetId)
{
    builder = gtk_builder_new_from_file(string(Directory::getInstance()->getUiTemplatesDirectory() + widgetFileName).c_str());
    widget = (GtkWidget *)gtk_builder_get_object (builder, widgetId.c_str());
}

Widget::~Widget()
{
}

GtkWidget* Widget::getWidget()
{
    return widget;
}
