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
 * @file main.h
 * Main file
 */

#ifndef MAIN_H
#define MAIN_H

#include <gtk/gtk.h>

typedef enum
{
	OMP_TAB_MAIN = 0,
	OMP_TAB_PLAYLISTS,
	OMP_TAB_PLAYLIST_EDITOR,
	OMP_TAB_FILE_CHOOSER,
	OMP_TABS
} omp_notebook_tab;

extern GtkWidget *omp_notebook;
extern GtkWidget *omp_window;

void omp_application_terminate();

void omp_tab_show(omp_notebook_tab tab);
void omp_tab_hide(omp_notebook_tab tab);
void omp_tab_focus(omp_notebook_tab tab);

#endif
