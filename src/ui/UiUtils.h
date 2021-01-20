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
#include <list>
#include <pthread.h>

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
     * Loads an image from a file into a gtkimage control, scaling to width x height respecting the aspect ratio.
     * @param image Image widget.
     * @param fileName Image filename (absolute path) to load.
     * @param width Desired width.
     * @param height Desired height.
     * @param aspectFill When > 0, stretches the image to width x height respecting the aspect ratio.
     * @return 1 on success, 0 if fails (usually because the file does not exists)
     */
    int loadImage(GtkImage *image, string fileName, int width, int height, int aspectFill = 0);
    
    /**
     * Queues an image to download. When finished, it will be saved to the file named fileName and it will be loaded into the GtkImage parameter with the size parameters.
     * @param image
     * @param url
     * @param fileName
     * @param width
     * @param height
     * @param aspectFill
     */
    void downloadImage(GtkImage *image, string url, string fileName, int width, int height, int aspectFill = 0);
    
    /**
     * Cancels the loading of a downloaded image into a GtkImage previously passed as parameter to the downloadImage method.
     * @param image
     */
    void cancelDownloadImage(GtkImage *image);
    
    /**
     * 
     * @return UiUtils instance
     */
    static UiUtils *getInstance();
    
private:
    UiUtils();
    virtual ~UiUtils();
    
    map<string, GdkPixbuf*> *imageCache;
    
    static const int DOWNLOADED_DONE;
    static const int DOWNLOADED_PENDING;
    static const int DOWNLOADED_FAILED;
    typedef struct{
        GtkImage *image;
        string url;
        string fileName;
        int width;
        int height;
        int aspectFill;
        int downloaded;
    }DownloadImage_t;    
    list<DownloadImage_t*> *downloadImages;
    int downloadingImages;
    pthread_mutex_t downloadImagesMutex;

    static UiUtils *instance;
};

#endif	/* UIUTILS_H */
