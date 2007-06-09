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

#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>

#include <gdk/gdkx.h>

#include <glib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <pulse/pulseaudio.h>

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

//#undef DEBUG_THIS_FILE
#define DEBUG_THIS_FILE

//FIXME find out through sysfs
#ifndef DEBUG_THIS_FILE
    #define AUX_BUTTON_EVENT_PATH "/dev/input/event1"
    #define AUX_BUTTON_KEYCODE 169
    #define POWER_BUTTON_EVENT_PATH "/dev/input/event2"
    #define POWER_BUTTON_KEYCODE 116
    #define TOUCHSCREEN_EVENT_PATH "/dev/input/touchscreen0"
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#else
    #define AUX_BUTTON_EVENT_PATH "/dev/input/event1"
    #define AUX_BUTTON_KEYCODE 0x25
    #define POWER_BUTTON_EVENT_PATH "/dev/input/event0"
    #define POWER_BUTTON_KEYCODE 0x25
    #define TOUCHSCREEN_EVENT_PATH "/dev/input/event2"
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

GPollFD aux_fd;
GPollFD power_fd;
GIOChannel* touchscreen_io;

int aux_timer = -1;
int power_timer = -1;
int powersave_timer1 = -1;
int powersave_timer2 = -1;
int powersave_timer3 = -1;

GtkWidget* aux_menu = 0;
GtkWidget* power_menu = 0;

typedef enum _PowerState
{
    NORMAL,
    DISPLAY_DIM,
    DISPLAY_OFF,
    SUSPEND,
} PowerState;
PowerState power_state = NORMAL;

static pa_context* pac;

/* Borrowed from libwnck */
static Window get_window_property( Window xwindow, Atom atom )
{
    Atom type;
    int format;
    gulong nitems;
    gulong bytes_after;
    Window *w;
    int err, result;
    Window retval;

    gdk_error_trap_push ();

    type = None;
    result = XGetWindowProperty (gdk_display,
                                 xwindow,
                                 atom,
                                 0, G_MAXLONG,
                                 False, XA_WINDOW, &type, &format, &nitems,
                                 &bytes_after, (unsigned char **) &w);
    err = gdk_error_trap_pop ();

    if (err != Success ||
        result != Success)
        return None;

    if (type != XA_WINDOW)
    {
        XFree (w);
        return None;
    }

    retval = *w;
    XFree (w);

    return retval;
}

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
    power_fd.fd = powerfd;
    power_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    power_fd.revents = 0;
    g_source_add_poll( button_watcher, &power_fd );
    g_source_attach( button_watcher, NULL );

    if ( getenv( "MOKO_POWERSAVE" ) == NULL )
    {

        int tsfd = open( TOUCHSCREEN_EVENT_PATH, O_RDONLY );
        if ( tsfd < 0 )
        {
            g_debug( "can't open " TOUCHSCREEN_EVENT_PATH " (%s)", strerror( errno ) );
            return FALSE;
        }
        touchscreen_io = g_io_channel_unix_new( tsfd );
        g_io_add_watch( touchscreen_io, G_IO_IN, panel_mainmenu_touchscreen_cb, NULL );

        panel_mainmenu_powersave_reset();
        panel_mainmenu_set_display( 100 );
        panel_mainmenu_sound_init();
    }
    else
        g_debug( "MOKO_POWERSAVE set. Not enabling power management." );

    return TRUE;
}


gboolean panel_mainmenu_input_prepare( GSource* source, gint* timeout )
{
    return FALSE;
}


gboolean panel_mainmenu_input_check( GSource* source )
{
    return ( ( aux_fd.revents & G_IO_IN ) || ( power_fd.revents & G_IO_IN ) );
}


