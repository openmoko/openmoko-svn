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
 * @file main_page.c
 * Main UI handling
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <math.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "guitools.h"
#include "main_page.h"
#include "main.h"
#include "playlist.h"
#include "playback.h"
#include "persistent.h"
#include "utils.h"

// This is the amount the cursor must have moved in either direction to be considered moving
// If we don't do this then it will constantly trigger gesture recognition due to jitter on the touchscreen
#define OMP_MAIN_MIN_CURSOR_DELTA 3



/// Contains all main window widgets that need to be changeable
struct
{
	GtkWidget *cover_image;
	GtkWidget *cover_eventbox;
	GtkWidget *cover_frame;
	GtkWidget *label1;
	GtkWidget *label1_frame;
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
} omp_main_widgets;

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
	gboolean cursor_idle;        // TRUE if cursor isn't moving

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
void omp_main_update_show_cover_art(gpointer instance, gboolean flag, gpointer user_data);
void omp_main_update_status_change(gpointer instance, gpointer user_data);
void omp_main_update_track_position(gpointer instance, gpointer user_data);
void omp_main_update_volume(gpointer instance, gpointer user_data);
void omp_main_update_label_type(gpointer instance, guint new_type, gpointer user_data);



/**
 * Self-explanatory :)
 */
gint
min(gint a, gint b)
{
	return (a > b) ? b : a;
}

/**
 * Self-explanatory :)
 */
gint
max(gint a, gint b)
{
	return (a > b) ? a : b;
}

/**
 * Find length of vector a+ib using the alpha min + beta max approximation
 * @param a X dimension of vector (real)
 * @param b Y dimension of vector (imaginary)
 * @return Length of vector a+ib
 * @note We use this approximation because a) finger movement is never more precise than our approximation;
 * @note b) we need speed and sqrt(a^2 + b^2) is too slow without an FPU while providing unnecessary precision
 */
guint
approx_radius(gint a, gint b)
{
	guint a_abs, b_abs, a_max, b_min;

	a_abs = abs(a);
	b_abs = abs(b);

	a_max = max(a_abs, b_abs);
	b_min = min(a_abs, b_abs);

	// Formula is |a+ib| = alpha*a_max + beta*b_min
	// We use alpha=15/16 and beta=15/32

	return ((a_max*15) >> 4) + ((b_min*15) >> 5);
}

/**
 * Event handler for the expand button
 */
void
omp_main_expand_clicked(GtkWidget *widget, gpointer data)
{
	// Toggle visibility of extended controls
	if (GTK_WIDGET_VISIBLE(omp_main_widgets.extended_controls))
		gtk_widget_hide(omp_main_widgets.extended_controls);
	else
		gtk_widget_show(omp_main_widgets.extended_controls);
}

/**
 * Event handler for the Fast Forward button
 */
void
omp_main_fast_forward_clicked(GtkWidget *widget, gpointer data)
{
	omp_playback_set_track_position(omp_playback_get_track_position()+omp_config_get_seek_distance());
}

/**
 * Event handler for the Rewind button
 */
void
omp_main_rewind_clicked(GtkWidget *widget, gpointer data)
{
	omp_playback_set_track_position(omp_playback_get_track_position()-omp_config_get_seek_distance());
}

/**
 * Event handler for the Play/Pause button
 */
