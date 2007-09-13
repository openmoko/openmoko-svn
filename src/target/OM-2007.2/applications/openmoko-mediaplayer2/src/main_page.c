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
 * @file main_page.c
 * Main UI handling
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "main_page.h"
#include "main.h"
#include "guitools.h"
#include "playlist.h"
#include "playback.h"
#include "persistent.h"

/// Contains all main window widgets that need to be changeable
struct _main_widgets
{
	GtkWidget *title_label;
	GtkWidget *artist_label;
	GtkWidget *track_number_label;
	GtkWidget *time_label;
	GtkWidget *time_hscale;
	GtkWidget *band_image[12];
	GtkWidget *volume_image;
	GtkWidget *volume_label;
	GtkWidget *balance_image;
	GtkWidget *play_pause_button_image;
	GtkWidget *shuffle_button_image;
	GtkWidget *repeat_button_image;
	GtkWidget *volume_hscale;
} main_widgets;

GtkWidget *omp_main_window = NULL;

/// Determines whether the time slider can be updated or not
gboolean omp_main_time_slider_can_update = TRUE;

/// Is toggled after the user finished dragging the time slider's button
gboolean omp_main_time_slider_was_dragged = FALSE;

// Forward declarations for internal use
void omp_main_playlist_loaded(gpointer instance, gchar *title, gpointer user_data);
void omp_main_update_track_change(gpointer instance, gpointer user_data);
void omp_main_update_track_info_changed(gpointer instance, guint track_id, gpointer user_data);
void omp_main_update_shuffle_state(gpointer instance, gboolean state, gpointer user_data);
void omp_main_update_repeat_mode(gpointer instance, guint mode, gpointer user_data);
void omp_main_update_status_change(gpointer instance, gpointer user_data);
void omp_main_update_track_position(gpointer instance, gpointer user_data);
void omp_main_update_volume(gpointer instance, gpointer user_data);



/**
 * Creates a button with a stock pixmap and returns it
 * @param image_name The name of the image resource to use, not a file name
 * @return The button
 */
GtkWidget *
omp_stock_button_create(gchar *image_name, GtkWidget **image, GCallback callback)
{
	GtkWidget *button;

	button = gtk_button_new();
	gtk_widget_set_size_request(GTK_WIDGET(button), 66, 66);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(callback), NULL);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(button), GTK_CAN_FOCUS);

	g_object_set(G_OBJECT(button), "xalign", (gfloat)0.37, "yalign", (gfloat)0.37, NULL);

	*image = gtk_image_new_from_icon_name(image_name, GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(*image));

	return button;
}

/**
 * Gets called when the time slider's value got changed (yes, that means it gets called at least once per second)
 */
void
omp_main_time_slider_changed(GtkRange *range, gpointer data)
{
	if (omp_main_time_slider_was_dragged)
	{
		omp_main_time_slider_was_dragged = FALSE;

		// Set new position and resume playback that was paused when dragging started
		omp_playback_set_track_position(gtk_range_get_value(GTK_RANGE(range)));

		// Update UI right away
		gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), omp_playback_get_track_position());
	}
}

/**
 * Gets called when the user starts dragging the time slider
 */
gboolean
omp_main_time_slider_drag_start(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	while (gtk_events_pending()) gtk_main_iteration();

	// Prevent UI callbacks from messing with the slider position
	omp_main_time_slider_can_update = FALSE;

	return FALSE;
}

/**
 * Gets called when the user stops dragging the time slider
 */
gboolean
omp_main_time_slider_drag_stop(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	while (gtk_events_pending()) gtk_main_iteration();

	// Allow UI callbacks to alter the the slider position again
	omp_main_time_slider_can_update = TRUE;

	// Notify the slider change callback that this time we indeed want to change position
	omp_main_time_slider_was_dragged = TRUE;

	return FALSE;
}

/**
 * Event handler for the "balance left" button
 * @todo Figure out how to set balance with gstreamer
 */
void
omp_main_balance_left_clicked(GtkWidget *widget, gpointer data)
{
	// ...
}

