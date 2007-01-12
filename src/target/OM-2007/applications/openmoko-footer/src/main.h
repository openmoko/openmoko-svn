/**
 * @file main.h
 * @brief openmoko-taskmanager head files based on main.h.
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

#ifndef OM_TASK_MANAGER_H
#define OM_TASK_MANAGER_H

#include "footer.h"

#ifndef DBUS_API_SUBJECT_TO_CHANGE
#define DBUS_API_SUBJECT_TO_CHANGE
#endif

#include <glib/gmain.h>
#include <gdk/gdk.h>
#include <dbus/dbus.h>
#include <gtk/gtkwidget.h>

/**
 * @typedef MokoFooter
 *
 * Opaque structure used for representing an Openmoko Task Manager App. 
 */ 
/* Types */
typedef struct _MokoFooter 
{
	DBusConnection* bus;
    	GMainLoop* loop;
    	GtkWidget* toplevel_win;
    	Footer* footer;
} MokoFooter;

#endif /* main.h */
