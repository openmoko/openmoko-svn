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
#include <gtk/gtklabel.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>

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
    g_set_application_name( "Paned-Demo" );

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

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /* navigation area */
    GtkLabel* navigation = gtk_label_new( "Add your widget for navigating\nthrough appplication specific\ndata here" );
    moko_paned_window_set_upper_pane( window, GTK_WIDGET(navigation) );

    /* tool bar */
    MokoToolBox* toolbox;
    if (!searchmode)
    {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new());
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
    } else {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
        moko_tool_box_add_action_button( toolbox );
    }
    moko_paned_window_add_toolbox( window, toolbox );

#if 0
    GtkToolButton* tool_action1 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action1" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action1, -1 );

    GtkMenu* actionmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( actionmenu, fooitem );
    gtk_menu_shell_append( actionmenu, baritem );

    GtkMenuToolButton* tool_menu = GTK_MENU_TOOL_BUTTON(gtk_menu_tool_button_new( NULL, "amenu" ));
    gtk_menu_tool_button_set_menu( tool_menu, actionmenu );
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), GTK_TOOL_BUTTON(tool_menu), -1 );

    GtkToolButton* tool_action3 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action3" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action3, -1 );

    GtkToolButton* tool_action4 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action4" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action4, -1 );
#endif
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
