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
 * File:   FirstSetupPanel.h
 * Author: ram
 *
 * Created on April 14, 2019, 8:45 PM
 */

#ifndef FIRSTSETUPPANEL_H
#define FIRSTSETUPPANEL_H

#include "Panel.h"
#include "UiThreadBridge.h"
#include "CallbackResult.h"

class FirstSetupPanel : public Panel
{
public:
    FirstSetupPanel(GtkWindow *parentWindow);
    virtual ~FirstSetupPanel();
    
    void setOnSetupReadyCallback(void (*onSetupReadyCallback)(void *), void *onSetupReadyCallbackReferenceData);
    
private:
    GtkImage *logoImage;
    GtkBox *progressBox;
    GtkSpinner *spinner;
    GtkLabel *progressLabel;
    GtkLabel *resultLabel;
    GtkButton *setupButton;
    GtkButton *successButton;
    
    UiThreadBridge *processUiThreadBridge;
    UiThreadBridge *dataUiThreadBridge;
    
    void (*onSetupReadyCallback)(void *);
    void *onSetupReadyCallbackReferenceData;
    
    void show() override;
    void prepareSetup();
    void startSetup();
    void setupFailed();
    void setupSucceeded();
    
    void startDatabase();
    void startDatabaseFinished(int error);
    void preloadData();
    void preloadDataFinished(int error);
    
    void setupReady();

    static void signalSetupButtonClicked(GtkButton *button, gpointer firstSetupPanel);    
    static void signalSuccessButtonClicked(GtkButton *button, gpointer firstSetupPanel);
    
    static void callbackElasticsearchProcess(CallbackResult *callbackResult);
    static void callbackElasticsearchData(CallbackResult *callbackResult);
};

#endif /* FIRSTSETUPPANEL_H */