gboolean panel_mainmenu_input_dispatch( GSource* source, GSourceFunc callback, gpointer data )
{
    if ( aux_fd.revents & G_IO_IN )
    {
        struct input_event event;
        int size = read( aux_fd.fd, &event, sizeof( struct input_event ) );
        g_debug( "read %d bytes from aux_fd %d", size, aux_fd.fd );
        g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
        if ( event.type == 1 && event.code == AUX_BUTTON_KEYCODE )
        {
            if ( event.value == 1 ) /* pressed */
            {
                g_debug( "triggering aux timer" );
                aux_timer = g_timeout_add( 1 * 1000, (GSourceFunc) panel_mainmenu_aux_timeout, (gpointer)1 );
            }
            else if ( event.value == 0 ) /* released */
            {
                g_debug( "resetting aux timer" );
                if ( aux_timer != -1 )
                {
                    g_source_remove( aux_timer );
                    panel_mainmenu_aux_timeout( 0 );
                }
                aux_timer = -1;
            }
        }
    }
    if ( power_fd.revents & G_IO_IN )
    {
        struct input_event event;
        int size = read( power_fd.fd, &event, sizeof( struct input_event ) );
        g_debug( "read %d bytes from power_fd %d", size, power_fd.fd );
        g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
        if ( event.type == 1 && event.code == POWER_BUTTON_KEYCODE )
        {
            if ( event.value == 1 ) /* pressed */
            {
                g_debug( "triggering power timer" );
                power_timer = g_timeout_add( 1 * 1000, (GSourceFunc) panel_mainmenu_power_timeout, (gpointer)1 );
            }
            else if ( event.value == 0 ) /* released */
            {
                g_debug( "resetting power timer" );
                if ( power_timer != -1 )
                {
                    g_source_remove( power_timer );
                    panel_mainmenu_power_timeout( 0 );
                }
                power_timer = -1;
            }
        }
    }
    return TRUE;
}

gboolean panel_mainmenu_aux_timeout( guint timeout )
{
    g_debug( "aux pressed for %d", timeout );
    aux_timer = -1;
    if ( timeout < 1 )
    {
        // make dialer interface show up
        // NOTE: temporary hack, will use dbus interface once dialer has it :)
        system( "openmoko-dialer &" );
    }
    else
    {
        // make main menu show up
        // NOTE: temporary hack, will use dbus interface once main menu has it :)
        system( "openmoko-mainmenu &" );
    }
    return FALSE;
}

// this is hardcoded to the Neo1973
void panel_mainmenu_popup_positioning_cb( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data )
{
    GtkRequisition req;
    gtk_widget_size_request( GTK_WIDGET(menu), &req );
    gint screen_width = gdk_screen_width();
    gint screen_height = gdk_screen_height();

    if ( GTK_WIDGET(menu) == aux_menu )
    {
        *x = 0;
        *y = 0;
    }
    else if ( GTK_WIDGET(menu) == power_menu )
    {
        *x = screen_width - req.width;
        *y = screen_height - req.height;
    }
    else
        g_assert( FALSE ); // fail here if called for unknown menu
}

void panel_mainmenu_popup_selected_lock( GtkMenuItem* menu, gpointer user_data )
{
    //FIXME talk to neod
    int fd = open( "/sys/power/state", O_WRONLY );
    if ( fd != -1 )
    {
        char command[] = "mem\n";
        write(fd, &command, sizeof(command) );
        close( fd );
    }
}

void panel_mainmenu_popup_selected_poweroff( GtkMenuItem* menu, gpointer user_data )
{
    //FIXME talk to neod
    //FIXME notify user
    system( "/sbin/poweroff");
}

gboolean panel_mainmenu_power_timeout( guint timeout )
{
    g_debug( "power pressed for %d", timeout );
    power_timer = -1;
    if ( timeout < 1 )
    {
        Window xwindow = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );
        g_debug( "active Window = %d", (int) xwindow );

        Display* display = XOpenDisplay( NULL );

        //xwindow = gdk_x11_drawable_get_xid (window);

        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = display;
        xev.xclient.window = xwindow;
        xev.xclient.message_type = gdk_x11_get_xatom_by_name( "_NET_CLOSE_WINDOW" );
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 0;
        xev.xclient.data.l[1] = 0;
        xev.xclient.data.l[2] = 0;
        xev.xclient.data.l[3] = 0;
        xev.xclient.data.l[4] = 0;

        //TODO: add timeout checking for response

        XSendEvent (display, gdk_x11_get_default_root_xwindow (), False,
                    SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        XCloseDisplay( display );

    }
    else
    {
        // show popup menu requesting for actions
        if ( !power_menu )
        {
            power_menu = gtk_menu_new();
            GtkWidget* lock = gtk_menu_item_new_with_label( "Lock" );
            g_signal_connect( G_OBJECT(lock), "activate", G_CALLBACK(panel_mainmenu_popup_selected_lock), NULL );
            gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), lock );
            GtkWidget* flightmode = gtk_menu_item_new_with_label( "Flight Mode" );
            gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), flightmode );
            GtkWidget* profilelist = gtk_menu_item_new_with_label( "<Profile List>" );
            gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), profilelist );
            GtkWidget* poweroff = gtk_menu_item_new_with_label( "Power Off" );
            g_signal_connect( G_OBJECT(poweroff), "activate", G_CALLBACK(panel_mainmenu_popup_selected_poweroff), NULL );
            gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), poweroff );
            gtk_widget_show_all( GTK_WIDGET(power_menu) );
        }
        gtk_menu_popup( GTK_MENU(power_menu), NULL, NULL, panel_mainmenu_popup_positioning_cb, 0, 0, GDK_CURRENT_TIME );
    }
    return FALSE;
}

