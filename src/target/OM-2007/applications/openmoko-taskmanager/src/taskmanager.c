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

#include <stdio.h> 
#include <X11/Xlib.h> 
#include <X11/Xatom.h> 
#include <glib.h> 
#include <gtk/gtk.h> 
#include <gdk/gdk.h> 
#include <gdk/gdkx.h> // GDK_WINDOW_XWINDOW 

#define OPAQUE  0x55555555
/* set the widget's transparency to opacity 
 * opacity is guint 0x00000000-0xffffffff 
 */ 


int 
gtk_widget_set_transparency(GtkWidget *widget, guint opacity) { 
        Display *display; 
        Window window; 
        Window parent_win; 
        Window root_win; 
        Window* child_windows; 
        int num_child_windows; 

        if(!GTK_IS_WIDGET(widget)){ 
                printf("gtk_widget_set_transparency: not a widget!\n"); 
                return -1; 
        } 

        if(widget->window == NULL){ 
                printf("gtk_widget_set_transparency: please init widget before set transparency!\n"); 
                return -1; 
        } 

        /* Set the Display and Screen */ 
        display = (Display*)gdk_x11_get_default_xdisplay(); 
        /* sync, so the window manager can know the new widget */ 
        XSync(display, False); 
        window = GDK_WINDOW_XWINDOW(widget->window); 

        /* Get the cureent window's top-level window */ 
        while(1){ 
                XQueryTree(display, window, 
                                &root_win, 
                                &parent_win, 
                                &child_windows, &num_child_windows); 
                XFree(child_windows); 
                /* found the top-level window */ 
                if(root_win == parent_win) break; 
                window = parent_win; 
        } 

        if(opacity == OPAQUE){ 
                XDeleteProperty(display, window, 
                                XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False)); 
        }else{ 
                XChangeProperty(display, window, 
                                XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False), 
                                XA_CARDINAL, 32, PropModeReplace, 
                                (unsigned char *) &opacity, 1L); 
        } 

        XSync(display, False); 

        return 0; 
} 

int 
main (int argc, char** argv) {
    //GtkWidget *taskmanager; //main window
    MokoTaskManager *tm;
    Display *dpy;
    //List *list;

    tm = g_malloc (sizeof (MokoTaskManager));

    memset (tm, 0, sizeof (MokoTaskManager));
	
    gtk_init (&argc, &argv);
    dpy = GDK_DISPLAY ();

    tm->app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Openmoko-taskmanager" );
    
   /* tm->gtk_window = gtk_window_new (GTK_WINDOW_POPUP);//It is a popup window
    gtk_window_set_title (tm->gtk_window, _("Task Manager"));
    gtk_widget_set_uposition (tm->gtk_window, TASK_MANAGER_PROPERTY_X, TASK_MANAGER_PROPERTY_Y);
    gtk_window_set_default_size (tm->gtk_window, TASK_MANAGER_PROPERTY_WIDTH, TASK_MANAGER_PROPERTY_HEIGHT);
    gtk_widget_show (tm->gtk_window);
*/
    tm->window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    tm->wheel = moko_finger_wheel_new();
    //moko_finger_wheel_show(tm->wheel);
    tm->toolbox = moko_finger_window_get_toolbox(tm->window);

    tm->close =  moko_finger_tool_box_add_button( tm->toolbox );
    gtk_widget_show (tm->close);
    tm->close_all =  moko_finger_tool_box_add_button( tm->toolbox );
    gtk_widget_show (tm->close_all);
    tm->quit =  moko_finger_tool_box_add_button( tm->toolbox );
    gtk_widget_show (tm->quit);
    
    tm->l = LIST(list_new());
    om_update_store_list(dpy, tm->l->list_store);
    om_set_list_highlight(dpy, tm->l);
    gtk_widget_show (tm->l);
	
    //gtk_container_add (tm->gtk_window, GTK_WIDGET (tm->l));
    moko_finger_window_set_contents (tm->window, GTK_WIDGET(tm->l));


    g_signal_connect (tm->l->btn_close, "clicked", gtk_main_quit, NULL);
//g_signal_connect (G_OBJECT (list->list_view), "cursor-changed", G_CALLBACK (om_cursor_changed), 
// GTK_TREE_MODEL (list->list_store));
    g_signal_connect (G_OBJECT (tm->l->tab), "clicked", G_CALLBACK (om_tab_event_cb), tm->l);
    g_signal_connect (G_OBJECT (tm->l->tabhold), "clicked", G_CALLBACK (om_hold_event_cb), tm->l);

    gdk_window_add_filter (NULL, om_window_filter, tm->l);
    XSelectInput (dpy, DefaultRootWindow (dpy), PropertyChangeMask);

    gtk_widget_set_transparency(tm->window, 50);

    gtk_widget_show_all (tm->window);

    gtk_main();
	
    g_free (tm);
}
