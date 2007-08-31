/*
 * Authored by Michael 'Mickey' Lauer <mickey@vanille-media.de>
 * Copyright (C) 2007 OpenMoko, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "moko-terminal.h"
#include <libmokoui2/moko-finger-scroll.h>
#include <gtk/gtk.h>

#define NUM_TERMINALS 3

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", gtk_main_quit, NULL);
    gtk_window_set_title (GTK_WINDOW (window), "Terminal");

    GtkNotebook* notebook = gtk_notebook_new ();
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);

    for ( int i = 0; i < NUM_TERMINALS; ++i )
    {

        MokoTerminal* terminal = moko_terminal_new();
        /* navigation */
        gtk_notebook_append_page( GTK_NOTEBOOK(notebook), terminal,
                                gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_LARGE_TOOLBAR));
        gtk_container_child_set( GTK_CONTAINER(notebook), terminal, "tab-expand",
                                TRUE, NULL);
    }

    gtk_widget_show_all (window);

    gtk_main ();

    return 0;
}