void
omp_main_play_pause_clicked(GtkWidget *widget, gpointer data)
{
	if (omp_playback_get_state() != OMP_PLAYBACK_STATE_PLAYING)
		omp_playback_play();
	else
		omp_playback_pause();
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

	if (mode >= OMP_REPEAT_COUNT) mode = 0;

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
	gint delta_x, delta_y, gamma;

	// Perform rect->polar conversion of the differential cursor movement
	delta_x = x - main_gesture_data.x_origin;
	delta_y = y - main_gesture_data.y_origin;

	main_gesture_data.radius = approx_radius(delta_x, delta_y);

	// angle = arccos(gamma) but arccos() is too slow to compute -> range comparison
	// We shift the comma by 3 digits so we can use integer math
	gamma = delta_x*1000 / main_gesture_data.radius;

	if (main_gesture_data.radius > omp_config_get_min_gesture_radius())
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
			omp_main_rewind_clicked(NULL, NULL);
			break;

		case OMP_MAIN_GESTURE_RIGHT:
			omp_main_fast_forward_clicked(NULL, NULL);
			break;

		case OMP_MAIN_GESTURE_UP:
			omp_playback_set_volume(min(100, omp_playback_get_volume()+10));
			break;

		case OMP_MAIN_GESTURE_DOWN:
			omp_playback_set_volume(max(0, omp_playback_get_volume()-10));
			break;

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
	GTimeVal current_time, delta_t;

	if (!main_gesture_data.repeating)
	{
		// Calculate duration of touchscreen press
		g_get_current_time(&current_time);
		delta_t.tv_sec  = current_time.tv_sec  - main_gesture_data.start_time.tv_sec;
		delta_t.tv_usec = current_time.tv_usec - main_gesture_data.start_time.tv_usec;

		if (delta_t.tv_usec >= omp_config_get_gesture_repeat_tresh()*1000)
		{
			main_gesture_data.repeating = TRUE;
			g_timeout_add(omp_config_get_gesture_repeat_intv(), omp_main_gesture_repeat_callback, NULL);
		}

	}
}

/**
 * Gets called whenever the cursor has been moved, handling gesture recognition and triggering
 */
