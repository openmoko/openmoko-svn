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
 * @file playlist.c
 * Playlist handling
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <uriparser/Uri.h>

#include <stdarg.h>
#include <string.h>

#include "guitools.h"
#include "main.h"
#include "omp_spiff_c.h"
#include "persistent.h"
#include "playback.h"
#include "playlist.h"
#include "utils.h"

/// Loaded playlist
omp_spiff_list *omp_playlist = NULL;

/// File name of currently loaded playlist
gchar *omp_playlist_file = NULL;

/// Number of tracks stored within the current playlist
guint omp_playlist_track_count = 0;

/// Current track's data
omp_spiff_track *omp_playlist_current_track = NULL;

/// Pointer to the last track of the playlist (saves time when appending tracks)
omp_spiff_track *omp_playlist_last_track = NULL;

/// Numerical id of the current track within the playlist
guint omp_playlist_current_track_id = -1;

/// This linked list holds all tracks that were played in this session, most recently played entry first
GSList *omp_track_history = NULL;

/// Single element for the track history
typedef struct _omp_track_history_entry
{
	omp_spiff_track *track;
	guint track_id;
} omp_track_history_entry;

// Forward declarations for internal use
void omp_playlist_process_eos_event(gpointer instance, gpointer user_data);
void omp_playlist_process_tag_artist_change(gpointer instance, gchar *artist, gpointer user_data);
void omp_playlist_process_tag_title_change(gpointer instance, gchar *title, gpointer user_data);
void omp_playlist_update_track_duration();



/**
 * Initialize all things playlist
 */
