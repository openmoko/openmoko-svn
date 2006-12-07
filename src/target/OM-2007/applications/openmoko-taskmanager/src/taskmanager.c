/**
 * @file taskmanager.c
 * @brief openmoko-taskmanager taskmanager.c based on callbacks.h and list_view.h.
 * @brief UI of openmoko-taskmanager based on list_view.c
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

#include "taskmanager.h"

int 
main (int argc, char** argv) {
    GtkWidget *taskmanager; //main window
    Display *dpy;
    List *list;

    list = g_malloc (sizeof (List));
	
    gtk_init (&argc, &argv);
    dpy = GDK_DISPLAY ();
    
    taskmanager = gtk_window_new (GTK_WINDOW_POPUP);//It is a popup window
    gtk_window_set_title (taskmanager, _("Task Manager"));
    gtk_widget_set_uposition (taskmanager, TASK_MANAGER_PROPERTY_X, TASK_MANAGER_PROPERTY_Y);
    gtk_window_set_default_size (taskmanager, TASK_MANAGER_PROPERTY_WIDTH, TASK_MANAGER_PROPERTY_HEIGHT);
    gtk_widget_show (taskmanager);

    list = LIST(list_new());
    om_update_store_list(dpy, list->list_store);
    om_set_list_highlight(dpy, list);
    gtk_widget_show (list);
	
    gtk_container_add (taskmanager, GTK_WIDGET (list));
    //moko_finger_window_set_contents( window, GTK_WIDGET(list) );


    g_signal_connect (list->btn_close, "clicked", gtk_main_quit, NULL);
//g_signal_connect (G_OBJECT (list->list_view), "cursor-changed", G_CALLBACK (om_cursor_changed), 
// GTK_TREE_MODEL (list->list_store));
    g_signal_connect (G_OBJECT (list->tab), "clicked", G_CALLBACK (om_tab_event_cb), list);
    g_signal_connect (G_OBJECT (list->tabhold), "clicked", G_CALLBACK (om_hold_event_cb), list);

    gdk_window_add_filter (NULL, om_window_filter, list);
    XSelectInput (dpy, DefaultRootWindow (dpy), PropertyChangeMask);

    gtk_widget_show_all (taskmanager);

    gtk_main();
	
    g_free (list);
}
