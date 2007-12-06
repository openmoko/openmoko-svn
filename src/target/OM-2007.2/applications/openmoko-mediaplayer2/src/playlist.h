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
 * @file playlist.h
 * Playlist handling
 */

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <glib.h>

#include "omp_spiff_c.h"

#define OMP_PLAYLIST_FILE_EXTENSION "xspf"

#define OMP_EVENT_PLAYLIST_LOADED "playlist_loaded"
#define OMP_EVENT_PLAYLIST_TRACK_CHANGED "playlist_track_changed"
#define OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED "playlist_track_info_changed"
#define OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED "playlist_track_count_changed"



/// Track info data used with the OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED signal
typedef struct _omp_track_info
{
	guint id;
	gchar *artist, *title;
	gulong duration;
} omp_track_info;

/// Modes available for repetitive track playback
typedef enum
{
	OMP_REPEAT_OFF = 0,     ///< Repeat off
	OMP_REPEAT_ONCE,        ///< Repeat current track once, then proceed with next track
	OMP_REPEAT_CURRENT,     ///< Repeat current track forever
	OMP_REPEAT_ALL,         ///< Repeat entire playlist
	OMP_REPEAT_COUNT        ///< End-of-list marker for mode enumeration
} omp_repeat_mode;

extern omp_spiff_list *omp_playlist;
extern guint omp_playlist_track_count;
extern gchar *omp_playlist_title;

extern omp_spiff_track *omp_playlist_current_track;
extern guint omp_playlist_current_track_id;

/// Playlist iterator
typedef struct _omp_playlist_iter
{
	omp_spiff_track *track;
	guint track_num;
} omp_playlist_iter;



void omp_playlist_init();
void omp_playlist_free();
gboolean omp_playlist_load(gchar *playlist_file, gboolean do_state_reset);
void omp_playlist_create(gchar *playlist_file);
void omp_playlist_save();
void omp_playlist_delete(gchar *playlist_file);

gboolean omp_playlist_set_current_track(gint playlist_pos);
gboolean omp_playlist_set_prev_track();
gboolean omp_playlist_set_next_track();

gchar *omp_playlist_resolve_track(omp_spiff_track *track);
gboolean omp_playlist_load_current_track();
void omp_playlist_get_track_info(gint track_id, gchar **artist, gchar **title, gulong *duration);
void omp_playlist_update_track_count();

omp_playlist_iter *omp_playlist_init_iterator();
void omp_playlist_get_track_from_iter(omp_playlist_iter *iter, guint *track_num,
	gchar **track_artist, gchar **track_title, gulong *duration);
void omp_playlist_advance_iter(omp_playlist_iter *iter);
gboolean omp_playlist_iter_finished(omp_playlist_iter *iter);

gboolean omp_playlist_track_append_file(gchar *file_name);
guint omp_playlist_track_append_directory(gchar *dir_name);

gchar *get_playlist_title(gchar *playlist_file);

// Taken from uriparser's UriCommon.h, which sadly is not installed with uriparser
char uriHexToLetterA(unsigned int value);

#endif
