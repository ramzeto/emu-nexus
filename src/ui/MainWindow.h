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
#include "UiThreadBridge.h"
#include "CallbackResult.h"

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
    
    static const int ITEM_HOME_INDEX;
    static const int ITEM_RECENTS_INDEX;
    static const int ITEM_FAVORITES_INDEX;
    static const int ITEM_PLATFORMS_OFFSET;
    
    
    GtkBuilder *builder;
    GtkApplicationWindow *mainWindow;
    GtkButton *addPlatformButton;
    GtkSearchEntry *gameSearchEntry;
    GtkButton *addGameButton;
    GtkButton *addDirectoryButton;
    GtkBox *itemListContainerBox;
    GtkListBox *itemListBox;
    GtkBox *bannerBox;
    GtkBox *contentBox;
    
    GtkBox *processBox;
    GtkSpinner *processSpinner;
    GtkLabel *processTitleLabel;
    GtkLabel *processMessageLabel;
    GtkProgressBar *processProgressBar;
    
    GtkCssProvider *cssProvider;
        
    list<Platform *> *platforms;
    Panel *currentPanel;
    int64_t selectedPlatformId;
    
    UiThreadBridge *processUiThreadBridge;

    /**
     * Starts GUI.
     */
    void startGui();
    
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
     */
    void showRecents();
    
    /**
     * 
     */
    void showFavorites();
    
    /**
     * 
     * @param platformId
     */
    void showPlatformEditDialog(int64_t platformId);
    
    /**
     * Loads the items list.
     */
    void loadItemsList();
    
    /**
     * 
     */
    void updateRecents();
    
    /**
     * 
     */
    void updateFavorites();
    
    /**
     * Updates an item from the list.
     * @param platformId
     */
    void updatePlatform(int64_t platformId);        
    
    /**
     * Selects an item from the list.
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
     * Signal triggered when itemListBox "row-selected" event happens.
     * @param listBox
     * @param row
     * @param mainWindow
     */
    static void signalItemListRowSelected (GtkListBox *listBox, GtkWidget *row, gpointer mainWindow);
    
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
     * @param callbackResult
     */
    static void callbackSerialProcessStatus(CallbackResult *callbackResult);
    
    /**
     * 
     * @param callbackResult
     */
    static void callbackNotification(CallbackResult *callbackResult);
    
};

#endif /* MAINWINDOW_H */

