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
 * File:   MainBannerOverlayWidget.h
 * Author: ram
 *
 * Created on September 9, 2020, 7:06 PM
 */

#ifndef MAINBANNEROVERLAYWIDGET_H
#define MAINBANNEROVERLAYWIDGET_H

#include "Widget.h"

class MainBannerOverlayWidget : public Widget
{
public:
    MainBannerOverlayWidget(string widgetFileName, string widgetId);
    virtual ~MainBannerOverlayWidget();
    
protected:
    
    int bannerWidth;
    int bannerHeight;    
    gulong signalSizeAllocateHandlerId;
    
    virtual void update();
};

#endif /* MAINBANNEROVERLAYWIDGET_H */

