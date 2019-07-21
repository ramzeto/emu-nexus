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
 * File:   GameSearchDialog.cpp
 * Author: ram
 * 
 * Created on June 30, 2019, 11:10 PM
 */

#include "GameSearchDialog.h"
#include "UiThreadHandler.h"
#include "UiUtils.h"

GameSearchDialog::GameSearchDialog(int64_t apiPlatformId, string query) : Dialog("SingleChoiceListDialog.ui", "singleChoiceListDialog")
{
    this->apiPlatformId = apiPlatformId;
    this->query = query;
    apiGames = NULL;
    selectedApiGame = NULL;
    
    noResultsLabel = (GtkLabel *)gtk_builder_get_object (builder, "noResultsLabel");
    gtk_widget_hide(GTK_WIDGET(noResultsLabel));
    
    listBox = (GtkListBox *)gtk_builder_get_object (builder, "listBox");
    g_signal_connect (listBox, "row-selected", G_CALLBACK(signalListRowSelected), this);
    
    spinner = (GtkSpinner *)gtk_builder_get_object (builder, "spinner");
             
    closeButton = (GtkButton *)gtk_builder_get_object(builder, "closeButton");
    g_signal_connect (closeButton, "clicked", G_CALLBACK (signalCloseButtonClicked), this);

    gtk_window_set_title(GTK_WINDOW(dialog), "Select a game");
    
    UiThreadHandler *uiThreadHandler = new UiThreadHandler(this, callbackElasticsearchGames);
    TheGamesDB::Elasticsearch::getInstance()->getGames(apiPlatformId, query, uiThreadHandler, UiThreadHandler::callback);   
}

GameSearchDialog::~GameSearchDialog()
{
    if(apiGames)
    {
        TheGamesDB::Game::releaseItems(apiGames);
    }
}

void GameSearchDialog::updateList()
{
    gtk_spinner_stop(spinner);
    
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(listBox), 1);
    if(!apiGames || apiGames->size() == 0)
    {
        gtk_widget_show(GTK_WIDGET(noResultsLabel));
        return;
    }
    
    for(unsigned int c = 0; c < apiGames->size(); c++)
    {
        TheGamesDB::Game *apiGame = TheGamesDB::Game::getItem(apiGames, c);
        
        GtkLabel *label = (GtkLabel *)gtk_label_new(apiGame->getName().c_str());
        gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_END);
        gtk_label_set_line_wrap(label, 1);
        gtk_label_set_line_wrap_mode(label, PANGO_WRAP_WORD_CHAR);
        gtk_label_set_lines(label, 3);
        
        gtk_widget_set_margin_start(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_end(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_top(GTK_WIDGET(label), 10);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 10);
        gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                
        gtk_container_add(GTK_CONTAINER(listBox), GTK_WIDGET(label));
        gtk_widget_show_all(GTK_WIDGET(label));
    }
}

void GameSearchDialog::close()
{
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void GameSearchDialog::select(unsigned int apiGameIndex)
{
    selectedApiGame = TheGamesDB::Game::getItem(apiGames, apiGameIndex);
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}


void GameSearchDialog::signalCloseButtonClicked(GtkButton *button, gpointer dialog)
{
    ((GameSearchDialog *)dialog)->close();
}

TheGamesDB::Game* GameSearchDialog::getSelectedApiGame()
{
    return selectedApiGame;
}


int GameSearchDialog::callbackElasticsearchGames(gpointer pUiThreadHandlerResult)
{
    UiThreadHandler::Result_t *uiThreadHandlerResult = (UiThreadHandler::Result_t *)pUiThreadHandlerResult;
    GameSearchDialog *gameSearchDialog = (GameSearchDialog *)uiThreadHandlerResult->uiThreadHandler->getRequesterInUiThread();
    
    TheGamesDB::Elasticsearch::Result_t *dbResult = (TheGamesDB::Elasticsearch::Result_t *)uiThreadHandlerResult->data;    
    gameSearchDialog->apiGames = (list<TheGamesDB::Game *> *)dbResult->data;
    gameSearchDialog->updateList();
    
    
    UiThreadHandler::releaseResult(uiThreadHandlerResult);
    
    return G_SOURCE_REMOVE;
}

void GameSearchDialog::signalListRowSelected (GtkListBox *listBox, GtkWidget *row, gpointer dialog)
{
    if(!dialog || !row || !GTK_IS_LIST_BOX_ROW(row))
    {
        return;
    }
        
    int position = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW(row));
    ((GameSearchDialog *)dialog)->select(position);
}