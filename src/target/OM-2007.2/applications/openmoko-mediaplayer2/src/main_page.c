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
#include <glib/gprintf.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "main_page.h"
#include "main.h"
#include "guitools.h"
#include "playlist.h"
#include "playback.h"
#include "persistent.h"



#include <math.h>

/// Contains all main window widgets that need to be changeable
struct
{
	GtkWidget *cover_image;
	GtkWidget *cover_frame;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *track_number_label;
	GtkWidget *time_label;
	GtkWidget *time_bar;
	GtkWidget *extended_controls;
	GtkWidget *volume_image;
	GtkWidget *play_pause_button_image;
	GtkWidget *shuffle_button_image;
	GtkWidget *repeat_button_image;
} main_widgets;

GtkWidget *omp_main_window = NULL;

/// Possible gestures
typedef enum
{
	OMP_MAIN_GESTURE_NONE,
	OMP_MAIN_GESTURE_LEFT,
	OMP_MAIN_GESTURE_RIGHT,
	OMP_MAIN_GESTURE_UP,
	OMP_MAIN_GESTURE_DOWN
} omp_main_gesture;

/// Holds the necessary infos to record and identify the gestures
struct
{
	gboolean pressed;
	GTimeVal start_time;
	guint x_origin, y_origin;
	guint last_x, last_y;
	gboolean cursor_idle;						// TRUE if cursor isn't moving

	gint radius, angle;
	omp_main_gesture gesture;
	gboolean repeating;
	guint repeat_timeout;
} main_gesture_data;

// Forward declarations for internal use
void omp_main_set_label(omp_main_label_type label_type, gchar *caption);
void omp_main_playlist_loaded(gpointer instance, gchar *title, gpointer user_data);
void omp_main_update_track_change(gpointer instance, gpointer user_data);
void omp_main_update_track_info_changed(gpointer instance, guint track_id, gpointer user_data);
void omp_main_update_shuffle_state(gpointer instance, gboolean state, gpointer user_data);
void omp_main_update_repeat_mode(gpointer instance, guint mode, gpointer user_data);
void omp_main_update_status_change(gpointer instance, gpointer user_data);
void omp_main_update_track_position(gpointer instance, gpointer user_data);
void omp_main_update_volume(gpointer instance, gpointer user_data);



/**
 * Self-explanatory :)
 */
gint min(gint a, gint b)
{
	return (a > b) ? b : a;
}

/**
 * Self-explanatory :)
 */
gint max(gint a, gint b)
{
	return (a > b) ? a : b;
}

/**
 * Event handler for the expand button
 */
void
omp_main_expand_clicked(GtkWidget *widget, gpointer data)
{
	// Toggle visibility of extended controls
	if (GTK_WIDGET_VISIBLE(main_widgets.extended_controls))
		gtk_widget_hide(main_widgets.extended_controls);
	else
		gtk_widget_show(main_widgets.extended_controls);
}

/**
 * Event handler for the Fast Forward button
 */
void
omp_main_fast_forward_clicked(GtkWidget *widget, gpointer data)
{
	omp_playback_set_track_position(omp_playback_get_track_position()+BUTTON_SEEK_DISTANCE);
}

/**
 * Event handler for the Rewind button
 */
