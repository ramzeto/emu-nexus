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
 * File:   UiUtils.cpp
 * Author: ram
 * 
 * Created on April 8, 2016, 2:21 PM
 */

#include "UiUtils.h"
#include "Utils.h"
#include "HttpConnector.h"
#include "ThreadManager.h"

#include <iostream>
#include <string.h>
#include <glib/glist.h>

using namespace std;

UiUtils *UiUtils::instance = NULL;
const int UiUtils::DOWNLOADED_DONE = 0;
const int UiUtils::DOWNLOADED_PENDING = 1;
const int UiUtils::DOWNLOADED_FAILED = 2;
    
UiUtils::UiUtils()
{
    imageCache = new map<string, GdkPixbuf*>;
    downloadImages = new list<DownloadImage_t*>;
    downloadingImages = 0;
    downloadImagesMutex = PTHREAD_MUTEX_INITIALIZER;
}

UiUtils::~UiUtils()
{
    for (map<string, GdkPixbuf*>::iterator it = imageCache->begin(); it != imageCache->end(); it++)
    {
        g_object_unref(it->second);
    }
    imageCache->clear();
    
    for (list<DownloadImage_t*>::iterator downloadImage = downloadImages->begin(); downloadImage != downloadImages->end(); downloadImage++)
    {
        delete *downloadImage;
    }
    downloadImages->clear();
    
    delete imageCache;
    delete downloadImages;
}

GtkWidget *UiUtils::getWidget(GtkContainer *gtkContainer, string name)
{
    GList *children = gtk_container_get_children(gtkContainer);
    GtkWidget *gtkWidget = NULL;
    
    while (children)
    {
        GtkWidget *child = GTK_WIDGET (children->data);
        
        if(strcmp(gtk_widget_get_name(child), name.c_str()) == 0)
        {
            gtkWidget = child;
            break;
        }
        else
        {
            if(GTK_IS_CONTAINER(child))
            {
                gtkWidget = UiUtils::getWidget(GTK_CONTAINER(child), name.c_str());
                if(gtkWidget)
                {
                    break;
                }
            }
        }
        children = children->next;
    }
    
    g_list_free(children);
    return gtkWidget;
}

void UiUtils::clearContainer(GtkContainer *gtkContainer, int destroy)
{
    GList *widgets, *widget;
    widgets = gtk_container_get_children(GTK_CONTAINER(gtkContainer));

    for(widget = widgets; widget != NULL; widget = g_list_next(widget))
    {
        if(destroy)
        {
            gtk_widget_destroy(GTK_WIDGET(widget->data));
        }
        else
        {
            gtk_container_remove(GTK_CONTAINER(gtkContainer), GTK_WIDGET(widget->data));
        }                
    }
    g_list_free(widgets);
}

void UiUtils::removeListRow(GtkListBox* listBox, GtkWidget* row)
{
    GList *widgets, *widget;
    widgets = gtk_container_get_children(GTK_CONTAINER(listBox));

    for(widget = widgets; widget != NULL; widget = g_list_next(widget))
    {
        if(UiUtils::getWidget(GTK_CONTAINER(widget->data), gtk_widget_get_name(row)))
        {
            gtk_widget_destroy(GTK_WIDGET(widget->data));
            break;
        }
    }
    g_list_free(widgets);
}


