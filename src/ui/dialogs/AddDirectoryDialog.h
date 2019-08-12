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
 * File:   AddDirectoryDialog.h
 * Author: ram
 *
 * Created on July 23, 2019, 1:12 AM
 */

#ifndef ADDDIRECTORYDIALOG_H
#define ADDDIRECTORYDIALOG_H

#include "Dialog.h"

class AddDirectoryDialog : public Dialog
{
public:
    AddDirectoryDialog(int64_t platformId);
    virtual ~AddDirectoryDialog();
    
private:
    
    GtkEntry *directoryEntry;
    GtkButton *directoryButton;
    GtkEntry *fileExtensionsEntry;
    GtkCheckButton *mameCheckButton;
    GtkEntry *mameExecutableEntry;
    GtkButton *mameExecutableButton;
    GtkEntry *boxFrontImagesEntry;
    GtkButton *boxFrontImagesButton;
    GtkEntry *boxBackImagesEntry;
    GtkButton *boxBackImagesButton;    
    GtkEntry *screenshotImagesEntry;
    GtkButton *screenshotImagesButton;    
    GtkEntry *logoImagesEntry;
    GtkButton *logoImagesButton;    
    GtkEntry *bannerImagesEntry;
    GtkButton *bannerImagesButton;
    GtkButton *cancelButton;
    GtkButton *acceptButton;
    
    int64_t platformId;
    
    void selectDirectory();
    void toggleMame();
    void selectMame();
    void selectBoxFrontImagesDirectory();
    void selectBoxBackImagesDirectory();
    void selectScreenshotImagesDirectory();
    void selectLogoImagesDirectory();
    void selectBannerImagesDirectory();    
    void accept();
    void cancel();
    
    
    static void signalDirectoryButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalMameToggled(GtkToggleButton *togglebutton, gpointer pAddDirectoryDialog);
    static void signalMameExecutableButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalBoxFrontImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalBoxBackImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalScreenshotImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalLogoImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalBannerImagesButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalCancelButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
    static void signalAcceptButtonClicked(GtkButton *button, gpointer pAddDirectoryDialog);
};

#endif /* ADDDIRECTORYDIALOG_H */

