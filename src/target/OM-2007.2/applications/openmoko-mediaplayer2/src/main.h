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

#ifndef MAIN_H
#define MAIN_H

#include <gtk/gtk.h>

// Where to find application-specific images relative to $DATA_DIR (/usr/share/openmoko-mediaplayer)?
#define RELATIVE_UI_IMAGE_PATH "/images"

// Where to find the playlist files relative to the user's home directory?
#define RELATIVE_PLAYLIST_PATH "/playlists"

// What file to save/load session data to/from? File name is relative to user's home directory
#define SESSION_FILE_NAME "/.openmoko-mediaplayer"

enum omp_notebook_tabs
{
	OMP_TAB_MAIN = 0,
	OMP_TAB_PLAYLISTS,
	OMP_TAB_PLAYLIST_EDITOR,
	OMP_TABS
};

extern GtkWidget *omp_notebook;
extern GtkWidget *omp_window;

void omp_application_terminate();

void omp_show_tab(guint tab_id);
void omp_hide_tab(guint tab_id);

#endif
