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
 * @file mainwin.c
 * Main window handling
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "mainwin.h"
#include "main.h"
#include "guitools.h"
#include "playlist.h"
#include "playback.h"

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
	GtkWidget *play_pause_button;
	GtkWidget *shuffle_button;
	GtkWidget *repeat_button;
	GtkWidget *playlist_button;
} main_widgets;

GtkWidget *omp_main_window = NULL;



/**
 * Terminate the entire program
 */
void
omp_application_terminate()
{
	// Free resources
	g_free(ui_image_path);

	// Tell GTK to leave the message loop
	gtk_main_quit();
}

/**
 * Program termination event triggered by main window
 */
void
omp_main_quit(GtkWidget* widget, gpointer data)
{
	omp_application_terminate();
}

/**
 * Updates the UI volume display
 * @param vol Volume to show, in percent
 */
void
omp_change_vol_img(gint vol)
{
	// Sanity checks
	if ( (vol < 0) || (vol > 100) )
		g_printerr("Warning: volume passed to omp_change_vol_img() out of bounds\n");

	gchar *image_file_name = g_strdup_printf("%s/ind-music-volume-%02d.png", ui_image_path, vol/10);
	gtk_image_set_from_file(GTK_IMAGE(main_widgets.volume_image), image_file_name);

	g_free(image_file_name);
}

/**
 * Sets a desired EQ/visualization band to a new level
 * @param pos Band to change (0..11)
 * @param h Level to set (0..15, anything higher gets capped)
 * @note The switch is supposed to make the levels pseudo-logarithmic?
 */
void
omp_update_band(gint pos, gint level)
{
	// Sanity checks
	g_return_if_fail( (pos < 0) && (pos > 11) );
	g_return_if_fail(level > -1);

	// Pseudo-logarithmize the value
	gint value = 0;

	switch (level)
	{
		case 0:
		case 1:
		case 2:
			value = 1;
			break;

		case 3:
		case 4:
			value = 2;
			break;

		case 5:
		case 6:
			value = 3;
			break;

		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			value = level-3;
			break;

		default:
			value = 12;
	}

	// Determine file name of the new image to use and apply it
	gchar *image_file_name = NULL;
	image_file_name = g_strdup_printf("%s/ind-music-eq-%02d.png", ui_image_path, value);

	gtk_image_set_from_file(GTK_IMAGE(main_widgets.band_image[pos]), image_file_name);
//	gtk_widget_show(GTK_IMAGE(main_widgets.band_image[pos]));

	g_free(image_file_name);
}

/**
 * Set artist label [Mockup arrow #1 - upper line]
 */
void
omp_set_artist(const gchar* artist)
{
	if (!artist)
	{
		gtk_label_set_text(GTK_LABEL(main_widgets.artist_label), "Unknown Artist");
		return;
	}

	gtk_label_set_text(GTK_LABEL(main_widgets.artist_label), artist);
}

/**
 * Set title label [Mockup arrow #1 - lower line]
 */
void
omp_set_title(const gchar *title)
{
	if (!title)
	{
		gtk_label_set_text(GTK_LABEL(main_widgets.title_label), "Unknown Title");
		return;
	}

	gtk_label_set_text(GTK_LABEL(main_widgets.title_label), title);
}

void
omp_shuffle_button_callback(GtkWidget* widget, gpointer data)
{
/*    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
	cfg.shuffle = TRUE;	    
//	playlist_set_shuffle(TRUE);
    }
    else
    {
	cfg.shuffle= FALSE;
//	playlist_set_shuffle(FALSE);
    } */
}

void
omp_repeat_button_callback(GtkWidget* widget, gpointer data)
{
    /*if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
	cfg.repeat = TRUE;
    }
    else
    {
	cfg.repeat = FALSE;
    }*/
}

void
omp_playlist_button_callback(GtkWidget *sender, gpointer data)
{
	// ...
}

/**
 * Event handler for the Play/Pause button
 * @todo State change, etc
 */
void
omp_main_button_play_pause_callback()
{
	omp_playback_play();
}