/**
 * Event handler for the "balance right" button
 * @todo Figure out how to set balance with gstreamer
 */
void
omp_main_balance_right_clicked(GtkWidget *widget, gpointer data)
{
	// ...
}

/**
 * Event handler for the Shuffle button
 */
void
omp_main_shuffle_clicked(GtkWidget *widget, gpointer data)
{
	omp_config_set_shuffle_state(!omp_config_get_shuffle_state());
}

/**
 * Event handler for the Repeat button
 */
void
omp_main_repeat_clicked(GtkWidget *widget, gpointer data)
{
	guint mode;

	// Cycle through all available modes
	mode = omp_config_get_repeat_mode()+1;

	if (mode == OMP_REPEAT_COUNT)
	{
		mode = 0;
	}

	omp_config_set_repeat_mode(mode);
}

/**
 * Event handler for the Fast Forward button
 */
void
omp_main_fast_forward_clicked(GtkWidget *widget, gpointer data)
{
	// Set new position and resume playback that was paused when dragging started
	omp_playback_set_track_position(omp_playback_get_track_position()+BUTTON_SEEK_DISTANCE);

	// Update UI right away
	gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), omp_playback_get_track_position());
}

/**
 * Event handler for the Rewind button
 */
void
omp_main_rewind_clicked(GtkWidget *widget, gpointer data)
{
	// Set new position and resume playback that was paused when dragging started
	omp_playback_set_track_position(omp_playback_get_track_position()-BUTTON_SEEK_DISTANCE);

	// Update UI right away
	gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), omp_playback_get_track_position());
}

/**
 * Event handler for the Play/Pause button
 */
void
omp_main_play_pause_clicked(GtkWidget *widget, gpointer data)
{
	if (omp_playback_get_state() != OMP_PLAYBACK_STATE_PLAYING)
	{
		omp_playback_play();

	} else {

		omp_playback_pause();
	}
}

/**
 * Gets called when the volume slider's value got changed
 */
void
omp_main_volume_slider_changed(GtkRange *range, gpointer data)
{
	omp_playback_set_volume(gtk_range_get_value(GTK_RANGE(range)));
}

/**
 * Resets the UI to a "no track loaded" state
 */
void
omp_main_reset_ui(gpointer instance, gpointer user_data)
{
	gchar *caption;

	gtk_label_set_text(GTK_LABEL(main_widgets.title_label), "No track info available");
	gtk_label_set_text(GTK_LABEL(main_widgets.artist_label), "");

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_NUM, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.track_number_label), caption);
	g_free(caption);

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_TIME, 0, 0, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.time_label), caption);
	g_free(caption);

	gtk_range_set_range(GTK_RANGE(main_widgets.time_hscale), 0, 1);
	gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), 0.0);
}

/**
 * Creates the main UI
 */
