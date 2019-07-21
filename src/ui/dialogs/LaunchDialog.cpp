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
 * File:   LaunchDialog.cpp
 * Author: ram
 * 
 * Created on July 21, 2019, 11:28 AM
 */

#include "LaunchDialog.h"
#include "Game.h"
#include "Database.h"

#include <iostream>

using namespace std;

LaunchDialog::LaunchDialog(int64_t gameId) : Dialog("LaunchDialog.ui", "launchDialog")
{
    spinner = (GtkSpinner *)gtk_builder_get_object(builder, "spinner");

    messageLabel = (GtkLabel *)gtk_builder_get_object(builder, "messageLabel");
    gtk_label_set_text(messageLabel, "");
    
    closeButton = (GtkButton *)gtk_builder_get_object(builder, "closeButton");
    g_signal_connect (closeButton, "clicked", G_CALLBACK (signalCloseButtonClicked), this);
    gtk_widget_hide(GTK_WIDGET(closeButton));
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    Game *game = new Game(gameId);
    game->load(sqlite);
    Database::getInstance()->release();
    
    gtk_window_set_title(GTK_WINDOW(dialog), game->getName().c_str());
    
    g_signal_connect(dialog, "delete-event", G_CALLBACK(signalDeleteEvent), this);
    g_signal_connect(dialog, "key-press-event", G_CALLBACK (signalKeyPressedEvent), this);
}

LaunchDialog::~LaunchDialog()
{
}

void LaunchDialog::setStatus(int activity, string message)
{
    if(activity)
    {
        gtk_spinner_start(spinner);
        gtk_widget_hide(GTK_WIDGET(closeButton));
    }
    else
    {
        gtk_spinner_stop(spinner);
        gtk_widget_show(GTK_WIDGET(closeButton));
    }
    
    gtk_label_set_text(messageLabel, message.c_str());    
}

void LaunchDialog::close()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE);
}

void LaunchDialog::signalCloseButtonClicked(GtkButton* button, gpointer pLaunchDialog)
{
    LaunchDialog *launchDialog = (LaunchDialog *)pLaunchDialog;
    launchDialog->close();        
}

gboolean LaunchDialog::signalDeleteEvent(GtkWidget* window, GdkEvent* event, gpointer data)
{
    cout << "LaunchDialog::" << __FUNCTION__ << endl;
    return 1;
}

gboolean LaunchDialog::signalKeyPressedEvent(GtkEntry* entry, GdkEvent* event, gpointer pLaunchDialog)
{
    cout << "LaunchDialog::" << __FUNCTION__ << endl;
    return 1;
}
