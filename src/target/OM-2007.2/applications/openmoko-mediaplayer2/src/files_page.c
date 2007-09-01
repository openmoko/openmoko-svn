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
 * @file files_page.c
 * Playlist editor, part 2: file adding window
 */

#include "files_page.h"

/**
 * Creates the file chooser UI page and all its elements
 */
GtkWidget *
omp_files_page_create()
{
	GtkWidget *main_vbox;

	// Create main container
	main_vbox = gtk_vbox_new(FALSE, 0);

	return main_vbox;
}
