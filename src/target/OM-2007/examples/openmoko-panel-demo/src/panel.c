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

int main( int argc, char** argv )
{
    g_debug( "openmoko-panel-demo starting" );

    gtk_init( &argc, &argv );

    // usually you should derive an object from the MokoPanelApplet
    // for this demo we go the simple way and just use it...

    MokoPanelApplet* applet = moko_panel_applet_new( &argc, &argv );
    moko_panel_applet_set_icon( applet, PKGDATADIR "/icon.png" );

    gtk_main();

    g_debug( "openmoko-panel-demo ending" );
    return 0;
}
