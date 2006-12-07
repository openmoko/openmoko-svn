/**
 * @file popupmen.c
 * @brief popupmenu based on gtk and gdk.
 * @author Sun Zhiyong
 * @date 2006-10
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
 */

#include "popupmenu.h"

void
om_kill_task_cb(GtkMenuItem *item, List *l) {
    g_debug ("kill task cb");
    om_wm_cmd(item, l->mokolist_view, MB_CMD_REMOVE_CLIENT);
    }

void 
om_kill_and_swith_cb(GtkMenuItem *item, List *l) {
    g_debug ("call kill and switch task function");
    om_wm_cmd(item, l->mokolist_view, MB_CMD_REMOVE_AND_ACTIVE);
    }

void
om_init_popup_menu (GtkWidget *my_widget, GdkEventButton *event, List *l) {
    GtkWidget *menu;
    GtkMenuItem *item;
    int button, event_time;
    
    menu = gtk_menu_new ();
    gtk_widget_show (menu);
    g_signal_connect (menu, "selection-done", G_CALLBACK (gtk_widget_destroy), NULL);

    /* ... add menu items ... */
    item = gtk_menu_item_new_with_label ("Close and switch");
    gtk_widget_show (item);
    gtk_menu_prepend (menu, item);
    g_signal_connect (item, "activate", G_CALLBACK (om_kill_and_swith_cb), l);
    item = gtk_menu_item_new_with_label ("Kill the Application");
    gtk_widget_show (item);
    gtk_menu_prepend (menu, item);
    g_signal_connect (item, "activate", G_CALLBACK (om_kill_task_cb), l);
    if (event) {
        button = event->button;
        event_time = event->time;
        }
    else {
    	 button = 0;
    	 event_time = gtk_get_current_event_time ();
    	 }
    gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 
    	 				button, event_time);
    }

