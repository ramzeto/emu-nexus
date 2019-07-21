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
 * File:   Widget.h
 * Author: ram
 *
 * Created on April 9, 2019, 1:32 AM
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <gtk/gtk.h>
#include <string>

using namespace std;

/**
 * This class is just a wrapper of a GtkWidget struct
 */

class Widget
{
public:
    
    /**
     * 
     * @param uiFileName Filename of the widget design.
     * @param rootId Id of the root widget.
     */
    Widget(string uiFileName, string rootId);
    virtual ~Widget();
    
    /**
     * 
     * @return root widget that this object wraps.
     */
    GtkWidget *getRootWidget();
    
protected:
    GtkBuilder *builder;
    GtkWidget *rootWidget;
};

#endif /* WIDGET_H */

