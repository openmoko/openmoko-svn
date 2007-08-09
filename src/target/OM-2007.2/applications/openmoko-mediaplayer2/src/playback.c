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
#include "main.h"

GstElement *omp_gst_playbin = NULL;						///< Our ticket to the gstreamer world
guint omp_playback_ui_timeout = 0;						///< Handle of the UI-updating timeout
gboolean omp_playback_ui_timeout_halted;			///< Flag that tells the UI-updating timeout to exit if set
gulong omp_playback_pending_position = 0;			///< Since we can't set a new position if element is not paused or playing we store the position here and set it when it reached either state



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
	g_signal_new(OMP_EVENT_PLAYBACK_EOS,								G_TYPE_OBJECT, 0, 0, 0, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
	g_signal_new(OMP_EVENT_PLAYBACK_STATUS_CHANGED,			G_TYPE_OBJECT, 0, 0, 0, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
	g_signal_new(OMP_EVENT_PLAYBACK_POSITION_CHANGED,		G_TYPE_OBJECT, 0, 0, 0, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);

	// Set up gstreamer pipe and bins
	omp_gst_playbin = gst_element_factory_make("playbin", "play");

	// Set up message hooks
	bus = gst_pipeline_get_bus(GST_PIPELINE(omp_gst_playbin));

	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::eos", 					G_CALLBACK(omp_gst_message_eos), NULL);
	g_signal_connect(bus, "message::error", 				G_CALLBACK(omp_gst_message_error), NULL);
	g_signal_connect(bus, "message::warning", 			G_CALLBACK(omp_gst_message_warning), NULL);
	g_signal_connect(bus, "message::state-changed",	G_CALLBACK(omp_gst_message_state_changed), NULL);

	gst_object_unref(bus);
}

/**
 * Releases resources used for gstreamer handling
 */
void
omp_playback_free()
{
	GstBus *bus;

	if (!omp_gst_playbin)
	{
		return;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(omp_gst_playbin));
	gst_bus_remove_signal_watch(bus);
	gst_object_unref(bus);

	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(omp_gst_playbin));
}

/**
 * Saves current state to session data
 */
void
omp_playback_save_state()
{
	omp_session_set_playback_state(
		omp_playback_get_track_position(),
		(omp_playback_get_state() == OMP_PLAYBACK_STATE_PLAYING) );
}

/**
 * Attempts to load a track from an URI
 * @return TRUE if successful, FALSE if failed
 */
gboolean
omp_playback_load_track_from_uri(gchar *uri)
{
	// Make sure we are all set
	if (!omp_gst_playbin)
	{
		omp_playback_init();
	}

	#ifdef DEBUG
		g_printf("Loading track: %s\n", uri);
	#endif

	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
	g_object_set(G_OBJECT(omp_gst_playbin), "uri", uri, NULL);
	gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED);

	return (gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE);
}

/**
 * This callback gets called at least once per second if a track is playing
 */
static gboolean
omp_playback_ui_timeout_callback(gpointer data)
{
	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_POSITION_CHANGED);
	
	if (omp_playback_ui_timeout_halted)
	{
		// Reset the timeout ID so we can prevent race conditions
		omp_playback_ui_timeout = 0;
		return FALSE;
	}

	return TRUE;
}

/**
 * Starts playback of the current stream
 */
void
omp_playback_play()
{
	gchar *track_uri;

	#ifdef DEBUG
		g_print("Starting playback\n");
	#endif

	g_object_get(G_OBJECT(omp_gst_playbin), "uri", &track_uri, NULL);
	if (!track_uri)
	{
		#ifdef DEBUG
			g_print("No track to play.\n");
		#endif
		return;
	}
	g_free(track_uri);

	// Set state
	gst_element_set_state(omp_gst_playbin, GST_STATE_PLAYING);

	// Add timer to update UI if it isn't already there
	omp_playback_ui_timeout_halted = FALSE;

	if (!omp_playback_ui_timeout)
	{
		omp_playback_ui_timeout = g_timeout_add(PLAYBACK_UI_UPDATE_INTERVAL, omp_playback_ui_timeout_callback, NULL);
	}
}

/**
 * Suspends playback of the current stream
 */
void
omp_playback_pause()
{
	#ifdef DEBUG
		g_print("Suspending playback\n");
	#endif

	// Set state
	gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED);

	// Stop timer
	omp_playback_ui_timeout_halted = TRUE;
}