void
omp_main_widgets_create(GtkContainer *destination)
{
	GtkWidget *alignment;
	GtkWidget *mainvbox;
	GtkWidget *upper_hbox, *middle_hbox, *lower_hbox;
	GtkWidget *middle_right_vbox;
	GtkWidget *button;

	gchar *image_file_name, *caption;
	gint i;

	// Add mainvbox to destination container
	mainvbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(destination), GTK_WIDGET(mainvbox));

	// Title label
	alignment = label_create(&main_widgets.title_label, "Sans 8", "black", 0, 0, 1, 0, PANGO_ELLIPSIZE_END);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 10, 0, 35, 30);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// Artist label
	alignment = label_create(&main_widgets.artist_label, "Sans 6", "black", 0, 0, 1, 0, PANGO_ELLIPSIZE_END);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 35, 30);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// --- --- --- --- --- Upper hbox --- --- --- --- ---

	// Add upper_hbox to mainvbox for track number and time display
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 10, 0, 35, 30);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	upper_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(upper_hbox));

	// Track number icon
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 14, 15);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image(GTK_CONTAINER(alignment), "ico-track.png");

	// Track number
	alignment = label_create(&main_widgets.track_number_label, "Sans 5", "black", 0, 0, 0, 0, PANGO_ELLIPSIZE_NONE);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// Time icon
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 86, 15);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image(GTK_CONTAINER(alignment), "ico-time.png");

	// Time
	alignment = label_create(&main_widgets.time_label, "Sans 5", "black", 0, 0, 0, 0, PANGO_ELLIPSIZE_NONE);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// --- --- --- --- --- Slider --- --- --- --- ---

	// Time hscale
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 48, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	main_widgets.time_hscale = gtk_hscale_new_with_range(0.0, 338.0, 1.0);
	gtk_scale_set_draw_value(GTK_SCALE(main_widgets.time_hscale), FALSE);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(main_widgets.time_hscale), GTK_CAN_FOCUS);
	gtk_widget_set_size_request(GTK_WIDGET(main_widgets.time_hscale), 338, 35);
	gtk_range_set_update_policy(GTK_RANGE(main_widgets.time_hscale), GTK_UPDATE_DISCONTINUOUS);

	g_signal_connect(G_OBJECT(main_widgets.time_hscale), "value_changed",
		G_CALLBACK(omp_main_time_slider_changed), NULL);
	g_signal_connect(G_OBJECT(main_widgets.time_hscale), "button-press-event",
		G_CALLBACK(omp_main_time_slider_drag_start), NULL);
	g_signal_connect(G_OBJECT(main_widgets.time_hscale), "button-release-event",
		G_CALLBACK(omp_main_time_slider_drag_stop), NULL);

	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(main_widgets.time_hscale));

	// --- --- --- --- --- Middle hbox --- --- --- --- ---

	// Add middle_hbox to mainvbox for the EQ/volume/balance widgets
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 42, 0, 65, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	middle_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(middle_hbox));

	// EQ/Visualization bands
	image_file_name = g_build_path("/", omp_ui_image_path, "ind-music-eq-12.png", NULL);

	for (i=0; i<12; i++)
	{
		alignment = gtk_alignment_new(0, 0, 0, 0);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(middle_hbox), alignment, TRUE, TRUE, 0);
		main_widgets.band_image[i] = gtk_image_new_from_file(image_file_name);
		gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(main_widgets.band_image[i]));
	}

	g_free(image_file_name);

	// Add vbox containing volume and balance controls
	middle_right_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_hbox), GTK_WIDGET(middle_right_vbox), TRUE, TRUE, 0);

	// Volume hbox
	GtkWidget* volume_box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_right_vbox), volume_box, TRUE, TRUE, 0);

	// Volume image
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 7, 0, 20, 0);
	gtk_box_pack_start(GTK_BOX(volume_box), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image_with_ref(GTK_CONTAINER(alignment), "ind-music-volume-00.png", &main_widgets.volume_image);

	// Volume label
	alignment = label_create(&main_widgets.volume_label, "Sans 5", "darkorange", 0, 0, 1, 0, PANGO_ELLIPSIZE_NONE);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 0, 10, 0);
	gtk_box_pack_start(GTK_BOX(volume_box), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	caption = g_strdup_printf(OMP_WIDGET_CAPTION_VOLUME, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.volume_label), caption);
	g_free(caption);

	//  Balance image
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 35, 0);
	gtk_box_pack_start(GTK_BOX(middle_right_vbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image_with_ref(GTK_CONTAINER(alignment), "ind-music-pan-0.png", &main_widgets.balance_image);

	// --- --- --- --- --- Lower hbox --- --- --- --- ---

	// Add lower hbox containing the shuffle/repeat/balance buttons
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 25, 0, 25, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);

	lower_hbox = gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(lower_hbox), 4);
	gtk_container_add(GTK_CONTAINER(alignment), lower_hbox);

	// "balance left" button
	button = button_create_with_image("ico-balance-left.png", NULL,
		G_CALLBACK(omp_main_balance_left_clicked));
	gtk_box_pack_start(GTK_BOX(lower_hbox), button, TRUE, TRUE, 0);

	// Shuffle toggle button
	button = button_create_with_image("ico-shuffle-off.png",
		&main_widgets.shuffle_button_image, G_CALLBACK(omp_main_shuffle_clicked));
	gtk_box_pack_start(GTK_BOX(lower_hbox), button, TRUE, TRUE, 0);

	// Repeat toggle button
	button = button_create_with_image("ico-repeat-off.png",
		&main_widgets.repeat_button_image, G_CALLBACK(omp_main_repeat_clicked));
	gtk_box_pack_start(GTK_BOX(lower_hbox), button, TRUE, TRUE, 0);

	// "balance right" button
	button = button_create_with_image("ico-balance-right.png", NULL,
		G_CALLBACK(omp_main_balance_right_clicked));
	gtk_box_pack_start(GTK_BOX(lower_hbox), button, TRUE, TRUE, 0);
}

