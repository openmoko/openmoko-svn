/*
 *  openmoko-panel-mainmenu: handle action buttons
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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
#include "buttonactions.h"

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

//FIXME find out through sysfs
#if 0
    #define AUX_BUTTON_EVENT_PATH "/dev/input/event0"
    #define AUX_BUTTON_KEYCODE 169
    #define POWER_BUTTON_EVENT_PATH "/dev/input/event2"
    #define POWER_BUTTON_KEYCODE 116
#else
    #define AUX_BUTTON_EVENT_PATH "/dev/input/event0"
    #define AUX_BUTTON_KEYCODE 0x25
    #define POWER_BUTTON_EVENT_PATH "/dev/input/event2"
    #define POWER_BUTTON_KEYCODE 0x25
#endif

GPollFD aux_fd;
GPollFD power_fd;

GTimer* aux_timer;
GTimer* power_timer;

gboolean panel_mainmenu_install_watcher()
{
    int auxfd = open( AUX_BUTTON_EVENT_PATH, O_RDONLY );
    if ( auxfd < 0 )
    {
        g_debug( "can't open " AUX_BUTTON_EVENT_PATH " (%s)", strerror( errno ) );
        return FALSE;
    }
    int powerfd = open( POWER_BUTTON_EVENT_PATH, O_RDONLY );
    if ( powerfd < 0 )
    {
        g_debug( "can't open " POWER_BUTTON_EVENT_PATH " (%s)", strerror( errno ) );
        return FALSE;
    }
    static GSourceFuncs funcs = {
        panel_mainmenu_input_prepare,
        panel_mainmenu_input_check,
        panel_mainmenu_input_dispatch,
    NULL,
    };
    GSource* button_watcher = g_source_new( &funcs, sizeof (GSource) );
    aux_fd.fd = auxfd;
    aux_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    aux_fd.revents = 0;
    g_source_add_poll( button_watcher, &aux_fd );
    aux_timer = g_timer_new();
    power_fd.fd = powerfd;
    power_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    power_fd.revents = 0;
    g_source_add_poll( button_watcher, &power_fd );
    power_timer = g_timer_new();
    g_source_attach( button_watcher, NULL );
    return TRUE;
}


gboolean panel_mainmenu_input_prepare( GSource* source, gint* timeout )
{
    g_debug( "prepare" );
    return FALSE;
}


gboolean panel_mainmenu_input_check( GSource* source )
{
    g_debug( "check" );
    return ( ( aux_fd.revents & G_IO_IN ) || ( power_fd.revents & G_IO_IN ) );
}


gboolean panel_mainmenu_input_dispatch( GSource* source, GSourceFunc callback, gpointer data )
{
    g_debug( "dispatch" );
    if ( aux_fd.revents & G_IO_IN )
    {
        struct input_event event;
        int size = read( aux_fd.fd, &event, sizeof( struct input_event ) );
        g_debug( "read %d bytes from aux_fd %d", size, aux_fd.fd );
        g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
        //g_timeout_add( 1 * 1000, (GSourceFunc) panel_mainmenu_input_timeout, NULL);
        if ( event.type == 1 && event.code == AUX_BUTTON_KEYCODE )
        {
            if ( event.value == 1 ) /* pressed */
            {
                g_debug( "resetting aux timer" );
                g_timer_reset( aux_timer );
            }
            else if ( event.value == 0 ) /* released */
            {
                g_debug( "triggering aux function" );
                panel_mainmenu_aux_timeout( g_timer_elapsed( aux_timer, NULL ) );
            }
        }
    }
    if ( power_fd.revents & G_IO_IN )
    {
        struct input_event event;
        int size = read( power_fd.fd, &event, sizeof( struct input_event ) );
        g_debug( "read %d bytes from power_fd %d", size, power_fd.fd );
        g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
        //g_timeout_add( 1 * 1000, (GSourceFunc) panel_mainmenu_power_timeout, NULL);
        if ( event.type == 1 && event.code == POWER_BUTTON_KEYCODE )
        {
            if ( event.value == 1 ) /* pressed */
            {
                g_debug( "resetting power timer" );
                g_timer_reset( power_timer );
            }
            else if ( event.value == 0 ) /* released */
            {
                g_debug( "triggering power function" );
                panel_mainmenu_power_timeout( g_timer_elapsed( power_timer, NULL ) );
            }
        }
    }
    return TRUE;
}

gboolean panel_mainmenu_aux_timeout( guint timeout )
{
    g_debug( "aux pressed for %d", timeout );
    return FALSE;
}


gboolean panel_mainmenu_power_timeout( guint timeout )
{
    g_debug( "power pressed for %d", timeout );
    return FALSE;
}

