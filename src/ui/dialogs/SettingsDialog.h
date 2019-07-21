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
 * File:   SettingsDialog.h
 * Author: ram
 *
 * Created on March 8, 2019, 12:29 AM
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "Dialog.h"

/**
 * Dialog to configure the application settings. The dialog will call gtk_dialog_response(..., GTK_RESPONSE_ACCEPT) if accepted.
 */
class SettingsDialog : public Dialog
{
public:
    
    SettingsDialog();
    virtual ~SettingsDialog();
    
private:
    GtkEntry *mameExecutableEntry;
    GtkButton *mameExecutableButton;
    GtkEntry *cacheDirectoryEntry;
    GtkButton *cacheDirectoryButton;
    GtkEntry *cacheSizeEntry;
    GtkButton *cancelButton;
    GtkButton *saveButton;
    
    void selectMameExecutable();
    void selectCacheDirectory();
    void cancel();
    void save();
    
    
    static void signalMameExecutableButtonClicked(GtkButton *button, gpointer settingsDialog);
    static void signalCacheDirectoryButtonClicked(GtkButton *button, gpointer settingsDialog);
    static void signalCancelButtonClicked(GtkButton *button, gpointer settingsDialog);
    static void signalSaveButtonClicked(GtkButton *button, gpointer settingsDialog);
    static gboolean signalCacheSizeEntryKeyPressedEvent(GtkEntry *entry, GdkEvent *event, gpointer settingsDialog);
};

#endif /* SETTINGSDIALOG_H */