/**
 * Creates the widgets below the UI background image
 */
void
omp_main_secondary_widgets_create(GtkContainer *destination)
{
	GtkWidget *alignment;
	GtkWidget *mainvbox;
	GtkWidget *hbox;
	GtkWidget *image, *button, *vol_scale;

	// Add mainvbox to destination container
	mainvbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(destination), GTK_WIDGET(mainvbox));

	// --- --- --- --- --- Player controls --- --- --- --- --- ---

	alignment = gtk_alignment_new(0, 0, 1, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 25, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);

	hbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_add(GTK_CONTAINER(alignment), hbox);

	// Previous Track button
	button = omp_stock_button_create("gtk-media-previous-ltr", &image,
		G_CALLBACK(omp_playlist_set_prev_track));
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	// Rewind button
	button = omp_stock_button_create("gtk-media-rewind-ltr", &image,
		G_CALLBACK(omp_main_rewind_clicked));
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	// Play/Pause button
	button = omp_stock_button_create("gtk-media-play-ltr", &main_widgets.play_pause_button_image,
		G_CALLBACK(omp_main_play_pause_clicked));
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	// Fast Forward button
	button = omp_stock_button_create("gtk-media-forward-ltr", &image,
		G_CALLBACK(omp_main_fast_forward_clicked));
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	// Next Track button
	button = omp_stock_button_create("gtk-media-next-ltr", &image,
		G_CALLBACK(omp_playlist_set_next_track));
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	// --- --- --- --- --- Volume control --- --- --- --- --- ---
	
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 10, 0, 71, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);

	// Volume hscale
	vol_scale = gtk_hscale_new_with_range(0.0, 100.1, 1.0);
	gtk_scale_set_draw_value(GTK_SCALE(vol_scale), FALSE);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(vol_scale), GTK_CAN_FOCUS);
	gtk_widget_set_size_request(GTK_WIDGET(vol_scale), 338, 35);
	gtk_range_set_update_policy(GTK_RANGE(vol_scale), GTK_UPDATE_CONTINUOUS);
	g_signal_connect(G_OBJECT(vol_scale), "value_changed", G_CALLBACK(omp_main_volume_slider_changed), NULL);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(vol_scale));
	main_widgets.volume_hscale = vol_scale;
}

/**
 * Creates the main UI page and all its elements
 */
GtkWidget *
omp_main_page_create()
{
	GtkWidget *alignment, *bg_muxer;

	bg_muxer = gtk_fixed_new();

	// Background image
	alignment = gtk_alignment_new(0, 0, 0, 0);
	container_add_image(GTK_CONTAINER(alignment), "background.png");
	gtk_fixed_put(GTK_FIXED(bg_muxer), GTK_WIDGET(alignment), 15, 10);
	
	// Create all widgets
	alignment = gtk_alignment_new(0, 0, 0, 0);
	omp_main_widgets_create(GTK_CONTAINER(alignment));
	gtk_fixed_put(GTK_FIXED(bg_muxer), GTK_WIDGET(alignment), 20, 26);

	alignment = gtk_alignment_new(0, 0, 0, 0);
	omp_main_secondary_widgets_create(GTK_CONTAINER(alignment));
	gtk_fixed_put(GTK_FIXED(bg_muxer), GTK_WIDGET(alignment), 0, 420);

	omp_main_reset_ui(NULL, NULL);


	// Set up playlist signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_LOADED,
		G_CALLBACK(omp_main_playlist_loaded), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED,
		G_CALLBACK(omp_main_update_track_change), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
		G_CALLBACK(omp_main_update_track_info_changed), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED,
		G_CALLBACK(omp_main_update_track_change), NULL);

	// Set up configuration signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_SHUFFLE_STATE_CHANGED,
		G_CALLBACK(omp_main_update_shuffle_state), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_REPEAT_MODE_CHANGED,
		G_CALLBACK(omp_main_update_repeat_mode), NULL);

	// Set up playback signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_RESET,
		G_CALLBACK(omp_main_reset_ui), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_STATUS_CHANGED,
		G_CALLBACK(omp_main_update_status_change), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_POSITION_CHANGED,
		G_CALLBACK(omp_main_update_track_position), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_VOLUME_CHANGED,
		G_CALLBACK(omp_main_update_volume), NULL);

	// Make all widgets visible
	gtk_widget_show_all(bg_muxer);

	return bg_muxer;
}

