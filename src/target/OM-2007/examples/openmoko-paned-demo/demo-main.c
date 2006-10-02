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
#include <mokoui/moko-toolbar.h>

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenutoolbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktoolbutton.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtkvbox.h>

#include <stdlib.h>

int main( int argc, char** argv )
{
    g_debug( "openmoko-paned-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko-Paned-Demo" );

    /* main window */
    MokoPanedWindow* window = MOKO_PANED_WINDOW(moko_paned_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    moko_paned_window_set_application_menu( window, appmenu );

    /* filter menu */
    GSList* list = NULL;
    list = g_slist_append( list, "All" );
    list = g_slist_append( list, "Odd" );
    list = g_slist_append( list, "Even" );
    moko_paned_window_set_filter_menu( window, list );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /* navigation area */
    GtkButton* navigationlist = gtk_button_new_with_label( "Hello Navigation Area!" );
    moko_paned_window_set_upper_pane( window, GTK_WIDGET(navigationlist) );

    /* tool bar */
    MokoToolBar* toolbar = MOKO_TOOLBAR(moko_toolbar_new());
    GtkToolButton* tool_search = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "search" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_search, 0 );
    moko_paned_window_add_toolbar( window, GTK_TOOLBAR(toolbar) );

    GtkToolButton* tool_action1 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action1" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action1, 1 );

    GtkMenu* actionmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( actionmenu, fooitem );
    gtk_menu_shell_append( actionmenu, baritem );

    GtkMenuToolButton* tool_menu = GTK_MENU_TOOL_BUTTON(gtk_menu_tool_button_new( NULL, "amenu" ));
    gtk_menu_tool_button_set_menu( tool_menu, actionmenu );
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), GTK_TOOL_BUTTON(tool_menu), 2 );

    GtkToolButton* tool_action3 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action3" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action3, 3 );

    GtkToolButton* tool_action4 = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "action4" ));
    gtk_toolbar_insert( GTK_TOOLBAR(toolbar), tool_action4, 4 );

    /* details area */
    GtkButton* detailslist = gtk_button_new_with_label( "Hello Details Area!" );
    moko_paned_window_set_lower_pane( window, GTK_WIDGET(detailslist) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-paned-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-paned-demo left main loop" );

    return 0;
}
