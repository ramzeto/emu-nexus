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
 * File:   Dialog.h
 * Author: ram
 *
 * Created on August 19, 2016, 2:35 AM
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <gtk/gtk.h>
#include <list>
#include <string>

using namespace std;

/**
 * Is a wrapper of a GtkDialog struct.
 */
class Dialog
{
public:
    
    /**
     *
     * @param parent Parent GtkWindow.
     * @param uiFileName Filename of the dialog design.
     * @param dialogId Id from the base widget of the dialog.
     */
    Dialog(GtkWindow *parent, string uiFileName, string dialogId);
    virtual ~Dialog();
    
    /**
     * Executes gtk_dialog_run
     * @return Result of the dialog. The dialog should call gtk_dialog_response.
     */
    virtual int execute();
    
    /**
     * Destroys the dialog calling gtk_widget_destroy.
     */
    virtual void dismiss();
    
protected:
    GtkBuilder *builder;
    GtkDialog *dialog;
    int dismissed;
};

#endif /* DIALOG_H */

