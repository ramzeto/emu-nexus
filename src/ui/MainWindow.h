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
 * File:   MainWindow.h
 * Author: ram
 *
 * Created on February 5, 2019, 11:34 PM
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "Panel.h"
#include "Platform.h"
#include "UiThreadHandler.h"

#include <gtk/gtk.h>
#include <list>

using namespace std;

class MainWindow {
public:    
    MainWindow();
    virtual ~MainWindow();
    
    
private:
    static const int PLATFORM_IMAGE_WIDTH;
    static const int PLATFORM_IMAGE_HEIGHT;
    
    GtkBuilder *builder;
    GtkApplicationWindow *mainWindow;
    GtkButton *addPlatformButton;
    GtkSearchEntry *gameSearchEntry;
    GtkButton *addGameButton;
    GtkButton *addDirectoryButton;
    GtkBox *platformListContainerBox;
    GtkListBox *platformListBox;
    GtkBox *contentBox;
    
    GtkBox *processBox;
    GtkSpinner *processSpinner;
    GtkLabel *processTitleLabel;
    GtkLabel *processMessageLabel;
    GtkProgressBar *processProgressBar;
    
    GtkLabel *versionLabel;
    GtkCssProvider *cssProvider;
    
    list<Platform *> *platforms;
    Panel *currentPanel;
    int64_t selectedPlatformId;

    UiThreadHandler *processUiThreadHandler;

    void loadStartGui();
    
    /**
     * Shows a panel in the contentBox.
     * @param panel
     */
    void showPanel(Panel *panel);   
    
    /**
     * 
     */
    void showHome();
    
    /**
     * 
     * @param platformId
     */
    void showPlatformDialog(int64_t platformId);
    
    /**
     * Loads the platform list.
     */
    void loadPlatformList();
    
    /**
     * Updates a platform from the list.
     * @param platformId
     */
    void updatePlatform(int64_t platformId);
    
    /**
     * Selects a platform.
     * @param platformId
     */
    void selectPlatform(int64_t platformId);
    
    /**
     * Removes a platform.
     * @param platformId
     */
    void removePlatform(int64_t platformId);        
    
    
    /**
     * Callback triggered from the FirstSetupPanel when the initial setup has been completed.
     * @param mainWindow MainWindow reference.
     */
    static void onSetupReadyCallback(void *mainWindow);
    
    /**
     * Signal triggered when a window "configure-event" (resize, maximized, etc.) happens.
     * @param window
     * @param event
     * @param mainWindow
     */
    static void signalMainWindowConfigureEvent(GtkWindow *window, GdkEvent *event, gpointer mainWindow);    
    
    /**
     * Signal triggered when paltformListBox "row-selected" event happens.
     * @param listBox
     * @param row
     * @param mainWindow
     */
    static void signalPlatformListRowSelected (GtkListBox *listBox, GtkWidget *row, gpointer mainWindow);
    
    /**
     * Signal triggered when the user presses a mouse button over a platform in the list.
     * @param widget
     * @param event
     * @param mainWindow
     * @return 
     */
    static gboolean signalPlatformListRowPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer mainWindow);
        
    /**
     * Signal triggered when the user selects the edit menu option on a platform in the list.
     * @param menuitem
     * @param mainWindow
     */
    static void signalPlatformMenuEditActivate(GtkMenuItem *menuitem, gpointer mainWindow);
    
    /**
     * Signal triggered when the user selects the remove menu option on a platform in the list.
     * @param menuitem
     * @param mainWindow
     */
    static void signalPlatformMenuRemoveActivate(GtkMenuItem *menuitem, gpointer mainWindow);    
    
    /**
     * 
     * @param button
     * @param mainWindow
     */
    static void signalAddPlatformButtonClicked(GtkButton *button, gpointer mainWindow);
    
    /**
     * 
     * @param button
     * @param mainWindow
     */
    static void signalAddGameButtonClicked(GtkButton *button, gpointer mainWindow);

    /**
     * 
     * @param button
     * @param mainWindow
     */
    static void signalAddDirectoryButtonClicked(GtkButton *button, gpointer mainWindow);
    
    /**
     * 
     * @param searchEntry
     * @param mainWindow
     */
    static void signalGameSearchEntrySearchChanged(GtkSearchEntry *searchEntry, gpointer mainWindow);
    
    /**
     * 
     * @param uiThreadHandler
     */
    static void serialProcessStatusCallBack(gpointer pUiThreadHandlerResult);
    
    /**
     * 
     * @param notification
     * @param mainWindow
     * @param notificationData
     */
    static void notificationReceived(string notification, void *mainWindow, void *notificationData);
    
};

#endif /* MAINWINDOW_H */

