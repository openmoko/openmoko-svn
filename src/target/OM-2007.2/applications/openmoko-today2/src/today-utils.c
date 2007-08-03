/*
 *  Today - At a glance view of date, time, calender events, todo items and
 *  other images.
 *
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>
#include <gtk/gtk.h>
#include <libtaku/launcher-util.h>
#include "today-utils.h"

GtkToolItem *
today_toolbutton_new (const gchar *icon_name)
{
	GtkWidget *icon = gtk_image_new_from_icon_name (icon_name,
		GTK_ICON_SIZE_DIALOG);
	GtkToolItem *button = gtk_tool_button_new (icon, NULL);
	gtk_tool_item_set_expand (button, TRUE);
	return button;
}

const LauncherData *
today_get_launcher (const gchar *exec, gboolean use_sn, gboolean single)
{
	static LauncherData launcher_data;
	static gboolean first = TRUE;
	
	if (first) {
		launcher_data.argv = NULL;
		first = FALSE;
	}
	
	if (launcher_data.argv) g_free (launcher_data.argv);
	launcher_data.argv = exec_to_argv (exec);
	launcher_data.name = (gchar *)exec;
	launcher_data.description = "";
	launcher_data.icon = NULL;
	launcher_data.categories = (char *[]){ "" };
	launcher_data.use_sn = use_sn;
	launcher_data.single_instance = single;
	
	return &launcher_data;
}
