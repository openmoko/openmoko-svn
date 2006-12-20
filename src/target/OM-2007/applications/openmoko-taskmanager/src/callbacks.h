/**
 * @file callbacks.h
 * @brief openmoko-taskmanager callbacks based on misc.c.
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
#ifndef _TASK_MANAGER_CALLBACKS_H
#define _TASK_MANAGER_CALLBACKS_H
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "misc.h"
#include "list_view.h"
#include "xatoms.h"
#include "popupmenu.h"

GdkFilterReturn
om_window_filter (GdkXEvent *xev, GdkEvent *gev, List *l);

void
om_activate_task (GtkWidget *w, GtkWidget *list_view);

gboolean
om_kill_task (GtkWidget *w, GtkWidget *list_view);

void
om_row_activated (GtkTreeView *treeview, GtkTreePath *path, 
			GtkTreeViewColumn *col, GtkTreeModel *model);

gboolean 
om_cursor_changed(GtkTreeView *treeview, GtkTreeModel *model);

void
om_tab_event_cb (GtkButton *btn, List *l);

void        
om_hold_event_cb (GtkButton *btn, List *l);

#endif /*callbacks.h*/
