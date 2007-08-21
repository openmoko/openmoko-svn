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
 * @file editor_page.c
 * Playlist editor, part 1: main editor
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libmokoui2/moko-finger-scroll.h>

#include "editor_page.h"
#include "playlist.h"
#include "guitools.h"

/// Enumeration for the track list columns
enum
{
	TYPE_COLUMN,
	NUMBER_COLUMN,
	TITLE_COLUMN,
	DURATION_COLUMN,
	COLUMN_COUNT
};

/// List store for the playlist selector
GtkListStore *omp_editor_page_list_store = NULL;

/// The label showing the editor's title caption
GtkWidget *omp_editor_title_label = NULL;

// Forward declarations for internal use
void omp_editor_page_list_populate();



/**
 * Gets called when a new playlist was loaded - we then update the editor window accordingly
 */
void
omp_editor_page_playlist_loaded(gpointer instance, gchar *title, gpointer user_data)
{
	gchar *text;

	text = g_strdup_printf(_(OMP_WIDGET_CAPTION_EDITOR), title);
	gtk_label_set_text(GTK_LABEL(omp_editor_title_label), text);
	g_free(text);

	omp_editor_page_list_populate();
}

/**
 * Updates a track's title and duration upon arrival of metadata
 */
void
omp_editor_page_update_track_info(gpointer instance, guint track_id, gpointer user_data)
{
	guint duration;
	gchar *track_title, *track_duration, *path;
	GtkTreeIter tree_iter;

	omp_playlist_get_track_info(track_id, &track_title, &duration);

	if (duration > 0)
	{
		track_duration = g_strdup_printf(OMP_WIDGET_CAPTION_EDITOR_TRACK_TIME,
			duration / 60000, (duration/1000) % 60);
	} else {
		track_duration = NULL;
	}

	path = g_strdup_printf("%d", track_id);
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(omp_editor_page_list_store),
		&tree_iter, path);

	gtk_list_store_set(omp_editor_page_list_store, &tree_iter,
		TITLE_COLUMN, track_title,
		DURATION_COLUMN, track_duration, -1);

	g_free(path);
	g_free(track_duration);
}

/**
 * Callback for the "add track" button
 */
void
omp_editor_page_add_clicked(gpointer instance, gpointer user_data)
{
	
}

/**
 * Monitors general click events on the list view and acts appropriately
 */
gboolean
omp_editor_page_list_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	
}

/**
 * Fills the data model with track titles from the playlist
 */
void
omp_editor_page_list_populate()
{
	GtkTreeIter tree_iter;
	guint track_num, duration;
	gchar *track_title, *track_duration;
	struct playlist_iter *pl_iter;

	gtk_list_store_clear(omp_editor_page_list_store);

	pl_iter = omp_playlist_init_iterator();

	// Iterate over the playlist and gather track infos to fill the list with
	while (!omp_playlist_iter_finished(pl_iter))
	{
		omp_playlist_get_track_from_iter(pl_iter, &track_num, &track_title, &duration);

		if (duration > 0)
		{
			track_duration = g_strdup_printf(OMP_WIDGET_CAPTION_EDITOR_TRACK_TIME,
				duration / 60000, (duration/1000) % 60);
		} else {
			track_duration = g_strdup("");
		}

		gtk_list_store_append(omp_editor_page_list_store, &tree_iter);
		gtk_list_store_set(omp_editor_page_list_store, &tree_iter,
			NUMBER_COLUMN, track_num+1,
			TITLE_COLUMN, track_title,
			DURATION_COLUMN, track_duration, -1);

		g_free(track_duration);
		g_free(track_title);

		omp_playlist_advance_iter(pl_iter);
	}
}

/**
 * Creates the track view
 * @param container Destination container of the view
 */
void
omp_editor_page_list_create(GtkContainer *container)
{
	GtkWidget *tree_view;
	GtkTreeSelection *select;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GdkPixbuf *track_icon;

	track_icon = pixbuf_new_from_file("ico-tracktype-general.png");

	// Create data model
	omp_editor_page_list_store = gtk_list_store_new(COLUMN_COUNT,
		G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

	// Create data view
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(omp_editor_page_list_store));
	g_object_unref(G_OBJECT(omp_editor_page_list_store));

	g_signal_connect(G_OBJECT(tree_view), "button-press-event",
		G_CALLBACK(omp_editor_page_list_clicked), NULL);

	// Configure selection handler
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(select), GTK_SELECTION_SINGLE);

	// Set up columns
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer), "pixbuf", track_icon, NULL);
	column = gtk_tree_view_column_new_with_attributes("", renderer, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, BUTTON_PIXMAP_SIZE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("#"), renderer,
		"text", NUMBER_COLUMN, NULL);
	gtk_tree_view_column_set_fixed_width(column, BUTTON_PIXMAP_SIZE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Track Title"), renderer,
		"text", TITLE_COLUMN, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Duration"), renderer,
		"text", DURATION_COLUMN, NULL);
	gtk_tree_view_column_set_fixed_width(column, 2*BUTTON_PIXMAP_SIZE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	// Add playlist view to container
	gtk_container_add(container, GTK_WIDGET(tree_view));
}

/**
 * Creates the playlist editor UI page and all its elements
 */
GtkWidget *
omp_editor_page_create()
{
	GtkWidget *main_vbox, *alignment, *label, *scroll_box, *hbox, *button, *image;

	// Create main container
	main_vbox = gtk_vbox_new(FALSE, 0);

	// Caption
	alignment = create_label(&label, "Sans 14", "black", 0, 0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 5, 5, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(alignment), FALSE, FALSE, 0);
	omp_editor_title_label = label;

	// Track list viewport
	scroll_box = moko_finger_scroll_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(scroll_box), TRUE, TRUE, 0);

	// Create track view
	omp_editor_page_list_create(GTK_CONTAINER(scroll_box));

	// Add "add tracks" button
	alignment = gtk_alignment_new(0, 0, 1, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 5, 5, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(alignment), FALSE, TRUE, 0);

	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(button));
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(hbox));

	image = gtk_image_new_from_icon_name("gtk-file", BUTTON_PIXMAP_SIZE);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(image), TRUE, TRUE, 0);

	alignment = create_label(&label, "Sans 14", "black", 0, 0, 0, 0, 0);
	gtk_label_set_text(GTK_LABEL(label), _("Add Tracks"));
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(omp_editor_page_add_clicked), NULL);

	// Make all widgets visible
	gtk_widget_show_all(main_vbox);

	// Set up signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_LOADED,
		G_CALLBACK(omp_editor_page_playlist_loaded), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
		G_CALLBACK(omp_editor_page_update_track_info), NULL);

	return main_vbox;
}
