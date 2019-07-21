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
 * File:   GenresSelectDialog.h
 * Author: ram
 *
 * Created on June 18, 2019, 11:18 PM
 */

#ifndef GENRESDIALOG_H
#define GENRESDIALOG_H

#include "Dialog.h"
#include "Genre.h"

#include <list>

using namespace std;

/**
 * A dialog to choose one or more genres. The dialog will call gtk_dialog_response(..., GTK_RESPONSE_ACCEPT) if accepted.
 */
class GenresSelectDialog  : public Dialog
{
public:
    
    /**
     * 
     * @param selectedItems List of selected items. For safety, this objects will be copied into a new list.
     */    
    GenresSelectDialog(list<Genre *> *selectedItems);
    virtual ~GenresSelectDialog();
    
    /**
     * 
     * @return List of selected items. For safety, this objects should be copied. This list and its objects are freed in the destructor.
     */    
    list<Genre *> *getSelectedItems();
        
private:
    static const int ITEMS_PER_PAGE = 50;
    
    GtkSearchEntry *searchEntry;
    GtkScrolledWindow *scrolledWindow;
    GtkListBox *listBox;
    GtkButton *cancelButton;
    GtkButton *selectButton;
    
    list<Genre *> *selectedItems;
    list<Genre *> *items;
    unsigned int page;
    
    void updateItems();
    void loadList();
    void setItemActive(int64_t itemId, int active);
    void cancel();
    void select();
    

    
    static void signalSearchEntrySearchChanged(GtkSearchEntry *searchEntry, gpointer dialog);    
    static void signalScrolledWindowEdgeReached(GtkScrolledWindow *scrolledWindow, GtkPositionType positionType, gpointer dialog);
    static void signalToggled(GtkToggleButton *togglebutton, gpointer dialog);    
    static void signalCancelButtonClicked(GtkButton *button, gpointer dialog);
    static void signalSelectButtonClicked(GtkButton *button, gpointer dialog);
};

#endif /* GENRESDIALOG_H */

