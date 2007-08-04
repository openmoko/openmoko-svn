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
 * @file main.h
 * Main file
 */

#ifndef _MAIN_H
#define _MAIN_H

/// Modes available for repetitive track playback
enum omp_repeat_modes
{
	OMP_REPEAT_OFF,									///< Repeat off
	OMP_REPEAT_CURRENT_ONCE,				///< Repeat current track once, then proceed with next track
	OMP_REPEAT_CURRENT,							///< Repeat current track forever
	OMP_REPEAT_PLAYLIST							///< Repeat playlist
};

/// Session-persistent configuration data
struct _omp_config
{
	gboolean shuffle;								///< Shuffle on/off
	gboolean resume_playback;				///< Resume playback on startup where it left off?
	gint repeat_mode;								///< Repeat mode @see omp_repeat_modes
//	gboolean auto_scroll;						///< Scroll title if it's too long?
	gboolean convert_underscore;		///< Convert '_' to ' '?
	gchar title_format[32];					///< Format string used for title display
	gdouble equalizer_gain;					///< Pre-amplification value before audio stream is fed to equalizer [0.0..1.0]
	gdouble equalizer_bands[11];		///< The gains for each of the equalizer bands [-1.0..1.0]
	gboolean show_numbers_in_pl;		///< Show numbers in playlist?
	gchar filesel_path[256];				///< Last path used in the file selection dialog
//	gchar *playlist_path;						///< Last path used for the playlist selection dialog
	gchar playlist_file[256];				///< Path and file name of current (=last used) playlist
	gint playlist_position;					///< Position within the playlist
	glong track_position;						///< Position to resume playback from within the last played track
};

extern struct _omp_config *omp_config;
extern struct _omp_config omp_default_config;

void omp_config_save();
void omp_config_load();
void omp_config_update();

#endif