void
omp_playlist_init()
{
	// Hook up event handlers to the playback routines
	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_EOS,
		G_CALLBACK(omp_playlist_process_eos_event), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_META_ARTIST_CHANGED,
		G_CALLBACK(omp_playlist_process_tag_artist_change), NULL);

	g_signal_connect(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_META_TITLE_CHANGED,
		G_CALLBACK(omp_playlist_process_tag_title_change), NULL);

	// Create the signals we emit
	g_signal_new(OMP_EVENT_PLAYLIST_LOADED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

	g_signal_new(OMP_EVENT_PLAYLIST_TRACK_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_signal_new(OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

	g_signal_new(OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED,
		G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, 0, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/**
 * Frees resources allocated for playlist handling
 */
void
omp_playlist_free()
{
	// Free the track history's memory by deleting the first element until the list is empty
	while (omp_track_history)
	{
		g_free(omp_track_history->data);
		omp_track_history = g_slist_delete_link(omp_track_history, omp_track_history);
	}

	// Let XSPF clean up, too
	if (omp_playlist)
	{
		omp_spiff_free(omp_playlist);
		g_free(omp_playlist_file);
	}
}

/**
 * Load playlist
 * @param playlist_file Absolute file name of playlist to load
 * @param do_state_reset Determines whether to reset playlist state on successful load
 * @return TRUE on success, FALSE on failure
 */
gboolean
omp_playlist_load(gchar *playlist_file, gboolean do_state_reset)
{
	gchar *title, *text;

	// Free the track history's memory by deleting the first element until the list is empty
	while (omp_track_history)
	{
		g_free(omp_track_history->data);
		omp_track_history = g_slist_delete_link(omp_track_history, omp_track_history);
	}

	// Let XSPF clean up, too
	if (omp_playlist)
	{
		omp_spiff_free(omp_playlist);
		omp_playlist = NULL;

		g_free(omp_playlist_file);
		omp_playlist_file = NULL;
	}

	// Update session unless no change happened
	if (omp_session->playlist_file != playlist_file)
	{
		omp_session_set_playlist(playlist_file);
	}

	// Load playlist
	omp_playlist = omp_spiff_parse(playlist_file);

	if (omp_playlist)
	{
		omp_playlist_file = g_strdup(playlist_file);

		if (do_state_reset)
		{
			// Reset playlist state and prepare playback
			omp_playback_reset();
			omp_playlist_current_track_id = 0;
			omp_playlist_current_track    = omp_playlist->tracks;
			omp_playlist_last_track = NULL;

			if (omp_playlist_current_track)
			{
				omp_playlist_set_current_track(0);
				omp_playlist_load_current_track();
			}
		}

		omp_playlist_update_track_count();

		title = get_playlist_title(omp_playlist_file);
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_LOADED, title);
		g_free(title);

	} else {

		omp_playlist_current_track_id = -1;
		omp_playlist_current_track    = NULL;
		omp_playlist_last_track = NULL;

		#ifdef DEBUG
			g_printerr("Could not load playlist: %s\n", playlist_file);
		#endif

		// Notify user
		text = g_strdup_printf(_("Could not load playlist '%s'"), playlist_file);
		error_dialog(text);
		g_free(text);
	}

	return omp_playlist ? TRUE : FALSE;
}

/**
 * Creates a new playlist and loads it so it can be used/edited
 */
void
omp_playlist_create(gchar *playlist_file)
{
	if (omp_playlist)
	{
		omp_spiff_free(omp_playlist);
		omp_playlist = NULL;

		g_free(omp_playlist_file);
		omp_playlist_file = NULL;
	}

	// Create new playlist, save and load it
	omp_playlist = omp_spiff_new();
	omp_playlist_file = g_strdup(playlist_file);
	omp_playlist_save();
	omp_playlist_load(playlist_file, TRUE);
}

/**
 * Saves the currently loaded playlist to disk
 */
void
omp_playlist_save()
{
	if (omp_playlist && omp_playlist_file)
	{
		omp_spiff_write(omp_playlist, omp_playlist_file);
	}
}

/**
 * Deletes a playlist file, making sure things stay sane if it's currently loaded
 * @todo Make unicode safe (-> g_filename_to_utf8())
 */
void
omp_playlist_delete(gchar *playlist_file)
{
	if (omp_playlist)
	{
		if (strcmp(omp_playlist_file, playlist_file) == 0)
		{
			omp_spiff_free(omp_playlist);
			omp_playlist = NULL;

			g_free(omp_playlist_file);
			omp_playlist_file = NULL;

			omp_playback_reset();
		}
	}

	g_unlink(playlist_file);
}

/**
 * Tries to set the position within the playlist and indicates success/failure
 * @param playlist_pos New position, counting starts at 0
 */
gboolean
omp_playlist_set_current_track(gint playlist_pos)
{
	gboolean position_valid = FALSE;
	omp_spiff_track *track;
	gint track_num = 0;

	if (!omp_playlist)
	{
		return FALSE;
	}

	// Walk through the playlist and see if the new position is valid
	for (track=omp_playlist->tracks; (track!=NULL) && (!position_valid); track=track->next, track_num++)
	{
		if (track_num == playlist_pos)
		{
			omp_playlist_current_track    = track;
			omp_playlist_current_track_id = track_num;
			position_valid = TRUE;
		}
	}

	if (position_valid)
	{
		// Update session
		omp_session_set_track_id(omp_playlist_current_track_id);

		// Emit signal to update UI and the like
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);
	}

	return position_valid;
}

/**
 * Moves one track backwards in playlist
 * @return TRUE if a new track is to be played, FALSE if current track didn't change
 * @todo What to do in shuffle mode if track history is empty?
 */
gboolean
omp_playlist_set_prev_track()
{
	omp_track_history_entry *history_entry;
	omp_spiff_track *track;
	gboolean was_playing;
	gboolean track_determined = FALSE;

	if (!omp_playlist_current_track)
	{
		return FALSE;
	}

	// If track playing time is >= n seconds we just jump back to the beginning of the track
	if (omp_playback_get_track_position() >= omp_config_get_prev_track_treshold())
	{
		omp_playback_set_track_position(0);
		return TRUE;
	}

	// Get player state so we can continue playback if necessary
	was_playing = (omp_playback_get_state() == OMP_PLAYBACK_STATE_PLAYING);

try_again:

	#ifdef DEBUG
		if (omp_track_history)
		{
			GSList *list;
			g_printf("--- Track History:\n");
			list = omp_track_history;
			while (list)
			{
				history_entry = list->data;
				g_printf("- %s\n", history_entry->track->locations->value);
				list = g_slist_next(list);
			}
			g_printf("---\n");
		}
	#endif

	// Do we have tracks in the history to go back to?
	if (omp_track_history)
	{
		history_entry = (omp_track_history_entry*)(omp_track_history->data);
		omp_playlist_current_track    = history_entry->track;
		omp_playlist_current_track_id = history_entry->track_id;
		
		// Delete first entry from the list
		g_free(history_entry);
		omp_track_history = g_slist_delete_link(omp_track_history, omp_track_history);

		track_determined = TRUE;

	} else {

		// Is current track the first in the playlist?
		// If not, find track previous to the current one
		if (omp_playlist_current_track != omp_playlist->tracks)
		{
			track = omp_playlist->tracks;

			while ( (track->next) && (track->next != omp_playlist_current_track) )
			{
				track = track->next;
			}

			// We only found the previous track if we're not at the end of the list
			if (track->next)
			{
				omp_playlist_current_track = track;
				omp_playlist_current_track_id--;
				track_determined = TRUE;
			}
		}
	}

	if (track_determined)
	{
		// Update session
		omp_session_set_track_id(omp_playlist_current_track_id);

		// Load track and start playing if needed
		if (omp_playlist_load_current_track())
		{
			// Emit signal to update UI and the like
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);

			if (was_playing) omp_playback_play();

		} else {

			// Uh-oh, track failed to load - let's find another one, shall we?
			track_determined = FALSE;
			goto try_again;
		}
	}

	return track_determined;
}

/**
 * Moves one track forward in playlist
 * @return TRUE if a new track is to be played, FALSE if current track didn't change
 * @todo Will cause an infinite loop if playlist only consists of tracks that can't be played and player is in shuffle mode
 */
gboolean
omp_playlist_set_next_track()
{
	omp_track_history_entry *history_entry;
	gboolean was_playing;
	guint repeat_mode;
	gboolean track_determined = FALSE;
	guint i, n;

	if (!omp_playlist_current_track)
	{
		return FALSE;
	}

	// Get player state so we can continue playback if necessary
	was_playing = (omp_playback_get_state() == OMP_PLAYBACK_STATE_PLAYING);

	repeat_mode = omp_config_get_repeat_mode();

	// Repeat once
	if (repeat_mode == OMP_REPEAT_ONCE)
	{
		// Play same track again and turn repeat off
		omp_config_set_repeat_mode(OMP_REPEAT_OFF);
		track_determined = TRUE;
	}

	// Repeat current
	if (repeat_mode == OMP_REPEAT_CURRENT)
	{
		// Play same track again
		track_determined = TRUE;
	}

try_again:

	// Prepare the history entry - if we don't need it we'll just free it again
	history_entry = g_new(omp_track_history_entry, 1);
	history_entry->track    = omp_playlist_current_track;
	history_entry->track_id = omp_playlist_current_track_id;

	// Repeat all: we forward 1 track
	// Shuffle on: we forward 0 < n < omp_playlist_track_count tracks
	if ( (repeat_mode == OMP_REPEAT_ALL) || (omp_config_get_shuffle_state()) )
	{
		n = (omp_config_get_shuffle_state()) ?
			g_random_int_range(1, omp_playlist_track_count) :
			1;

		for (i=0; i<n; i++)
		{
			if (omp_playlist_current_track->next)
			{
				omp_playlist_current_track = omp_playlist_current_track->next;
				omp_playlist_current_track_id++;
			} else {
				omp_playlist_current_track = omp_playlist->tracks;
				omp_playlist_current_track_id = 0;
			}
		}

		track_determined = TRUE;
	}

	// Repeat off and shuffle off: Do we have a track to play?
	if ( (repeat_mode == OMP_REPEAT_OFF) &&
		   (!omp_config_get_shuffle_state()) &&
		   omp_playlist_current_track->next
		 )
	{
		omp_playlist_current_track = omp_playlist_current_track->next;
		omp_playlist_current_track_id++;

		track_determined = TRUE;
	}

	if (track_determined)
	{
		// Add track to track history
		omp_track_history = g_slist_prepend(omp_track_history, (gpointer)history_entry);

		// Update session
		omp_session_set_track_id(omp_playlist_current_track_id);

		// Load track and start playing if needed
		if (omp_playlist_load_current_track())
		{
			// Emit signal to update UI and the like
			g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);

			if (was_playing) omp_playback_play();

		} else {

			// Uh-oh, track failed to load - let's find another one, shall we?
			track_determined = FALSE;
			goto try_again;
		}

	} else {

		// We're not making use of the history entry as we didn't find a new track to play
		g_free(history_entry);
	}

	return track_determined;
}

