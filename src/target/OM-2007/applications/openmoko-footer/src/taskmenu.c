/*
 *  Footer - Task manager menu
 *
 *  Authored by Daniel Willmann <daniel@totalueberwachung.de>
 *  Pieces taken from openmoko-taskmanager by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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

#include "taskitem.h"
#include "taskmenu.h"
#include "callbacks.h"

#include <libmokoui/moko-application.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <glib.h>

//GdkFilterReturn
//moko_window_filter (GdkXEvent *xev, GdkEvent *gev, MokoTaskMenu *tm)
//{
//    XEvent *ev = (XEvent *)xev;
//    Display *dpy = ev->xany.display;
//
//    if (ev->xany.type == PropertyNotify && ev->xproperty.window == DefaultRootWindow (dpy)
//        && (ev->xproperty.atom == atoms[_NET_CLIENT_LIST]))
//    {
//        moko_update_task_list(dpy, tm->list);
//    }
//
//    return GDK_FILTER_CONTINUE;
//}
//

gchar *
moko_get_window_name (Display *dpy, Window w)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    gchar *name = NULL;
    int rc;

    gdk_error_trap_push ();
    rc = XGetWindowProperty (dpy, w, gdk_x11_atom_to_xatom(gdk_atom_intern("_NET_WM_NAME", FALSE)),
        0, G_MAXLONG, False, gdk_x11_atom_to_xatom(gdk_atom_intern("UTF8_STRING", FALSE)), &actual_type, &actual_format,
        &nitems, &bytes_after, &prop);
    if (gdk_error_trap_pop () || rc != Success)
        return NULL;
    if (nitems)
    {
        name = g_strdup (prop);
        XFree (prop);
    }
    else
    {
        gdk_error_trap_push ();
        rc = XGetWindowProperty (dpy, w, XA_WM_NAME,
            0, G_MAXLONG, False, XA_STRING, &actual_type, &actual_format,
            &nitems, &bytes_after, &prop);
        if (gdk_error_trap_pop () || rc != Success)
            return FALSE;
        if (nitems)
        {
            name = g_locale_to_utf8 (prop, -1, NULL, NULL, NULL);
            XFree (prop);
        }
    }
    return name;
}


Atom
moko_get_window_property (Display *dpy, Window w, Atom property)
{
    Atom result = None;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    int rc;

    gdk_error_trap_push();
    rc = XGetWindowProperty (dpy, w, property,
        0, 1, False, XA_ATOM, &actual_type, &actual_format,
        &nitems, &bytes_after, &prop);
    if (gdk_error_trap_pop() || rc != Success)
    {
        //g_debug ("Have not obtain the property");
        ////return None;
    }

    if (prop)
    {
        memcpy (&result, prop, sizeof (result));
        XFree (prop);
    }
    return result;
}

gboolean moko_update_task_list (Display *dpy, MokoTaskMenu *tm)
{
    Atom actual_type, type, normal_window;
    Window *temp_list;
    int actual_format;
    unsigned long nitems, bytes_after = 0;
    unsigned char *prop = NULL;
    int counter = 0, i = 0, rc;

    rc = XGetWindowProperty (dpy, DefaultRootWindow (dpy),
               gdk_x11_atom_to_xatom(gdk_atom_intern("_NET_CLIENT_LIST", FALSE)),
                0, G_MAXLONG, False, XA_WINDOW, &actual_type, &actual_format,
               &nitems, &bytes_after, &prop);
    if (rc != Success || prop == NULL)
        return FALSE;

    temp_list = g_malloc0 (sizeof (Window) * nitems);
    if (temp_list == NULL)
    {
        XFree (prop);
        return FALSE;
    }
    memcpy (temp_list, prop, sizeof (Window) * nitems);
    XFree (prop);


    /*need to make clear thar whether the "Client List" is ordered by the
    atom "_NET_WM_WINDOW_TYPE", if it does, what we need to do will become
    more simple that only find the boundary of "dock" and "Undock" window in the list.
    */

    normal_window = gdk_x11_atom_to_xatom(gdk_atom_intern ("_NET_WM_WINDOW_TYPE_NORMAL", FALSE));

    for (i=0; i<nitems; i++)
    {
        type = moko_get_window_property (dpy, temp_list[i], gdk_x11_atom_to_xatom(gdk_atom_intern("_NET_WM_WINDOW_TYPE", FALSE)));
        if (type == normal_window)
        {
            temp_list[counter] = temp_list[i];
            counter++;
        }
    }

    tm->list = g_realloc(tm->list, sizeof(Window) * counter);
    memcpy(tm->list, temp_list, sizeof(Window) * counter);
    tm->listnr = counter;

    XFree (temp_list);
    return TRUE;
}


void moko_taskmenu_init (MokoTaskMenu *tm)
{
    Display *dpy;

    dpy = GDK_DISPLAY();

    tm->menu = GTK_MENU(gtk_menu_new());

    tm->list = NULL;

    moko_update_task_list(dpy, tm);
    moko_taskmenu_populate(dpy, tm);

    gtk_widget_show_all( GTK_WIDGET(tm->menu) );

    //gdk_window_add_filter (NULL, moko_window_filter, tm);
    //XSelectInput (dpy, DefaultRootWindow (dpy), PropertyChangeMask);
}

void moko_taskmenu_populate(Display *dpy, MokoTaskMenu *tm)
{
    int i;
    char *temp;
    GtkWidget *item;

    for (i=0;i<tm->listnr;i++) {
        temp = moko_get_window_name(dpy, tm->list[i]);
        item = gtk_menu_item_new_with_label( temp );
        free(temp);
        //g_signal_connect( G_OBJECT(lock), "activate", G_CALLBACK(panel_mainmenu_popup_selected_lock), NULL );

        gtk_menu_shell_append( GTK_MENU_SHELL(tm->menu), item );
    }
}

void moko_taskmenu_popup_positioning_cb( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, GtkWidget *parent )
{
    GtkRequisition req;
    gint parent_x, parent_y;
    gtk_widget_size_request( GTK_WIDGET(menu), &req );

    if (GTK_IS_WINDOW(parent)) {
        // Position menu above footer
        gtk_window_get_position(GTK_WINDOW(parent), &parent_x, &parent_y);
        *x = parent_x;
        *y = parent_y - req.height;
    }
}

