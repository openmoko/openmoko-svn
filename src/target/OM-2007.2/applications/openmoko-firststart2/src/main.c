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

#include <libmokoui2/moko-finger-scroll.h>

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <glade/glade-build.h>

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);
    GtkWidget* assistant = gtk_assistant_new();
    g_signal_connect( assistant, "close", gtk_main_quit, NULL);
    g_signal_connect( assistant, "cancel", gtk_main_quit, NULL);
    gtk_window_set_title( GTK_WINDOW(assistant), "Welcome");

    GladeXML* ui1 = glade_xml_new( PKGDATADIR "/ui.glade", "page1", NULL );
    GtkWidget* page1 = glade_xml_get_widget( ui1, "page1" );

    GladeXML* ui2 = glade_xml_new( PKGDATADIR "/ui.glade", "page2", NULL );
    GtkWidget* page2 = glade_xml_get_widget( ui2, "page2" );

    GladeXML* ui3 = glade_xml_new( PKGDATADIR "/ui.glade", "page3", NULL );
    GtkWidget* page3 = glade_xml_get_widget( ui3, "page3" );

    GladeXML* ui4 = glade_xml_new( PKGDATADIR "/ui.glade", "page4", NULL );
    GtkWidget* page4 = glade_xml_get_widget( ui4, "page4" );

    gtk_assistant_append_page( GTK_ASSISTANT(assistant), page1 );
    gtk_assistant_set_page_type( GTK_ASSISTANT(assistant), page1, GTK_ASSISTANT_PAGE_CONTENT );
    gtk_assistant_set_page_title( GTK_ASSISTANT(assistant), page1, "Welcome to OpenMoko" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT(assistant), page1, TRUE );

    gtk_assistant_append_page( GTK_ASSISTANT(assistant), page2 );
    gtk_assistant_set_page_type( GTK_ASSISTANT(assistant), page2, GTK_ASSISTANT_PAGE_CONTENT );
    gtk_assistant_set_page_title( GTK_ASSISTANT(assistant), page2, "Time Settings" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT(assistant), page2, TRUE );

    gtk_assistant_append_page( GTK_ASSISTANT(assistant), page3 );
    gtk_assistant_set_page_type( GTK_ASSISTANT(assistant), page3, GTK_ASSISTANT_PAGE_CONTENT );
    gtk_assistant_set_page_title( GTK_ASSISTANT(assistant), page3, "Sound Settings" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT(assistant), page3, TRUE );

    gtk_assistant_append_page( GTK_ASSISTANT(assistant), page4 );
    gtk_assistant_set_page_type( GTK_ASSISTANT(assistant), page4, GTK_ASSISTANT_PAGE_SUMMARY );
    gtk_assistant_set_page_title( GTK_ASSISTANT(assistant), page4, "Congratulations, you're done!" );
    gtk_assistant_set_page_complete( GTK_ASSISTANT(assistant), page4, TRUE );

    gtk_widget_show_all( assistant );

    gtk_main ();

    return 0;
}