/**
 * Creates a toggle button framed by a GtkAlignment
 */
GtkWidget*
omp_toggle_button_create(gchar *image_name, gint pad_left, GCallback callback, GtkWidget **button)
{
	GtkWidget *image;
	gchar *image_file_name;

	GtkWidget *alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, pad_left, 0);

	*button = gtk_toggle_button_new();
	gtk_widget_set_size_request(GTK_WIDGET(*button), 66, 66);
	gtk_widget_set_name(GTK_WIDGET(*button), "mokofingerbutton-white");
	g_signal_connect(G_OBJECT(*button), "clicked", G_CALLBACK(callback), NULL);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(*button), GTK_CAN_FOCUS);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(*button));

	g_object_set(G_OBJECT(*button), "xalign", (gfloat)0.37, "yalign", (gfloat)0.37, NULL);

	image_file_name = g_build_path("/", ui_image_path, image_name, NULL);
	image = gtk_image_new_from_file(image_file_name);
	g_free(image_file_name);
	gtk_container_add(GTK_CONTAINER(*button), GTK_WIDGET(image));

	return alignment;
}

/**
 * Creates a button and returns it
 */
GtkWidget*
omp_button_create(gchar *image_name, GCallback callback)
{
	GtkWidget *image, *button;
	gchar *image_file_name;

	button = gtk_button_new();
	gtk_widget_set_size_request(GTK_WIDGET(button), 66, 66);
	gtk_widget_set_name(GTK_WIDGET(button), "mokofingerbutton-white");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(callback), NULL);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(button), GTK_CAN_FOCUS);

	g_object_set(G_OBJECT(button), "xalign", (gfloat)0.37, "yalign", (gfloat)0.37, NULL);

	image_file_name = g_build_path("/", ui_image_path, image_name, NULL);
	image = gtk_image_new_from_file(image_file_name);
	g_free(image_file_name);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));

	return button;
}

/**
 * Show the main window, create it first if necessary
 */
void
omp_main_window_show()
{
	if (!omp_main_window)
	{
		omp_main_window_create();
	}

	gtk_widget_show(GTK_WIDGET(omp_main_window));
}

/**
 * Hide the main window
 */
void
omp_main_window_hide()
{
	gtk_widget_hide(GTK_WIDGET(omp_main_window));
}

/**
 * Creates the main window's UI
 */
