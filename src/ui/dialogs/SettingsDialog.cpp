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
 * File:   SettingsDialog.cpp
 * Author: ram
 * 
 * Created on March 8, 2019, 12:29 AM
 */

#include "SettingsDialog.h"
#include "Settings.h"
#include "Preferences.h"
#include "Utils.h"
#include "MessageDialog.h"
#include "Database.h"

#include <iostream>

using namespace std;

SettingsDialog::SettingsDialog() : Dialog("SettingsDialog.ui", "settingsDialog")
{    
    mameExecutableEntry = (GtkEntry *)gtk_builder_get_object(builder, "mameExecutableEntry");
    gtk_entry_set_text(mameExecutableEntry, Settings::getInstance()->getMameExecutable().c_str());
    
    mameExecutableButton = (GtkButton *)gtk_builder_get_object(builder, "mameExecutableButton");
    g_signal_connect (mameExecutableButton, "clicked", G_CALLBACK (signalMameExecutableButtonClicked), this);
    
    cacheDirectoryEntry = (GtkEntry *)gtk_builder_get_object(builder, "cacheDirectoryEntry");
    gtk_entry_set_text(cacheDirectoryEntry, Settings::getInstance()->getCacheDirectory().c_str());
    
    cacheDirectoryButton = (GtkButton *)gtk_builder_get_object(builder, "cacheDirectoryButton");
    g_signal_connect (cacheDirectoryButton, "clicked", G_CALLBACK (signalCacheDirectoryButtonClicked), this);
    
    cacheSizeEntry = (GtkEntry *)gtk_builder_get_object(builder, "cacheSizeEntry");
    gtk_entry_set_max_length(cacheSizeEntry, 4);
    g_signal_connect (cacheSizeEntry, "key-press-event", G_CALLBACK (signalCacheSizeEntryKeyPressedEvent), this);
    gtk_entry_set_text(cacheSizeEntry, to_string(Settings::getInstance()->getCacheSize()).c_str());       
    
    cancelButton = (GtkButton *)gtk_builder_get_object(builder, "cancelButton");
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (signalCancelButtonClicked), this);

    saveButton = (GtkButton *)gtk_builder_get_object(builder, "saveButton");
    g_signal_connect (saveButton, "clicked", G_CALLBACK (signalSaveButtonClicked), this);    
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::selectMameExecutable()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select mame executable", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(mameExecutableEntry, cFileName);

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        
        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free (cFileName);
    }
    gtk_widget_destroy (fileChooserDialog);
}

void SettingsDialog::selectCacheDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select cache directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(cacheDirectoryEntry, cFileName);
        
        Preferences::getInstance()->setLastPath(string(cFileName));
        
        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free (cFileName);
    }
    gtk_widget_destroy (fileChooserDialog);
}

void SettingsDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void SettingsDialog::save()
{
    string cacheDirectory = Utils::getInstance()->trim(string(gtk_entry_get_text(cacheDirectoryEntry)));
    if(cacheDirectory.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("Cache directory cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    string mameExecutable = Utils::getInstance()->trim(string(gtk_entry_get_text(mameExecutableEntry)));
    
    unsigned int cacheSize = (unsigned int)atol(gtk_entry_get_text(cacheSizeEntry));

    
    Settings::getInstance()->setMameExecutable(mameExecutable);
    Settings::getInstance()->setCacheDirectory(cacheDirectory);
    Settings::getInstance()->setCacheSize(cacheSize);
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    Settings::getInstance()->save(sqlite);
    Database::getInstance()->release();
    
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);        
}

void SettingsDialog::signalMameExecutableButtonClicked(GtkButton *button, gpointer settingsDialog)
{
    ((SettingsDialog *)settingsDialog)->selectMameExecutable();
}

void SettingsDialog::signalCacheDirectoryButtonClicked(GtkButton *button, gpointer settingsDialog)
{
    ((SettingsDialog *)settingsDialog)->selectCacheDirectory();
}

void SettingsDialog::signalCancelButtonClicked(GtkButton *button, gpointer settingsDialog)
{
    ((SettingsDialog *)settingsDialog)->cancel();
}

void SettingsDialog::signalSaveButtonClicked(GtkButton *button, gpointer settingsDialog)
{
    ((SettingsDialog *)settingsDialog)->save();
}

gboolean SettingsDialog::signalCacheSizeEntryKeyPressedEvent(GtkEntry* entry, GdkEvent *event, gpointer settingsDialog)
{    
    if(event->key.keyval >= '0' && event->key.keyval <= '9')
    {        
        return 0;
    }
    else if(event->key.keyval == GDK_KEY_BackSpace || event->key.keyval == GDK_KEY_Left || event->key.keyval == GDK_KEY_Right || event->key.keyval == GDK_KEY_End || event->key.keyval == GDK_KEY_Home)
    {
        return 0;
    }
    
    return 1;
}
