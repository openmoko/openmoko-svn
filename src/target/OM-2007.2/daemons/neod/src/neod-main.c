/*
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
 */

#include "buttonactions.h"

#include <gtk/gtk.h>

int main( int argc, char** argv )
{
    //FIXME add daemonizing
    gtk_init( &argc, &argv );
    if ( neod_buttonactions_install_watcher() )
    {
        neod_buttonactions_powersave_reset();
        neod_buttonactions_set_display( 100 );
        neod_buttonactions_sound_init();
        g_timeout_add_seconds( 10, (GSourceFunc) neod_buttonactions_initial_update, NULL );
        gtk_main();
        return 0;
    }
    else
        return -1;
}

