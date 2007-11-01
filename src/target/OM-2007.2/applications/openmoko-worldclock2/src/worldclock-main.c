/*
 *  openmoko-worldclock -- OpenMoko Clock Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <gtk/gtk.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libjana-gtk/jana-gtk.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "worldclock-data.h"

#define GCONF_POKY_INTERFACE_PREFIX "/desktop/poky/interface"
#define GCONF_POKY_DIGITAL "/digital_clock"

static gchar *location;

typedef struct {
	GtkWidget *window;
	GtkWidget *map;
	GtkWidget *map_aspect;
	GtkWidget *load_window;
	GtkWidget *load_bar;
	
	guint render_idle;
	guint time_idle;
	
	gchar *location;
	gdouble zoom_level;
} WorldClockData;

static inline GtkToolItem *
worldclock_utils_toolbutton_new (const gchar *icon_name)
{
	GtkToolItem *button = gtk_tool_button_new_from_stock (icon_name);
	gtk_tool_item_set_expand (button, TRUE);
	return button;
}

static void
zoom_map (WorldClockData *data)
{
	if (data->zoom_level <= 0.95) {
		data->zoom_level = 1;
		gtk_widget_set_size_request (data->map_aspect, -1, -1);
	} else {
		gint width, height;
		gtk_window_get_size (GTK_WINDOW (data->window),
			&width, &height);
		width *= data->zoom_level;
		gtk_widget_set_size_request (data->map_aspect,
			width, (height > (width/2)) ? -1 : width / 2);
	}
}

static void
zoom_in_clicked_cb (GtkToolButton *button, WorldClockData *data)
{
	data->zoom_level *= 1.2;
	zoom_map (data);
}

static void
zoom_out_clicked_cb (GtkToolButton *button, WorldClockData *data)
{
	data->zoom_level /= 1.2;
	zoom_map (data);
}

static gboolean ignore_next = FALSE;

static gboolean
increment_time_timeout (JanaGtkDateTime *dt)
{
	JanaTime *time;

	ignore_next = TRUE;

	time = jana_gtk_date_time_get_time (dt);
	jana_time_set_seconds (time, jana_time_get_seconds (time) + 1);
	jana_gtk_date_time_set_time (dt, time);
	g_object_unref (time);
	
	return TRUE;
}

static void
date_time_changed_cb (JanaGtkDateTime *dt, WorldClockData *data)
{
	if (ignore_next) {
		ignore_next = FALSE;
		return;
	}
	if (data->time_idle) {
		g_source_remove (data->time_idle);
		data->time_idle = 0;
	}
}

static void
settings_clicked_cb (GtkToolButton *button, WorldClockData *data)
{
	GtkWidget *time_dialog, *datetime, *check;
	gchar *location;
	JanaTime *time;
	
	time_dialog = gtk_dialog_new_with_buttons ("Set time",
		GTK_WINDOW (data->window),
		GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_MODAL,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	
	location = jana_ecal_utils_guess_location ();
	time = jana_ecal_utils_time_now (location);
	g_free (location);
	datetime = jana_gtk_date_time_new (time);
	jana_gtk_date_time_set_editable (JANA_GTK_DATE_TIME (datetime), TRUE);
	g_object_unref (time);
	
	g_signal_connect (datetime, "changed",
		G_CALLBACK (date_time_changed_cb), data);

#if GLIB_CHECK_VERSION(2,14,0)
	data->time_idle = g_timeout_add_seconds (1, (GSourceFunc)
		increment_time_timeout, datetime);
#else
	data->time_idle = g_timeout_add (1000, (GSourceFunc)
		increment_time_timeout, datetime);
#endif
	
	check = gtk_check_button_new_with_label ("Use a digital clock");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
		gconf_client_get_bool (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_DIGITAL, NULL));
	
	gtk_container_set_border_width (GTK_CONTAINER (time_dialog), 6);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (time_dialog)->vbox), 12);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (time_dialog)->vbox),
		datetime, FALSE, TRUE, 0);
	gtk_widget_show (datetime);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (time_dialog)->vbox),
		check, FALSE, TRUE, 0);
	gtk_widget_show (check);
	
	gtk_dialog_run (GTK_DIALOG (time_dialog));
	
	/* Set system time */
	if (data->time_idle) {
		/* Time hasn't changed, don't set */
		g_source_remove (data->time_idle);
	} else {
		struct timeval time_tv;
		struct tm time_tm;
		
		time = jana_gtk_date_time_get_time (
			JANA_GTK_DATE_TIME (datetime));

		/* TODO: Maybe this should convert to UTC before setting? */
		time_tm = jana_utils_time_to_tm (time);
		time_tv.tv_sec = timegm (&time_tm);
		time_tv.tv_usec = 0;
		
		settimeofday (&time_tv, NULL);
		
		g_object_unref (time);
	}
	
	gconf_client_set_bool (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_DIGITAL,
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check)), NULL);
	
	gtk_widget_destroy (time_dialog);
}