/**
 * Signal handler that gets called whenever the current stream ends
 */
void
omp_playlist_process_eos_event(gpointer instance, gpointer user_data)
{
	if (omp_playlist_set_next_track())
	{
		// If we received an eos event we were obviously playing so let's continue doing so
		omp_playback_play();
	}
}

/**
 * Updates the track's artist information in the playlist on incoming tag data
 * @note This is also our way of caching metadata information so we can display it in the playlist editor
 * @param instance Ignored
 * @param title Artist of currently played track
 * @param user_data Ignored
 */
void
omp_playlist_process_tag_artist_change(gpointer instance, gchar *artist, gpointer user_data)
{
	// Now that we have received metadata information we might also have the track duration ready
	omp_playlist_update_track_duration();

	if (!omp_playlist_current_track) return;

	if (!omp_playlist_current_track->creator)
	{
		omp_playlist_current_track->creator = g_strdup(artist);

		// Save changes to disk
		omp_playlist_save();

		// Notify UI of the change
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
			omp_playlist_current_track_id);
	}
}

/**
 * Updates the track's artist information in the playlist on incoming tag data
 * @note This is also our way of caching metadata information so we can display it in the playlist editor
 * @param instance Ignored
 * @param title Song title of currently played track
 * @param user_data Ignored
 */
