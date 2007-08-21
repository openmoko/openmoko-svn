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
 * @file playlist.h
 * Playlist handling
 */

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <glib.h>
#include <spiff/spiff_c.h>

#define OMP_PLAYLIST_FILE_EXTENSION "xspf"

#define OMP_EVENT_PLAYLIST_LOADED "playlist_loaded"
#define OMP_EVENT_PLAYLIST_TRACK_CHANGED "playlist_track_changed"
#define OMP_EVENT_PLAYLIST_TRACK_INFO_CHANGED "playlist_track_info_changed"
#define OMP_EVENT_PLAYLIST_TRACK_COUNT_CHANGED "playlist_track_count_changed"

/// Modes available for repetitive track playback
enum omp_repeat_modes
{
	OMP_REPEAT_OFF,									///< Repeat off
	OMP_REPEAT_CURRENT_ONCE,				///< Repeat current track once, then proceed with next track
	OMP_REPEAT_CURRENT,							///< Repeat current track forever
	OMP_REPEAT_PLAYLIST							///< Repeat entire playlist
};

extern struct spiff_list *omp_playlist;
extern guint omp_playlist_track_count;
extern gchar *omp_playlist_title;

extern struct spiff_track *omp_playlist_current_track;
extern guint omp_playlist_current_track_id;

/// Playlist iterator
typedef struct playlist_iter
{
	struct spiff_track *track;
	guint track_num;
} playlist_iter;

void omp_playlist_init();
void omp_playlist_free();
gboolean omp_playlist_load(gchar *playlist_file, gboolean do_state_reset);
void omp_playlist_create(gchar *playlist_file);
void omp_playlist_save();
void omp_playlist_delete(gchar *playlist_file);

gboolean omp_playlist_set_current_track(gint playlist_pos);
gboolean omp_playlist_set_prev_track();
gboolean omp_playlist_set_next_track();

gchar *omp_playlist_resolve_track(struct spiff_track *track);
gboolean omp_playlist_load_current_track();
void omp_playlist_get_track_info(guint track_id, gchar **title, guint *duration);
void omp_playlist_update_track_count();

playlist_iter *omp_playlist_init_iterator();
void omp_playlist_get_track_info_from_iter(playlist_iter *iter, guint *track_num,
	gchar **track_title, guint *duration);
void omp_playlist_advance_iter(playlist_iter *iter);
gboolean omp_playlist_iter_finished(playlist_iter *iter);

gchar *get_playlist_title(gchar *playlist_file);

#endif
