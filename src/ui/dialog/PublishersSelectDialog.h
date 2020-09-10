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
 * File:   PublishersSelectDialog.h
 * Author: ram
 *
 * Created on June 29, 2019, 1:05 PM
 */

#ifndef PUBLISHERSSELECTDIALOG_H
#define PUBLISHERSSELECTDIALOG_H

#include "Dialog.h"
#include "Publisher.h"

#include <list>

using namespace std;

/**
 * A dialog to choose one or more publishers. The dialog will call gtk_dialog_response(..., GTK_RESPONSE_ACCEPT) if accepted.
 */
class PublishersSelectDialog : public Dialog
{
public:
    
    /**
     * 
     * @param parent Parent GtkWindow.
     * @param selectedItems List of selected items. For safety, this objects will be copied into a new list.
     */     
    PublishersSelectDialog(GtkWindow *parent, list<Publisher *> *selectedItems);
    virtual ~PublishersSelectDialog();
    
    /**
     * 
     * @return List of selected items. For safety, this objects should be copied. This list and its objects are freed in the destructor.
     */    
    list<Publisher *> *getSelectedItems();
    
private:
    static const int ITEMS_PER_PAGE = 50;
    
    GtkSearchEntry *searchEntry;
    GtkScrolledWindow *scrolledWindow;
    GtkListBox *listBox;
    GtkButton *cancelButton;
    GtkButton *selectButton;
    
    list<Publisher *> *selectedItems;
    list<Publisher *> *items;
    unsigned int page;
    
    void updateItems();
    void loadList();
    void setItemActive(int64_t itemId, int active);
    void cancel();
    void select();
    
    static void signalToggled(GtkToggleButton *togglebutton, gpointer dialog);    
};

#endif /* PUBLISHERSSELECTDIALOG_H */

