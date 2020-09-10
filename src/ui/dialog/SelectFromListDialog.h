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
 * File:   SelectFromListDialog.h
 * Author: ram
 *
 * Created on May 2, 2020, 11:04 AM
 */

#ifndef SELECTFROMLISTDIALOG_H
#define SELECTFROMLISTDIALOG_H

#include "Dialog.h"
#include <string>
#include <list>

using namespace std;

class SelectFromListDialog : public Dialog
{    
public:
    SelectFromListDialog(GtkWindow *parent, string title, list<string> items);
    virtual ~SelectFromListDialog();
    
    int getSelectedIndex();
    string getSelectedItem();
    
private:
    GtkLabel *noResultsLabel;
    GtkListBox *listBox;
    GtkSpinner *spinner;
    GtkButton *closeButton;
   
    list<string> items;
    int selectedIndex;
    string selectedItem;
    
    void select(int index);
    void updateList();
    void cancel();    
};

#endif /* SELECTFROMLISTDIALOG_H */

