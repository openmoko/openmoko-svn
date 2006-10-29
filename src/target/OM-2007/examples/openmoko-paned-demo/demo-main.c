/*
 *  Paned-Demo -- OpenMoko Demo Application
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <mokoui/moko-application.h>
#include <mokoui/moko-paned-window.h>
#include <mokoui/moko-toolbox.h>

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>

void cb_filter_changed(GtkMenu* menu, gchar* text, gpointer user_data )
{
    g_debug( "openmoko-paned-demo: filter changed to '%s'", text );
}

void cb_button1_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-paned-demo: button1 clicked" );
}

void cb_button2_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-paned-demo: button2 clicked" );
}

void cb_button3_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-paned-demo: button3 clicked" );
}

void cb_button4_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-paned-demo: button4 clicked" );
}

static gboolean searchmode = TRUE;

int main( int argc, char** argv )
{
    g_debug( "openmoko-paned-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    if ( argc > 1 && strcmp( argv[1], "-no-search" ) == 0)
    {
        g_debug( "disabling search mode" );
        searchmode = FALSE;
    }

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Demo" );

    /* main window */
    MokoPanedWindow* window = MOKO_PANED_WINDOW(moko_paned_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    moko_paned_window_set_application_menu( window, appmenu );

    /* filter menu */
    GtkMenu* filtmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* item1 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "All" ));
    gtk_menu_shell_append( filtmenu, item1 );
    GtkMenuItem* item2 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Odd" ));
    gtk_menu_shell_append( filtmenu, item2 );
    GtkMenuItem* item3 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Even" ));
    gtk_menu_shell_append( filtmenu, item3 );
    moko_paned_window_set_filter_menu( window, filtmenu );
    MokoMenuBox* menubox = moko_paned_window_get_menubox( window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), NULL );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL );

    /* navigation area */
    GtkLabel* navigation = gtk_label_new( "Add your widget for navigating\nthrough appplication specific\ndata here" );
    moko_paned_window_set_upper_pane( window, GTK_WIDGET(navigation) );

    GtkButton* button1;
    GtkButton* button2;
    GtkButton* button3;
    GtkButton* button4;

    /* tool bar */
    MokoToolBox* toolbox;
    if (!searchmode)
    {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new());
    } else {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    }
    button1 = moko_tool_box_add_action_button( toolbox );
    gtk_button_set_label( button1, "Action 1" );
    button2 = moko_tool_box_add_action_button( toolbox );
    gtk_button_set_label( button2, "Action 2" );
    button3 = moko_tool_box_add_action_button( toolbox );
    gtk_button_set_label( button3, "ActMenu" );
    button4 = moko_tool_box_add_action_button( toolbox );
    gtk_button_set_label( button4, "Action 4" );
    moko_paned_window_add_toolbox( window, toolbox );

    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_button1_clicked), NULL );
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_button2_clicked), NULL );
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_button3_clicked), NULL );
    g_signal_connect( G_OBJECT(button4), "clicked", G_CALLBACK(cb_button4_clicked), NULL );

    GtkMenu* actionmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( actionmenu, fooitem );
    gtk_menu_shell_append( actionmenu, baritem );
    moko_pixmap_button_set_menu( MOKO_PIXMAP_BUTTON(button3), actionmenu );
    gtk_widget_show_all( actionmenu );

    /* details area */
    GtkLabel* details = gtk_label_new( "Add your widget for showing\ndetails for the selected\ndata entry here" );
    moko_paned_window_set_lower_pane( window, GTK_WIDGET(details) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-paned-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-paned-demo left main loop" );

    return 0;
}