/**
 * Returns the current state the playback engine is in, simplifying it a bit to hide internal states
 */
gint
omp_playback_get_state()
{
	GstState state;

	// Poll state with an immediate timeout
	state = GST_STATE(omp_gst_playbin);

	// The NULL and READY element states are no different from PAUSED for more abstract layers
	if ( (state == GST_STATE_NULL) || (state == GST_STATE_READY) )
	{
		state = GST_STATE_PAUSED;
	}

	return (gint)state;
}

/**
 * Returns the number of seconds that the track has been playing so far
 */
gulong
omp_playback_get_track_position()
{
	GstFormat format = GST_FORMAT_TIME;
	gint64 position;

	if (!omp_gst_playbin)
	{
		return 0;
	}

	// Return 0 if function returns FALSE, position otherwise
	return (gst_element_query_position(omp_gst_playbin, &format, &position)) ? (position/GST_SECOND) : 0;
}

/**
 * Sets the playback position of the currently loaded track
 */
void
omp_playback_set_track_position(glong position)
{
	GstState pipe_state;

	if (!omp_gst_playbin)
	{
		return;
	}

	// If we don't clamp it to values >= 0 we trigger EOS messages which make the playlist mess up
	if (position < 0) position = 0;

	// Check if the pipe is even ready to seek
	pipe_state = GST_STATE(omp_gst_playbin);

	if ( (pipe_state != GST_STATE_PAUSED) && (pipe_state != GST_STATE_PLAYING) )
	{
		// It's not, so make the position change pending
		omp_playback_pending_position = position;

		#ifdef DEBUG
			g_printf("Pended track position change to %d:%.2ds\n", position / 60, position % 60);
		#endif
		return;
	}
	omp_playback_pending_position = 0;

	#ifdef DEBUG
		g_printf("Setting track position to %d:%.2ds\n", position / 60, position % 60);
	#endif

	gst_element_seek(GST_ELEMENT(omp_gst_playbin), 1.0,
		GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, position*GST_SECOND,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_POSITION_CHANGED);
}

/**
 * Returns the current track's playing length
 */
gulong
omp_playback_get_track_length()
{
	GstFormat format = GST_FORMAT_TIME;
	gint64 length;

	if (!omp_gst_playbin)
	{
		return 0;
	}

	// Return 0 if function returns FALSE, track length otherwise
	return (gst_element_query_duration(omp_gst_playbin, &format, &length)) ? (length/GST_SECOND) : 0;
}

/**
 * Handles gstreamer's end-of-stream notification
 */
static gboolean
omp_gst_message_eos(GstBus *bus, GstMessage *message, gpointer data)
{
	#ifdef DEBUG
		g_printf("End of stream reached.\n");
	#endif

	// Reset playback engine
	gst_element_set_state(omp_gst_playbin, GST_STATE_READY);
	omp_playback_set_track_position(0);

	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_EOS);

	return TRUE;
}

/**
 * Handles gstreamer's state change notification
 */
static gboolean
omp_gst_message_state_changed(GstBus *bus, GstMessage *message, gpointer data)
{
	static gint previous_state = GST_STATE_VOID_PENDING;
	gint new_state;

	// Do we have a pending playback position change that we can apply?
	if ( ( (GST_STATE(omp_gst_playbin) == GST_STATE_PLAYING) || (GST_STATE(omp_gst_playbin) == GST_STATE_PAUSED) )
 		&& omp_playback_pending_position)
	{
		omp_playback_set_track_position(omp_playback_pending_position);
	}

	// Only propagate this event if it's of interest for higher-level routines
	new_state = omp_playback_get_state();
	if (new_state != previous_state)
	{
		previous_state = new_state;
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_STATUS_CHANGED);
	}
}

/**
 * Handles gstreamer's error messages
 */
static gboolean
omp_gst_message_error(GstBus *bus, GstMessage *message, gpointer data)
{
	GError *error;

	#ifdef DEBUG
		gst_message_parse_error(message, &error, NULL);
		g_printerr("gstreamer error: %s\n", error->message);
		g_error_free(error);
	#endif

	return TRUE;
}

/**
 * Handles gstreamer's warnings
 */
static gboolean
omp_gst_message_warning(GstBus *bus, GstMessage *message, gpointer data)
{
	GError *error;

	#ifdef DEBUG
		gst_message_parse_warning(message, &error, NULL);
		g_printerr("gstreamer warning: %s\n", error->message);
		g_error_free(error);
	#endif

	return TRUE;
}
