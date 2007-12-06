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
 * @file playback.c
 * Playback engine interface
 */

#include <glib/gi18n.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <uriparser/Uri.h>

#include "main.h"
#include "main_page.h"
#include "mplayer_playback.h"
#include "persistent.h"
#include "playback.h"
#include "utils.h"

/// Our ticket to the gstreamer world
GstElement *omp_gst_playbin = NULL;

/// gstreamer audio output element
GstElement *omp_gst_audiosink = NULL;

/// Handle of the UI-updating timeout
guint omp_playback_ui_timeout = 0;

/// Flag that tells the UI-updating timer to exit if set
gboolean omp_playback_ui_timeout_halted;

/// Since we can't set a new position if element is not paused or playing we
/// store the position here and set it when it reached either state
gulong omp_playback_pending_position = 0;

/// Contains the final volume when fading (-1 means "not fading")
guint omp_playback_fade_final_vol = -1;

/// Holds the volume increment per UI-update timer call
guint omp_playback_fade_increment = 0;

/// Handle of the fade-in timeout
guint omp_playback_fade_timeout = 0;

// Some private forward declarations
static gboolean omp_gst_message_eos(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_state_changed(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_error(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_warning(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_tag(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_element(GstBus *bus, GstMessage *message, gpointer data);



/**
 * Initializes gstreamer by setting up pipe, message hooks and bins
 * @return TRUE on success, FAIL if an error occured
 */
gboolean
omp_playback_init()
{
	GstBus *bus;

	// Bail if everything is already set up
	if (omp_gst_playbin) return TRUE;

	// Create the signals we'll emit
	g_signal_new(OMP_EVENT_PLAYBACK_RESET, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYBACK_EOS, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYBACK_STATUS_CHANGED, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYBACK_POSITION_CHANGED, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYBACK_VOLUME_CHANGED, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYBACK_META_ARTIST_CHANGED, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);

	g_signal_new(OMP_EVENT_PLAYBACK_META_TITLE_CHANGED, G_TYPE_OBJECT,
		G_SIGNAL_RUN_FIRST, 0, 0, NULL, g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);

	// Create audio sink for PulseAudio
	omp_gst_audiosink = gst_element_factory_make("pulsesink", NULL);

	if (!omp_gst_audiosink)
	{
		error_dialog_modal(_("Error: gstreamer failed to create the PulseAudio sink.\nPlease make sure gstreamer and its modules are properly installed (esp. gst-plugin-pulse)."));

		return FALSE;
	}

	// Set up gstreamer pipe
	omp_gst_playbin = gst_element_factory_make("playbin", NULL);

	if (!omp_gst_playbin)
	{
		error_dialog_modal(_("Error: gstreamer failed to initialize.\nPlease make sure gstreamer and its modules are properly installed (esp. gst-meta-audio)."));

		return FALSE;
	}

	// Force playbin to use our own sink
	g_object_set(G_OBJECT(omp_gst_playbin), "audio-sink", omp_gst_audiosink, NULL);

	// Let's have gstreamer and PulseAudio meet embedded requirements
	g_object_set(G_OBJECT(omp_gst_audiosink), "buffer-time", omp_config_get_pulsesink_buffer_time(), NULL);
	g_object_set(G_OBJECT(omp_gst_audiosink), "latency-time", omp_config_get_pulsesink_latency_time(), NULL);

	// Set up message hooks
	bus = gst_pipeline_get_bus(GST_PIPELINE(omp_gst_playbin));

	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::eos", 					G_CALLBACK(omp_gst_message_eos), NULL);
	g_signal_connect(bus, "message::error", 				G_CALLBACK(omp_gst_message_error), NULL);
	g_signal_connect(bus, "message::warning", 			G_CALLBACK(omp_gst_message_warning), NULL);
	g_signal_connect(bus, "message::state-changed",	G_CALLBACK(omp_gst_message_state_changed), NULL);
	g_signal_connect(bus, "message::tag",						G_CALLBACK(omp_gst_message_tag), NULL);
	g_signal_connect(bus, "message::element",				G_CALLBACK(omp_gst_message_element), NULL);

	gst_object_unref(bus);

	// Initialize mplayer interface and return its result as we would return TRUE now
	return omp_mplayback_init();
}

/**
 * Releases resources used for gstreamer handling
 */
void
omp_playback_free()
{
	GstBus *bus;

	if (!omp_gst_playbin) return;

	// Free resources used by the mplayer interface
	omp_mplayback_free();

	// Free resources used by gstreamer
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
 * Stops playback and unloads the current track
 */
void
omp_playback_reset()
{
	omp_mplayback_pause();
	gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);

	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_RESET);
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
		omp_playback_init();

	#ifdef DEBUG
		g_printf("Loading track: %s\n", uri);
	#endif

	// Make sure mplayer stays quiet
	omp_mplayback_pause();

	if (omp_mplayback_load_video_from_uri(uri))
	{
		// As gstreamer won't be fed a new track we need to make sure it's silent
		gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);

		return TRUE;

	} else {

		// Update gstreamer pipe
		gst_element_set_state(omp_gst_playbin, GST_STATE_NULL);
		g_object_set(G_OBJECT(omp_gst_playbin), "uri", uri, NULL);
		gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED);

		return (gst_element_set_state(omp_gst_playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE);
	}
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
 * This callback raises the volume little by little when we're fading in
 */
static gboolean
omp_playback_fade_timeout_callback(gpointer data)
{
	guint volume;

	// Fade in
	volume = omp_playback_get_volume()+omp_playback_fade_increment;

	if (volume >= omp_playback_fade_final_vol)
	{
		omp_playback_set_volume(omp_playback_fade_final_vol);
		omp_playback_fade_final_vol = -1;
		omp_playback_fade_timeout = 0;
		return FALSE;

	} else {

		omp_playback_set_volume(volume);
		return TRUE;
	}
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

	// Start mplayer playback if a video was loaded
	if (omp_mplayback_video_loaded())
	{
		omp_mplayback_play();
		return;
	}

	// No video loaded, let's try playing with gstreamer
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
		omp_playback_ui_timeout = g_timeout_add(PLAYBACK_UI_UPDATE_INTERVAL, omp_playback_ui_timeout_callback, NULL);

	// Do we need to add the fade-in timer as well?
	if ( (omp_playback_fade_final_vol != -1) && (!omp_playback_fade_timeout) )
		omp_playback_fade_timeout = g_timeout_add(PLAYBACK_FADE_INTERVAL, omp_playback_fade_timeout_callback, NULL);
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

	if (omp_mplayback_video_loaded())
		omp_mplayback_pause();
	else
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

	// Query mplayer interface first
	if (omp_mplayback_is_playing())
	{
		return omp_mplayback_get_state();
	}

	// Poll gstreamer state with an immediate timeout
	state = GST_STATE(omp_gst_playbin);

	// The NULL and READY element states are no different from PAUSED for more abstract layers
	if ( (state == GST_STATE_NULL) || (state == GST_STATE_READY) )
	{
		state = GST_STATE_PAUSED;
	}

	return (gint)state;
}

/**
 * Returns the number of milliseconds that the track has been playing so far
 */
gulong
omp_playback_get_track_position()
{
	GstFormat format = GST_FORMAT_TIME;
	gint64 position = 0;

	// Query mplayer interface first
	if (omp_mplayback_is_playing())
	{
		return omp_mplayback_get_video_position();
	}

	// Let's see if gstreamer has anything to say
	if (!omp_gst_playbin) return 0;

	// Return 0 if function returns FALSE, position otherwise
	return (gst_element_query_position(omp_gst_playbin, &format, &position)) ? (position/1000000) : 0;
}

/**
 * Sets the playback position of the currently loaded track
 * @param position Track position in milliseconds
 */
void
omp_playback_set_track_position(gulong position)
{
	GstState pipe_state;
	gint64 pos;

	// Use the mplayer interface if a video had been loaded
	if (omp_mplayback_video_loaded())
	{
		omp_mplayback_set_video_position(position);
		omp_playback_save_state();
		return;
	}

	if (!omp_gst_playbin) return;

	// If we don't clamp it to values >= 0 we trigger EOS messages which make the playlist mess up
	if (position < 0) position = 0;

	// Check if the pipe is even ready to seek
	pipe_state = GST_STATE(omp_gst_playbin);

	if ( (pipe_state != GST_STATE_PAUSED) && (pipe_state != GST_STATE_PLAYING) )
	{
		// It's not, so make the position change pending
		omp_playback_pending_position = position;

		#ifdef DEBUG
			g_printf("Pended track position change to %d:%.2ds\n", position / 60000, (position/1000) % 60);
		#endif
		return;
	}
	omp_playback_pending_position = 0;

	#ifdef DEBUG
		g_printf("Setting track position to %d:%.2ds\n", position / 60000, (position/1000) % 60);
	#endif

	// Overflow workaround
	pos = position;
	pos = pos*1000000;

	gst_element_seek(GST_ELEMENT(omp_gst_playbin), 1.0,
		GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, pos,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

	// Save session data
	omp_playback_save_state();

	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_POSITION_CHANGED);
}

/**
 * Returns the current track's playing length
 */
gulong
omp_playback_get_track_length()
{
	GstFormat format = GST_FORMAT_TIME;
	gint64 length = 0;

	if (!omp_gst_playbin) return 0;

	gst_element_query_duration(omp_gst_playbin, &format, &length);
	return (length > 0) ? (length/1000000) : 0;
}

/**
 * Sets the playback volume
 * @param volume Volume in percent (0..100)
 */
void
omp_playback_set_volume(guint volume)
{
	if (volume > 100) volume = 100;

	// Set playbin volume which ranges from 0.0 to 1.0
	g_object_set(G_OBJECT(omp_gst_playbin), "volume", volume/100.0, NULL);

	// Volume fading shouldn't be saved to session and be invisible to the user
	if (omp_playback_fade_final_vol == -1)
	{
		omp_session_set_volume(volume);

		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_VOLUME_CHANGED);
	}

	// Update mplayer interface, too
	omp_mplayback_set_volume(volume);
}

/**
 * Returns the playback volume
 * @return Volume in percent (0..100)
 */
guint
omp_playback_get_volume()
{
	gdouble volume;
	g_object_get(G_OBJECT(omp_gst_playbin), "volume", &volume, NULL);

	// We don't care about the mplayer interface here because
	// gstreamer's and mplayer's volume are always kept in sync

	return volume*100;
}

/**
 * Sets up the fade-in timer
 */
void
omp_playback_fade_volume()
{
	omp_playback_fade_final_vol = omp_playback_get_volume();
	omp_playback_set_volume(0);

	omp_playback_fade_increment =
		omp_playback_fade_final_vol / (omp_session_get_fade_speed()/PLAYBACK_FADE_INTERVAL);

	// Make sure the volume actually increases
	if (omp_playback_fade_increment == 0)
		omp_playback_fade_increment = 1;
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

	return TRUE;
}

/**
 * Handles gstreamer's error messages
 */
static gboolean
omp_gst_message_error(GstBus *bus, GstMessage *message, gpointer data)
{
	GError *error;
	gchar *text;

	gst_message_parse_error(message, &error, NULL);
	g_printerr("gstreamer error: %s\n", error->message);

	text = g_strdup_printf("gstreamer error:\n%s", error->message);
	error_dialog(text);
	g_free(text);

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

	gst_message_parse_warning(message, &error, NULL);
	g_printerr("gstreamer warning: %s\n", error->message);

	g_error_free(error);

	return TRUE;
}

/**
 * Handles gstreamer's tag data notification
 * @note We can not assume that all meta data will be sent in one go so we use one signal per entry
 */
static gboolean
omp_gst_message_tag(GstBus *bus, GstMessage *message, gpointer data)
{
	GstTagList *tag_list;
	gchar *s;

	#ifdef DEBUG
		g_printf("gstreamer discovered tag info\n");
	#endif

	gst_message_parse_tag(message, &tag_list);

	// Read artist
	if (gst_tag_list_get_string(tag_list, GST_TAG_ARTIST, &s))
	{
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_META_ARTIST_CHANGED, s);
		g_free(s);
	}

	// Read title
	if (gst_tag_list_get_string(tag_list, GST_TAG_TITLE, &s))
	{
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_META_TITLE_CHANGED, s);
		g_free(s);
	}

	return TRUE;
}

/**
 * Handles gstreamer's "we need a window for video playback" notification
 */
static gboolean
omp_gst_message_element(GstBus *bus, GstMessage *message, gpointer data)
{
	if (gst_structure_has_name(message->structure, "prepare-xwindow-id"))
		gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), omp_main_get_video_window());

	return TRUE;
}