gboolean
omp_main_pointer_moved(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	gint delta_last_x, delta_last_y;

	if (main_gesture_data.pressed)
	{
		delta_last_x = abs((guint)event->x - main_gesture_data.last_x);
		delta_last_y = abs((guint)event->y - main_gesture_data.last_y);

		// Did the cursor move a substantial amount?
		if ( (delta_last_x > OMP_MAIN_MIN_CURSOR_DELTA) && (delta_last_y > OMP_MAIN_MIN_CURSOR_DELTA) )
		{
			// Yes it did, so it's most likely being moved
			main_gesture_data.cursor_idle = FALSE;
			main_gesture_data.last_x = event->x;
			main_gesture_data.last_y = event->y;

			// Make sure we won't trigger anymore (if we were before, that is)
			main_gesture_data.repeating = FALSE;

		} else {

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
	{
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.cover_image), "no_cover", -1);
		gtk_widget_queue_draw(omp_main_widgets.cover_image);	// Re-draw the cover as it might have been used as video display before
	}

	// Determine which label we can use for showing the "No track information" line
	// #2 is preferred, followed by #1 and #3
	// Of course we could set all labels to NULL first and then assign the notice,
	// however not doing this saves us a couple cycles on the cost of readability :)
	if (omp_config_get_main_ui_label2() != OMP_MAIN_LABEL_HIDDEN)
	{
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label1), NULL);
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label2), _("No track information"));
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label3), NULL);

	} else {

		if (omp_config_get_main_ui_label1() != OMP_MAIN_LABEL_HIDDEN)
		{
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label1), NULL);
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label2), NULL);
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label3), _("No track information"));
		} else {
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label1), _("No track information"));
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label2), NULL);
			gtk_label_set_text(GTK_LABEL(omp_main_widgets.label3), NULL);
		}

	}

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_NUM, 0, 0);
	gtk_label_set_text(GTK_LABEL(omp_main_widgets.track_number_label), caption);
	g_free(caption);

	caption = g_strdup_printf(OMP_WIDGET_CAPTION_TRACK_TIME, 0, 0, 0, 0);
	gtk_label_set_text(GTK_LABEL(omp_main_widgets.time_label), caption);
	g_free(caption);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(omp_main_widgets.time_bar), 0);
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

	// Pack the image into an eventbox (for video playback) and that into another frame to give it a black border
	omp_main_widgets.cover_eventbox = gtk_event_box_new();

	omp_main_widgets.cover_image = gtk_image_new();
	gtk_widget_set_name(GTK_WIDGET(omp_main_widgets.cover_image), "omp-main-top-cover");
	gtk_container_add(GTK_CONTAINER(omp_main_widgets.cover_eventbox), omp_main_widgets.cover_image);

	omp_main_widgets.cover_frame = widget_wrap(omp_main_widgets.cover_eventbox, "omp-main-top-cover");
	gtk_frame_set_shadow_type(GTK_FRAME(omp_main_widgets.cover_frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(omp_main_widgets.cover_frame), FALSE, FALSE, 0);

	// Add the placeholder that makes sure the vbox retains its height even when the cover image is hidden
	// We do this so the background image is still present and can be used for the labels
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

	omp_main_widgets.label1 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(omp_main_widgets.label1), "omp-main-top-label1");
	gtk_label_set_ellipsize(GTK_LABEL(omp_main_widgets.label1), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(omp_main_widgets.label1), 0, 0);
	omp_main_widgets.label1_frame = widget_wrap(omp_main_widgets.label1, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), omp_main_widgets.label1_frame, FALSE, FALSE, 0);

	omp_main_widgets.label2 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(omp_main_widgets.label2), "omp-main-top-label2");
	gtk_label_set_ellipsize(GTK_LABEL(omp_main_widgets.label2), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(omp_main_widgets.label2), 0, 0);
	label = widget_wrap(omp_main_widgets.label2, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	alignment = gtk_alignment_new(0, 0, 1, 1);
	gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);


	// Title label
	omp_main_widgets.label3 = gtk_label_new(NULL);
	gtk_widget_set_name(GTK_WIDGET(omp_main_widgets.label3), "omp-main-top-label3");
	gtk_label_set_ellipsize(GTK_LABEL(omp_main_widgets.label3), PANGO_ELLIPSIZE_END);
	label3 = widget_wrap(omp_main_widgets.label3, NULL);
	gtk_box_pack_start(GTK_BOX(parent), label3, FALSE, FALSE, 0);


	// Show all widgets, then hide the ones we don't want visible
	gtk_widget_show_all(GTK_WIDGET(frame));

	if (omp_config_get_main_ui_label1() == OMP_MAIN_LABEL_HIDDEN) gtk_widget_hide(omp_main_widgets.label1_frame);
	if (omp_config_get_main_ui_label2() == OMP_MAIN_LABEL_HIDDEN) gtk_widget_hide(omp_main_widgets.label2);

	if (omp_config_get_main_ui_label3() != OMP_MAIN_LABEL_HIDDEN) gtk_widget_show_all(label3);

	if (!omp_config_get_main_ui_show_cover())
		gtk_widget_hide(omp_main_widgets.cover_frame);
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
	omp_main_widgets.track_number_label = gtk_label_new(NULL);
	gtk_widget_set_name(omp_main_widgets.track_number_label, "omp-main-btm-info-bar");
	label = widget_wrap(omp_main_widgets.track_number_label, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	// Track time label
	omp_main_widgets.time_label = gtk_label_new(NULL);
	gtk_widget_set_name(omp_main_widgets.time_label, "omp-main-btm-info-bar");
	label = widget_wrap(omp_main_widgets.time_label, NULL);
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	// Track time icon
	icon = gtk_image_new();
	gtk_widget_set_name(icon, "omp-main-btm-info-bar");
	gtk_image_set_from_stock(GTK_IMAGE(icon), "time", -1);
	icon = widget_wrap(icon, NULL);
	gtk_box_pack_end(GTK_BOX(hbox), icon, FALSE, FALSE, 0);


	// Progress bar
	omp_main_widgets.time_bar = gtk_progress_bar_new();
	gtk_widget_set_name(omp_main_widgets.time_bar, "omp-main-btm-progressbar");
	gtk_box_pack_start(GTK_BOX(main_vbox), omp_main_widgets.time_bar, FALSE, FALSE, 0);


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
		&omp_main_widgets.play_pause_button_image,
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
	omp_main_widgets.volume_image = gtk_image_new();
	gtk_widget_set_name(omp_main_widgets.volume_image, "omp-main-btm-volume");
	icon = widget_wrap(omp_main_widgets.volume_image, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);


	// Button container - second row
	omp_main_widgets.extended_controls = gtk_event_box_new();
	gtk_widget_set_name(omp_main_widgets.extended_controls, "omp-main-btm-button-box2");
	gtk_box_pack_start(GTK_BOX(main_vbox), omp_main_widgets.extended_controls, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	btn_box = widget_wrap(hbox, "omp-main-btm-button-box2");
	gtk_container_add(GTK_CONTAINER(omp_main_widgets.extended_controls), btn_box);

	// Expand button placeholder
	image = gtk_image_new();
	gtk_widget_set_name(image, "omp-main-btm-button-expand-placeholder");
	gtk_image_set_from_stock(GTK_IMAGE(image), "image", -1);
	image = widget_wrap(image, "omp-main-btm-button-padding-y");
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

	// Shuffle button
	button = button_create_with_image("omp-main-btm-buttons", "shuffle_off",
		&omp_main_widgets.shuffle_button_image,
		G_CALLBACK(omp_main_shuffle_clicked));
	button = widget_wrap(button, "omp-main-btm-button-padding-xy");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Play/pause button
	button = button_create_with_image("omp-main-btm-buttons", "repeat_off",
		&omp_main_widgets.repeat_button_image,
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
	gtk_widget_hide(omp_main_widgets.extended_controls);
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

//	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
//		G_CALLBACK(omp_main_update_track_info_changed), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED,
		G_CALLBACK(omp_main_update_track_change), NULL);

	// Set up configuration signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_SHUFFLE_STATE_CHANGED,
		G_CALLBACK(omp_main_update_shuffle_state), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_REPEAT_MODE_CHANGED,
		G_CALLBACK(omp_main_update_repeat_mode), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_UI_SHOW_COVER_CHANGED,
		G_CALLBACK(omp_main_update_show_cover_art), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL1_TYPE_CHANGED,
		G_CALLBACK(omp_main_update_label_type), (gpointer)1);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL2_TYPE_CHANGED,
		G_CALLBACK(omp_main_update_label_type), (gpointer)2);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL3_TYPE_CHANGED,
		G_CALLBACK(omp_main_update_label_type), (gpointer)3);

	// Set up playback signal handlers
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_RESET,
		G_CALLBACK(omp_main_reset_ui), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_STATUS_CHANGED,
		G_CALLBACK(omp_main_update_status_change), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_POSITION_CHANGED,
		G_CALLBACK(omp_main_update_track_position), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_VOLUME_CHANGED,
		G_CALLBACK(omp_main_update_volume), NULL);

	// Update UI with current configuration values
	omp_main_update_shuffle_state(NULL, omp_config_get_shuffle_state(), NULL);
	omp_main_update_repeat_mode(NULL, omp_config_get_repeat_mode(), NULL);

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
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label1), caption);

	if (omp_config_get_main_ui_label2() == label_type)
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label2), caption);

	if (omp_config_get_main_ui_label3() == label_type)
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.label3), caption);
}

