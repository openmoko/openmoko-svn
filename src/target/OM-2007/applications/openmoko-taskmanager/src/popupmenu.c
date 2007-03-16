/**
 *  popupmenu.c
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
 */

#include "popupmenu.h"

void
moko_kill_task_cb(GtkMenuItem *item, MokoTaskList *l) {
    g_debug ("kill task cb");
    //moko_wm_cmd(item, l->mokolist_view, MB_CMD_REMOVE_CLIENT);
    moko_wm_cmd(item, l->list_view, MB_CMD_REMOVE_CLIENT);
    }

void 
moko_kill_and_swith_cb(GtkMenuItem *item, MokoTaskList *l) {
    g_debug ("call kill and switch task function");
    //moko_wm_cmd(item, l->mokolist_view, MB_CMD_REMOVE_AND_ACTIVE);
    moko_wm_cmd(item, l->list_view, MB_CMD_REMOVE_AND_ACTIVE);
    }

void
moko_init_popup_menu (GtkWidget *my_widget, GdkEventButton *event, MokoTaskList *l) {
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
    g_signal_connect (item, "activate", G_CALLBACK (moko_kill_and_swith_cb), l);
    item = gtk_menu_item_new_with_label ("Kill the Application");
    gtk_widget_show (item);
    gtk_menu_prepend (menu, item);
    g_signal_connect (item, "activate", G_CALLBACK (moko_kill_task_cb), l);
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

