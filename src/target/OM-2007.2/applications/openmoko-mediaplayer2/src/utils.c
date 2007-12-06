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
 * @file utils.c
 * Various utility functions
 */

#include <gtk/gtk.h>

#include "main.h"
#include "utils.h"

/// Absolute path to the UI pixmaps
gchar *omp_ui_image_path = NULL;



/**
 * Creates a button containing a stock image
 * @param widget_name Name to set for the button and image widgets
 * @param image_name Name of the stock image to use
 * @param image Destination for the image's handle (can be NULL)
 * @param callback Callback to set
 * @return The button
 */
GtkWidget *
button_create_with_image(gchar *widget_name, gchar *image_name, GtkWidget **image, GCallback callback)
{
	GtkWidget *btn_image, *button;

	button = gtk_button_new();
	gtk_widget_set_name(GTK_WIDGET(button), widget_name);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(callback), NULL);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(button), GTK_CAN_FOCUS);

	btn_image = gtk_image_new();
	gtk_widget_set_name(GTK_WIDGET(btn_image), widget_name);
	gtk_image_set_from_stock(GTK_IMAGE(btn_image), image_name, -1);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(btn_image));

	if (image) *image = btn_image;

	return button;
}

/**
 * Wraps a widget in an invisible GtkFrame so the widget can be padded using x/ythickness in the frame's style
 * @param widget Widget to be put inside the frame
 * @param name Name to assign to the frame, uses the widget's name if NULL
 * @return Frame containing the widget
 */
GtkWidget *
widget_wrap(GtkWidget *widget, gchar *name)
{
	GtkWidget *frame;

	frame = gtk_frame_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(frame), (name) ? name : gtk_widget_get_name(widget));
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(widget));

	return frame;
}

/**
 * Adds a child to a GtkNotebook, filling the page handle with a stock icon
 */
void
notebook_add_page_with_stock(GtkWidget *notebook, GtkWidget *child, const gchar *icon_name, int padding)
{
	GtkWidget *icon, *alignment;

	icon = gtk_image_new_from_icon_name(icon_name, -1);//GTK_ICON_SIZE_LARGE_TOOLBAR);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), padding, padding, padding, padding);
	gtk_container_add(GTK_CONTAINER(alignment), icon);
	gtk_widget_show_all(GTK_WIDGET(alignment));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), child, alignment);
	gtk_container_child_set(GTK_CONTAINER(notebook), child, "tab-expand", TRUE, NULL);
}

/**
 * Presents a simple non-modal error dialog to the user
 */
void
error_dialog(gchar *message)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(0,
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		"%s", message);

	// We don't want a title of "<unnamed>"
	gtk_window_set_title(GTK_WINDOW(dialog), " ");

	g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
	gtk_widget_show_all(dialog);
}

/**
 * Presents a simple modal error dialog to the user
 */
void
error_dialog_modal(gchar *message)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(0,
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		"%s", message);

	// We don't want a title of "<unnamed>"
	gtk_window_set_title(GTK_WINDOW(dialog), " ");

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/**
 * Utility function that removes the path and extension of a file name
 * @param file_name File name to extract title from, can contain a path
 * @return String holding the title, must be freed after use
 * @todo Make unicode safe
 * @note Yes, this is quick'n'dirty. It will be replaced.
 */
gchar *
get_base_file_name(gchar *file_name)
{
	gchar base[256];
	guint i, j, last_delim, extension_pos;

	// Find last directory delimiter
	last_delim = 0;
	for (i=0; file_name[i]; i++)
	{
		if (file_name[i] == '/') last_delim = i+1;
	}

	// Find file extension
	for(extension_pos = strlen(file_name);
		(extension_pos) && (file_name[extension_pos] != '.');
		extension_pos--);

	// Extract title
	for (j=0, i=last_delim; i<extension_pos; i++)
	{
		base[j++] = file_name[i];
	}
	base[j] = 0;
	
	return g_strdup((gchar *)&base);
}
