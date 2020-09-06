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
 * File:   UiUtils.h
 * Author: ram
 *
 * Created on April 8, 2016, 2:21 PM
 */

#ifndef UIUTILS_H
#define	UIUTILS_H

#include <gtk/gtk.h>
#include <string>
#include <map>

using namespace std;

/**
 * Class containing convenience methods to modify de UI. 
 */
class UiUtils {
public:

    /**
     * Gets a widget with a specific name from a container
     * @param gtkContainer Container that contains the desired widget
     * @param name Name of the widget
     * @return widget with name or NULL if not found
     */
    GtkWidget *getWidget(GtkContainer *gtkContainer, string name);
    
    /**
     * Clears a container
     * @param gtkContainer Container to clear
     * @param destroy Flag that states if the widgets from the container should be destroyed
     */
    void clearContainer(GtkContainer *gtkContainer, int destroy);
    
    /**
     * Removes a row from a list box
     * @param listBox
     * @param row
     */
    void removeListRow(GtkListBox *listBox, GtkWidget *row);
    
    /**
     * Loads an image from a file into a gtkimage control, scaling to width x height respecting the aspect ratio
     * @param image Image control
     * @param fileName Image filename (absolute path) to load
     * @param width Desired width
     * @param height Desired height
     */
    void loadImage(GtkImage *image, string fileName, int width, int height);        
    
    /**
     * 
     * @return UiUtils instance
     */
    static UiUtils *getInstance();
    
private:
    UiUtils();
    virtual ~UiUtils();
    
    map<string, GdkPixbuf*> *imageCache;

    static UiUtils *instance;
};

#endif	/* UIUTILS_H */
