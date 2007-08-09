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
 * @file persistent.h
 * Manages application configuration and session data
 */

#ifndef PERSISTENT_H
#define PERSISTENT_H

#include "playlist.h"
#include "main.h"

/// Application configuration data
struct _omp_config
{
	gboolean shuffle;									///< Shuffle on/off
	gboolean resume_playback;					///< Resume playback on startup where it left off?
	gint repeat_mode;									///< Repeat mode @see omp_repeat_modes
//	gboolean auto_scroll;						///< Scroll title if it's too long?
	gboolean convert_underscore;			///< Convert '_' to ' '?
	gchar title_format[32];						///< Format string used for title display
	gdouble equalizer_gain;						///< Pre-amplification value before audio stream is fed to equalizer [0.0..1.0]
	gdouble equalizer_bands[11];			///< The gains for each of the equalizer bands [-1.0..1.0]
	gboolean show_numbers_in_pl;			///< Show numbers in playlist?
};

/// Session-persistent data
/// @note Default values should be 0/FALSE as session data will be zeroed on error
struct _omp_session
{
	guint playlist_position;					///< Position within the playlist
	glong track_position;							///< Position to resume playback from within the last played track
	gboolean was_playing;							///< Set to TRUE of track was being played as the player was closed
	gchar filesel_path[256];					///< Last path used in the file selection dialog
	gchar playlist_file[256];					///< Path and file name of current (=last used) playlist
};

extern struct _omp_config *omp_config;
extern struct _omp_session *omp_session;

void omp_config_init();
void omp_config_free();
void omp_config_save();

void omp_session_restore_state();
void omp_session_free();
void omp_session_save();
void omp_session_load();

void omp_session_set_playlist(gchar *playlist_file);
void omp_session_set_track_id(guint track_id);

#endif
