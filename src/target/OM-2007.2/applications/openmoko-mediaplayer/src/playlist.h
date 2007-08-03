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

#ifndef _PLAYLIST_H
#define _PLAYLIST_H

#include <spiff/spiff_c.h>

#define OMP_EVENT_PREV_TRACK "prev_track"
#define OMP_EVENT_NEXT_TRACK "next_track"


extern struct _omp_playlist_events omp_playlist_events;


extern struct spiff_list *omp_playlist;
extern guint omp_playlist_track_count;

extern struct spiff_track *omp_playlist_current_track;
extern guint omp_playlist_current_track_id;

void omp_playlist_init();
void omp_playlist_free();
void omp_playlist_load(gchar *playlist_file);

gboolean omp_playlist_set_current_track(gint playlist_pos);
gboolean omp_playlist_set_prev_track();
gboolean omp_playlist_set_next_track();
void omp_playlist_process_eos_event();

gchar *omp_playlist_resolve_track(struct spiff_track *track);
gboolean omp_playlist_load_current_track();

#endif