/**
 * Callback for the "playlist loaded" event
 */
void
omp_main_playlist_loaded(gpointer instance, gchar *list_title, gpointer user_data)
{
	// Playlist editor can now be used
	omp_tab_show(OMP_TAB_PLAYLIST_EDITOR);
}

/**
 * Evaluates current track information and updates the config/UI if necessary
 * @note This function only checks elements that don't change too often
 * @note For the rest we have specialized functions below
 */
void
omp_main_update_track_change(gpointer instance, gpointer user_data)
{
	static gint old_track_count = 0;
	static gint old_track_id = 0;
	static gulong old_track_length = 0;

	gulong track_length, track_position;
	gchar *artist = NULL;
	gchar *title = NULL;
	gchar *text;
	gint track_id;

	// Set preliminary artist/title strings (updated on incoming metadata)
	omp_playlist_get_track_info(omp_playlist_current_track_id, &artist, &title, &track_length);
	gtk_label_set_text(GTK_LABEL(main_widgets.artist_label), artist);
	gtk_label_set_text(GTK_LABEL(main_widgets.title_label), title);
	if (artist) g_free(artist);
	if (title) g_free(title);

	// Track id/track count changed?
	if (
			(omp_playlist_track_count != old_track_count) ||
			(omp_playlist_current_track_id != old_track_id)
		 )
	{
		old_track_count = omp_playlist_track_count;
		old_track_id = omp_playlist_current_track_id;

		// Update label
		track_id = (omp_playlist_track_count) ? omp_playlist_current_track_id+1 : 0;
		text = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_NUM, track_id, omp_playlist_track_count);
		gtk_label_set_text(GTK_LABEL(main_widgets.track_number_label), text);
		g_free(text);
	}

	// Got a track length change?
	if ( (track_length) && (track_length != old_track_length) )
	{
		old_track_length = track_length;
		track_position = omp_playback_get_track_position();

		// Update label and slider
		text = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_TIME,
			(guint)(track_position / 60000), (guint)(track_position/1000) % 60,
			(guint)(track_length / 60000), (guint)(track_length/1000) % 60);
		gtk_label_set_text(GTK_LABEL(main_widgets.time_label), text);
		g_free(text);

		if (omp_main_time_slider_can_update)
		{
			// We don't want to set both min/max to 0 as this triggers a
			// critial GTK warning, so we set 0/1 instead in that case
			gtk_range_set_range(GTK_RANGE(main_widgets.time_hscale), 0, track_length ? track_length : 1);
			gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), track_position);

			// Set new time slider increments: one tap is 10% of the track's playing time
			gtk_range_set_increments(GTK_RANGE(main_widgets.time_hscale), track_length/10, track_length/10);
		}
	}
}

/**
 * Updates the UI when the track's meta data changed
 */
void
omp_main_update_track_info_changed(gpointer instance, guint track_id, gpointer user_data)
{
	omp_main_update_track_change(NULL, NULL);
}

/**
 * Updates the UI after a change to the shuffle flag
 */
