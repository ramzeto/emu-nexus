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
 * File:   MainBannerOverlayWidget.cpp
 * Author: ram
 * 
 * Created on September 9, 2020, 7:06 PM
 */

#include "MainBannerOverlayWidget.h"

MainBannerOverlayWidget::MainBannerOverlayWidget(string widgetFileName, string widgetId) : Widget(widgetFileName, widgetId)
{
    bannerWidth = 0;
    bannerHeight = 0;
    updateHandlerId = 0;
    
    signalSizeAllocateHandlerId = g_signal_connect (widget, "size-allocate", G_CALLBACK(+[](GtkWidget* widget, GtkAllocation* allocation, gpointer mainBannerOverlayWidget) -> void {
        if(((MainBannerOverlayWidget *)mainBannerOverlayWidget)->bannerWidth != allocation->width || ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->bannerHeight != allocation->height)
        {        
            ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->bannerWidth = allocation->width;
            ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->bannerHeight = allocation->height;

            // Hacky reload
            if(((MainBannerOverlayWidget *)mainBannerOverlayWidget)->updateHandlerId) // If it is a valid id, then there is a call queued
            {
                return;
            }
            ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->updateHandlerId = g_timeout_add(10, [](gpointer mainBannerOverlayWidget) -> gint {
                ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->update();
                
                ((MainBannerOverlayWidget *)mainBannerOverlayWidget)->updateHandlerId = 0;
                return 0;
            }, mainBannerOverlayWidget);
        }
    }), this);
}

MainBannerOverlayWidget::~MainBannerOverlayWidget()
{
    g_signal_handler_disconnect(widget, signalSizeAllocateHandlerId);
    
    if(updateHandlerId)
    {
        g_source_remove(updateHandlerId);
    }    
}

void MainBannerOverlayWidget::update()
{

}