void
omp_main_rewind_clicked(GtkWidget *widget, gpointer data)
{
	omp_playback_set_track_position(omp_playback_get_track_position()-BUTTON_SEEK_DISTANCE);
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
 * Event handler for the preferences button
 */
void
omp_main_preferences_clicked(GtkWidget *widget, gpointer data)
{
	error_dialog("Not implemented yet, sorry :)");
}

/**
 * Tries to determine which gesture the user has performed and fills the gesture data struct accordingly
 * @param x X coordinate of current pressure point
 * @param y Y coordinate of current pressure point
 */
void
omp_main_gesture_identify(guint x, guint y)
{
	#define MIN_RADIUS 15

	gint delta_x, delta_y, gamma;

	// Perform rect->polar conversion of the differential cursor movement
	delta_x = x - main_gesture_data.x_origin;
	delta_y = y - main_gesture_data.y_origin;

	main_gesture_data.radius = sqrt(delta_x * delta_x + delta_y * delta_y);

	// angle = arccos(gamma) but arccos() is too slow to compute -> range comparison
	// We shift the comma by 3 digits so we can use integer math
	gamma = delta_x*1000 / main_gesture_data.radius;

	if (main_gesture_data.radius > MIN_RADIUS)
	{

		// Determine direction of movement
		if (gamma < -707)
		{
			main_gesture_data.gesture = OMP_MAIN_GESTURE_LEFT;

		} else {

			if (gamma > 707)
			{
				main_gesture_data.gesture = OMP_MAIN_GESTURE_RIGHT;
			} else {
				main_gesture_data.gesture = (delta_y < 0) ? OMP_MAIN_GESTURE_UP : OMP_MAIN_GESTURE_DOWN;
			}

		}

	} else {

		// Radius too small
		main_gesture_data.gesture = OMP_MAIN_GESTURE_NONE;
	}

}

/**
 * Performs the action the current gesture commands
 */
void
omp_main_gesture_trigger()
{
	if (main_gesture_data.gesture == OMP_MAIN_GESTURE_NONE) return;

	switch (main_gesture_data.gesture)
	{
		case OMP_MAIN_GESTURE_LEFT:
		{
			omp_main_rewind_clicked(NULL, NULL);
			break;
		}

		case OMP_MAIN_GESTURE_RIGHT:
		{
			omp_main_fast_forward_clicked(NULL, NULL);
			break;
		}

		case OMP_MAIN_GESTURE_UP:
		{
			omp_playback_set_volume(min(100, omp_playback_get_volume()+10));
			break;
		}

		case OMP_MAIN_GESTURE_DOWN:
		{
			omp_playback_set_volume(max(0, omp_playback_get_volume()-10));
			break;
		}

		default: break;
	}
}

/**
 * This callback repeatedly performs the action the current gesture commands
 */
static gboolean
omp_main_gesture_repeat_callback(gpointer data)
{
	if (!main_gesture_data.repeating) return FALSE;

	omp_main_gesture_trigger();

	return TRUE;
}

/**
 * Sets up the repeat timeout if the touchscreen is being pressed for a certain amount of time
 */
void
omp_main_gesture_check_repeat()
{
	#define REPEAT_TIME_TRESHOLD_USEC 0750000
	#define REPEAT_INTERVAL_MSEC 1000

	GTimeVal current_time, delta_t;

	if (!main_gesture_data.repeating)
	{
		// Calculate duration of touchscreen press
		g_get_current_time(&current_time);
		delta_t.tv_sec  = current_time.tv_sec  - main_gesture_data.start_time.tv_sec;
		delta_t.tv_usec = current_time.tv_usec - main_gesture_data.start_time.tv_usec;

		if (delta_t.tv_usec >= REPEAT_TIME_TRESHOLD_USEC)
		{
			main_gesture_data.repeating = TRUE;
			g_timeout_add(REPEAT_INTERVAL_MSEC, omp_main_gesture_repeat_callback, NULL);
		}

	}
}

/**
 * Gets called whenever the cursor has been moved, handling gesture recognition and triggering
 */
gboolean
omp_main_pointer_moved(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	#define MAX_DELTA_LAST 3
	gint delta_last_x, delta_last_y;

//	g_printf("Pointer: X=%d /\t Y=%d /\t state=%d /\t is_hint=%d\n",
//		(gint)event->x, (gint)event->y, event->state, event->is_hint);

	if (main_gesture_data.pressed)
	{
		delta_last_x = abs((guint)event->x - main_gesture_data.last_x);
		delta_last_y = abs((guint)event->y - main_gesture_data.last_y);

		// Did the cursor move a substantial amount?
		if ( (delta_last_x > MAX_DELTA_LAST) && (delta_last_y > MAX_DELTA_LAST) )
		{
			// Yes it did, so it's most likely being moved
			main_gesture_data.cursor_idle = FALSE;
			main_gesture_data.last_x = event->x;
			main_gesture_data.last_y = event->y;

			// Make sure we won't trigger anymore (if we were before, that is)
			main_gesture_data.repeating = FALSE;
//			g_printf("-- movement\n");

		} else {
//			g_printf("-- NO movement\n");

			// Cursor is idle, so lets update the gesture data if it wasn't idle before
			if (!main_gesture_data.cursor_idle)
				omp_main_gesture_identify(event->x, event->y);

			omp_main_gesture_check_repeat();

			main_gesture_data.cursor_idle = TRUE;
		}
	}

	return FALSE;
}

/**
 * Gets called whenever the touchscreen has been pressed, handling gesture recognition and triggering
 */
gboolean
omp_main_pointer_pressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
//	g_printf("- Pressed: X=%d /\t Y=%d /\t state=%d /\t button=%d\n",
//		(gint)event->x, (gint)event->y, event->state, event->button);

	main_gesture_data.pressed = TRUE;
	g_get_current_time(&main_gesture_data.start_time);
	main_gesture_data.x_origin = event->x;
	main_gesture_data.y_origin = event->y;
	main_gesture_data.last_x = event->x;
	main_gesture_data.last_y = event->y;
	main_gesture_data.cursor_idle = FALSE;
	main_gesture_data.gesture = OMP_MAIN_GESTURE_NONE;
	main_gesture_data.repeating = FALSE;

	return FALSE;
}

