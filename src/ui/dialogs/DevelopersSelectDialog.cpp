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
 * File:   DevelopersSelectDialog.cpp
 * Author: ram
 * 
 * Created on June 28, 2019, 11:03 PM
 */

#include "DevelopersSelectDialog.h"
#include "Database.h"
#include "UiUtils.h"
#include "Directory.h"

#include <iostream>

DevelopersSelectDialog::DevelopersSelectDialog(list<Developer *> *selectedItems) : Dialog("SearchableMultiChoiceListDialog.ui", "searchableMultiChoiceListDialog")
{
    this->selectedItems = new list<Developer *>;
    for(unsigned int c = 0; c < selectedItems->size(); c++)
    {
        Developer *item = new Developer(*Developer::getItem(selectedItems, c));
        this->selectedItems->push_back(item);
    }
    
    items = NULL;
    page = 0;
    
    searchEntry = (GtkSearchEntry *)gtk_builder_get_object (builder, "searchEntry");
    g_signal_connect (searchEntry, "search-changed", G_CALLBACK (signalSearchEntrySearchChanged), this);
    
    scrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "scrolledWindow");
    g_signal_connect (scrolledWindow, "edge-reached", G_CALLBACK(signalScrolledWindowEdgeReached), this);
    
    listBox = (GtkListBox *)gtk_builder_get_object (builder, "listBox");
             
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (signalCancelButtonClicked), this);

    selectButton = (GtkButton *)gtk_builder_get_object(builder, "selectButton");
    g_signal_connect (selectButton, "clicked", G_CALLBACK (signalSelectButtonClicked), this);
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Select developers");
    
    updateItems();
}

DevelopersSelectDialog::~DevelopersSelectDialog()
{
    Developer::releaseItems(selectedItems);
    if(items)
    {
        Developer::releaseItems(items);
    }
}

void DevelopersSelectDialog::updateItems()
{    
    if(items)
    {
        Developer::releaseItems(items);
    }
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    items = Developer::getItems(sqlite, string(gtk_entry_get_text(GTK_ENTRY(searchEntry))));
    Database::getInstance()->release();
    
    page = 0;
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(listBox), 1);
    loadList();
}

void DevelopersSelectDialog::loadList()
{   
    unsigned int startIndex = page * ITEMS_PER_PAGE;
    for(unsigned int index = startIndex; index < startIndex + ITEMS_PER_PAGE; index++)
    {
        if(index >= items->size())
        {
            break;
        }
        
        Developer *item = Developer::getItem(items, index);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "CheckRowBox.ui").c_str());
        GtkWidget *checkRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "checkRowBox");
        GtkCheckButton *checkButton = (GtkCheckButton *)gtk_builder_get_object (rowBuilder, "checkButton");
        GtkLabel *label = (GtkLabel *)gtk_builder_get_object (rowBuilder, "label");
        
        gtk_label_set_text(label, item->getName().c_str());               
        gtk_widget_set_name(GTK_WIDGET(checkButton), to_string(item->getId()).c_str());
        
        for(unsigned int c = 0; c < selectedItems->size(); c++)
        {
            Developer *selectedItem = Developer::getItem(selectedItems, c);
            if(selectedItem->getId() == item->getId())
            {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton), 1);
                break;
            }
        }        
        g_signal_connect (checkButton, "toggled", G_CALLBACK (signalToggled), this);
        
        gtk_container_add(GTK_CONTAINER(listBox), GTK_WIDGET(checkRowBox));
        gtk_widget_show_all(GTK_WIDGET(checkRowBox));
    }    
}

void DevelopersSelectDialog::setItemActive(int64_t itemId, int active)
{
    if(active)
    {
        Developer *item = new Developer(itemId);
        sqlite3 *sqlite = Database::getInstance()->acquire();
        item->load(sqlite);
        Database::getInstance()->release();
        
        selectedItems->push_back(item);
    }
    else
    {
        for(unsigned int c = 0; c < selectedItems->size(); c++)
        {
            Developer *selectedItem = Developer::getItem(selectedItems, c);
            if(selectedItem->getId() == itemId)
            {
                selectedItems->remove(selectedItem);
                delete selectedItem;
                break;
            }
        }
    }
}

void DevelopersSelectDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void DevelopersSelectDialog::select()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}

list<Developer*>* DevelopersSelectDialog::getSelectedItems()
{
    return selectedItems;
}


void DevelopersSelectDialog::signalSearchEntrySearchChanged(GtkSearchEntry *searchEntry, gpointer dialog)
{
    ((DevelopersSelectDialog *)dialog)->updateItems();
}

void DevelopersSelectDialog::signalScrolledWindowEdgeReached(GtkScrolledWindow *scrolledWindow, GtkPositionType positionType, gpointer dialog)
{
    ((DevelopersSelectDialog *)dialog)->page++;
    ((DevelopersSelectDialog *)dialog)->loadList();
}

void DevelopersSelectDialog::signalToggled(GtkToggleButton *togglebutton, gpointer dialog)
{
    int64_t itemId = atol(gtk_widget_get_name(GTK_WIDGET(togglebutton)));
    ((DevelopersSelectDialog *) dialog)->setItemActive(itemId, gtk_toggle_button_get_active(togglebutton));
}

void DevelopersSelectDialog::signalCancelButtonClicked(GtkButton *button, gpointer dialog)
{
    ((DevelopersSelectDialog *)dialog)->cancel();
}

void DevelopersSelectDialog::signalSelectButtonClicked(GtkButton *button, gpointer dialog)
{
    ((DevelopersSelectDialog *)dialog)->select();
}

