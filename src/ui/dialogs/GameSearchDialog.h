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
 * File:   GameSearchDialog.h
 * Author: ram
 *
 * Created on June 30, 2019, 11:10 PM
 */

#ifndef GAMESEARCHDIALOG_H
#define GAMESEARCHDIALOG_H

#include "Dialog.h"
#include "thegamesdb.h"
#include "UiThreadBridge.h"
#include "CallbackResult.h"

#include <list>
#include <string>

using namespace std;

class GameSearchDialog : public Dialog
{
public:
    
    /**
     * 
     * @param parent Parent GtkWindow.
     * @param apiPlatformId Id of the API (TheGamesDB) platform.
     * @param query Query to search.
     */
    GameSearchDialog(GtkWindow *parent, int64_t apiPlatformId,  string query);
    virtual ~GameSearchDialog();
    
    /**
     * 
     * @return Selected API (TheGamesDB) game, it is only valid when the result of the dialog is GTK_RESPONSE_ACCEPT. This object should be copied by the caller, it will be deleted when the dialog is destroyed.
     */
    TheGamesDB::Game *getSelectedApiGame();
    
private:    
    GtkLabel *noResultsLabel;
    GtkListBox *listBox;
    GtkSpinner *spinner;
    GtkButton *closeButton;
    
    int64_t apiPlatformId;
    string query;
    list<TheGamesDB::Game *> *apiGames;
    TheGamesDB::Game *selectedApiGame;
    
    UiThreadBridge *dataUiThreadBridge;
    
    void updateList();
    void close();
    void select(unsigned int apiGameIndex);
    
    static void signalCloseButtonClicked(GtkButton *button, gpointer dialog);    
    static void signalListRowSelected (GtkListBox *listBox, GtkWidget *row, gpointer dialog);
    static void callbackElasticsearch(CallbackResult *callbackResult);
};

#endif /* GAMESEARCHDIALOG_H */

