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
 * @file mplayer_playback.h
 * Playback engine interface for utilizing mplayer
 */

#ifndef MPLAYER_PLAYBACK_H
#define MPLAYER_PLAYBACK_H

gboolean omp_mplayback_init();
void omp_mplayback_free();

gboolean omp_mplayback_load_video_from_uri(gchar *uri);
gboolean omp_mplayback_video_loaded();

void omp_mplayback_play();
gboolean omp_mplayback_is_playing();
gint omp_mplayback_get_state();
gulong omp_mplayback_get_video_position();
void omp_mplayback_set_video_position(gulong position);
void omp_mplayback_set_volume(guint volume);

#endif
