/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by the OpenMoko team
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

#include "guitools.h"

/// Absolute path to the UI pixmaps
extern gchar *ui_image_path = NULL;




/**
 * Loads an image from a file into a pixel buffer
 */
GdkPixbuf*
pixbuf_new_from_file(const gchar* file_name)
{
	gchar* image_file_name;
	GdkPixbuf* pixbuf = NULL;
	GError* error = NULL;

	image_file_name = g_strdup_printf("%s/%s", ui_image_path, file_name);

	if(g_file_test(image_file_name, G_FILE_TEST_EXISTS))
	{
		pixbuf = gdk_pixbuf_new_from_file(image_file_name, &error);
		if(!pixbuf)
		{
			g_print("File found but failed to load: %s\n", image_file_name);
			g_error_free(error);
		}
	} else {
		g_debug("Can't find %s\n", image_file_name);
	}

	g_free(image_file_name);
	return pixbuf;
}

/**
 * Creates a label with default properties, wraps it up in a GtkAlignment and returns the latter for direct use
 * @param label Will be filled with a GtkLabel
 * @param font_info The desired font to be used (e.g. "Times 10")
 * @param xalign Horizontal alignment (0..1)
 * @param yalign Vertical alignment (0..1)
 * @param xscale Horizontal expansion (0..1)
 * @param yscale Vertical expansion (0..1)
 * @note See gtk_alignment_new() for further info
 */
GtkWidget*
create_label(GtkWidget **label, gchar *font_info, gchar *color_desc,
	gfloat xalign, gfloat yalign, gfloat xscale, gfloat yscale,
	gint max_char_count)
{
	GdkColor color;
	PangoFontDescription *font_desc;

	GtkWidget *alignment = gtk_alignment_new(xalign, yalign, xscale, yscale);
	*label = gtk_label_new(NULL);

	font_desc = pango_font_description_from_string(font_info);
	gtk_widget_modify_font(*label, font_desc);
	pango_font_description_free(font_desc);
	gtk_label_set_width_chars(GTK_LABEL(*label), max_char_count);
	gtk_misc_set_alignment(GTK_MISC(*label), 0, 0.5);
	gtk_label_set_ellipsize(GTK_LABEL(*label), PANGO_ELLIPSIZE_END);

	gdk_color_parse(color_desc, &color);
	gtk_widget_modify_fg(GTK_WIDGET(*label), GTK_STATE_NORMAL, &color);

	gtk_container_add(GTK_CONTAINER(alignment), *label);

	return alignment;
}

/**
 * Loads an image from disk and adds it to a given container, returning a reference to the image as well
 */
void
container_add_image_with_ref(GtkContainer *container, gchar *image_name, GtkWidget **image)
{
	gchar *image_file_name;

	image_file_name = g_build_path("/", ui_image_path, image_name, NULL);
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
omp_notebook_add_page_with_icon(GtkWidget *notebook, GtkWidget *child, const gchar *icon_name, int padding)
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

