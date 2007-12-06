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
 * @file persistent.c
 * Manages application configuration and session data
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <gconf/gconf-client.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "files_page.h"
#include "main.h"
#include "main_page.h"
#include "persistent.h"
#include "playlist.h"
#include "playback.h"

/// The literal de-fault configuration, will only be used if gconf is unavailable or schema file wasn't installed
struct _omp_config omp_default_config =
{
	FALSE,                      // shuffle
	OMP_REPEAT_OFF,             // repeat_mode
	TRUE,                       // resume_playback
	10000,                      // seek_distance
	10000,                      // prev_track_treshold
	TRUE,                       // show_numbers_in_pl
	500000,                     // pulsesink_buffer_time
	100000,                     // pulsesink_latency_time
	TRUE,                       // main_ui_show_cover
	OMP_MAIN_LABEL_HIDDEN,      // main_ui_label1
	OMP_MAIN_LABEL_ARTIST,      // main_ui_label2
	OMP_MAIN_LABEL_TITLE,       // main_ui_label3
	15,                         // min_gesture_radius
	750,                        // gesture_repeat_tresh
	1000,                       // gesture_repeat_intv
};

struct _omp_config *omp_config = NULL;    ///< Global and persistent configuration data
struct _omp_session *omp_session = NULL;  ///< Global and persistent session data

/// The GConf instance we'll use
GConfClient *omp_gconf_client = NULL;



/**
 * Handler for GConf change notifications
 */
static void
omp_config_gconf_notification(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GConfValue *value;

	value = gconf_entry_get_value(entry);

	switch ((omp_config_options)user_data)
	{
		case OMP_CONFIG_SHUFFLE:
		{
			omp_config->shuffle = gconf_value_get_bool(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_SHUFFLE_STATE_CHANGED, omp_config->shuffle);
			break;
		}

		case OMP_CONFIG_REPEAT_MODE:
		{
			omp_config->repeat_mode = gconf_value_get_int(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_REPEAT_MODE_CHANGED, omp_config->repeat_mode);
			break;
		}

		case OMP_CONFIG_RESUME_PLAYBACK:
		{
			omp_config->resume_playback = gconf_value_get_bool(value);
			// No notification needed, value is used on demand
			break;
		}

		case OMP_CONFIG_SEEK_DISTANCE:
		{
			omp_config->seek_distance = gconf_value_get_int(value);
			// No notification needed, value is used on demand
			break;
		}

		case OMP_CONFIG_PREV_TRACK_TRESHOLD:
		{
			omp_config->prev_track_treshold = gconf_value_get_int(value);
			// No notification needed, value is used on demand
			break;
		}

		case OMP_CONFIG_SHOW_NUMBERS_IN_PL:
		{
			omp_config->show_numbers_in_pl = gconf_value_get_bool(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_SHOW_NUMBERS_IN_PL_CHANGED, omp_config->show_numbers_in_pl);
			break;
		}

		case OMP_CONFIG_MAIN_UI_SHOW_COVER:
		{
			omp_config->main_ui_show_cover = gconf_value_get_bool(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_UI_SHOW_COVER_CHANGED, omp_config->main_ui_show_cover);
			break;
		}

		case OMP_CONFIG_MAIN_UI_LABEL1:
		{
			omp_config->main_ui_label1 = gconf_value_get_int(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL1_TYPE_CHANGED, omp_config->main_ui_label1);
			break;
		}

		case OMP_CONFIG_MAIN_UI_LABEL2:
		{
			omp_config->main_ui_label2 = gconf_value_get_int(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL2_TYPE_CHANGED, omp_config->main_ui_label2);
			break;
		}

		case OMP_CONFIG_MAIN_UI_LABEL3:
		{
			omp_config->main_ui_label3 = gconf_value_get_int(value);
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_CONFIG_MAIN_LABEL3_TYPE_CHANGED, omp_config->main_ui_label3);
			break;
		}

		case OMP_CONFIG_MIN_GESTURE_RADIUS:
		{
			omp_config->min_gesture_radius = gconf_value_get_int(value);
			// No notification needed, value is used on demand
			break;
		}

		case OMP_CONFIG_GESTURE_REPEAT_TRESH:
		{
			omp_config->gesture_repeat_tresh = gconf_value_get_int(value);
			// No notification needed, value is used on demand
			break;
		}

		case OMP_CONFIG_GESTURE_REPEAT_INTV:
		{
			omp_config->gesture_repeat_intv = gconf_value_get_int(value);
			// No notification needed, value is used on demand
			break;
		}
	}
}

/**
 * Initalize and load application configuration data
 */
