/*
 *  OpenMoko Main Menu + Panel Applet
 *
 *  Copyright (C) 2007 First International Computer Inc.
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
    g_debug( "openmoko-panel-demo-simple starting" );

    moko_panel_system_init( &argc, &argv );

    MokoPanelApplet* applet = moko_panel_applet_new();
    moko_panel_applet_set_icon( applet, PKGDATADIR "/btn_menu.png", TRUE );
    //moko_panel_applet_request_size( applet, 79, 32 );
    moko_panel_applet_request_size( applet, 40, 32 );
    moko_panel_applet_show( applet );

    gtk_main();

    g_debug( "openmoko-panel-demo-simple ending" );
    return 0;
}
