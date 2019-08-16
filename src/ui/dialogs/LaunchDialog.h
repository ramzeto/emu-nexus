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
 * File:   LaunchDialog.h
 * Author: ram
 *
 * Created on July 21, 2019, 11:28 AM
 */

#ifndef LAUNCHDIALOG_H
#define LAUNCHDIALOG_H

#include "Dialog.h"

#include <string>

using namespace std;

class LaunchDialog : public Dialog
{
public:
    LaunchDialog(int64_t gameId);
    virtual ~LaunchDialog();
    
    void setStatus(int activity, string message, int progress);
    
private:
    GtkSpinner *spinner;
    GtkLabel *messageLabel;
    GtkProgressBar *progressBar;
    GtkButton *closeButton;
    
    void close();
    
    static void signalCloseButtonClicked(GtkButton *button, gpointer pLaunchDialog);
    static gboolean signalDeleteEvent(GtkWidget *window, GdkEvent *event, gpointer pLaunchDialog);
    static gboolean signalKeyPressedEvent(GtkEntry *entry, GdkEvent *event, gpointer pLaunchDialog);
};

#endif /* LAUNCHDIALOG_H */