void
omp_main_update_shuffle_state(gpointer instance, gboolean state, gpointer user_data)
{
	gchar *image_file_name;

	if (state)
	{
		image_file_name = g_build_filename(omp_ui_image_path, "ico-shuffle-on.png", NULL);
	} else {
		image_file_name = g_build_filename(omp_ui_image_path, "ico-shuffle-off.png", NULL);
	}

	gtk_image_set_from_file(GTK_IMAGE(main_widgets.shuffle_button_image), image_file_name);
	g_free(image_file_name);
}

/**
 * Updates the UI after the repeat mode changed
 */
void
omp_main_update_repeat_mode(gpointer instance, guint mode, gpointer user_data)
{
	gchar *image_file_name = NULL;

	switch (mode)
	{
		case OMP_REPEAT_OFF:
			image_file_name = g_build_filename(omp_ui_image_path, "ico-repeat-off.png", NULL);
			break;

		case OMP_REPEAT_ONCE:
			image_file_name = g_build_filename(omp_ui_image_path, "ico-repeat-once.png", NULL);
			break;

		case OMP_REPEAT_CURRENT:
			image_file_name = g_build_filename(omp_ui_image_path, "ico-repeat-current.png", NULL);
			break;

		case OMP_REPEAT_ALL:
			image_file_name = g_build_filename(omp_ui_image_path, "ico-repeat-all.png", NULL);
	}

	gtk_image_set_from_file(GTK_IMAGE(main_widgets.repeat_button_image), image_file_name);
	g_free(image_file_name);
}

/**
 * Updates the UI after a switch between "paused" and "playing" modes
 */
void
omp_main_update_status_change(gpointer instance, gpointer user_data)
{
	// Update Play/Pause button pixmap
	if (omp_playback_get_state() == OMP_PLAYBACK_STATE_PAUSED)
	{
		gtk_image_set_from_icon_name(GTK_IMAGE(main_widgets.play_pause_button_image),
			"gtk-media-play-ltr", GTK_ICON_SIZE_BUTTON);

	} else {

		gtk_image_set_from_icon_name(GTK_IMAGE(main_widgets.play_pause_button_image),
			"gtk-media-pause", GTK_ICON_SIZE_BUTTON);
	}
}

/**
 * Updates the UI if the playback position changed
 */
void
omp_main_update_track_position(gpointer instance, gpointer user_data)
{
	static gulong old_track_position = 0;

	gulong track_position, track_length;
	gchar *text;

	// Got a track position change?
	track_position = omp_playback_get_track_position();

	if (track_position != old_track_position)
	{
		old_track_position = track_position;
		track_length = omp_playback_get_track_length();

		// Update UI
		text = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_TIME,
			(guint)(track_position / 60000), (guint)(track_position/1000) % 60,
			(guint)(track_length / 60000), (guint)(track_length/1000) % 60);
		gtk_label_set_text(GTK_LABEL(main_widgets.time_label), text);
		g_free(text);

		if (omp_main_time_slider_can_update)
		{
			// We don't want to set both min/max to 0 as this triggers a
			// critial GTK warning, so we set 0/1 instead in that case
			gtk_range_set_range(GTK_RANGE(main_widgets.time_hscale), 0, track_length ? track_length : 1);
			gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), track_position);

			// Set new time slider increments: one tap is 10% of the track's playing time
			gtk_range_set_increments(GTK_RANGE(main_widgets.time_hscale), track_length/10, track_length/10);
		}
	}

}

/**
 * Updates the UI due to a change in playback volume
 */
void
omp_main_update_volume(gpointer instance, gpointer user_data)
{
	gchar *text, *image_file_name;
	guint volume;

	volume = omp_playback_get_volume();

	image_file_name = g_strdup_printf("%s/ind-music-volume-%02d.png", omp_ui_image_path, volume/10);
	gtk_image_set_from_file(GTK_IMAGE(main_widgets.volume_image), image_file_name);
	g_free(image_file_name);

	text = g_strdup_printf(OMP_WIDGET_CAPTION_VOLUME, volume);
	gtk_label_set_text(GTK_LABEL(main_widgets.volume_label), text);
	g_free(text);

	gtk_range_set_value(GTK_RANGE(main_widgets.volume_hscale), volume);
}