/**
 * Gets called whenever the touchscreen has been released, handling gesture recognition and triggering
 */
gboolean
omp_main_pointer_released(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
//	g_printf("- Released: X=%d /\t Y=%d /\t state=%d /\t button=%d\n",
//		(gint)event->x, (gint)event->y, event->state, event->button);

	// Stop repeat trigger if necessary - or trigger action
	if (main_gesture_data.repeating)
	{
		main_gesture_data.repeating = FALSE;
	} else {
		omp_main_gesture_trigger();
	}

	main_gesture_data.pressed = FALSE;

	return FALSE;
}

/**
 * Resets the UI to a "no track loaded" state
 */
void
omp_main_reset_ui(gpointer instance, gpointer user_data)
{
	gchar *caption;

	if (omp_config_get_main_ui_show_cover())
		gtk_image_set_from_stock(GTK_IMAGE(main_widgets.cover_image), "no_cover", -1);

	// Determine which label we can use for showing the "No track information" line
	// #2 is preferred, followed by #1 and #3
	// Of course we could set all labels to NULL first and then assign the notice,
	// however not doing this saves us a couple cycles on the cost of readability :)
	if (omp_config_get_main_ui_label2() != OMP_MAIN_LABEL_HIDDEN)
	{
		gtk_label_set_text(GTK_LABEL(main_widgets.label1), NULL);
		gtk_label_set_text(GTK_LABEL(main_widgets.label2), _("No track information"));
		gtk_label_set_text(GTK_LABEL(main_widgets.label3), NULL);

	} else {

		if (omp_config_get_main_ui_label1() != OMP_MAIN_LABEL_HIDDEN)
		{
			gtk_label_set_text(GTK_LABEL(main_widgets.label1), NULL);
			gtk_label_set_text(GTK_LABEL(main_widgets.label2), NULL);
			gtk_label_set_text(GTK_LABEL(main_widgets.label3), _("No track information"));
		} else {
			gtk_label_set_text(GTK_LABEL(main_widgets.label1), _("No track information"));
			gtk_label_set_text(GTK_LABEL(main_widgets.label2), NULL);
			gtk_label_set_text(GTK_LABEL(main_widgets.label3), NULL);
		}

	}

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_NUM, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.track_number_label), caption);
	g_free(caption);

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_TIME, 0, 0, 0, 0);
	gtk_label_set_text(GTK_LABEL(main_widgets.time_label), caption);
	g_free(caption);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(main_widgets.time_bar), 0);
}

/**
 * Creates the widgets at the top of the interface (cover, title label, artist label)
 * @todo Add RTL/LTR support
 */
