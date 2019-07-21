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
 * File:   MessageDialog.cpp
 * Author: ram
 * 
 * Created on March 8, 2019, 8:13 PM
 */

#include "MessageDialog.h"

MessageDialog::MessageDialog(string message, string positiveButtonText, string negativeButtonText) : Dialog("MessageDialog.ui", "messageDialog")
{
    messageLabel = (GtkLabel *)gtk_builder_get_object(builder, "messageLabel");
    gtk_label_set_text(messageLabel, message.c_str());
    
    positiveButton = (GtkButton *)gtk_builder_get_object(builder, "positiveButton");
    gtk_button_set_label(positiveButton, positiveButtonText.c_str());
    g_signal_connect (positiveButton, "clicked", G_CALLBACK (signalPositiveButtonClicked), this);    
    if(positiveButtonText.length() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(positiveButton));
    }
    
    negativeButton = (GtkButton *)gtk_builder_get_object(builder, "negativeButton");
    gtk_button_set_label(negativeButton, negativeButtonText.c_str());
    g_signal_connect (negativeButton, "clicked", G_CALLBACK (signalNegativeButtonClicked), this);
    if(negativeButtonText.length() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(negativeButton));
    }
}

MessageDialog::~MessageDialog()
{
}

void MessageDialog::positive()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
}

void MessageDialog::negative()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
}

void MessageDialog::signalNegativeButtonClicked(GtkButton *button, gpointer messageDialog)
{
    ((MessageDialog *)messageDialog)->negative();
}

void MessageDialog::signalPositiveButtonClicked(GtkButton *button, gpointer messageDialog)
{
    ((MessageDialog *)messageDialog)->positive();
}