static gboolean
set_time (GtkWidget *map)
{
	JanaTime *time;

	time = jana_ecal_utils_time_now (location);
	jana_gtk_world_map_set_time (JANA_GTK_WORLD_MAP (map), time);
	g_object_unref (time);

	return TRUE;
}

static gboolean
render_idle (WorldClockData *data)
{
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (data->load_bar));
	gtk_widget_queue_draw (data->map);
	return TRUE;
}

static void
render_start_cb (JanaGtkWorldMap *map, WorldClockData *data)
{
	data->render_idle = g_timeout_add (
		1000/5, (GSourceFunc)render_idle, data);
	gtk_widget_show (data->load_window);
}

static void
render_stop_cb (JanaGtkWorldMap *map, WorldClockData *data)
{
	g_source_remove (data->render_idle);
	gtk_widget_hide (data->load_window);
}

static void
add_marks (WorldClockData *data)
{
	gint i, width, height;
	
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
	
	for (i = 0; i < G_N_ELEMENTS (world_clock_tzdata); i++) {
		JanaGtkWorldMapMarker *mark;
		gchar *path, *image_name;
		GdkPixbuf *pixbuf;
		
		GError *error = NULL;

		image_name = g_strdelimit (g_utf8_strdown (
			world_clock_tzdata[i].country, -1), " '", '_');
		path = g_strconcat (PKGDATADIR G_DIR_SEPARATOR_S,
			image_name, ".svg", NULL);
		g_free (image_name);

		if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
			pixbuf = NULL;
		} else if (!(pixbuf = gdk_pixbuf_new_from_file_at_size (path,
			     width, -1, &error))) {
			g_warning ("Error loading '%s': %s",
				path, error->message);
			g_error_free (error);
		}
		if (pixbuf)
			mark = jana_gtk_world_map_marker_pixbuf_new (pixbuf);
		else
			mark = jana_gtk_world_map_marker_new ();

		g_object_set_data (G_OBJECT (mark), "zone",
			(gpointer)(&world_clock_tzdata[i]));
		jana_gtk_world_map_add_marker (JANA_GTK_WORLD_MAP (data->map),
			mark, world_clock_tzdata[i].lat,
			world_clock_tzdata[i].lon);
		g_free (path);
	}
}

static gboolean
map_button_press_event_cb (JanaGtkWorldMap *map, GdkEventButton *event,
			   WorldClockData *data)
{
	GList *markers, *m;
	gdouble lat, lon, old_distance;
	JanaGtkWorldMapMarker *marker;
	
	jana_gtk_world_map_get_latlon (map, event->x, event->y, &lat, &lon);
	markers = jana_gtk_world_map_get_markers (map);
	
	marker = NULL;
	old_distance = G_MAXDOUBLE;
	for (m = markers; m; m = m->next) {
		gdouble distance;
		JanaGtkWorldMapMarker *marker2 =
			(JanaGtkWorldMapMarker *)m->data;
		
		distance = sqrt (pow (marker2->lat - lat, 2) +
			pow (marker2->lon - lon, 2));
		if (distance < old_distance) {
			marker = marker2;
			old_distance = distance;
		}
	}
	
	if (marker) {
		WorldClockZoneData *tzdata = (WorldClockZoneData *)
			g_object_get_data (G_OBJECT (marker), "zone");
		g_debug ("Nearest location: %s", tzdata->name);
	}
	
	g_list_free (markers);
	
	return FALSE;
}

