/**
 *  @file callbacks.h
 *  @brief The Main Menu in the Openmoko
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */

#ifndef _MAIN_MENU_CALLBACKS_H
#define _MAIN_MENU_CALLBACKS_H
#include <gtk/gtk.h>
#include "main.h"

void moko_wheel_bottom_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void moko_wheel_left_up_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void moko_wheel_right_down_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void moko_up_btn_cb (GtkButton *button, MokoMainMenu *mm);

void moko_down_btn_cb (GtkButton *button, MokoMainMenu *mm);

void moko_item_select_cb(MokoIconView *icon_view, 
				GtkTreePath *path, MokoMainmenuApp *mma);

void moko_icon_view_item_acitvated_cb(MokoIconView *iconview, 
				GtkTreePath *path, MokoMainmenuApp *mma);

void moko_icon_view_selection_changed_cb(MokoIconView *iconview, 
				MokoMainmenuApp *mma);

#endif /*_MAIN_MENU_CALLBACKS_H*/
