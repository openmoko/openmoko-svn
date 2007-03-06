/**
 *  misc.h
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

#ifndef _MOKO_TASK_MANAGER_MISC_H
#define _MOKO_TASK_MANAGER_MISC_H

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

#define MB_CMD_ACTIVATE_CLIENT 100
#define MB_CMD_REMOVE_CLIENT 101
#define MB_CMD_REMOVE_AND_ACTIVE 102

Window my_win;


gboolean 
moko_X_ev_init (Display *dpy,GtkWidget *gtkwidget);

gboolean 
moko_update_net_undocked_client_list (Display* dpy, Window** list, guint* nr);

GdkPixbuf *
moko_get_window_icon (Display *dpy, Window w);

gchar *
moko_get_window_name (Display *dpy, Window w);

void 
moko_print_win_list (Display* dpy, Window* win_list, guint win_num);

Atom
moko_get_window_property (Display *dpy, Window w, Atom property);

gboolean 
moko_get_current_active_client (Display* dpy, Window* window_return);

void
mbcommand(Display *dpy, int cmd_id, Window win, char *data);

gboolean 
moko_active_next_client (Display* dpy);



/********/
gboolean 
moko_set_atoms_name(const char** src_name, int src_num);


#endif /*_MOKO_TASK_MANAGER_MISC_H*/
