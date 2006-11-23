/**
 * @file initwidgets.h 
 * @brief Init all widgets with theme
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-11-09
 */

#ifndef _FIC_INITWIDGETS_H
#define _FIC_INITWIDGETS_H

#include <gtk/gtk.h>

#define MAIN_WINDOW_HEIGHT 560
#define MENUBAR_HEIGHT  40
#define NAVIGATION_LIST_HEIGHT  187
#define NAVIGATION_SW_HEIGHT  162
#define NAVIGATION_SW_POS_Y  14
#define NAVIGATION_SW_POS_X  17
#define TOOLBAR_HEIGHT  52
#define BAR_SEPARATE_HEIGTH 14
#define STEPER_HEIGHT  50
#define MIN_NAVIGATION_LIST_HEIGHT 100
#define MIN_DETAIL_AREA_HEIGHT  100
#define DETAIL_SW_HEIGHT 227
#define DETAIL_SW_POS_X  9
#define DETAIL_SW_POS_Y  33
#define DETAIL_FULLSCREEN_BUTTON_POS_X  437
#define DETAIL_FULLSCREEN_BUTTON_HEIGHT  26

void init_main_window_widget (GtkWidget *widget);

void resize_navigation_detail_area (GtkWidget *widget, gint stepper);

void full_screen_detail (GtkWidget *widget);

#endif  ///_FIC_INITWIDGETS_H
