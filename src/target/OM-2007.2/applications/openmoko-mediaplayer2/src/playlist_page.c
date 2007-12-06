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
 * @file playlist_page.c
 * Playlist UI handling
 */

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-stock.h>

#include <string.h>

#include "guitools.h"
#include "main.h"
#include "persistent.h"
#include "playlist.h"
#include "playlist_page.h"
#include "utils.h"

/// Enumeration of the playlist list columns
enum
{
	TYPE_COLUMN,
	NAME_COLUMN,
	ACT_DELETE_COLUMN,
	FILE_NAME_COLUMN,    // This one isn't shown, it's for internal storage only
	COLUMN_COUNT
};

/// Input field where a new playlist name is entered
GtkWidget *omp_playlist_page_entry = NULL;

/// List store for the playlist selector
GtkListStore *omp_playlist_page_list_store = NULL;

// Just a forward declaration we don't want in the header file
void omp_playlist_page_list_populate();



/**
 * Called when a row was selected and queries user whether he wants to load selected playlist
 * @param playlist_name Name of the playlist
 * @param playlist_file_abs File name of playlist with absolute path
 */
void
omp_playlist_page_list_entry_select(gchar *playlist_name, gchar *playlist_file_abs)
{
	GtkWidget *dialog;
	gint dialog_result;

	// Get user confirmation
	dialog = gtk_message_dialog_new(0,
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		_("Load playlist '%s'?"), playlist_name);

	// We don't want a title of "<unnamed>"
	gtk_window_set_title(GTK_WINDOW(dialog), " ");

	dialog_result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (dialog_result == GTK_RESPONSE_YES)
	{
		// Load playlist with state reset to have sane playlist values
		omp_playlist_load(playlist_file_abs, TRUE);
	}

	// Clean up
	gtk_widget_destroy(dialog);
}

/**
 * Called after the user clicked the "delete" icon in a row of the list view
 * @param playlist_name Name of the playlist
 * @param playlist_file_abs File name of playlist with absolute path
 */
void
omp_playlist_page_list_entry_delete(gchar *playlist_name, gchar *playlist_file)
{
	GtkWidget *dialog;
	gint dialog_result;

	// Get user confirmation
	dialog = gtk_message_dialog_new(0,
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		_("Delete playlist '%s'?"), playlist_name);

	// We don't want a title of "<unnamed>"
	gtk_window_set_title(GTK_WINDOW(dialog), " ");

	dialog_result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (dialog_result == GTK_RESPONSE_YES)
	{
		// Delete playlist
		omp_playlist_delete(playlist_file);

		// Rebuild the list
		omp_playlist_page_list_populate();
	}

	// Clean up
	gtk_widget_destroy(dialog);
}

/**
 * Monitors general click events on the list view and acts appropriately
 */
gboolean
omp_playlist_page_list_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GtkTreePath *tree_path;
	GtkTreeViewColumn *tree_column;
	GtkTreeIter iterator;
	GtkTreeModel *model;
	GList *columns;
	gint column_id;
	gchar *playlist_name, *playlist_file, *playlist_file_abs;

	g_return_val_if_fail(GTK_IS_TREE_VIEW(widget), TRUE);

	// Find colum that was hit
	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), event->x, event->y,
		&tree_path, &tree_column, NULL, NULL);
	if (!tree_path) return TRUE;

	columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(widget));
	column_id = g_list_index(columns, (gpointer)tree_column);
	g_list_free(columns);

	// Find row that was hit
	model = GTK_TREE_MODEL(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_model_get_iter(model, &iterator, tree_path);

	// Select row so the selection gets updated right now
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), tree_path, NULL, FALSE);

	// Get playlist names
	gtk_tree_model_get(model, &iterator,
		NAME_COLUMN, &playlist_name,
		FILE_NAME_COLUMN, &playlist_file, -1);

	playlist_file_abs =
		g_build_filename(g_get_home_dir(), OMP_RELATIVE_PLAYLIST_PATH, playlist_file, NULL);

	// Determine what to do
	switch (column_id)
	{
		case NAME_COLUMN:
		{
			omp_playlist_page_list_entry_select(playlist_name, playlist_file_abs);
			break;
		}

		case ACT_DELETE_COLUMN:
		{
			omp_playlist_page_list_entry_delete(playlist_name, playlist_file_abs);
			break;
		}
	}

	// Clean up
	g_free(playlist_file_abs);
	g_free(playlist_file);
	g_free(playlist_name);
	gtk_tree_path_free(tree_path);

	return TRUE;
}

/**
 * Click handler for the "add new list" button
 */
