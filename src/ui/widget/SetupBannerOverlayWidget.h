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
 * File:   SetupBannerOverlayWidget.h
 * Author: ram
 *
 * Created on September 6, 2020, 3:01 PM
 */

#ifndef SETUPBANNERWIDGET_H
#define SETUPBANNERWIDGET_H

#include "MainBannerOverlayWidget.h"
#include "Notification.h"

class SetupBannerOverlayWidget : public MainBannerOverlayWidget
{
public:
    SetupBannerOverlayWidget();
    virtual ~SetupBannerOverlayWidget();

private:
    GtkImage *logoImage;
    GtkLabel *versionLabel;
    GtkButton *startButton;
    GtkBox *activityBox;    
    GtkSpinner *spinner;
    GtkLabel *activityLabel;
    
    int started;
    
    void update() override;    
    
    /**
     * 
     * @param notification
     */
    static void onNotification(Notification *notification);    
};

#endif /* SETUPBANNERWIDGET_H */