void
omp_main_widgets_create(GtkContainer *destination)
{
	GtkWidget *alignment;
	GtkWidget *mainvbox;
	GtkWidget *background_vbox;
	GtkWidget *upper_hbox, *middle_hbox, *lower_hbox, *controls_hbox;
	GtkWidget *middle_right_vbox;
	GtkWidget *image, *button;

	gchar *image_file_name, *caption;
	gint i;

	// Add mainvbox to destination container
	mainvbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(destination), GTK_WIDGET(mainvbox));

	// Title label
	alignment = create_label(&main_widgets.title_label, "Bitstream Vera Sans 24", "black", 0, 0, 1, 0, 18);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 18, 0, 50, 30);
	gtk_label_set_text(GTK_LABEL(main_widgets.title_label), "No track available");
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// Artist label
	alignment = create_label(&main_widgets.artist_label, "Bitstream Vera Sans 14", "black", 0, 0, 1, 0, 30);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 50, 30);
	gtk_label_set_text(GTK_LABEL(main_widgets.artist_label), "");
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// --- --- --- --- --- Upper hbox --- --- --- --- ---

	// Add upper_hbox to mainvbox for track number and time display
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 20, 0, 50, 30);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	upper_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(upper_hbox));

	// Track number icon
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 2, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image(GTK_CONTAINER(alignment), "ico-track.png");

	// Track number
	alignment = create_label(&main_widgets.track_number_label, "Bitstream Vera Sans 14", "black", 0, 0, 0, 0, 12);
	caption = g_strdup_printf(WIDGET_CAPTION_TRACK_NUM, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.track_number_label), caption);
	g_free(caption);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	// Time icon
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 2, 0, 48, 15);
	gtk_box_pack_start(GTK_BOX(upper_hbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image(GTK_CONTAINER(alignment), "ico-time.png");

	// Time
	alignment = create_label(&main_widgets.time_label, "Bitstream Vera Sans 14", "black", 0, 0, 0, 0, 12);
	caption = g_strdup_printf(WIDGET_CAPTION_TRACK_TIME, 0, 0, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.time_label), caption);
	g_free(caption);
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
	gtk_range_set_value(GTK_RANGE(main_widgets.time_hscale), 0.0);
//    g_signal_connect(G_OBJECT(time_hscale), "change_value",
//		    G_CALLBACK(omp_main_set_time), NULL);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(main_widgets.time_hscale));

	// --- --- --- --- --- Middle hbox --- --- --- --- ---

	// Add middle_hbox to mainvbox for the EQ/volume/balance widgets
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 41, 0, 70, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);

	middle_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(middle_hbox));

	// EQ/Visualization bands
	image_file_name = g_build_path("/", ui_image_path, "ind-music-eq-12.png", NULL);

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
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 4, 0, 25, 0);
	gtk_box_pack_start(GTK_BOX(volume_box), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image_with_ref(GTK_CONTAINER(alignment), "ind-music-volume-00.png", &main_widgets.volume_image);

	// Volume label
	alignment = create_label(&main_widgets.volume_label, "Sans 14", "darkorange", 0, 0, 1, 0, 4);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 0, 10, 0);
	gtk_box_pack_start(GTK_BOX(volume_box), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	caption = g_strdup_printf(WIDGET_CAPTION_VOLUME, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.volume_label), caption);
	g_free(caption);

	//  Balance image
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 10, 0, 35, 0);
	gtk_box_pack_start(GTK_BOX(middle_right_vbox), GTK_WIDGET(alignment), TRUE, TRUE, 0);
	container_add_image_with_ref(GTK_CONTAINER(alignment), "ind-music-pan-0.png", &main_widgets.balance_image);

	// --- --- --- --- --- Lower hbox --- --- --- --- ---

	// Add lower hbox containing the three rectangular buttons
	alignment = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 25, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
	lower_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), lower_hbox);

	// Shuffle toggle button
	alignment = omp_toggle_button_create("ico-shuffle.png", 108, G_CALLBACK(omp_shuffle_button_callback), &main_widgets.shuffle_button);
	gtk_box_pack_start(GTK_BOX(lower_hbox), alignment, TRUE, TRUE, 0);

	// Repeat toggle button
	alignment = omp_toggle_button_create("ico-repeat.png", 10, G_CALLBACK(omp_repeat_button_callback), &main_widgets.repeat_button);
	gtk_box_pack_start(GTK_BOX(lower_hbox), alignment, TRUE, TRUE, 0);

	// Playlist button
	alignment = omp_toggle_button_create("ico-list.png", 10, G_CALLBACK(omp_playlist_button_callback), &main_widgets.playlist_button);
	gtk_box_pack_start(GTK_BOX(lower_hbox), alignment, TRUE, TRUE, 0);

	// --- --- --- --- --- Player controls --- --- --- --- --- ---

	alignment = gtk_alignment_new(0, 0, 1, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 40, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);

	controls_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(alignment), controls_hbox);

	gtk_box_set_homogeneous(GTK_BOX(controls_hbox), TRUE);

	// Previous Track button
	button = omp_button_create("ico-prevtrack.png", G_CALLBACK(omp_playlist_set_prev_track));
	gtk_box_pack_start(GTK_BOX(controls_hbox), button, TRUE, TRUE, 0);
	gtk_box_set_child_packing(GTK_BOX(controls_hbox), GTK_WIDGET(button), FALSE, FALSE, 0, GTK_PACK_START);

	// Rewind button
	button = omp_button_create("ico-rwd.png", G_CALLBACK(omp_shuffle_button_callback));
	gtk_box_pack_start(GTK_BOX(controls_hbox), button, TRUE, TRUE, 0);
	gtk_box_set_child_packing(GTK_BOX(controls_hbox), GTK_WIDGET(button), FALSE, FALSE, 0, GTK_PACK_START);

	// Play/Pause button
	button = omp_button_create("ico-play.png", G_CALLBACK(omp_main_button_play_pause_callback));
	gtk_box_pack_start(GTK_BOX(controls_hbox), button, TRUE, TRUE, 0);
	gtk_box_set_child_packing(GTK_BOX(controls_hbox), GTK_WIDGET(button), FALSE, FALSE, 0, GTK_PACK_START);

	// Fast Forward button
	button = omp_button_create("ico-ffwd.png", G_CALLBACK(omp_shuffle_button_callback));
	gtk_box_pack_start(GTK_BOX(controls_hbox), button, TRUE, TRUE, 0);
	gtk_box_set_child_packing(GTK_BOX(controls_hbox), GTK_WIDGET(button), FALSE, FALSE, 0, GTK_PACK_START);

	// Next Track button
	button = omp_button_create("ico-nexttrack.png", G_CALLBACK(omp_playlist_set_next_track));
	gtk_box_pack_start(GTK_BOX(controls_hbox), button, TRUE, TRUE, 0);
	gtk_box_set_child_packing(GTK_BOX(controls_hbox), GTK_WIDGET(button), FALSE, FALSE, 0, GTK_PACK_START);
}

