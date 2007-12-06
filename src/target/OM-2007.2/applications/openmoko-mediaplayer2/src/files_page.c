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
 * @file files_page.c
 * Playlist editor, part 2: file adding window
 */

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <string.h>

#include "files_page.h"
#include "guitools.h"
#include "persistent.h"
#include "playlist.h"
#include "utils.h"

/// Enumeration of the file list columns
enum
{
	ICON_COLUMN,
	NAME_COLUMN,
	ACT_ADD_COLUMN,
	COLUMN_COUNT
};

/// Enumeration of the types the list entries may get assigned
enum
{
	UNPLAYABLE_TYPE,
	FILE_TYPE,
	DIRECTORY_TYPE,
	MP3_TYPE,
	OGG_TYPE,
	TYPE_COUNT
};

/// Array holding the icons' GdkPixbufs so we can assign them to the list view rows
GdkPixbuf *omp_files_page_type_icons[TYPE_COUNT];

/// Array holding the file extensions the icons depict
gchar *omp_files_page_type_extensions[TYPE_COUNT];

/// Hash table allowing matching of file extensions to icons
GHashTable *omp_files_page_type_table = NULL;

/// List store for the file selector
GtkListStore *omp_files_page_list_store = NULL;

/// The label showing the current file system path
GtkWidget *omp_files_path_label = NULL;



/**
 * Finds a given file's extension and returns a pointer to its first char
 * @return A pointer to the first char of the extension, belongs to original string so do not free
 * @todo Make unicode safe?
 */
gchar *
get_file_extension(gchar *file_name)
{
	guint i;

	g_return_val_if_fail(file_name, NULL);

	for (i=strlen(file_name); (i>0) && (file_name[i]!='.'); i--);

	return file_name+i+1;
}

/**
 * Confirms to the user that the files have been added successfully
 * @param track_count Number of tracks that were added
 */
void
omp_files_page_success_report(guint track_count)
{
	GtkWidget *dialog;

	if (!track_count) return;

	if (track_count == 1)
	{
		dialog = gtk_message_dialog_new(0,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			_("File added successfully"));
	} else {
		dialog = gtk_message_dialog_new(0,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			_("Successfully added %d files"), track_count);
	}

	// We don't want a title of "<unnamed>"
	gtk_window_set_title(GTK_WINDOW(dialog), " ");

	g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
	gtk_widget_show_all(dialog);
}

/**
 * Moves one level upwards in the directory hierarchy
 * @todo Unicode compatibility
 */
void
omp_files_page_set_prev_dir(GtkButton *button, gpointer user_data)
{
	gchar *new_path;
	guint i;

	new_path = g_strdup(omp_session_get_file_chooser_path());

	// Find last directory level and remove it
	for (i=strlen(new_path); (i>0) && (new_path[i] != '/'); i--);

	if (new_path[i] == '/')
	{
		// Root directory is handled differently
		if (i == 0)
		{
			new_path[1] = 0;  // Directory becomes '/'
		} else {
			new_path[i] = 0;  // Last directory gets cut off
		}

		omp_session_set_file_chooser_path(new_path);
		omp_files_page_update_path();
	}

	g_free(new_path);
}

/**
 * Reads the directory contents of the current path and updates the view's model accordingly
 */
void
omp_files_page_update_path()
{
	gchar *path, *dir_entry, *dir_entry_abs, *temp;
	GDir *dir;
	GError *error;
	GtkTreeIter iterator;
	GdkPixbuf *icon;

	gtk_list_store_clear(omp_files_page_list_store);

	path = omp_session_get_file_chooser_path();
	gtk_label_set_text(GTK_LABEL(omp_files_path_label), path);

	if (path[0] == 0) return;
	dir = g_dir_open(path, 0, &error);

	if (!dir)
	{
		g_printerr("Could not read directory %s: %s\n", path, error->message);
		temp = g_strdup_printf(_("Could not read directory: %s"), error->message);
		error_dialog(temp);
		g_free(temp);

		g_error_free(error);
		return;
	}

	do
	{
		dir_entry = (gchar*)g_dir_read_name(dir);
		icon = NULL;

		if (!dir_entry) break;

		// Skip hidden entries
		if (dir_entry[0] == '.') continue;

		// Do we need the directory icon?
		dir_entry_abs = g_build_path("/", path, dir_entry, NULL);
		if (g_file_test(dir_entry_abs, G_FILE_TEST_IS_DIR))
		{
			icon = omp_files_page_type_icons[DIRECTORY_TYPE];
		}
		g_free(dir_entry_abs);

		// Determine icon through file extension
		if (!icon)
		{
			temp = g_ascii_strdown(get_file_extension(dir_entry), -1);
			icon = g_hash_table_lookup(omp_files_page_type_table, temp);
			g_free(temp);
		}

		if (!icon)
			icon = omp_files_page_type_icons[UNPLAYABLE_TYPE];

		// Add entry to list
		gtk_list_store_append(omp_files_page_list_store, &iterator);
		gtk_list_store_set(omp_files_page_list_store, &iterator,
			ICON_COLUMN, icon,
			NAME_COLUMN, dir_entry, -1);

	} while (TRUE);

	g_dir_close(dir);
}

