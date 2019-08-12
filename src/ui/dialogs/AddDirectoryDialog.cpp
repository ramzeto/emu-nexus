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
 * File:   AddDirectoryDialog.cpp
 * Author: ram
 * 
 * Created on July 23, 2019, 1:12 AM
 */


#include "AddDirectoryDialog.h"
#include "Preferences.h"
#include "Database.h"
#include "Utils.h"
#include "ParseDirectory.h"
#include "MessageDialog.h"
#include "NotificationManager.h"
#include "Notifications.h"


AddDirectoryDialog::AddDirectoryDialog(int64_t platformId) : Dialog("AddDirectoryDialog.ui", "addDirectoryDialog")
{
    this->platformId = platformId;

    directoryEntry = (GtkEntry *)gtk_builder_get_object (builder, "directoryEntry");
    
    directoryButton = (GtkButton *)gtk_builder_get_object (builder, "directoryButton");
    g_signal_connect (directoryButton, "clicked", G_CALLBACK (signalDirectoryButtonClicked), this);
    
    fileExtensionsEntry = (GtkEntry *)gtk_builder_get_object (builder, "fileExtensionsEntry");
    
    mameCheckButton = (GtkCheckButton *)gtk_builder_get_object (builder, "mameCheckButton");
    g_signal_connect (mameCheckButton, "toggled", G_CALLBACK (signalMameToggled), this);
    
    mameExecutableEntry = (GtkEntry *)gtk_builder_get_object (builder, "mameExecutableEntry");
    gtk_entry_set_text(mameExecutableEntry, Preferences::getInstance()->getMameExecutable().c_str());
    
    mameExecutableButton = (GtkButton *)gtk_builder_get_object (builder, "mameExecutableButton");
    g_signal_connect (mameExecutableButton, "clicked", G_CALLBACK (signalMameExecutableButtonClicked), this);
    
    boxFrontImagesEntry = (GtkEntry *)gtk_builder_get_object (builder, "boxFrontImagesEntry");
    
    boxFrontImagesButton = (GtkButton *)gtk_builder_get_object (builder, "boxFrontImagesButton");
    g_signal_connect (boxFrontImagesButton, "clicked", G_CALLBACK (signalBoxFrontImagesButtonClicked), this);
        
    boxBackImagesEntry = (GtkEntry *)gtk_builder_get_object (builder, "boxBackImagesEntry");    
    
    boxBackImagesButton = (GtkButton *)gtk_builder_get_object (builder, "boxBackImagesButton");
    g_signal_connect (boxBackImagesButton, "clicked", G_CALLBACK (signalBoxBackImagesButtonClicked), this);
    
    screenshotImagesEntry = (GtkEntry *)gtk_builder_get_object (builder, "screenshotImagesEntry");
    
    screenshotImagesButton = (GtkButton *)gtk_builder_get_object (builder, "screenshotImagesButton");
    g_signal_connect (screenshotImagesButton, "clicked", G_CALLBACK (signalScreenshotImagesButtonClicked), this);
    
    logoImagesEntry = (GtkEntry *)gtk_builder_get_object (builder, "logoImagesEntry");
    
    logoImagesButton = (GtkButton *)gtk_builder_get_object (builder, "logoImagesButton");
    g_signal_connect (logoImagesButton, "clicked", G_CALLBACK (signalLogoImagesButtonClicked), this);
    
    bannerImagesEntry = (GtkEntry *)gtk_builder_get_object (builder, "bannerImagesEntry");
    
    bannerImagesButton = (GtkButton *)gtk_builder_get_object (builder, "bannerImagesButton");
    g_signal_connect (bannerImagesButton, "clicked", G_CALLBACK (signalBannerImagesButtonClicked), this);
        
    cancelButton = (GtkButton *)gtk_builder_get_object (builder, "cancelButton");
    g_signal_connect (cancelButton, "clicked", G_CALLBACK (signalCancelButtonClicked), this);
    
    acceptButton = (GtkButton *)gtk_builder_get_object (builder, "acceptButton");
    g_signal_connect (acceptButton, "clicked", G_CALLBACK (signalAcceptButtonClicked), this);
    
}

AddDirectoryDialog::~AddDirectoryDialog()
{
}

void AddDirectoryDialog::selectDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(directoryEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::toggleMame()
{
    gtk_widget_set_sensitive(GTK_WIDGET(mameExecutableEntry), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)));
    gtk_widget_set_sensitive(GTK_WIDGET(mameExecutableButton), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)));
}

