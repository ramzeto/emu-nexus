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
#include "glib/glist.h"
#include "Utils.h"

#include <iostream>
#include <string.h>

using namespace std;

UiUtils *UiUtils::instance = NULL;

UiUtils::UiUtils()
{
    imageCache = new map<string, GdkPixbuf*>;
}

UiUtils::~UiUtils()
{
    for (map<string, GdkPixbuf*>::iterator it = imageCache->begin(); it != imageCache->end(); it++)
    {
        g_object_unref(it->second);
    }
    imageCache->clear();
    
    delete imageCache;
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


void UiUtils::loadImage(GtkImage* image, string fileName, int width, int height, int aspectFill)
{
    //@TODO Clear cache at some point
    
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
            if(Utils::getInstance()->fileExists(fileName))
            {
                pixBuf = gdk_pixbuf_new_from_file(fileName.c_str(), NULL);
                imageCache->insert(pair<string, GdkPixbuf *>(key, pixBuf));
            }
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
            if(Utils::getInstance()->fileExists(fileName))
            {
                pixBuf = gdk_pixbuf_new_from_file_at_scale (fileName.c_str(), width, height, 1, NULL);
                imageCache->insert(pair<string, GdkPixbuf *>(key, pixBuf));
            }
        }

        gtk_widget_set_size_request(GTK_WIDGET(image), width, height);
        gtk_image_set_from_pixbuf(image, pixBuf);
    }
}

UiUtils* UiUtils::getInstance()
{
    if(!instance)        
    {
        instance = new UiUtils();
    }
    
    return instance;
}