/**
 * Monitors general click events on the list view and acts appropriately
 */
gboolean
omp_files_page_list_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GtkTreePath *tree_path;
	GtkTreeViewColumn *tree_column;
	GtkTreeIter iterator;
	GtkTreeModel *model;
	GList *columns;
	gint column_id;
	gchar *entry_name, *entry_name_abs;
	GdkPixbuf *icon;

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

	// Get file/dir name
	gtk_tree_model_get(model, &iterator,
		ICON_COLUMN, &icon,
		NAME_COLUMN, &entry_name, -1);

	entry_name_abs =
		g_build_filename(omp_session_get_file_chooser_path(), entry_name, NULL);

	// Determine what to do
	switch (column_id)
	{
		case NAME_COLUMN:
		{
			// Enter directory if entry is one
			if (icon == omp_files_page_type_icons[DIRECTORY_TYPE])
			{
				omp_session_set_file_chooser_path(entry_name_abs);
				omp_files_page_update_path();
			}

			break;
		}

		case ACT_ADD_COLUMN:
		{
			// Add dir/file
			if (icon == omp_files_page_type_icons[DIRECTORY_TYPE])
			{
				omp_files_page_success_report(
					omp_playlist_track_append_directory(entry_name_abs));

			} else {

				if (omp_playlist_track_append_file(entry_name_abs))
					omp_files_page_success_report(1);
			}

			// Save playlist
			omp_playlist_save();

			break;
		}
	}

	// Clean up
	g_free(entry_name);
	g_free(entry_name_abs);
	gtk_tree_path_free(tree_path);

	return TRUE;
}

/**
 * Compare function to determine sort order for the file list view
 */
gint
omp_files_page_view_compare_func(GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b,
	gpointer userdata)
{
	gchar *entry1, *entry2;
	GdkPixbuf *icon1, *icon2;
	gint order;

	// Directories come before files
	gtk_tree_model_get(model, a, ICON_COLUMN, &icon1, -1);
	gtk_tree_model_get(model, b, ICON_COLUMN, &icon2, -1);

	if ( (icon1 == omp_files_page_type_icons[DIRECTORY_TYPE]) ||
		   (icon2 == omp_files_page_type_icons[DIRECTORY_TYPE]) )
	{
		// Look at the names if both are directories
		if (icon1 == icon2)
		{
			gtk_tree_model_get(model, a, NAME_COLUMN, &entry1, -1);
			gtk_tree_model_get(model, b, NAME_COLUMN, &entry2, -1);
			order = g_utf8_collate(entry1, entry2);
			g_free(entry1);
			g_free(entry2);

		} else {

			// One is a dir and one is not, so no need to look at names
			order = (icon1 == omp_files_page_type_icons[DIRECTORY_TYPE]) ? -1 : 1;
		}

	} else {

		// Neither entry is a directory, so sort by name
		gtk_tree_model_get(model, a, NAME_COLUMN, &entry1, -1);
		gtk_tree_model_get(model, b, NAME_COLUMN, &entry2, -1);
		order = g_utf8_collate(entry1, entry2);
		g_free(entry1);
		g_free(entry2);
	}

	return order;
}

/**
 * Sets up the icons and file extensions used for distinguishing files in the list view
 */
