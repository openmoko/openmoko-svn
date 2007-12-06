/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by Soeren Apel (abraxa@dar-clan.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @file utils.h
 * Various utility functions
 */

#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

extern gchar *omp_ui_image_path;



GtkWidget *button_create_with_image(gchar *widget_name, gchar *image_name, GtkWidget **image, GCallback callback);
GtkWidget *widget_wrap(GtkWidget *widget, gchar *name);

void notebook_add_page_with_stock(GtkWidget *notebook, GtkWidget *child, const gchar *icon_name, int padding);

void error_dialog(gchar *message);
void error_dialog_modal(gchar *message);

gchar *get_base_file_name(gchar *file_name);

#endif
