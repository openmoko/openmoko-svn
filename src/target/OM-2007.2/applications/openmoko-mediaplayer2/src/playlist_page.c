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
 * @file playlist_page.c
 * Playlist UI handling
 */

#include <gtk/gtk.h>
#include <libmokoui2/moko-finger-scroll.h>

#include "playlist_page.h"
#include "main.h"

/// Enumeration for the list columns
enum
{
	ICON_COLUMN,
	NAME_COLUMN,
	ACTION_COLUMN,
	COLUMN_COUNT
};

/// The input field where a new playlist name is entered
GtkWidget *omp_playlist_page_entry;

/**
 * Creates the playlist view
 */
void
omp_playlist_page_list_create(GtkContainer *container)
{
	GtkListStore *store;
	GtkWidget *tree_view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	// Create and populate data model
	store = gtk_list_store_new(1, G_TYPE_STRING);

	// Create data view
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, BUTTON_PIXMAP_SIZE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Playlist Name", renderer,
		"name", NAME_COLUMN, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("Actions", renderer, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 2*BUTTON_PIXMAP_SIZE+10);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	// Add view to container
	gtk_container_add(container, GTK_WIDGET(tree_view));
}

/**
 * Create the playlist UI page and all its elements
 */
GtkWidget *
omp_playlist_page_create(GtkWindow *window)
{
	GtkWidget *main_vbox, *scroll_box, *input_box, *button, *image;

	// Create main container
	main_vbox = gtk_vbox_new(FALSE, 0);

	scroll_box = moko_finger_scroll_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(scroll_box), TRUE, TRUE, 0);

	// Create playlist view
	omp_playlist_page_list_create(GTK_CONTAINER(scroll_box));

	// Add entry field for creation of a new playlist
	input_box = gtk_hbox_new(FALSE, 0);
	omp_playlist_page_entry = gtk_entry_new();
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name("gtk-add", BUTTON_PIXMAP_SIZE);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));

	gtk_box_pack_start(GTK_BOX(input_box), GTK_WIDGET(omp_playlist_page_entry), TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(input_box), GTK_WIDGET(button), FALSE, TRUE, 5);

	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(input_box), FALSE, TRUE, 10);

	return main_vbox;
}

