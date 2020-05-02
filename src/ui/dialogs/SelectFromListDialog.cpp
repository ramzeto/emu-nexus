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
 * File:   SelectFromListDialog.cpp
 * Author: ram
 * 
 * Created on May 2, 2020, 11:04 AM
 */

#include "SelectFromListDialog.h"

SelectFromListDialog::SelectFromListDialog(GtkWindow *parent, string title, list<string> items) : Dialog(parent, "SingleChoiceListDialog.ui", "singleChoiceListDialog")
{
    this->items = items;
    
    noResultsLabel = (GtkLabel *)gtk_builder_get_object (builder, "noResultsLabel");
    gtk_widget_hide(GTK_WIDGET(noResultsLabel));
    
    listBox = (GtkListBox *)gtk_builder_get_object (builder, "listBox");
    g_signal_connect (listBox, "row-selected", G_CALLBACK(signalListRowSelected), this);
    
    spinner = (GtkSpinner *)gtk_builder_get_object (builder, "spinner");
    gtk_spinner_stop(spinner);
             
    closeButton = (GtkButton *)gtk_builder_get_object(builder, "closeButton");
    g_signal_connect (closeButton, "clicked", G_CALLBACK (signalCloseButtonClicked), this);
    gtk_button_set_label(closeButton, "Cancel");
    
    gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
    
    for(list<string>::iterator item = items.begin(); item != items.end(); item++)
    {
        GtkLabel *label = (GtkLabel *)gtk_label_new((*item).c_str());
        gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_END);
        gtk_label_set_line_wrap(label, 1);
        gtk_label_set_line_wrap_mode(label, PANGO_WRAP_WORD_CHAR);
        gtk_label_set_lines(label, 3);
        
        gtk_widget_set_margin_start(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_end(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_top(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 10);
        gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                
        gtk_container_add(GTK_CONTAINER(listBox), GTK_WIDGET(label));
        gtk_widget_show_all(GTK_WIDGET(label));
    }
}

SelectFromListDialog::~SelectFromListDialog()
{
}

int SelectFromListDialog::getSelectedIndex()
{
    return selectedIndex;
}

string SelectFromListDialog::getSelectedItem()
{
    return selectedItem;
}

void SelectFromListDialog::select(int index)
{
    selectedIndex = index;
    
    list<string>::iterator item = items.begin();
    advance(item, selectedIndex);
    selectedItem = *item;
    
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}

void SelectFromListDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void SelectFromListDialog::signalCloseButtonClicked(GtkButton *button, gpointer selectFromListDialog)
{
    ((SelectFromListDialog *)selectFromListDialog)->cancel();
}


void SelectFromListDialog::signalListRowSelected (GtkListBox *listBox, GtkWidget *row, gpointer selectFromListDialog)
{
    if(!selectFromListDialog || !row || !GTK_IS_LIST_BOX_ROW(row))
    {
        return;
    }
        
    int position = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW(row));
    ((SelectFromListDialog *)selectFromListDialog)->select(position);
}
