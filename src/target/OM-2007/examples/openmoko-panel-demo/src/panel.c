/*
 *  Panel-Demo -- OpenMoko Demo Panel Applet
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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

#include <libmokoui/moko-panel-applet.h>

#include <gtk/gtk.h>
#include <glib.h>

static void button_callback( GtkButton* button, MokoPanelApplet* applet )
{
    moko_panel_applet_close_popup( applet );
}

int main( int argc, char** argv )
{
    g_debug( "openmoko-panel-demo starting" );

    gtk_init( &argc, &argv );

    // usually you should derive an object from the MokoPanelApplet
    // for this demo we go the simple way and just use it...

    MokoPanelApplet* applet = moko_panel_applet_new( &argc, &argv );
    moko_panel_applet_set_icon( applet, PKGDATADIR "/icon.png" );

    // you can add a menu
    GtkMenu* panelmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( panelmenu, fooitem );
    gtk_menu_shell_append( panelmenu, baritem );
    gtk_widget_show_all( GTK_WIDGET(panelmenu) );

    moko_panel_applet_set_popup( applet, GTK_WIDGET(panelmenu), MOKO_PANEL_APPLET_TAP_HOLD_POPUP );

    // or something else
    GtkButton* button = gtk_button_new_with_label( "Hello Applet World!" );
    g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button_callback), applet );
    moko_panel_applet_set_popup( applet, GTK_WIDGET(button), MOKO_PANEL_APPLET_CLICK_POPUP );


    gtk_main();

    g_debug( "openmoko-panel-demo ending" );
    return 0;
}