void
omp_playlist_process_tag_title_change(gpointer instance, gchar *title, gpointer user_data)
{
	// Now that we have received metadata information we might also have the track duration ready
	omp_playlist_update_track_duration();

	if (!omp_playlist_current_track) return;

	if (
			(!omp_playlist_current_track->title) ||
			(omp_playlist_current_track->title_is_preliminary)
		 )
	{
		if (omp_playlist_current_track->title)
			g_free(omp_playlist_current_track->title);

		omp_playlist_current_track->title_is_preliminary = FALSE;

		omp_playlist_current_track->title = g_strdup(title);

		// Save changes to disk
		omp_playlist_save();

		// Notify UI of the change
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
			omp_playlist_current_track_id);
	}
}

/**
 * Checks to see if we can get track duration information from the playback interface
 */
void
omp_playlist_update_track_duration()
{
	gulong duration;

	if (!omp_playlist_current_track) return;

	// Check if we can update duration information (spiff saves it in milliseconds as well)
	duration = omp_playback_get_track_length();
	if ( (duration > 0) && (duration != omp_playlist_current_track->duration) )
	{
		omp_playlist_current_track->duration = duration;
		omp_playlist_save();

		// Notify UI of the change
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
			omp_playlist_current_track_id);
	}
}

/**
 * Sets current track's meta data from the URI unless the playlist already has meta data available
 * @todo Unicode support
 */
