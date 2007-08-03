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
 * @file playback.c
 * Playback engine interface
 */

#include <gst/gst.h>

#include "playback.h"
#include "mainwin.h"

GstElement *omp_gst_playbin = NULL;



/**
 * Initializes gstreamer by setting up pipe, message hooks and bins
 */
void
omp_playback_init()
{
	GstBus *bus;

	// Bail if everything is already set up
	if (omp_gst_playbin)
	{
		return;
	}

	// Create the signals we'll emit
	g_signal_new(OMP_EVENT_PLAYBACK_EOS, G_TYPE_OBJECT, 0, 0, 0, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);

	// Set up gstreamer pipe and bins
	omp_gst_playbin = gst_element_factory_make("playbin", "play");

	// Set up message hooks
	bus = gst_pipeline_get_bus(GST_PIPELINE(omp_gst_playbin));

	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::eos", 		G_CALLBACK(omp_gst_message_eos), NULL);
	g_signal_connect(bus, "message::error", 	G_CALLBACK(omp_gst_message_error), NULL);
	g_signal_connect(bus, "message::warning", G_CALLBACK(omp_gst_message_warning), NULL); 

	gst_object_unref(bus);
}

/**
 * Releases resources used for gstreamer handling
 */
void
omp_playback_free()
{
	if (!omp_gst_playbin)
	{
		return;
	}

	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(omp_gst_playbin));
}

/**
 * Attempts to load a track from an URI
 */
gboolean
omp_playback_load_track_from_uri(gchar *uri)
{
	// Make sure we are all set
	if (!omp_gst_playbin)
	{
		omp_playback_init();
	}

	// DEBUG
	g_printf("Loading %s\n", uri);

	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
	g_object_set(G_OBJECT(omp_gst_playbin), "uri", uri, NULL);
	gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED);
}

/**
 * Starts playback of the current stream
 */
void
omp_playback_play()
{
	// DEBUG
	g_print("Starting playback\n");

	gst_element_set_state(omp_gst_playbin, GST_STATE_PLAYING);
}

/**
 * Returns the current state the playback engine is in
 */
gint
omp_playback_get_state()
{
	GstState state;
	GstSystemClock *system_clock;

	// Poll state with an immediate timeout
	system_clock = gst_system_clock_obtain();
	gst_element_get_state(GST_OBJECT(omp_gst_playbin), &state, NULL, gst_clock_get_time(system_clock));
	gst_object_unref(system_clock);

	// The NULL element state is no different from READY for more abstract layers
	if (state == GST_STATE_NULL)
	{
		state = GST_STATE_READY;
	}

	return (gint)state;
}

/**
 * Handles gstreamer's end-of-stream notification
 */
static gboolean
omp_gst_message_eos(GstBus *bus, GstMessage *message, gpointer data)
{
	// DEBUG
	g_printf("End of stream reached.\n");

	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
	g_signal_emit_by_name(G_OBJECT(omp_main_window), OMP_EVENT_PLAYBACK_EOS);

	return TRUE;
}

/**
 * Handles gstreamer's error messages
 */
static gboolean
omp_gst_message_error(GstBus *bus, GstMessage *message, gpointer data)
{
	GError *error;

	gst_message_parse_error(message, &error, NULL);
	g_printerr("gstreamer error: %s\n", error->message);
	g_error_free(error);

	return TRUE;
}

/**
 * Handles gstreamer's warnings
 */
static gboolean
omp_gst_message_warning(GstBus *bus, GstMessage *message, gpointer data)
{
	GError *error;

	gst_message_parse_error(message, &error, NULL);
	g_printerr("gstreamer warning: %s\n", error->message);
	g_error_free(error);

	return TRUE;
}