void
omp_main_top_widgets_create(GtkBox *parent)
{
	GtkWidget *frame, *hbox, *image, *alignment, *vbox, *label, *label3;

	// Pack everything into a frame to allow using x/ythickness in the theme
	frame = gtk_frame_new(NULL);
	gtk_widget_set_name(frame, "omp-main-top-cover-padding");
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
	gtk_box_pack_start(parent, frame, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	// Pack the image itself into another frame to give it a black border
	main_widgets.cover_image = gtk_image_new();
	gtk_widget_set_name(GTK_WIDGET(main_widgets.cover_image), "omp-main-top-cover");
	main_widgets.cover_frame = widget_wrap(main_widgets.cover_image, NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(main_widgets.cover_frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(main_widgets.cover_frame), FALSE, FALSE, 0);

	// Add the placeholder that makes sure the vbox retains its height even when the cover image is hidden
	image = gtk_image_new();
	gtk_widget_set_name(GTK_WIDGET(image), "omp-main-top-cover-placeholder");
	gtk_image_set_from_stock(GTK_IMAGE(image), "image", -1);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(image), FALSE, FALSE, 0);


	// Vertical centering made easy: we fill a vbox with
	// "alignment - label - label - alignment" and make it homogeneous
	// while leaving the alignments empty
	vbox = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

	alignment = gtk_alignment_new(0, 0, 1, 1);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);

	main_widgets.label1 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(main_widgets.label1), "omp-main-top-label1");
	gtk_label_set_ellipsize(GTK_LABEL(main_widgets.label1), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(main_widgets.label1), 0, 0);
	label = widget_wrap(main_widgets.label1, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	main_widgets.label2 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(main_widgets.label2), "omp-main-top-label2");
	gtk_label_set_ellipsize(GTK_LABEL(main_widgets.label2), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(main_widgets.label2), 0, 0);
	label = widget_wrap(main_widgets.label2, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	alignment = gtk_alignment_new(0, 0, 1, 1);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);


	// Title label
	main_widgets.label3 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(main_widgets.label3), "omp-main-top-label3");
	gtk_label_set_ellipsize(GTK_LABEL(main_widgets.label3), PANGO_ELLIPSIZE_END);
	label3 = widget_wrap(main_widgets.label3, NULL);
	gtk_box_pack_start(GTK_BOX(parent), label3, FALSE, FALSE, 0);


	// Show all widgets, then hide the ones we don't want visible
	gtk_widget_show_all(GTK_WIDGET(frame));

	if (omp_config_get_main_ui_label1() == OMP_MAIN_LABEL_HIDDEN) gtk_widget_hide(main_widgets.label1);
	if (omp_config_get_main_ui_label2() == OMP_MAIN_LABEL_HIDDEN) gtk_widget_hide(main_widgets.label2);

	if (omp_config_get_main_ui_label3() != OMP_MAIN_LABEL_HIDDEN) gtk_widget_show_all(label3);

	if (!omp_config_get_main_ui_show_cover())
		gtk_widget_hide(main_widgets.cover_frame);
}

/**
 * Creates the widgets at the bottom of the interface (playlist counter, track time, player controls, etc.)
 */