int
main (int argc, char **argv)
{
	WorldClockData data;
	GtkToolItem *button;
	GtkWidget *vbox, *scroll, *toolbar, *ebox, *label, *load_vbox, *frame;
	guint id;
	
	gtk_init (&argc, &argv);
	
	data.zoom_level = 1;
	
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	
	/* Create toolbar */
	toolbar = gtk_toolbar_new ();

	/* Settings button */
	button = worldclock_utils_toolbutton_new (GTK_STOCK_PREFERENCES);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (button, "clicked",
		G_CALLBACK (settings_clicked_cb), &data);
	
	/* Zoom in button */
	button = worldclock_utils_toolbutton_new (GTK_STOCK_ZOOM_IN);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (button, "clicked",
		G_CALLBACK (zoom_in_clicked_cb), &data);

	/* Zoom out button */
	button = worldclock_utils_toolbutton_new (GTK_STOCK_ZOOM_OUT);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	g_signal_connect (button, "clicked",
		G_CALLBACK (zoom_out_clicked_cb), &data);
	
	gtk_widget_show_all (toolbar);

	/* Create scrolling map */
	data.map = jana_gtk_world_map_new ();
	jana_gtk_world_map_set_width (JANA_GTK_WORLD_MAP (data.map), 2048);
	jana_gtk_world_map_set_height (JANA_GTK_WORLD_MAP (data.map), 1024);
	jana_gtk_world_map_set_static (JANA_GTK_WORLD_MAP (data.map), TRUE);
	add_marks (&data);
	gtk_widget_add_events (GTK_WIDGET (data.map), GDK_BUTTON_PRESS_MASK);
	g_signal_connect (data.map, "button-press-event",
		G_CALLBACK (map_button_press_event_cb), NULL);
	
	data.map_aspect = gtk_aspect_frame_new (NULL, 0.5, 0.5, 2.0, FALSE);
	gtk_frame_set_shadow_type (GTK_FRAME (
		data.map_aspect), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (data.map_aspect), data.map);
	
	scroll = moko_finger_scroll_new ();
	moko_finger_scroll_add_with_viewport (MOKO_FINGER_SCROLL (scroll),
		data.map_aspect);
	g_object_set (G_OBJECT (scroll), "mode", MOKO_FINGER_SCROLL_MODE_PUSH,
		NULL);
	gtk_widget_show_all (scroll);
	g_signal_connect (data.map, "render_start",
		G_CALLBACK (render_start_cb), &data);
	g_signal_connect (data.map, "render_stop",
		G_CALLBACK (render_stop_cb), &data);
	
	/* Create rendering indicator */
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	ebox = gtk_event_box_new ();
	load_vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (load_vbox), 12);
	label = gtk_label_new ("Loading...");
	data.load_bar = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (load_vbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (load_vbox), data.load_bar, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (ebox), load_vbox);
	gtk_container_add (GTK_CONTAINER (frame), ebox);
	gtk_widget_show_all (frame);
	data.load_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_transient_for (GTK_WINDOW (data.load_window),
		GTK_WINDOW (data.window));
	gtk_window_set_position (GTK_WINDOW (data.load_window),
		GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_container_add (GTK_CONTAINER (data.load_window), frame);
	
	/* Pack */
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data.window), vbox);
	gtk_widget_show (vbox);
	
#if 0
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2", /* Moko */
		"gtk-icon-theme-name", "openmoko-standard",
		"gtk-xft-dpi", 285 * 1024,
		"gtk-font-name", "Sans 6",
		NULL);
#endif
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);

	location = jana_ecal_utils_guess_location ();
	id = g_timeout_add (1000 * 60 * 10, (GSourceFunc)set_time, data.map);
	set_time (data.map);
	
	g_signal_connect (data.window, "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (data.window);

	gtk_main ();
	
	g_source_remove (id);
	g_free (location);

	return 0;
}

