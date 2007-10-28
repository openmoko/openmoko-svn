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
 * @file guitools.c
 * Various helper functions to aid with GUI creation and handling
 */

#include <gtk/gtk.h>

#include "guitools.h"

/// Absolute path to the UI pixmaps
gchar *omp_ui_image_path = NULL;



/**
 * Creates a label with default properties, wraps it up in a GtkAlignment and returns the latter for direct use
 * @param label Will be filled with a GtkLabel
 * @param font_info The desired font to be used (e.g. "Times 10")
 * @param color_desc A string containing the name of the desired color (e.g. "black")
 * @param xalign Horizontal alignment (0..1)
 * @param yalign Vertical alignment (0..1)
 * @param xscale Horizontal expansion (0..1)
 * @param yscale Vertical expansion (0..1)
 * @param ellipsize_mode Pango ellipsization mode to set for this label
 * @note See gtk_alignment_new() for further info
 */
GtkWidget *
label_create(GtkWidget **label, gchar *font_info, gchar *color_desc,
	gfloat xalign, gfloat yalign, gfloat xscale, gfloat yscale,
	PangoEllipsizeMode ellipsize_mode)
{
	GdkColor color;
	PangoFontDescription *font_desc;

	GtkWidget *alignment = gtk_alignment_new(xalign, yalign, xscale, yscale);
	*label = gtk_label_new(NULL);

	font_desc = pango_font_description_from_string(font_info);
	gtk_widget_modify_font(*label, font_desc);
	pango_font_description_free(font_desc);
	gtk_misc_set_alignment(GTK_MISC(*label), 0, 0.5);

	if (ellipsize_mode != PANGO_ELLIPSIZE_NONE)
	{
		gtk_label_set_ellipsize(GTK_LABEL(*label), ellipsize_mode);
	}

	gdk_color_parse(color_desc, &color);
	gtk_widget_modify_fg(GTK_WIDGET(*label), GTK_STATE_NORMAL, &color);

	gtk_container_add(GTK_CONTAINER(alignment), *label);

	return alignment;
}

/**
 * Loads an image from a file into a pixel buffer
 * @return Pixbuf with the image, must be unref'd after use
 */
GdkPixbuf*
pixbuf_new_from_file(const gchar *file_name)
{
	gchar *image_file_name;
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;

	image_file_name = g_strdup_printf("%s/%s", omp_ui_image_path, file_name);

	if(g_file_test(image_file_name, G_FILE_TEST_EXISTS))
	{
		pixbuf = gdk_pixbuf_new_from_file(image_file_name, &error);
		if(!pixbuf)
		{
			g_printerr("File found but failed to load: %s\n", image_file_name);
			g_error_free(error);
		}
	} else {
		g_printerr("Can't find %s\n", image_file_name);
	}

	g_free(image_file_name);
	return pixbuf;
}

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
 * Loads an image from disk and adds it to a given container, returning a reference to the image as well
 */
void
container_add_image_with_ref(GtkContainer *container, gchar *image_name, GtkWidget **image)
{
	gchar *image_file_name;

	image_file_name = g_build_path("/", omp_ui_image_path, image_name, NULL);
	*image = gtk_image_new_from_file(image_file_name);
	g_free(image_file_name);
	gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(*image));
}

/**
 * Loads an image from disk and adds it to a given container
 */
void
container_add_image(GtkContainer *container, gchar *image_name)
{
	GtkWidget *image;
	container_add_image_with_ref(container, image_name, &image);
}

/**
 * Adds a child to a GtkNotebook, filling the page handle with a stock icon
 */
void
notebook_add_page_with_stock(GtkWidget *notebook, GtkWidget *child, const gchar *icon_name, int padding)
{
	GtkWidget *icon, *alignment;

	icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), padding, padding, padding, padding);
	gtk_container_add(GTK_CONTAINER(alignment), icon);
	gtk_widget_show_all(GTK_WIDGET(alignment));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), child, alignment);
	gtk_container_child_set(GTK_CONTAINER(notebook), child, "tab-expand", TRUE, NULL);
}

/**
 * Adds a child to a GtkNotebook, filling the page handle with an application-specific image
 */
void
notebook_add_page_with_image(GtkWidget *notebook, GtkWidget *child, const gchar *image_name, int padding)
{
	gchar *image_file_name;
	GtkWidget *icon, *alignment;

	image_file_name = g_build_path("/", omp_ui_image_path, image_name, NULL);
	icon = gtk_image_new_from_file(image_file_name);
	g_free(image_file_name);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), padding, padding, padding, padding);
	gtk_container_add(GTK_CONTAINER(alignment), icon);
	gtk_widget_show_all(GTK_WIDGET(alignment));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), child, alignment);
	gtk_container_child_set(GTK_CONTAINER(notebook), child, "tab-expand", TRUE, NULL);
}
