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

// This size is valid for the entire application and determines the size of the button's icons
#define BUTTON_PIXMAP_SIZE 36

// Where to find application-specific images relative to $DATA_DIR (/usr/share/openmoko-mediaplayer)?
#define RELATIVE_UI_IMAGE_PATH "/images"

// Where to find the playlist files relative to the user's home directory?
#define RELATIVE_PLAYLIST_PATH "/playlists"

// What file to save/load session data to/from? File name is relative to user's home directory
#define SESSION_FILE_NAME "/.openmoko-mediaplayer"


struct _omp_notebook_tab_ids
{
	guint main, playlists, editor, files;
};

struct _omp_notebook_tabs
{
	GtkWidget *main, *playlists, *editor, *files;
};

extern struct _omp_notebook_tab_ids *omp_notebook_tab_ids;
extern struct _omp_notebook_tabs *omp_notebook_tabs;
extern GtkWidget *omp_notebook;
extern GtkWidget *omp_window;

void omp_application_terminate();

#endif