void
omp_config_init()
{
	GError *error = NULL;

	#ifdef DEBUG
		g_print("Loading application configuration\n");
	#endif

	// This mustn't be called more than once
	g_assert(!omp_config);

	// Set de-fault config
	omp_config = g_new(struct _omp_config, 1);
	g_memmove(omp_config, &omp_default_config, sizeof(struct _omp_config));

	// Create the signals we emit
	g_signal_new(OMP_EVENT_CONFIG_SHUFFLE_STATE_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	g_signal_new(OMP_EVENT_CONFIG_REPEAT_MODE_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

	g_signal_new(OMP_EVENT_CONFIG_MAIN_UI_SHOW_COVER_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	g_signal_new(OMP_EVENT_CONFIG_SHOW_NUMBERS_IN_PL_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	g_signal_new(OMP_EVENT_CONFIG_MAIN_LABEL1_TYPE_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

	g_signal_new(OMP_EVENT_CONFIG_MAIN_LABEL2_TYPE_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

	g_signal_new(OMP_EVENT_CONFIG_MAIN_LABEL3_TYPE_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

	// Set up GConf, fetch default values and attach notification handler
	omp_gconf_client = gconf_client_get_default();

	if (!omp_gconf_client)
	{
		g_printerr("GConf error: could not connect to gconfd\nWill continue execution, though program will have limited functionality.\n");
		return;
	}

	gconf_client_add_dir(omp_gconf_client, OMP_GCONF_PATH, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);

	omp_config->shuffle =
		gconf_client_get_bool(omp_gconf_client, OMP_GCONF_PATH "/shuffle", &error);

	if (error)
	{
		g_printerr("GConf error: %s\nWill continue execution, though program will have limited functionality.\n", error->message);
		g_error_free(error);

		return;
	}

	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/shuffle",
		(GConfClientNotifyFunc)omp_config_gconf_notification, (gpointer)OMP_CONFIG_SHUFFLE, NULL, &error);

	omp_config->repeat_mode =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/repeat_mode", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/repeat_mode",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_REPEAT_MODE, NULL, NULL);

	omp_config->resume_playback =
		gconf_client_get_bool(omp_gconf_client, OMP_GCONF_PATH "/resume_playback", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/resume_playback",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_RESUME_PLAYBACK, NULL, NULL);

	omp_config->prev_track_treshold =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/seek_distance", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/seek_distance",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_SEEK_DISTANCE, NULL, NULL);

	omp_config->prev_track_treshold =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/prev_track_treshold", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/prev_track_treshold",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_PREV_TRACK_TRESHOLD, NULL, NULL);

	omp_config->show_numbers_in_pl =
		gconf_client_get_bool(omp_gconf_client, OMP_GCONF_PATH "/show_numbers_in_playlist", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/show_numbers_in_playlist",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_SHOW_NUMBERS_IN_PL, NULL, NULL);

	omp_config->main_ui_show_cover =
		gconf_client_get_bool(omp_gconf_client, OMP_GCONF_PATH "/show_cover_art", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/show_cover_art",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_MAIN_UI_SHOW_COVER, NULL, NULL);

	omp_config->main_ui_label1 =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/main_label1_type", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/main_label1_type",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_MAIN_UI_LABEL1, NULL, NULL);

	omp_config->main_ui_label2 =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/main_label2_type", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/main_label2_type",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_MAIN_UI_LABEL2, NULL, NULL);

	omp_config->main_ui_label3 =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/main_label3_type", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/main_label3_type",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_MAIN_UI_LABEL3, NULL, NULL);

	omp_config->min_gesture_radius =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/min_gesture_radius", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/min_gesture_radius",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_MIN_GESTURE_RADIUS, NULL, NULL);

	omp_config->gesture_repeat_tresh =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/gesture_repeat_tresh", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/gesture_repeat_tresh",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_GESTURE_REPEAT_TRESH, NULL, NULL);

	omp_config->gesture_repeat_intv =
		gconf_client_get_int(omp_gconf_client, OMP_GCONF_PATH "/gesture_repeat_intv", NULL);
	gconf_client_notify_add(omp_gconf_client, OMP_GCONF_PATH "/gesture_repeat_intv",
		omp_config_gconf_notification, (gpointer)OMP_CONFIG_GESTURE_REPEAT_INTV, NULL, NULL);
}

/**
 * Releases resources allocated for configuration data
 */
void
omp_config_free()
{
	if (omp_gconf_client) g_object_unref(G_OBJECT(omp_gconf_client));
	g_free(omp_config);
}

/**
 * Sets state of shuffle flag
 */
void
omp_config_set_shuffle_state(gboolean state)
{
	omp_config->shuffle = state;
	gconf_client_set_bool(omp_gconf_client, OMP_GCONF_PATH "/shuffle", state, NULL);
}

/**
 * Returns state of shuffle flag
 */
gboolean
omp_config_get_shuffle_state()
{
	return omp_config->shuffle;
}

/**
 * Sets repeat mode in config data
 */
void
omp_config_set_repeat_mode(guint mode)
{
	omp_config->repeat_mode = mode;
	gconf_client_set_int(omp_gconf_client, OMP_GCONF_PATH "/repeat_mode", mode, NULL);
}

/**
 * Returns repeat mode
 */
guint
omp_config_get_repeat_mode()
{
	return omp_config->repeat_mode;
}

/**
 * Returns amount of milliseconds the playback engine should seek on FFWD/REW'ing
 */
guint
omp_config_get_seek_distance()
{
	return omp_config->seek_distance;
}

/**
 * Returns amount of milliseconds that determine behavior of the "prev track" event
 */
guint
omp_config_get_prev_track_treshold()
{
	return omp_config->prev_track_treshold;
}

/**
 * Returns the value pulsesink's "buffer-time" property should be set to
 */
gulong
omp_config_get_pulsesink_buffer_time()
{
	return omp_config->pulsesink_buffer_time;
}

/**
 * Returns the value pulsesink's "latency-time" property should be set to
 */
gulong
omp_config_get_pulsesink_latency_time()
{
	return omp_config->pulsesink_latency_time;
}

/**
 * Returns the flag that determines whether the album cover is shown on the main UI
 */
gboolean
omp_config_get_main_ui_show_cover()
{
	return omp_config->main_ui_show_cover;
}

/**
 * Returns the content type to be used for main UI's label #1
 */
guint
omp_config_get_main_ui_label1()
{
	return omp_config->main_ui_label1;
}

/**
 * Returns the content type to be used for main UI's label #2
 */
guint
omp_config_get_main_ui_label2()
{
	return omp_config->main_ui_label2;
}

/**
 * Returns the content type to be used for main UI's label #3
 */
guint
omp_config_get_main_ui_label3()
{
	return omp_config->main_ui_label3;
}

/**
 * Returns the minimum length of a gesture stroke in order to consider it a valid gesture
 */
guint
omp_config_get_min_gesture_radius()
{
	return omp_config->min_gesture_radius;
}

/**
 * Returns the time after which a gesture's action will be repeated for as long as the finger is down
 */
guint
omp_config_get_gesture_repeat_tresh()
{
	return omp_config->gesture_repeat_tresh;
}

/**
 * Returns the interval at which a gesture's action will be repeated
 */
guint
omp_config_get_gesture_repeat_intv()
{
	return omp_config->gesture_repeat_intv;
}

/**
 * Fills the session data with sane default values
 */
void
omp_session_reset()
{
	memset(omp_session, 0, sizeof(struct _omp_session));

	omp_session->volume = 100;
	omp_session->fade_speed = 5000;

	// Set file chooser path to default - or home if that doesn't exist
	if (g_file_test(OMP_DEFAULT_FILE_CHOOSER_PATH, G_FILE_TEST_IS_DIR))
	{
		g_snprintf(omp_session->file_chooser_path, sizeof(omp_session->file_chooser_path),
			"%s", OMP_DEFAULT_FILE_CHOOSER_PATH);
	} else {
		g_snprintf(omp_session->file_chooser_path, sizeof(omp_session->file_chooser_path),
			"%s", g_get_home_dir());
	}
}

/**
 * Initialize the session handling mechanism
 */
void
omp_session_init()
{
	gchar *path_name;

	// This mustn't be called more than once
	g_return_if_fail(omp_session == NULL);

	omp_session = g_new0(struct _omp_session, 1);
	omp_session_reset();

	// Make sure the playlist directory exists
	path_name = g_build_path("/", g_get_home_dir(), OMP_RELATIVE_PLAYLIST_PATH, NULL);
	g_mkdir(path_name, S_IRUSR | S_IWUSR | S_IXUSR);  // rwx for the user, nothing for anyone else
	g_free(path_name);
}

/**
 * Frees the resources used for session data
 */
void
omp_session_free()
{
	g_free(omp_session);
}

/**
 * Restores program state from last session
 */
void
omp_session_restore_state()
{
	#ifdef DEBUG
		g_print("Loading session data\n");
	#endif

	// Load config
	omp_session_load();

	omp_playback_set_volume(omp_session->volume);

	if (omp_session->playlist_file[0])
	{
		// Don't reset playlist state on load or else we'll alter the session
		// data in unwanted ways since the new session state would be saved
		omp_playlist_load(omp_session->playlist_file, FALSE);
	}

	// Check whether playlist_position is valid
	if (!omp_playlist_set_current_track(omp_session->playlist_position))
	{
		// Reset playlist state as playlist must have been modified since it was last loaded
		omp_session->playlist_position = 0;
		omp_session->track_position = 0;
		omp_playlist_set_current_track(0);
	}

	// Try to load the track, set the playback position and resume playback if needed
	if (omp_playlist_load_current_track())
	{
		if (omp_session->was_playing && omp_config->resume_playback)
		{
			omp_playback_fade_volume();
			omp_playback_play();
		}

		omp_playback_set_track_position(omp_session->track_position);
	}

	// Restore various states
	omp_files_page_update_path();
}

/**
 * Save session data to persistent storage
 */
void
omp_session_save()
{
	gint session_file, result;
	gchar *file_name;

	g_return_if_fail(omp_session);

	// OMP_SESSION_FILE_NAME is relative to user's home dir
	file_name = g_build_filename(g_get_home_dir(), OMP_SESSION_FILE_NAME, NULL);

	// Permissions for created session file are 0600
	session_file = creat(file_name, S_IRUSR | S_IWUSR);
	if (session_file == -1) goto io_error;

	result = write(session_file, omp_session, sizeof(struct _omp_session));
	if (result != sizeof(struct _omp_session)) goto io_error;

	result = close(session_file);
	if (result == -1) goto io_error;

	g_free(file_name);
	return;

	// We use a label to avoid duplicating code and improve readability
io_error:
	#ifdef DEBUG
		g_printerr("Failed trying to save session data to %s: %s\n", file_name, strerror(errno));
	#endif

	g_free(file_name);
}

/**
 * Load session data from persistent storage
 */
void
omp_session_load()
{
	gint session_file, result;
	gchar *file_name;

	g_return_if_fail(omp_session);

	// OMP_SESSION_FILE_NAME is relative to user's home dir
	file_name = g_build_filename(g_get_home_dir(), OMP_SESSION_FILE_NAME, NULL);

	session_file = open(file_name, O_RDONLY);
	if (session_file == -1) goto io_error;

	result = read(session_file, omp_session, sizeof(struct _omp_session));
	if (result != sizeof(struct _omp_session)) goto io_error;

	result = close(session_file);
	if (result == -1) goto io_error;

	g_free(file_name);
	return;

	// We use a label to avoid duplicating code and improve readability
io_error:
	#ifdef DEBUG
		g_printerr("Failed trying to load session data from %s: %s\n", file_name, strerror(errno));
		g_printerr("Resetting session data\n");
	#endif

	g_free(file_name);

	// Reset session data on error - just to be safe
	omp_session_reset();
}

/**
 * Saves values the playback engine needs to resume operation next time the player is started
 */
void
omp_session_set_playback_state(glong track_position, gboolean is_playing)
{
	g_return_if_fail(omp_session);

	omp_session->track_position = track_position;
	omp_session->was_playing = is_playing;

	omp_session_save();
}

/**
 * Set playlist to be loaded next session
 */
void
omp_session_set_playlist(gchar *playlist_file)
{
	g_return_if_fail(omp_session);

	g_snprintf(omp_session->playlist_file, sizeof(omp_session->playlist_file), "%s", playlist_file);
	omp_session_save();
}

/**
 * Set track to be loaded next session
 */
void
omp_session_set_track_id(guint track_id)
{
	g_return_if_fail(omp_session);

	omp_session->playlist_position = track_id;
	omp_session_save();
}

/**
 * Set volume to be set next session
 */
void
omp_session_set_volume(guint volume)
{
	g_return_if_fail(omp_session);

	omp_session->volume = volume;
	omp_session_save();
}

/**
 * Set path to be used for the file chooser UI
 */
void
omp_session_set_file_chooser_path(gchar *path)
{
	g_return_if_fail(omp_session);

	g_snprintf(omp_session->file_chooser_path, sizeof(omp_session->file_chooser_path),
		"%s", path);

	// We don't save the session immediately - saving at app termination is good enough
}

/**
 * Returns the number of milliseconds defining the duration of a volume fade
 */
guint
omp_session_get_fade_speed()
{
	g_return_val_if_fail(omp_session, 0);

	return omp_session->fade_speed;
}

/**
 * Returns the path used for the file chooser UI; must not be freed
 */
gchar *
omp_session_get_file_chooser_path()
{
	g_return_val_if_fail(omp_session, NULL);

	// Set the default value if not set
	if (omp_session->file_chooser_path[0] == 0)
	{
		g_snprintf(omp_session->file_chooser_path, sizeof(omp_session->file_chooser_path),
			"%s", OMP_DEFAULT_FILE_CHOOSER_PATH);
	}

	return (gchar *)&omp_session->file_chooser_path;
}