void
omp_files_page_type_setup()
{
	// Load file type icons
	omp_files_page_type_icons[UNPLAYABLE_TYPE] =
		pixbuf_new_from_file("ico-filetype-unplayable.png");

	omp_files_page_type_icons[FILE_TYPE] =
		pixbuf_new_from_file("ico-filetype-generic.png");

	omp_files_page_type_icons[DIRECTORY_TYPE] =
		gtk_widget_render_icon(GTK_WIDGET(omp_window), "gtk-directory",
		GTK_ICON_SIZE_BUTTON, NULL);

	omp_files_page_type_icons[MP3_TYPE] =
		pixbuf_new_from_file("ico-filetype-mp3.png");

	omp_files_page_type_icons[OGG_TYPE] =
		pixbuf_new_from_file("ico-filetype-ogg.png");

	// Fill file extension array
	omp_files_page_type_extensions[MP3_TYPE] = g_strdup("mp3");
	omp_files_page_type_extensions[OGG_TYPE] = g_strdup("ogg");

	// Create and fill type hash table
	omp_files_page_type_table = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(omp_files_page_type_table,
		omp_files_page_type_extensions[MP3_TYPE], omp_files_page_type_icons[MP3_TYPE]);

	g_hash_table_insert(omp_files_page_type_table,
		omp_files_page_type_extensions[OGG_TYPE], omp_files_page_type_icons[OGG_TYPE]);
}

/**
 * Creates the file view
 * @param container Destination container of the view
 */
void
omp_files_page_list_create(GtkContainer *container)
{
	GtkWidget *tree_view;
	GtkTreeSelection *select;
	GtkTreeSortable *sortable;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	// Create data model
	omp_files_page_list_store = gtk_list_store_new(COLUMN_COUNT,
		GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);

	// Set up sorting
	sortable = GTK_TREE_SORTABLE(omp_files_page_list_store);
	gtk_tree_sortable_set_sort_func(sortable, 0, omp_files_page_view_compare_func, 0, NULL);
	gtk_tree_sortable_set_sort_column_id(sortable, 0, GTK_SORT_ASCENDING);

	// Create data view
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(omp_files_page_list_store));
	g_object_unref(G_OBJECT(omp_files_page_list_store));

	g_signal_connect(G_OBJECT(tree_view), "button-press-event",
		G_CALLBACK(omp_files_page_list_clicked), NULL);

	// Configure selection handler
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(select), GTK_SELECTION_SINGLE);

	// Set up columns
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer,
		"pixbuf", ICON_COLUMN, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, "ellipsize-set", 1, NULL);
	column = gtk_tree_view_column_new_with_attributes(_("File/Folder Name"), renderer,
		"text", NAME_COLUMN, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer), "stock-id", "gtk-add", NULL);
	column = gtk_tree_view_column_new_with_attributes("", renderer, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	// Add file view to container
	gtk_container_add(container, GTK_WIDGET(tree_view));
}

/**
 * Creates the file chooser UI page and all its elements
 */
GtkWidget *
omp_files_page_create()
{
	GtkWidget *main_vbox, *alignment, *label, *scroll_box, *hbox, *button, *image;

	// Create main container
	main_vbox = gtk_vbox_new(FALSE, 0);

	// HBox containing "back" button and path label
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);

	button = gtk_button_new();
	image = gtk_image_new_from_icon_name("gtk-undo-ltr", GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(omp_files_page_set_prev_dir), NULL);

	alignment = label_create(&label, "Sans 6", "black", 0.05, 0.5, 1, 0, PANGO_ELLIPSIZE_START);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	omp_files_path_label = label;

	// File list viewport
	scroll_box = moko_finger_scroll_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(scroll_box), TRUE, TRUE, 0);

	// Set up file types we distinguish
	omp_files_page_type_setup();

	// Create and populate file view
	omp_files_page_list_create(GTK_CONTAINER(scroll_box));

	// Make all widgets visible
	gtk_widget_show_all(main_vbox);

	return main_vbox;
}

/**
 * Frees resources used by the file chooser UI
 */
void
omp_files_page_free()
{
	guint i;

	g_hash_table_destroy(omp_files_page_type_table);

	for (i=0; i<TYPE_COUNT; i++)
		if (omp_files_page_type_extensions[i])
			g_free(omp_files_page_type_extensions[i]);

	for (i=0; i<TYPE_COUNT; i++)
		g_object_unref(omp_files_page_type_icons[i]);
}
