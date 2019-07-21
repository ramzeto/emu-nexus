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
 * File:   HomePanel.h
 * Author: ram
 *
 * Created on May 6, 2019, 8:49 PM
 */

#ifndef HOMEPANEL_H
#define HOMEPANEL_H

#include "Panel.h"

class HomePanel : public Panel
{
public:
    HomePanel();
    virtual ~HomePanel();
private:
    GtkBox *recentsBox;
    GtkLabel *informationLabel;
};

#endif /* HOMEPANEL_H */