/**
 * Helper function that returns the X window handle to use for video playback and prepares the UI for video playback
 */
gulong
omp_main_get_video_window()
{
	if (GTK_WIDGET_NO_WINDOW(omp_main_widgets.cover_eventbox))
		g_error("Video display widget has no window!\n");

	return GDK_WINDOW_XWINDOW(omp_main_widgets.cover_eventbox->window);
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

	gulong track_length = 0;
	gulong track_position = 0;
	gchar *artist = NULL;
	gchar *title = NULL;
	gchar *text;
	gint track_id;

	// Restore and invalidate default cover
	if (omp_config_get_main_ui_show_cover())
	{
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.cover_image), "no_cover", -1);
		gtk_widget_queue_draw(omp_main_widgets.cover_frame);	// Re-draw the default cover
	}

	// Set preliminary artist/title strings (updated on incoming metadata)
	omp_playlist_get_track_info(-1, &artist, &title, &track_length);
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
		gtk_label_set_text(GTK_LABEL(omp_main_widgets.track_number_label), text);
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

		gtk_label_set_text(GTK_LABEL(omp_main_widgets.time_label), text);
		g_free(text);

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(omp_main_widgets.time_bar), (gdouble)track_position/(gdouble)track_length);
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
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.shuffle_button_image), "shuffle_on", -1);
	else
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.shuffle_button_image), "shuffle_off", -1);
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
			gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.repeat_button_image), "repeat_off", -1);
			break;

		case OMP_REPEAT_ONCE:
			gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.repeat_button_image), "repeat_once", -1);
			break;

		case OMP_REPEAT_CURRENT:
			gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.repeat_button_image), "repeat_current", -1);
			break;

		case OMP_REPEAT_ALL:
			gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.repeat_button_image), "repeat_all", -1);
	}
}

