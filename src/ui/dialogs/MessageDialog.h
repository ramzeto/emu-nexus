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
 * File:   MessageDialog.h
 * Author: ram
 *
 * Created on March 8, 2019, 8:13 PM
 */

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include "Dialog.h"

#include <string>

using namespace std;

/**
 * A message dialog. Can be used for confirmation or simply for information.
 */
class MessageDialog : public Dialog
{
public:
    
    /**
     * 
     * @param parent Parent GtkWindow.
     * @param message Message to show
     * @param positiveButtonText Positive button text. If is an empty string, the button is not showed. If pressed, gtk_dialog_response(..., GTK_RESPONSE_YES) will be called.
     * @param negativeButtonText Negative button text. If is an empty string, the button is not showed. If pressed, gtk_dialog_response(..., GTK_RESPONSE_NO) will be called.
     */
    MessageDialog(GtkWindow *parent, string message, string positiveButtonText, string negativeButtonText);
    
    virtual ~MessageDialog();

private:
    GtkLabel *messageLabel;
    GtkButton *positiveButton;
    GtkButton *negativeButton;
    
    void positive();
    void negative();
    
    static void signalNegativeButtonClicked(GtkButton *button, gpointer messageDialog);
    static void signalPositiveButtonClicked(GtkButton *button, gpointer messageDialog);
};

#endif /* MESSAGEDIALOG_H */