void
omp_playlist_page_add_list(GtkButton *button, gpointer user_data)
{
	gchar *path, *file_name;
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(omp_playlist_page_entry));

	g_return_if_fail(name[0] != 0);

	// Playlist path is relative to user's home dir
	path = g_build_path("/", g_get_home_dir(), OMP_RELATIVE_PLAYLIST_PATH, NULL);
	file_name = g_strdup_printf("%s/%s.%s", path, name, OMP_PLAYLIST_FILE_EXTENSION);

	omp_playlist_create(file_name);
	omp_playlist_load(file_name, TRUE);

	// Show playlist editor because a user obviously wants
	// to edit a playlist that was just created
	omp_tab_show(OMP_TAB_PLAYLIST_EDITOR);
	omp_tab_focus(OMP_TAB_PLAYLIST_EDITOR);

	// Rebuild the list
	omp_playlist_page_list_populate();

	gtk_entry_set_text(GTK_ENTRY(omp_playlist_page_entry), "");
}

/**
 * Fills the playlist data model with names from the file system
 */
void
omp_playlist_page_list_populate()
{
	gchar *path, *dir_entry, *title;
	GDir *playlist_dir;
	GError *error;
	GtkTreeIter iterator;

	gtk_list_store_clear(omp_playlist_page_list_store);

	// Playlist path is relative to user's home dir
	path = g_build_path("/", g_get_home_dir(), OMP_RELATIVE_PLAYLIST_PATH, NULL);

	playlist_dir = g_dir_open(path, 0, &error);

	if (!playlist_dir)
	{
		g_printerr("Could not read playlist directory: %s\n", error->message);
		g_error_free(error);
		return;
	}

	do
	{
		dir_entry = (gchar*)g_dir_read_name(playlist_dir);

		// Add entry to list if it's valid
		if (dir_entry)
		{
			title = get_playlist_title(dir_entry);

			gtk_list_store_append(omp_playlist_page_list_store, &iterator);
			gtk_list_store_set(omp_playlist_page_list_store, &iterator,
				NAME_COLUMN, title,
				FILE_NAME_COLUMN, dir_entry, -1);

			g_free(title);
		}
	} while (dir_entry);

	g_dir_close(playlist_dir);
	g_free(path);
}

/**
 * Creates the playlist view
 * @param container Destination container of the view
 */
void
omp_playlist_page_list_create(GtkContainer *container)
{
	GtkWidget *tree_view;
	GtkTreeSelection *select;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GdkPixbuf *list_icon;

	// Create and populate data model
	omp_playlist_page_list_store = gtk_list_store_new(COLUMN_COUNT,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	omp_playlist_page_list_populate();

	// Create data view
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(omp_playlist_page_list_store));
	g_object_unref(G_OBJECT(omp_playlist_page_list_store));

	g_signal_connect(G_OBJECT(tree_view), "button-press-event",
		G_CALLBACK(omp_playlist_page_list_clicked), NULL);

	// Configure selection handler
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(select), GTK_SELECTION_SINGLE);

	// Set up columns
	list_icon = pixbuf_new_from_file("ico-playlists.png");
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer), "pixbuf", list_icon, NULL);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, "ellipsize-set", 1, NULL);
	column = gtk_tree_view_column_new_with_attributes(_("Playlist Name"), renderer,
		"text", NAME_COLUMN, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	// Column with "delete" icon
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer), "stock-id", "gtk-delete", NULL);
	column = gtk_tree_view_column_new_with_attributes("", renderer, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	// Add playlist view to container
	gtk_container_add(container, GTK_WIDGET(tree_view));
}

/**
 * Creates the playlist UI page and all its elements
 */
GtkWidget *
omp_playlist_page_create()
{
	GtkWidget *main_vbox, *scroll_box, *input_box, *button, *image, *alignment, *label;
	gchar *image_file_name;

	// Create main container
	main_vbox = gtk_vbox_new(FALSE, 0);

	// Caption #1
	alignment = label_create(&label, "Sans 6", "black", 0, 0, 0, 0, PANGO_ELLIPSIZE_NONE);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 5, 5, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(alignment), FALSE, FALSE, 0);
	gtk_label_set_text(GTK_LABEL(label), _("Select Playlist to load:"));

	// Playlist list viewport
	scroll_box = moko_finger_scroll_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(scroll_box), TRUE, TRUE, 0);

	// Create playlist view
	omp_playlist_page_list_create(GTK_CONTAINER(scroll_box));

	// Caption #2
	alignment = label_create(&label, "Sans 6", "black", 0, 0, 0, 0, PANGO_ELLIPSIZE_NONE);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 5, 5, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(alignment), FALSE, FALSE, 0);
	gtk_label_set_text(GTK_LABEL(label), _("Enter name to create a new playlist:"));

	// Add entry field for creation of a new playlist
	input_box = gtk_hbox_new(FALSE, 0);
	omp_playlist_page_entry = gtk_entry_new();
	button = gtk_button_new();

	image_file_name = g_build_filename(omp_ui_image_path, "ico-playlist-new.png", NULL);
	image = gtk_image_new_from_file(image_file_name);
	g_free(image_file_name);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(omp_playlist_page_add_list), NULL);

	gtk_box_pack_start(GTK_BOX(input_box), GTK_WIDGET(omp_playlist_page_entry), TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(input_box), GTK_WIDGET(button), FALSE, TRUE, 5);

	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(input_box), FALSE, TRUE, 10);

	// Make all widgets visible
	gtk_widget_show_all(main_vbox);

	return main_vbox;
}
