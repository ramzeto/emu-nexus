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
 * File:   PublishersSelectDialog.cpp
 * Author: ram
 * 
 * Created on June 29, 2019, 1:05 PM
 */

#include "PublishersSelectDialog.h"
#include "UiUtils.h"
#include "Directory.h"

PublishersSelectDialog::PublishersSelectDialog(GtkWindow *parent, list<Publisher *> *selectedItems) : Dialog(parent, "SearchableMultiChoiceListDialog.ui", "searchableMultiChoiceListDialog")
{
    this->selectedItems = new list<Publisher *>;
    for(unsigned int c = 0; c < selectedItems->size(); c++)
    {
        Publisher *item = new Publisher(*Publisher::getItem(selectedItems, c));
        this->selectedItems->push_back(item);
    }
    
    items = NULL;
    page = 0;
    
    searchEntry = (GtkSearchEntry *)gtk_builder_get_object (builder, "searchEntry");        
    scrolledWindow = (GtkScrolledWindow *)gtk_builder_get_object (builder, "scrolledWindow");        
    listBox = (GtkListBox *)gtk_builder_get_object (builder, "listBox");             
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");    
    selectButton = (GtkButton *)gtk_builder_get_object(builder, "selectButton");
    
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Select publishers");
    
    g_signal_connect (searchEntry, "search-changed", G_CALLBACK (+[](GtkSearchEntry *searchEntry, gpointer dialog) -> void {
        ((PublishersSelectDialog *)dialog)->updateItems();
    }), this);
    
    g_signal_connect (scrolledWindow, "edge-reached", G_CALLBACK(+[](GtkScrolledWindow *scrolledWindow, GtkPositionType positionType, gpointer dialog) -> void {
        ((PublishersSelectDialog *)dialog)->page++;
        ((PublishersSelectDialog *)dialog)->loadList();
    }), this);
    
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer dialog) -> void {
        ((PublishersSelectDialog *)dialog)->cancel();
    }), this);
    
    g_signal_connect (selectButton, "clicked", G_CALLBACK (+[](GtkButton *button, gpointer dialog) -> void {
        ((PublishersSelectDialog *)dialog)->select();
    }), this);
    
    updateItems();
}

PublishersSelectDialog::~PublishersSelectDialog()
{
    Publisher::releaseItems(selectedItems);
    if(items)
    {
        Publisher::releaseItems(items);
    }
}

void PublishersSelectDialog::updateItems()
{    
    if(items)
    {
        Publisher::releaseItems(items);
    }
    
    items = Publisher::getItems(string(gtk_entry_get_text(GTK_ENTRY(searchEntry))));
    
    page = 0;
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(listBox), 1);
    loadList();
}

void PublishersSelectDialog::loadList()
{   
    unsigned int startIndex = page * ITEMS_PER_PAGE;
    for(unsigned int index = startIndex; index < startIndex + ITEMS_PER_PAGE; index++)
    {
        if(index >= items->size())
        {
            break;
        }
        
        Publisher *item = Publisher::getItem(items, index);
        
        GtkBuilder *rowBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "CheckRowBox.ui").c_str());
        GtkWidget *checkRowBox = (GtkWidget *)gtk_builder_get_object (rowBuilder, "checkRowBox");
        GtkCheckButton *checkButton = (GtkCheckButton *)gtk_builder_get_object (rowBuilder, "checkButton");
        GtkLabel *label = (GtkLabel *)gtk_builder_get_object (rowBuilder, "label");
        
        gtk_label_set_text(label, item->getName().c_str());               
        gtk_widget_set_name(GTK_WIDGET(checkButton), to_string(item->getId()).c_str());
        
        for(unsigned int c = 0; c < selectedItems->size(); c++)
        {
            Publisher *selectedItem = Publisher::getItem(selectedItems, c);
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

void PublishersSelectDialog::setItemActive(int64_t itemId, int active)
{
    if(active)
    {
        Publisher *item = new Publisher(itemId);
        item->load();
        
        selectedItems->push_back(item);
    }
    else
    {
        for(unsigned int c = 0; c < selectedItems->size(); c++)
        {
            Publisher *selectedItem = Publisher::getItem(selectedItems, c);
            if(selectedItem->getId() == itemId)
            {
                selectedItems->remove(selectedItem);
                delete selectedItem;
                break;
            }
        }
    }
}

void PublishersSelectDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void PublishersSelectDialog::select()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}

list<Publisher*>* PublishersSelectDialog::getSelectedItems()
{
    return selectedItems;
}


void PublishersSelectDialog::signalToggled(GtkToggleButton *togglebutton, gpointer dialog)
{
    int64_t itemId = atol(gtk_widget_get_name(GTK_WIDGET(togglebutton)));
    ((PublishersSelectDialog *) dialog)->setItemActive(itemId, gtk_toggle_button_get_active(togglebutton));
}
