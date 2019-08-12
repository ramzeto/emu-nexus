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
 * File:   Dialog.cpp
 * Author: ram
 * 
 * Created on August 19, 2016, 2:35 AM
 */

#include "Dialog.h"
#include "Directory.h"

Dialog::Dialog(string uiFileName, string dialogId)
{
    dismissed = 0;
    builder = gtk_builder_new_from_file(string(Directory::getInstance()->getUiTemplatesDirectory() + uiFileName).c_str());
    dialog = (GtkWidget *)gtk_builder_get_object(builder, dialogId.c_str());

    gtk_window_set_resizable(GTK_WINDOW(dialog), 0);
}

Dialog::~Dialog()
{
    if(dialog)
    {
        gtk_widget_destroy (dialog);
        dialog = NULL;
    }
    
    if(builder)
    {
        builder = NULL;
    }
}

int Dialog::execute()
{
    return gtk_dialog_run (GTK_DIALOG (dialog));
}

void Dialog::dismiss()
{
    dismissed = 1;
    gtk_widget_destroy (dialog);
    dialog = NULL;
}