gboolean panel_mainmenu_touchscreen_cb( GIOChannel *source, GIOCondition condition, gpointer data )
{
    g_debug( "mainmenu touchscreen event" );

    struct input_event event;
    int size = read( g_io_channel_unix_get_fd( source ), &event, sizeof( struct input_event ) );
    g_debug( "read %d bytes from touchscreen_fd %d", size, g_io_channel_unix_get_fd( source ) );
    g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );

    if ( event.type == 1 && event.code == TOUCHSCREEN_BUTTON_KEYCODE )
    {
        if ( event.value == 1 ) /* pressed */
        {
            g_debug( "stylus pressed" );
            panel_mainmenu_sound_play( "touchscreen" );
        }
        else if ( event.value == 0 ) /* released */
        {
            g_debug( "stylus released" );
        }

        panel_mainmenu_powersave_reset();
        if ( power_state != NORMAL )
        {
            panel_mainmenu_set_display( 100 );
            power_state = NORMAL;
        }
    }
    return TRUE;
}

void panel_mainmenu_powersave_reset()
{
    g_debug( "mainmenu powersave reset" );
    if ( powersave_timer1 != -1 )
        g_source_remove( powersave_timer1 );
    if ( powersave_timer2 != -1 )
        g_source_remove( powersave_timer2 );
    if ( powersave_timer3 != -1 )
        g_source_remove( powersave_timer3 );

    //TODO load this from preferences
    powersave_timer1 = g_timeout_add( 10 * 1000, (GSourceFunc) panel_mainmenu_powersave_timeout1, (gpointer)1 );
    powersave_timer2 = g_timeout_add( 20 * 1000, (GSourceFunc) panel_mainmenu_powersave_timeout2, (gpointer)1 );
    powersave_timer3 = g_timeout_add( 40 * 1000, (GSourceFunc) panel_mainmenu_powersave_timeout3, (gpointer)1 );
}

void panel_mainmenu_set_display( int brightness )
{
    g_debug( "mainmenu set display %d", brightness );
}

gboolean panel_mainmenu_powersave_timeout1( guint timeout )
{
    g_debug( "mainmenu powersave timeout 1" );
    //FIXME talk to neod
    //FIXME dim display
    power_state = DISPLAY_DIM;
    panel_mainmenu_set_display( 50 );
    return FALSE;
}

gboolean panel_mainmenu_powersave_timeout2( guint timeout )
{
    g_debug( "mainmenu powersave timeout 2" );
    //FIXME talk to neod
    //FIXME turn off display
    panel_mainmenu_set_display( 0 );
    power_state = DISPLAY_OFF;
    return FALSE;
}

gboolean panel_mainmenu_powersave_timeout3( guint timeout )
{
    g_debug( "mainmenu powersave timeout 3" );
    //FIXME talk to neod
    power_state = SUSPEND;
    system( "/usr/bin/apm -s");
    panel_mainmenu_powersave_reset();
    panel_mainmenu_set_display( 100 );
    power_state = NORMAL;
    return FALSE;
}

void panel_mainmenu_sound_state_cb( pa_context* pac, void* userdata )
{
    if ( pa_context_get_state( pac ) == PA_CONTEXT_READY )
    {
        panel_mainmenu_sound_play( "startup" );
    }
}

void panel_mainmenu_sound_init()
{
    pa_threaded_mainloop* mainloop = pa_threaded_mainloop_new();

    if ( !mainloop )
    {
        printf( "couldn't create mainloop: %s", strerror( errno ) );
        return;
    }

    pa_mainloop_api* mapi = pa_threaded_mainloop_get_api( mainloop );

    pac = pa_context_new( mapi, "test client" );
    if ( !pac )
    {
        printf( "couldn't create pa_context: %s", strerror( errno ) );
        return;
    }

    pa_context_set_state_callback( pac, panel_mainmenu_sound_state_cb, NULL );
    pa_context_connect( pac, NULL, 0, NULL );
    pa_threaded_mainloop_start( mainloop );
}

void panel_mainmenu_sound_play( const gchar* samplename )
{
    pa_context_play_sample( pac,
                            "startup",       // Name of my sample
                            NULL,            // Use default sink
                            PA_VOLUME_NORM,  // Full volume
                            NULL,            // Don't need a callback
                            NULL
                           );

}
