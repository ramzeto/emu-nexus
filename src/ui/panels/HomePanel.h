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
 * File:   HomePanel.h
 * Author: ram
 *
 * Created on May 6, 2019, 8:49 PM
 */

#ifndef HOMEPANEL_H
#define HOMEPANEL_H

#include "Panel.h"
#include "GameActivity.h"
#include "GameGridItemWidget.h"
#include "UiThreadBridge.h"
#include "CallbackResult.h"

#include <list>
#include <map>

using namespace std;

class HomePanel : public Panel
{
public:
    HomePanel(GtkWindow *parentWindow);    
    virtual ~HomePanel();
    
private:    
    GtkLabel *informationLabel;
    
    int isShown;
    int panelWidth;
    int panelHeight;
    
    
    /**
     * Signal triggered when the "size-allocate" event happens.
     * @param widget
     * @param allocation
     * @param homePanel
     */
    static void signalSizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer homePanel);
    
    /**
     * 
     * @param widget
     * @param homePanel
     */
    static void signalShow(GtkWidget *widget, gpointer homePanel);
    
    
    /**
     * Callback that gets fired when the horrible hacky timer triggers to force the first draw of the widget.
     * @param homePanel
     * @return 
     */
    static gint callbackFirstShowHackyTimeout(gpointer homePanel);    
};

#endif /* HOMEPANEL_H */