/**
 * Updates the UI after the "show cover art" flag changed
 */
void
omp_main_update_show_cover_art(gpointer instance, gboolean flag, gpointer user_data)
{
	if (flag)
		gtk_widget_show(omp_main_widgets.cover_frame);
	else
		gtk_widget_hide(omp_main_widgets.cover_frame);
}

/**
 * Updates the UI after a switch between "paused" and "playing" modes
 */
void
omp_main_update_status_change(gpointer instance, gpointer user_data)
{
	// Update Play/Pause button pixmap
	if (omp_playback_get_state() == OMP_PLAYBACK_STATE_PAUSED)
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.play_pause_button_image), "play", -1);
	else
		gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.play_pause_button_image), "pause", -1);
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

		gtk_label_set_text(GTK_LABEL(omp_main_widgets.time_label), text);
		g_free(text);

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(omp_main_widgets.time_bar), (gdouble)track_position/(gdouble)track_length);
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
	gtk_image_set_from_stock(GTK_IMAGE(omp_main_widgets.volume_image), image, -1);
	g_free(image);
}

/**
 * Updates the UI when the type of the first dynamic label changed
 * @param instance Unused
 * @param new_type The new type to set for the label
 * @param user_data Contains the number of the label to update (1..3)
 */
void
omp_main_update_label_type(gpointer instance, guint new_type, gpointer user_data)
{
	GtkWidget *label, *frame;
	gchar *artist = NULL;
	gchar *title = NULL;

	g_return_if_fail( (user_data > 0) && (user_data < 4) );

	switch ((gint)user_data)
	{
		case 1:
		{
			label = omp_main_widgets.label1;
			frame = omp_main_widgets.label1_frame;
			break;
		}

		case 2:
		{
			label = omp_main_widgets.label2;
			frame = NULL;	// Can't be hidden
			break;
		}

		case 3:
		{
			label = omp_main_widgets.label3;
			frame = NULL;	// Can't be hidden
			break;
		}
	}

	// Fetch track information if needed
	if ( (new_type != OMP_MAIN_LABEL_HIDDEN) && (new_type != OMP_MAIN_LABEL_EMPTY) )
	 omp_playlist_get_track_info(-1, &artist, &title, NULL);

	// Update label
	switch ((omp_main_label_type)new_type)
	{
		case OMP_MAIN_LABEL_HIDDEN:
		{
			if (frame) gtk_widget_hide(frame);
			break;
		}

		case OMP_MAIN_LABEL_EMPTY:
		{
			gtk_label_set_text(GTK_LABEL(label), NULL);
			break;
		}

		case OMP_MAIN_LABEL_ARTIST:
		{
			gtk_label_set_text(GTK_LABEL(label), artist);
			break;
		}

		case OMP_MAIN_LABEL_TITLE:
		{
			gtk_label_set_text(GTK_LABEL(label), title);
			break;
		}
	}

	if (artist) g_free(artist);
	if (title) g_free(title);

	// Make sure label is visible - it might have been hidden previously
	if ( (frame) && (new_type != OMP_MAIN_LABEL_HIDDEN) )
		gtk_widget_show(frame);
}
