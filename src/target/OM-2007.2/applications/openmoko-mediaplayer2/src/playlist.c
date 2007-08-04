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
 * @file playlist.c
 * Playlist handling
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <spiff/spiff_c.h>

#include "playlist.h"
#include "main.h"
#include "mainwin.h"
#include "playback.h"

struct spiff_list *omp_playlist = NULL;										///< Loaded playlist
guint omp_playlist_track_count = 0;												///< Number of tracks stored within the current playlist

struct spiff_track *omp_playlist_current_track = NULL;		///< Current track's data
guint omp_playlist_current_track_id = -1;									///< Numerical id of the current track within the playlist

/// This linked list holds all tracks that were played in this session, most recently played entry first
GSList *omp_track_history = NULL;

/// Single element for the track history
struct omp_track_history_entry
{
	struct spiff_track *track;
	guint track_id;
};



/**
 * Initialize all things playlist
 */
void
omp_playlist_init()
{
	// Hook up event handlers to the playback routines
	g_signal_connect(G_OBJECT(omp_main_window), OMP_EVENT_PLAYBACK_EOS, G_CALLBACK(omp_playlist_process_eos_event), NULL);

	// Create the signals we emit: no params, no return value
	g_signal_new(OMP_EVENT_PLAYLIST_TRACK_CHANGED, G_TYPE_OBJECT, 0, 0, 0, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}

/**
 * Frees resouces allocated for playlist handling
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
		spiff_free(omp_playlist);
	}
}

/**
 * Load playlist
 * @todo Count playlist entries on load, trigger "playlist loaded" event to update UI
 */
void
omp_playlist_load(gchar *playlist_file)
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
		spiff_free(omp_playlist);
	}

	// Update config unless target and source are the same
	if (omp_config->playlist_file != playlist_file)
	{
		g_snprintf(omp_config->playlist_file, sizeof(omp_config->playlist_file), "%s", playlist_file);
	}

	// Load playlist
	omp_playlist = spiff_parse(playlist_file);

	if (!omp_playlist)
	{
		g_printerr("Could not load playlist: %s\n", playlist_file);
	}
}

/**
 * Tries to set the position within the playlist and indicates success/failure
 * @param playlist_pos New position, counting starts at 0
 */
gboolean
omp_playlist_set_current_track(gint playlist_pos)
{
	gboolean position_valid = FALSE;
	struct spiff_track *track;
	gint track_num = 0;

	#ifdef DEBUG
		g_printf("Setting current track to #%d\n", playlist_pos);
	#endif

	if (!omp_playlist)
	{
		return FALSE;
	}

	// Walk through the playlist and see if the new position is valid
	for (track=omp_playlist->tracks; track!=NULL; track=track->next, track_num++)
	{
		if (track_num == playlist_pos)
		{
			omp_playlist_current_track		= track;
			omp_playlist_current_track_id	= track_num;
			position_valid = TRUE;
		}
	}

	if (position_valid)
	{
		omp_playlist_track_count = track_num;

		// Emit signal to update UI and the like
		g_signal_emit_by_name(G_OBJECT(omp_main_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);
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
	struct omp_track_history_entry *history_entry;
	struct spiff_track *track;
	gboolean was_playing;
	gboolean is_new_track = FALSE;

	if (!omp_playlist_current_track)
	{
		return;
	}

	// If track playing time is >= 10 seconds we just jump back to the beginning of the track
	if (omp_playback_get_track_position() >= 10)
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
		history_entry = (struct omp_track_history_entry*)(omp_track_history->data);
		omp_playlist_current_track		= history_entry->track;
		omp_playlist_current_track_id	= history_entry->track_id;
		
		// Delete first entry from the list
		g_free(history_entry);
		omp_track_history = g_slist_delete_link(omp_track_history, omp_track_history);

		is_new_track = TRUE;

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
			}
		}
	}

	if (is_new_track)
	{
		// Emit signal to update UI and the like
		g_signal_emit_by_name(G_OBJECT(omp_main_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);

		// Load track and start playing if needed
		if (omp_playlist_load_current_track())
		{
			if (was_playing) omp_playback_play();

		} else {

			// Uh-oh, track failed to load - let's find another one, shall we?
			is_new_track = FALSE;
			goto try_again;
		}
	}

	return is_new_track;
}

/**
 * Moves one track forward in playlist
 * @return TRUE if a new track is to be played, FALSE if current track didn't change
 * @todo Shuffle mode, repeat
 * @todo Will cause an infinite loop if playlist only consists of tracks that can't be played and player is in shuffle mode
 */
gboolean
omp_playlist_set_next_track()
{
	struct omp_track_history_entry *history_entry;
	gboolean was_playing;
	gboolean is_new_track = FALSE;

	if (!omp_playlist_current_track)
	{
		return;
	}

	// Get player state so we can continue playback if necessary
	was_playing = (omp_playback_get_state() == OMP_PLAYBACK_STATE_PLAYING);

try_again:

	// Prepare the history entry - if we don't need it we'll just free it again
	history_entry = g_new(struct omp_track_history_entry, 1);
	history_entry->track		= omp_playlist_current_track;
	history_entry->track_id	= omp_playlist_current_track_id;

	// Do we have a track to play?
	if (omp_playlist_current_track->next)
	{
		omp_playlist_current_track = omp_playlist_current_track->next;
		omp_playlist_current_track_id++;

		// Yes, we were able to find a new track to play
		is_new_track = TRUE;
	}

	if (is_new_track)
	{
		// Add track to track history
		omp_track_history = g_slist_prepend(omp_track_history, (gpointer)history_entry);

		// Emit signal to update UI and the like
		g_signal_emit_by_name(G_OBJECT(omp_main_window), OMP_EVENT_PLAYLIST_TRACK_CHANGED);

		// Load track and start playing if needed
		if (omp_playlist_load_current_track())
		{
			if (was_playing) omp_playback_play();

		} else {

			// Uh-oh, track failed to load - let's find another one, shall we?
			is_new_track = FALSE;
			goto try_again;
		}

	} else {

		// We're not making use of the history entry as the track didn't change
		g_free(history_entry);
	}

	return is_new_track;
}

/**
 * Signal handler that gets called whenever the current stream ends
 */
void omp_playlist_process_eos_event()
{
	if (omp_playlist_set_next_track())
	{
		// If we received an eos event we were obviously playing so let's continue doing so
		omp_playback_play();
	}
}

/**
 * Uses the URI(s) and metadata information of a track to locate the resource to play, then returns a playable URI
 * @return URI of a playable resource
 * @note If the URI of the found resource is not in the track's locations list already it is added and the playlist saved
 * @todo Actually make this function do what it's supposed to do :)
 */
gchar*
omp_playlist_resolve_track(struct spiff_track *track)
{
	if (!track)
	{
		g_printerr("Resolve request for an undefined track, returning null as URI.\n");
		return NULL;
	}

	if (track->locations->value)
	{
		return(g_strdup(track->locations->value));
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
		g_free(track_uri);
		return track_loaded;
	}

	return FALSE;
}