void
omp_playlist_set_preliminary_metadata(omp_spiff_track *track, gchar *track_uri)
{
	UriParserStateA state;
	UriUriA uri;
	UriPathSegmentA *segment;
	gchar *title = NULL;

	state.uri = &uri;

	g_return_if_fail(track);

	if (uriParseUriA(&state, track_uri) != 0)
	{
		#ifdef DEBUG
			g_printerr("UriParser could not parse URI %s\n", track_uri);
		#endif
		return;
	}

	// The last part of the URI path is the file name of the request - which we want
	segment = uri.pathTail;

	if (!segment)
	{
		uriFreeUriMembersA(&uri);
		#ifdef DEBUG
			g_printerr("UriParser did not deliver path tail for %s\n", track_uri);
		#endif
		return;
	}

	if (!segment->text.first)
	{
		uriFreeUriMembersA(&uri);
		#ifdef DEBUG
			g_printerr("UriParser did not deliver first text element for %s\n", track_uri);
		#endif
		return;
	}

	title = get_base_file_name((gchar*)segment->text.first);
	uriUnescapeInPlaceA(title);

	// Set preliminary metadata if necessary
	if (!track->title)
	{
		track->title = g_strdup(title);
		track->title_is_preliminary = TRUE;

	} else {

		// If there already is a title and it's the same title we *would* set, mark it
		if (strcmp(track->title, title) == 0)
		{
			track->title_is_preliminary = TRUE;
		}
	}

	g_free(title);

	uriFreeUriMembersA(&uri);
}

/**
 * Uses the URI(s) and metadata information of a track to locate the resource to play, then returns a playable URI
 * @return URI of a playable resource
 * @note If the URI of the found resource is not in the track's locations list already it is added and the playlist saved
 * @todo Actually make this function do what it's supposed to do :)
 */
gchar *
omp_playlist_resolve_track(omp_spiff_track *track)
{
	if (!track)
	{
		g_printerr("Resolve request for an undefined track, returning null as URI.\n");
		return NULL;
	}

	if (!track->locations)
	{
		g_printerr("Resolve request for a track without any locations, returning null as URI.\n");
		return NULL;
	}

	if (track->locations->value)
	{
		return g_strdup(track->locations->value);

	} else {

		g_printerr("Resolve request for a track without a valid location, ignoring. Will be implemented later.\n");
		return NULL;
	}
}

/**
 * Tries to resolve the current track and feed it to gstreamer
 * @return FALSE if current track failed to load, TRUE otherwise
 */
gboolean
omp_playlist_load_current_track()
{
	gchar *track_uri;
	gboolean track_loaded;

	// Make sure we got something to handle
	if (!omp_playlist_current_track)
	{
		return FALSE;
	}

	// Resolve playlist entry to make it available for playback
	track_uri = omp_playlist_resolve_track(omp_playlist_current_track);

	// Let gstreamer have its fun
	if (track_uri)
	{
		track_loaded = omp_playback_load_track_from_uri(track_uri);

		// Maybe we already know the new track duration
		omp_playlist_update_track_duration();

		// Obtain preliminary track title from URI if needed
		omp_playlist_set_preliminary_metadata(omp_playlist_current_track, track_uri);

		// Notify UI of the metadata change
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED,
			omp_playlist_current_track_id);

		g_free(track_uri);

		return track_loaded;
	}

	return FALSE;
}

/**
 * Retrieves a track's meta data if possible
 * @param track_id Track ID to get meta data of, starting at 0; set to -1 to use current track
 * @param artist Destination for the artist string, can be NULL; must be freed after use
 * @param title Destination for the title string, can be NULL; must be freed after use
 * @param duration Destination for the track duration (in milliseconds), can be NULL
 * @todo List walking
 */