void
omp_main_bottom_widgets_create(GtkBox *parent)
{
	GtkWidget *main_vbox, *hbox, *label, *btn_box, *icon, *eventbox, *button, *image;

	main_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(parent), main_vbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_vbox), hbox, FALSE, FALSE, 0);

	// Playlist counter icon
	icon = gtk_image_new();
	gtk_widget_set_name(icon, "omp-main-btm-info-bar");
	gtk_image_set_from_stock(GTK_IMAGE(icon), "track", -1);
	icon = widget_wrap(icon, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);

	// Playlist counter label
	main_widgets.track_number_label = gtk_label_new(NULL);
	gtk_widget_set_name(main_widgets.track_number_label, "omp-main-btm-info-bar");
	label = widget_wrap(main_widgets.track_number_label, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	// Track time label
	main_widgets.time_label = gtk_label_new(NULL);
	gtk_widget_set_name(main_widgets.time_label, "omp-main-btm-info-bar");
	label = widget_wrap(main_widgets.time_label, NULL);
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	// Track time icon
	icon = gtk_image_new();
	gtk_widget_set_name(icon, "omp-main-btm-info-bar");
	gtk_image_set_from_stock(GTK_IMAGE(icon), "time", -1);
	icon = widget_wrap(icon, NULL);
	gtk_box_pack_end(GTK_BOX(hbox), icon, FALSE, FALSE, 0);


	// Progress bar
	main_widgets.time_bar = gtk_progress_bar_new();
	gtk_widget_set_name(main_widgets.time_bar, "omp-main-btm-progressbar");
	gtk_box_pack_start(GTK_BOX(main_vbox), main_widgets.time_bar, FALSE, FALSE, 0);


	// Button container - the event box is only used to paint the background
	eventbox = gtk_event_box_new();
	gtk_widget_set_name(eventbox, "omp-main-btm-button-box");
	gtk_box_pack_start(GTK_BOX(main_vbox), eventbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	btn_box = widget_wrap(hbox, "omp-main-btm-button-box");
	gtk_container_add(GTK_CONTAINER(eventbox), btn_box);

	// Expand button
	button = button_create_with_image("omp-main-btm-button-expand", "image",
		NULL,
		G_CALLBACK(omp_main_expand_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-y");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Rewind/previous track button
	button = button_create_with_image("omp-main-btm-buttons", "rew_prev",
		NULL,
		G_CALLBACK(omp_playlist_set_prev_track));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Play/pause button
	button = button_create_with_image("omp-main-btm-buttons", "play",
		&main_widgets.play_pause_button_image,
		G_CALLBACK(omp_main_play_pause_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Fast forward/next track button
	button = button_create_with_image("omp-main-btm-buttons", "ffwd_next",
		NULL,
		G_CALLBACK(omp_playlist_set_next_track));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Volume indicator
	main_widgets.volume_image = gtk_image_new();
	gtk_widget_set_name(main_widgets.volume_image, "omp-main-btm-volume");
	icon = widget_wrap(main_widgets.volume_image, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);


	// Button container - second row
	main_widgets.extended_controls = gtk_event_box_new();
	gtk_widget_set_name(main_widgets.extended_controls, "omp-main-btm-button-box2");
	gtk_box_pack_start(GTK_BOX(main_vbox), main_widgets.extended_controls, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	btn_box = widget_wrap(hbox, "omp-main-btm-button-box2");
	gtk_container_add(GTK_CONTAINER(main_widgets.extended_controls), btn_box);

	// Expand button placeholder
	image = gtk_image_new();
	gtk_widget_set_name(image, "omp-main-btm-button-expand-placeholder");
	gtk_image_set_from_stock(GTK_IMAGE(image), "image", -1);
	image = widget_wrap(image, "omp-main-btm-button-padding-y");
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

	// Shuffle button
	button = button_create_with_image("omp-main-btm-buttons", "shuffle_off",
		&main_widgets.shuffle_button_image,
		G_CALLBACK(omp_main_shuffle_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Play/pause button
	button = button_create_with_image("omp-main-btm-buttons", "repeat_off",
		&main_widgets.repeat_button_image,
		G_CALLBACK(omp_main_repeat_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Preferences button
	button = button_create_with_image("omp-main-btm-buttons", "preferences",
		NULL,
		G_CALLBACK(omp_main_preferences_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);


	// Spacer between button box and UI page tabs
	eventbox = gtk_event_box_new();
	gtk_widget_set_name(eventbox, "omp-main-btm-spacer");
	eventbox = widget_wrap(eventbox, NULL);
	gtk_box_pack_end(GTK_BOX(main_vbox), eventbox, FALSE, FALSE, 0);


	// Show all widgets except the extended controls
	gtk_widget_show_all(main_vbox);
	gtk_widget_hide(main_widgets.extended_controls);
}

/**
 * Creates the main UI page and all its elements
 */
GtkWidget *
omp_main_page_create()
{
	GtkWidget *eventbox, *vbox;

	eventbox = gtk_event_box_new();
	gtk_widget_set_name(eventbox, "omp-main-background");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(eventbox), vbox);

	// Create UI widgets
	omp_main_top_widgets_create(GTK_BOX(vbox));
	omp_main_bottom_widgets_create(GTK_BOX(vbox));

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

	// Set up gesture recognition handlers
	gtk_widget_add_events(eventbox,
		GDK_POINTER_MOTION_MASK | 
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	main_gesture_data.pressed = FALSE;

	g_signal_connect(G_OBJECT(eventbox), "motion-notify-event", G_CALLBACK(omp_main_pointer_moved), NULL);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event", G_CALLBACK(omp_main_pointer_pressed), NULL);
	g_signal_connect(G_OBJECT(eventbox), "button-release-event", G_CALLBACK(omp_main_pointer_released), NULL);

	// Make widgets visible - can't use gtk_widget_show_all() here as some widgets should stay hidden
	gtk_widget_show(vbox);
	gtk_widget_show(eventbox);

	return eventbox;
}

/**
 * Fills one of the dynamic labels with a new caption
 * @param label_type Content to set
 * @param caption Text to assign to the label containing selected type
 */
void
omp_main_set_label(omp_main_label_type label_type, gchar *caption)
{
	if (omp_config_get_main_ui_label1() == label_type)
		gtk_label_set_text(GTK_LABEL(main_widgets.label1), caption);

	if (omp_config_get_main_ui_label2() == label_type)
		gtk_label_set_text(GTK_LABEL(main_widgets.label2), caption);

	if (omp_config_get_main_ui_label3() == label_type)
		gtk_label_set_text(GTK_LABEL(main_widgets.label3), caption);
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
	omp_main_set_label(OMP_MAIN_LABEL_ARTIST, artist);
	omp_main_set_label(OMP_MAIN_LABEL_TITLE, title);
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

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(main_widgets.time_bar), (gdouble)track_position/(gdouble)track_length);
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
	if (state)
	{
		gtk_image_set_from_stock(GTK_IMAGE(main_widgets.shuffle_button_image), "shuffle_on", -1);
	} else {
		gtk_image_set_from_stock(GTK_IMAGE(main_widgets.shuffle_button_image), "shuffle_off", -1);
	}
}

/**
 * Updates the UI after the repeat mode changed
 */
void
omp_main_update_repeat_mode(gpointer instance, guint mode, gpointer user_data)
{
	switch (mode)
	{
		case OMP_REPEAT_OFF:
			gtk_image_set_from_stock(GTK_IMAGE(main_widgets.repeat_button_image), "repeat_off", -1);
			break;

		case OMP_REPEAT_ONCE:
			gtk_image_set_from_stock(GTK_IMAGE(main_widgets.repeat_button_image), "repeat_once", -1);
			break;

		case OMP_REPEAT_CURRENT:
			gtk_image_set_from_stock(GTK_IMAGE(main_widgets.repeat_button_image), "repeat_current", -1);
			break;

		case OMP_REPEAT_ALL:
			gtk_image_set_from_stock(GTK_IMAGE(main_widgets.repeat_button_image), "repeat_all", -1);
	}
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
		gtk_image_set_from_stock(GTK_IMAGE(main_widgets.play_pause_button_image), "play", -1);
	} else {
		gtk_image_set_from_stock(GTK_IMAGE(main_widgets.play_pause_button_image), "pause", -1);
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

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(main_widgets.time_bar), (gdouble)track_position/(gdouble)track_length);
	}

}

/**
 * Updates the UI due to a change in playback volume
 */
void
omp_main_update_volume(gpointer instance, gpointer user_data)
{
	gchar *image;
	guint volume;

	volume = omp_playback_get_volume();

	image = g_strdup_printf("volume%02d", volume/10);
	gtk_image_set_from_stock(GTK_IMAGE(main_widgets.volume_image), image, -1);
	g_free(image);
}