int UiUtils::loadImage(GtkImage* image, string fileName, int width, int height, int aspectFill)
{
    //@TODO Clear cache at some point
    if(!Utils::getInstance()->fileExists(fileName))
    {
        return 0;
    }
    
    GdkPixbuf *pixBuf = NULL;
    string key;
    if(aspectFill)
    {
        key = fileName;
        
        if(imageCache->find(key) != imageCache->end())
        {
            pixBuf = imageCache->at(key);
        }
        else
        {
            pixBuf = gdk_pixbuf_new_from_file(fileName.c_str(), NULL);
            imageCache->insert(pair<string, GdkPixbuf *>(key, pixBuf));
        }

        width = gdk_pixbuf_get_width (pixBuf);
        height = gdk_pixbuf_get_height (pixBuf);
        
        gtk_widget_set_size_request(GTK_WIDGET(image), width, height);
        gtk_image_set_from_pixbuf(image, pixBuf);
    }
    else
    {
        key = fileName + "_" + to_string(width) + "_" + to_string(height);

        if(imageCache->find(key) != imageCache->end())
        {
            pixBuf = imageCache->at(key);
        }
        else
        {
            pixBuf = gdk_pixbuf_new_from_file_at_scale (fileName.c_str(), width, height, 1, NULL);
            imageCache->insert(pair<string, GdkPixbuf *>(key, pixBuf));
        }

        gtk_widget_set_size_request(GTK_WIDGET(image), width, height);
        gtk_image_set_from_pixbuf(image, pixBuf);
    }
    
    return 1;
}

void UiUtils::downloadImage(GtkImage* image, string url, string fileName, int width, int height, int aspectFill) 
{
    pthread_mutex_lock(&downloadImagesMutex);
    DownloadImage_t *downloadImage = new DownloadImage_t;
    downloadImage->image = image;
    downloadImage->url = url;
    downloadImage->fileName = fileName;
    downloadImage->width = width;
    downloadImage->height = height;
    downloadImage->aspectFill = aspectFill;
    downloadImage->downloaded = DOWNLOADED_PENDING;
    downloadImages->push_back(downloadImage);
    pthread_mutex_unlock(&downloadImagesMutex);
    
    if(!downloadingImages)
    {
        downloadingImages = 1;
        ThreadManager::getInstance()->execute(0, [this]() -> void {            
            while(1)
            {
                DownloadImage_t *downloadImage = NULL;
                pthread_mutex_lock(&downloadImagesMutex);
                for(list<DownloadImage_t *>::iterator aDownloadImage = downloadImages->begin(); aDownloadImage != downloadImages->end(); aDownloadImage++)
                {
                    if((*aDownloadImage)->downloaded == DOWNLOADED_PENDING)
                    {
                        downloadImage = *aDownloadImage;
                        break;
                    }
                }
                pthread_mutex_unlock(&downloadImagesMutex);
                
                if(!downloadImage)
                {
                    downloadingImages = 0;
                    break;
                }
                
                HttpConnector *httpConnector = new HttpConnector(downloadImage->url);
                httpConnector->get();            
                if(httpConnector->getHttpStatus() == HttpConnector::HTTP_OK)
                {
                    Utils::getInstance()->writeToFile(httpConnector->getResponseData(), httpConnector->getResponseDataSize(), downloadImage->fileName);
                    downloadImage->downloaded = DOWNLOADED_DONE;
                }
                else
                {
                    downloadImage->downloaded = DOWNLOADED_FAILED;
                }
                delete httpConnector;
                
                ThreadManager::getInstance()->execute(1, [this, downloadImage]() -> void {
                    pthread_mutex_lock(&downloadImagesMutex);
                    if(downloadImage->downloaded == DOWNLOADED_DONE && downloadImage->image)
                    {
                        UiUtils::getInstance()->loadImage(downloadImage->image, downloadImage->fileName, downloadImage->width, downloadImage->height, downloadImage->aspectFill);
                    }
                    downloadImages->remove(downloadImage);
                    delete downloadImage;
                    pthread_mutex_unlock(&downloadImagesMutex);
                });
            }                                                            
        });
    }    
}

void UiUtils::cancelDownloadImage(GtkImage *image) 
{
    pthread_mutex_lock(&downloadImagesMutex);
    for(list<DownloadImage_t *>::iterator downloadImage = downloadImages->begin(); downloadImage != downloadImages->end(); downloadImage++)
    {
        if((*downloadImage)->image == image)
        {
            (*downloadImage)->image = NULL;
        }        
    }
    pthread_mutex_unlock(&downloadImagesMutex);
}



UiUtils* UiUtils::getInstance()
{
    if(!instance)        
    {
        instance = new UiUtils();
    }
    
    return instance;
}