void
omp_playlist_get_track_info(gint track_id, gchar **artist, gchar **title, gulong *duration)
{
	if ( (track_id == omp_playlist_current_track_id) || (track_id == -1) )
	{
		if (!omp_playlist_current_track) return;

		if (artist) *artist = g_strdup(omp_playlist_current_track->creator);
		if (title) *title = g_strdup(omp_playlist_current_track->title);

		if (duration) *duration =
			(omp_playlist_current_track->duration > 0) ? omp_playlist_current_track->duration : 0;

	} else {

		// It's not the current track we want to get the infos of so we need to walk the list
		#ifdef DEBUG
			g_print("List walking in omp_playlist_get_track_info() not yet implemented.\n");
		#endif
	}
}

/**
 * Counts the number of tracks in the playlist and updates the UI
 */
void
omp_playlist_update_track_count()
{
	omp_spiff_track *track, *last_track = NULL;
	gint old_count, count = 0;

	if (!omp_playlist) return;

	old_count = omp_playlist_track_count;

	// Walk through the entire list to count number of tracks and find last track pointer
	for (track=omp_playlist->tracks; track!=NULL; last_track=track, track=track->next, count++);

	omp_playlist_track_count = count;
	omp_playlist_last_track  = last_track;

	// Notify UI only if track count actually changed
	if (old_count != count)
	{
		g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED);
	}
}

/**
 * Creates an iterator for iterating over the playlist
 * @return Returns a new iterator which is deallocated by omp_playlist_advance_iter() - or yourself
 */
omp_playlist_iter *
omp_playlist_init_iterator()
{
	omp_playlist_iter *iter;

	if (!omp_playlist) return NULL;
	if (!omp_playlist->tracks) return NULL;

	iter = g_new(omp_playlist_iter, 1);

	iter->track = omp_playlist->tracks;
	iter->track_num = 0;

	return iter;
}

/**
 * Fetches information about the track the iter points at
 * @param iter The iterator
 * @param track_num Destination for the track number (can be NULL)
 * @param track_title Destination for the track title (can be NULL), must be freed after use
 */
void
omp_playlist_get_track_from_iter(omp_playlist_iter *iter, guint *track_num, gchar **track_artist,
 gchar **track_title, gulong *duration)
{
	// Sanity checks - one silent, one not
	if (!iter) return;
	g_return_if_fail(iter->track);

	// Assign values
	if (track_num)
	{
		*track_num = iter->track_num;
	}

	if (track_artist)
	{
		if (iter->track)
			*track_artist = g_strdup(iter->track->creator);
		else
			*track_artist = NULL;
	}

	if (track_title)
	{
		if (iter->track)
			*track_title = g_strdup(iter->track->title);
		else
			*track_title = NULL;
	}

	if (duration)
	{
		// Spiff saves the duration in milliseconds, too
		*duration = (iter->track->duration > 0) ? iter->track->duration : 0;
	}
}

/**
 * Advances a playlist iterator by one track
 */
void
omp_playlist_advance_iter(omp_playlist_iter *iter)
{
	if (iter)
	{
		if (iter->track)
		{
			iter->track = iter->track->next;
			iter->track_num++;
		}
	}
}

/**
 * Determines whether an iterator has reached the end of the playlist
 */
gboolean
omp_playlist_iter_finished(omp_playlist_iter *iter)
{
	if (!iter) return TRUE;

	return (iter->track) ? FALSE : TRUE;
}

/**
 * Appends a track to the end of the playlist
 * @return TRUE on success, FALSE on failure
 * @todo Make unicode-safe
 */