/**
 * Create the main window and all its UI elements
 * @todo Make all widget positions and aligments relative
 */
void
omp_main_window_create()
{
	GtkWidget *alignment, *bg_muxer;

	// Sanity check
	g_assert(omp_main_window == NULL);

	bg_muxer = gtk_fixed_new();

	// Create the main window
	omp_main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(omp_main_window), _("Media Player"));
	gtk_container_add(GTK_CONTAINER(omp_main_window), GTK_WIDGET(bg_muxer));
	g_signal_connect(G_OBJECT(omp_main_window), "destroy", G_CALLBACK(omp_main_quit), NULL);

	// Background image
	alignment = gtk_alignment_new(0, 0, 0, 0);
	container_add_image(GTK_CONTAINER(alignment), "background.png");
	gtk_fixed_put(GTK_FIXED(bg_muxer), GTK_WIDGET(alignment), 15, 30);
	
	// Create all widgets
	alignment = gtk_alignment_new(0, 0, 0, 0);
	omp_main_widgets_create(GTK_CONTAINER(alignment));
	gtk_fixed_put(GTK_FIXED(bg_muxer), GTK_WIDGET(alignment), 20, 47);

	// Show everything but the window itself
	gtk_widget_show_all(GTK_WIDGET(bg_muxer));

	return;
}

/**
 * Attaches the event handlers to the appropriate signals
 * @note Can't be done in omp_main_window_create() because the signals need to be created first by the subsystems
 */
void
omp_main_connect_signals()
{
	g_signal_connect(G_OBJECT(omp_main_window), OMP_EVENT_PREV_TRACK, G_CALLBACK(omp_main_update_track_info), NULL);
	g_signal_connect(G_OBJECT(omp_main_window), OMP_EVENT_NEXT_TRACK, G_CALLBACK(omp_main_update_track_info), NULL);
}


/**
 * Evaluates current track information and updates the config/UI if necessary
 */
void
omp_main_update_track_info()
{
	static gint old_track_count = 0;
	static gint old_track_id = 0;

	gchar *text;

	// Track id/track count changed?
	if ( (omp_playlist_track_count != old_track_count) || (omp_playlist_current_track_id != old_track_id) )
	{
		old_track_count = omp_playlist_track_count;
		old_track_id = omp_playlist_current_track_id;

		// Update config
		omp_config_update();

		// Update UI
		text = g_strdup_printf(WIDGET_CAPTION_TRACK_NUM, omp_playlist_current_track_id+1, omp_playlist_track_count);
		gtk_label_set_text(GTK_LABEL(main_widgets.track_number_label), text);
		g_free(text);
	}


}

