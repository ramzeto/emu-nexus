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
 * File:   Panel.h
 * Author: ram
 *
 * Created on August 2, 2016, 5:48 PM
 */

#ifndef PANEL_H
#define	PANEL_H

#include <gtk/gtk.h>
#include <string>

using namespace std;

/**
 * A wrapper for a GtkBox struct. This class is the base of the Panels showed in the MainWindow.
 */
class Panel
{
public:
    
    /**
     * @param parentWindow Parent GtkWindow.
     * @param panelFileName Filename of the panel design.
     * @param panelBoxId Id from the base box of the panel.
     */
    Panel(GtkWindow *parentWindow, string panelFileName, string panelBoxId);
    
    virtual ~Panel();
    
    /**
     * 
     * @return Base box of the panel.
     */
    GtkBox *getPanelBox();
        
    /**
     * Shows the panel. It calls gtk_widget_show_all.
     */
    virtual void show();
    
    
protected:
    GtkWindow *parentWindow;
    GtkBuilder *builder;
    GtkBox *panelBox;        

};

#endif	/* PANEL_H */

