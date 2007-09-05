/* 
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef HAVE_LAUNCHER_UTIL_H
#define HAVE_LAUNCHER_UTIL_H

#include <glib.h>
#include <gtk/gtkicontheme.h>
#include <gtk/gtkwidget.h>

typedef struct {
  /*< public >*/
  char *name; /* Human-readable name */
  char *description; /* Description */
  char *icon; /* Icon name (can be NULL) */
  char **categories; /* Categories */
  /*< private >*/
  char **argv; /* argv to execute when starting this program */
  gboolean use_sn;
  gboolean single_instance;
} LauncherData;

LauncherData *launcher_parse_desktop_file (const char *filename, GError **error);

void launcher_start (GtkWidget *widget, const LauncherData *data);

char * launcher_get_icon (GtkIconTheme *icon_theme, LauncherData *data, int size);

void launcher_destroy (LauncherData *data);

#endif
