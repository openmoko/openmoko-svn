/*
 *  ChordMaster -- A Chord Application for the OpenMoko Framework
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "callbacks.h"
#include "chordsdb.h"
#include "fretboard-widget.h"
#include "main.h"

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-tree-view.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkscrolledwindow.h>

int main( int argc, char** argv )
{
    g_debug( "openmoko-chordmaster starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application data object */
    ChordMasterData* d = g_new( ChordMasterData, 1 );
    d->chordsdb = chordsdb_new();

    /* application object */
    d->app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "ChordMaster" );

    /* ui */
    setup_ui( d );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(d->window) );
    gtk_main();

    return 0;
}

void setup_ui( ChordMasterData* d )
{
    /* main window */
    d->window = MOKO_PANED_WINDOW(moko_paned_window_new());

    /* application menu */
    d->menu = gtk_menu_new();
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(closeitem) );
    moko_paned_window_set_application_menu( d->window, GTK_MENU(d->menu) );

    /* filter menu */
    GtkMenu* filtmenu = GTK_MENU(gtk_menu_new());
    for (GSList* c = chordsdb_get_categories( d->chordsdb ); c; c = g_slist_next(c) )
    {
        gchar* category = (gchar*) c->data;
        g_debug( "adding category '%s'", category );
        gtk_menu_shell_append( filtmenu, gtk_menu_item_new_with_label( category ) );
    }
    gtk_menu_shell_append( filtmenu, gtk_separator_menu_item_new() );
    gtk_menu_shell_append( filtmenu, gtk_menu_item_new_with_label( "All" ) );

    moko_paned_window_set_filter_menu( d->window, filtmenu );
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );

    /* connect close event */
    g_signal_connect( G_OBJECT(d->window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /* toolboox */
    GtkButton* button1;
    GtkButton* button2;
    GtkButton* button3;
    GtkButton* button4;

    /* tool bar */
    d->toolbox = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    gtk_widget_grab_focus( GTK_WIDGET(d->toolbox) );
    g_signal_connect( G_OBJECT(d->toolbox), "searchbox_visible", G_CALLBACK(cb_search_visible), d );
    g_signal_connect( G_OBJECT(d->toolbox), "searchbox_invisible", G_CALLBACK(cb_search_invisible), d );

    button1 = moko_tool_box_add_action_button( d->toolbox );
    gtk_button_set_label( button1, "Add" );
    button2 = moko_tool_box_add_action_button( d->toolbox );
    gtk_button_set_label( button2, "Remove" );
    button3 = moko_tool_box_add_action_button( d->toolbox );
    gtk_button_set_label( button3, "ActMenu" );
    button4 = moko_tool_box_add_action_button( d->toolbox );
    gtk_button_set_label( button4, "Action 4" );
    moko_paned_window_add_toolbox( d->window, d->toolbox );

    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_button1_clicked), d );
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_button2_clicked), d );
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_button3_clicked), d );
    g_signal_connect( G_OBJECT(button4), "clicked", G_CALLBACK(cb_button4_clicked), d );

    GtkMenu* actionmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( GTK_MENU_SHELL(actionmenu), GTK_WIDGET(fooitem) );
    gtk_menu_shell_append( GTK_MENU_SHELL(actionmenu), GTK_WIDGET(baritem) );
    moko_pixmap_button_set_menu( MOKO_PIXMAP_BUTTON(button3), actionmenu );
    gtk_widget_show_all( actionmenu );

    populate_navigation_area( d );
    populate_details_area( d );

    moko_menu_box_set_active_filter( menubox, "All" );
}

void populate_navigation_area( ChordMasterData* d )
{
    d->liststore = gtk_list_store_new( NUM_COLS, G_TYPE_STRING, G_TYPE_STRING );
    d->view = moko_tree_view_new_with_model( d->liststore );

    GtkCellRenderer* ren = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "Chordname", ren, "text", COLUMN_NAME, NULL );
    moko_tree_view_append_column( d->view, column );

    ren = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes( "Fretboard", ren, "text", COLUMN_FRETS, NULL );
    moko_tree_view_append_column( d->view, column );

    moko_paned_window_set_navigation_pane( d->window, GTK_WIDGET(moko_tree_view_put_into_scrolled_window(d->view)) );

    GtkTreeSelection* selection = gtk_tree_view_get_selection( d->view );
    g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(cb_cursor_changed), d );

    GtkEntryCompletion* ec = gtk_entry_completion_new();
    gtk_entry_completion_set_model( ec, d->liststore );
    gtk_entry_completion_set_text_column( ec, COLUMN_NAME );
    GtkEntry* entry = moko_tool_box_get_entry( d->toolbox );
    //FIXME what happens in case the toolbox has no entry?
    gtk_entry_set_completion( entry, ec );
    g_signal_connect( G_OBJECT(ec), "match-selected", G_CALLBACK(cb_entry_completion_completed), d );
}

void populate_details_area( ChordMasterData* d )
{
    d->fretboard = fretboard_widget_new();
    moko_paned_window_set_details_pane( d->window, GTK_WIDGET(d->fretboard) );
}