void AddDirectoryDialog::selectMame()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select mame binary", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(mameExecutableEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::selectBoxFrontImagesDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select box front images directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(boxFrontImagesEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::selectBoxBackImagesDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select box back images directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(boxBackImagesEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::selectScreenshotImagesDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select screenshot images directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(screenshotImagesEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::selectLogoImagesDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select logo images directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(logoImagesEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::selectBannerImagesDirectory()
{
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Select banner images directory", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));
        gtk_entry_set_text(bannerImagesEntry, cFileName);

        sqlite3 *sqlite = Database::getInstance()->acquire();
        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save(sqlite);
        Database::getInstance()->release();
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void AddDirectoryDialog::accept()
{
    string directory = Utils::getInstance()->trim(string(gtk_entry_get_text(directoryEntry)));
    if(directory.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("Directory cannot be empty", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    string fileExtensions = Utils::getInstance()->trim(string(gtk_entry_get_text(fileExtensionsEntry)));
    if(fileExtensions.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("You should provide at least one file extension", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    string mame = Utils::getInstance()->trim(string(gtk_entry_get_text(mameExecutableEntry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)) && mame.length() == 0)
    {
        MessageDialog *messageDialog = new MessageDialog("MAME path is not valid", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
    
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)) && !Utils::getInstance()->fileExists(mame))
    {
        MessageDialog *messageDialog = new MessageDialog("MAME binary does not exists", "Ok", "");   
        messageDialog->execute();
        delete messageDialog;
        
        return;
    }
        
    ParseDirectory *parseDirectory = new ParseDirectory((int64_t)0);
    parseDirectory->setPlatformId(platformId);
    parseDirectory->setTimestamp(Utils::getInstance()->nowIsoDateTime());
    parseDirectory->setDirectory(directory);
    parseDirectory->setFileExtensions(fileExtensions);
    parseDirectory->setUseMame(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)));
    parseDirectory->setMame(mame);
    parseDirectory->setBoxFrontImagesDirectory(Utils::getInstance()->trim(string(gtk_entry_get_text(boxFrontImagesEntry))));
    parseDirectory->setBoxBackImagesDirectory(Utils::getInstance()->trim(string(gtk_entry_get_text(boxBackImagesEntry))));
    parseDirectory->setScreenshotImagesDirectory(Utils::getInstance()->trim(string(gtk_entry_get_text(screenshotImagesEntry))));
    parseDirectory->setLogoImagesDirectory(Utils::getInstance()->trim(string(gtk_entry_get_text(logoImagesEntry))));
    parseDirectory->setBannerImagesDirectory(Utils::getInstance()->trim(string(gtk_entry_get_text(bannerImagesEntry))));
    
    
    sqlite3 *sqlite = Database::getInstance()->acquire();
    parseDirectory->save(sqlite);    
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mameCheckButton)))
    {
        Preferences::getInstance()->setMameExecutable(mame);
        Preferences::getInstance()->save(sqlite);
    }    
    Database::getInstance()->release();

    NotificationManager::getInstance()->postNotification(NOTIFICATION_ADD_DIRECTORY, NULL);
    
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}

void AddDirectoryDialog::cancel()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}



void AddDirectoryDialog::signalDirectoryButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectDirectory();
}

void AddDirectoryDialog::signalMameToggled(GtkToggleButton *togglebutton, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->toggleMame();    
}
   
void AddDirectoryDialog::signalMameExecutableButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectMame();
}

void AddDirectoryDialog::signalBoxFrontImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectBoxFrontImagesDirectory();
}

void AddDirectoryDialog::signalBoxBackImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectBoxBackImagesDirectory();
}

void AddDirectoryDialog::signalScreenshotImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectScreenshotImagesDirectory();
}

void AddDirectoryDialog::signalLogoImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectLogoImagesDirectory();
}

void AddDirectoryDialog::signalBannerImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->selectBannerImagesDirectory();
}

void AddDirectoryDialog::signalCancelButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->cancel();
}

void AddDirectoryDialog::signalAcceptButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog)
{
    AddDirectoryDialog *addDirectoryDialog = (AddDirectoryDialog *)pAddDirectoryDialog;
    addDirectoryDialog->accept();
}
