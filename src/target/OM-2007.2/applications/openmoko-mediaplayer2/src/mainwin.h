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
 * @file mainwin.h
 * Main window handling
 */

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include <gtk/gtk.h>

#define WIDGET_CAPTION_TRACK_TIME "%d:%.2d / %d:%.2d"
#define WIDGET_CAPTION_TRACK_NUM "%.3d / %.3d"
#define WIDGET_CAPTION_VOLUME "%d%%"

// Determines how many seconds the engine will seek if the FFWD/REW buttons are clicked
#define BUTTON_SEEK_DISTANCE 10


extern GtkWidget *omp_main_window;

void omp_application_terminate();

void omp_main_window_show();
void omp_main_window_hide();
void omp_main_window_create();
void omp_main_connect_signals();

void omp_main_update_track_change();
void omp_main_update_status_change();
void omp_main_update_track_position();

#endif