gboolean
omp_playlist_track_append_file(gchar *file_name)
{
	omp_spiff_track *new_track;
	omp_spiff_mvalue *location;
	gchar *uri, name_char;
	guint name_pos, uri_pos, name_len;
	gboolean list_was_empty;

	if (!omp_playlist) return FALSE;
	if (!file_name) return FALSE;

	// Try to make the "last track" pointer valid - if it stays NULL then the list is empty
	if (!omp_playlist_last_track)
	{
		omp_playlist_update_track_count();
	}

	// Append track
	if (omp_playlist_last_track)
	{
		// List already has entries - we append
		new_track = omp_spiff_new_track_before(&omp_playlist_last_track->next);
		omp_playlist_last_track = omp_playlist_last_track->next;
		list_was_empty = FALSE;

	} else {

		// List is empty - we start filling it...
		new_track = omp_spiff_new_track_before(&omp_playlist->tracks);
		omp_playlist_last_track = omp_playlist->tracks;

		// ...and set this as the current track as no track was loaded before
		omp_playlist_current_track = omp_playlist->tracks;
		omp_playlist_current_track_id = 0;
		list_was_empty = TRUE;
	}

	location = omp_spiff_new_mvalue_before(&new_track->locations);
	omp_playlist_track_count++;

	// Build URI for the file location
	name_len = strlen(file_name);
	uri = g_malloc(7+3*name_len);  // Enough for worst case: every char becomes %xx character sequence

	g_sprintf(uri, "file://");
	uri_pos = 7;

	// We could use uriEscapeA() here but that will also transform '/' to %2F, which we do not want
	for (name_pos=0; name_pos < name_len; name_pos++)
	{
		name_char = file_name[name_pos];
		if (  ((name_char >= 'a') && (name_char <= 'z'))
		   || ((name_char >= 'A') && (name_char <= 'Z'))
		   || ((name_char >= '0') && (name_char <= '9'))
		   || (name_char == '-') || (name_char == '.')
		   || (name_char == '_') || (name_char == '~')
		   || (name_char == '/') )
		{
			uri[uri_pos++] = name_char;

		} else {

			uri[uri_pos+0] = '%';
			uri[uri_pos+1] = uriHexToLetterA(name_char >> 4);
			uri[uri_pos+2] = uriHexToLetterA(name_char & 0x0F);
			uri_pos += 3;
		}
	}
	uri[uri_pos] = 0;

	location->value = g_strdup(uri);
	g_free(uri);

	// Give the track list something to show
	omp_playlist_last_track->title = get_base_file_name(file_name);
	omp_playlist_last_track->title_is_preliminary = TRUE;

	// If the playlist was empty we need to load the newly added track right away
	if (list_was_empty) omp_playlist_load_current_track();

	// Notify UI of the change
	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED);

	return TRUE;
}

/**
 * Recursively adds all files from a directory and its subdirectories
 * @param dir_name Directory to add
 * @return Number of files added
 */
guint
omp_playlist_track_append_directory(gchar *dir_name)
{
	gchar *dir_entry, *dir_entry_abs, *temp;
	GDir *dir;
	GError *error;
	guint file_count = 0;

	g_return_val_if_fail(dir_name, 0);

	dir = g_dir_open(dir_name, 0, &error);

	if (!dir)
	{
		g_printerr("Could not read directory %s: %s\n", dir_name, error->message);
		temp = g_strdup_printf(_("Could not read directory: %s"), error->message);
		error_dialog(temp);
		g_free(temp);

		g_error_free(error);
		return 0;
	}

	do
	{
		dir_entry = (gchar*)g_dir_read_name(dir);

		if (!dir_entry) break;

		// Skip hidden entries
		if (dir_entry[0] == '.') continue;

		// Do we need to dive into a subdirectory?
		dir_entry_abs = g_build_path("/", dir_name, dir_entry, NULL);
		if (g_file_test(dir_entry_abs, G_FILE_TEST_IS_DIR))
		{
			file_count += omp_playlist_track_append_directory(dir_entry_abs);
		} else {
			omp_playlist_track_append_file(dir_entry_abs);
			file_count++;
		}
		g_free(dir_entry_abs);

	} while (TRUE);

	g_dir_close(dir);

	return file_count;
}

/**
 * Utility function that extracts a playlist's name from its file name
 * @param playlist_file File name to extract title from, can contain a path
 * @return String holding the title, must be freed after use
 */
gchar *
get_playlist_title(gchar *playlist_file)
{
	return get_base_file_name(playlist_file);
}
