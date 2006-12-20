/**
 * @file misc.h
 * @brief misc.h based on Xlib glib and gtk+-2.0.
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
#ifndef _MISC_H
#define _MISC_H

#include <sys/types.h>
#include <stdio.h>
#include <libintl.h>
#include <stdlib.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "xatoms.h"

/*Matchbox windows manager structs*/
#define MB_CMD_SET_THEME     1
#define MB_CMD_EXIT          2
#define MB_CMD_DESKTOP       3
#define MB_CMD_NEXT          4
#define MB_CMD_PREV          5
#define MB_CMD_SHOW_EXT_MENU 6
#define MB_CMD_MISC          7
#define MB_CMD_COMPOSITE     8
#define MB_CMB_KEYS_RELOAD   9

#define MB_CMD_ACTIVAE_CLIENT 100
#define MB_CMD_REMOVE_CLIENT 101
#define MB_CMD_REMOVE_AND_ACTIVE 102

Window my_win;


gboolean 
om_X_ev_init (Display *dpy,GtkWidget *gtkwidget);

gboolean 
om_update_net_undocked_client_list (Display* dpy, Window** list, guint* nr);

GdkPixbuf *
om_get_window_icon (Display *dpy, Window w);

gchar *
om_get_window_name (Display *dpy, Window w);

void 
om_print_win_list (Display* dpy, Window* win_list, guint win_num);

Atom
om_get_window_property (Display *dpy, Window w, Atom property);

gboolean 
om_get_current_active_client (Display* dpy, Window* window_return);

void
mbcommand(Display *dpy, int cmd_id, Window win, char *data);

gboolean 
om_active_next_client (Display* dpy);

gboolean
om_kill_window (Display *dpy, Window w);

/********/
// the functions below have not implemented.
gboolean 
om_set_atoms_name(const char** src_name, int src_num);


#endif/*misc.h*/
