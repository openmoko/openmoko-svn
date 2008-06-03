/*
 *  neod - buttonactions.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko, Inc.
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

#include "wifi.h"

#include <gconf/gconf-client.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <glib.h>
#include <glib/gstdio.h>

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

#include <apm.h>

#define SYS_CLASS_BACKLIGHT "/sys/class/backlight/"

static gchar* backlight_node = NULL;
static int backlight_max_brightness = 1;

#ifdef NEOD_PLATFORM_FIC_NEO1973
    #define AUX_BUTTON_KEYCODE 169    /* aux */
    #define POWER_BUTTON_KEYCODE 116  /* power */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#elif NEOD_PLATFORM_MOTOROLA_EZX
    #define AUX_BUTTON_KEYCODE 0xa7   /* voice */
    #define POWER_BUTTON_KEYCODE 0xd4 /* camera */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#elif NEOD_PLATFORM_HTC
    #define AUX_BUTTON_KEYCODE 0xd4   /* camera */
    #define POWER_BUTTON_KEYCODE 0x74 /* power */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#elif NEOD_PLATFORM_IPAQ
    #define AUX_BUTTON_KEYCODE 89     /* record */
    #define POWER_BUTTON_KEYCODE 0x74 /* power */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#else
    #define AUX_BUTTON_KEYCODE 0x22
    #define POWER_BUTTON_KEYCODE 0x23
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

#define HEADPHONE_INSERTION_SWITCHCODE 0x02
#define CHARGER_INSERTION_BUTTON 0x164

#define WIFI_IFACE "eth0"

#define BIT_MASK( name, numbits )                                        \
    unsigned short  name[ ((numbits) - 1) / (sizeof( short ) * 8) + 1 ];    \
    memset( name, 0, sizeof( name ) )
#define BIT_TEST( bitmask, bit )    \
    ( bitmask[ (bit) / sizeof(short) / 8 ] & (1u << ( (bit) % (sizeof(short) * 8))) )

static gboolean moko_debug = TRUE;

GPollFD input_fd[10];
int max_input_fd = 0;

GIOChannel* touchscreen_io;

int aux_timer = -1;
int power_timer = -1;
int powersave_timer1 = -1;
int powersave_timer2 = -1;
int powersave_timer3 = -1;

GtkWidget* aux_menu = 0;
GtkWidget* power_menu = 0;
GtkWidget* lock_display = 0;

GConfClient* gconfc = 0;


enum PeripheralUnit
{
    GSM = 0,
    BLUETOOTH = 1,
    GPS = 2,
    WIFI = 3,
};

enum PowerManagementMode
{
    FULL = 0,
    DIM_ONLY = 1,
    NONE = 2,
};

int pm_value = 0;
gboolean orientation = FALSE;
gboolean lock_display_last_orientation = FALSE;

typedef enum _PowerState
{
    NORMAL,
    DISPLAY_DIM,
    DISPLAY_OFF,
    SUSPEND,
} PowerState;
PowerState power_state = NORMAL;

static pa_threaded_mainloop* pam;
static pa_context* pac;

static Window last_active_window = 0;

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

/* Retrieves the text property @atom from @window */
static char* get_text_property( Window window, Atom atom )
{
    XTextProperty text;
    char *ret, **list;
    int result, count;

    gdk_error_trap_push ();
    result = XGetTextProperty (gdk_display,
                               window,
                               &text,
                               atom);
    if (gdk_error_trap_pop () || result == 0)
        return NULL;

    count = gdk_text_property_to_utf8_list
            (gdk_x11_xatom_to_atom (text.encoding),
             text.format,
             text.value,
             text.nitems,
             &list);
    if (count > 0) {
        int i;

        ret = list[0];

        for (i = 1; i < count; i++)
            g_free (list[i]);
        g_free (list);
    } else
        ret = NULL;

        if (text.value)
            XFree (text.value);

        return ret;
}

/* Checks if type property of @window is "desktop window" */
gboolean is_desktop_window( Window window )
{
    unsigned long nitems, bytesafter;
    Atom *window_type = NULL;
    Atom actual_type;
    int actual_format;

    Display* display = XOpenDisplay( NULL );

    /* not using gdk_property_get() here, due to note at
     * http://www.gtk.org/api/2.6/gdk/gdk-Properties-and-Atoms.html#gdk-property-get
     */
    XGetWindowProperty(display, window,
            gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE"), 0, 1, False,
            XA_ATOM, &actual_type, &actual_format, &nitems, &bytesafter,
            (unsigned char **) &window_type);

    if ( !window_type )
        return FALSE;

    if (strcmp(XGetAtomName(display, *window_type), "_NET_WM_WINDOW_TYPE_DESKTOP") == 0)
        return TRUE;

    return FALSE;
}

gboolean neod_buttonactions_install_watcher()
{
    int i = 0;
    for ( ; i < 10; ++i )
    {
        char* filename = g_strdup_printf( "/dev/input/event%d", i );
        input_fd[i].fd = open( filename, O_RDONLY );
        if ( input_fd[i].fd < 0 )
            break;
        else
            g_debug( "'%s' open OK, fd = '%d'", filename, input_fd[i].fd );
    }

    max_input_fd = i - 1;
    g_debug( "opened %d input nodes.", max_input_fd + 1 );
    if ( max_input_fd <= 0 )
    {
        g_debug( "can't open ANY input node. no watcher installed." );
        return FALSE;
    }
    static GSourceFuncs funcs = {
        neod_buttonactions_input_prepare,
        neod_buttonactions_input_check,
        neod_buttonactions_input_dispatch,
    NULL,
    };
    GSource* button_watcher = g_source_new( &funcs, sizeof (GSource) );

    for ( i = 0; i <= max_input_fd; ++i )
    {
        input_fd[i].events = G_IO_IN | G_IO_HUP | G_IO_ERR;
        input_fd[i].revents = 0;
        g_source_add_poll( button_watcher, &input_fd[i] );
        g_debug( "added fd %d to list of watchers", input_fd[i].fd );
    }
    g_source_attach( button_watcher, NULL );

    // get PM profile setting from gconf and listen for changes
    gconfc = gconf_client_get_default();
    if (!gconfc)
    {
        g_error( "can't get connection to gconfd" );
        return FALSE;
    }
    GError* error = 0;
    pm_value = gconf_client_get_int( gconfc, "/desktop/openmoko/neod/power_management", &error );
    if ( error ) g_debug( "gconf error: %s", error->message );
    gconf_client_add_dir( gconfc, "/desktop/openmoko/neod", GCONF_CLIENT_PRELOAD_NONE, &error );
    if ( error ) g_debug( "gconf error: %s", error->message );
    gconf_client_notify_add( gconfc, "/desktop/openmoko/neod/power_management", (GConfClientNotifyFunc) neod_buttonactions_gconf_cb, NULL, NULL, &error );
    if ( error ) g_debug( "gconf error: %s", error->message );

    neod_buttonactions_powersave_reset();

    moko_debug = ( getenv( "MOKO_DEBUG" ) != NULL );
    g_debug( "setting debug output to %s", moko_debug ? "true" : "false" );

    return TRUE;
}

gboolean neod_buttonactions_input_prepare( GSource* source, gint* timeout )
{
    return FALSE;
}


gboolean neod_buttonactions_input_check( GSource* source )
{
    for ( int i = 0; i <= max_input_fd; ++i )
        if ( input_fd[i].revents & G_IO_IN )
            return TRUE;
    return FALSE;
}


gboolean neod_buttonactions_input_dispatch( GSource* source, GSourceFunc callback, gpointer data )
{
    for ( int i = 0; i <= max_input_fd; ++i )
    {
        if ( input_fd[i].revents & G_IO_IN )
        {
            struct input_event event;
            int size = read( input_fd[i].fd, &event, sizeof( struct input_event ) );
            if ( moko_debug )
            {
                g_debug( "read %d bytes from fd %d", size, input_fd[i].fd );
                g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
            }

            if ( event.type == 1 && event.code == AUX_BUTTON_KEYCODE )
            {
                if ( event.value == 1 ) /* pressed */
                {
                    if ( moko_debug ) g_debug( "triggering aux timer" );
                    aux_timer = g_timeout_add( 1000, (GSourceFunc) neod_buttonactions_aux_timeout, (gpointer)1 );
                }
                else if ( event.value == 0 ) /* released */
                {
                    if ( moko_debug ) g_debug( "resetting aux timer" );
                    if ( aux_timer != -1 )
                    {
                        g_source_remove( aux_timer );
                        neod_buttonactions_aux_timeout( 0 );
                    }
                    aux_timer = -1;
                }
            }
            else
            if ( event.type == 1 && event.code == POWER_BUTTON_KEYCODE )
            {
                if ( event.value == 1 ) /* pressed */
                {
                    if ( moko_debug ) g_debug( "triggering power timer" );
                    power_timer = g_timeout_add( 1000, (GSourceFunc) neod_buttonactions_power_timeout, (gpointer)1 );
                }
                else if ( event.value == 0 ) /* released */
                {
                    if ( moko_debug ) g_debug( "resetting power timer" );
                    if ( power_timer != -1 )
                    {
                        g_source_remove( power_timer );
                        neod_buttonactions_power_timeout( 0 );
                    }
                    power_timer = -1;
                }
            }
            else
            if ( event.type == 1 && event.code == TOUCHSCREEN_BUTTON_KEYCODE )
            {
                if ( event.value == 1 ) /* pressed */
                {
                    if ( moko_debug ) g_debug( "stylus pressed" );
                    neod_buttonactions_sound_play( "touchscreen" );
                }
                else if ( event.value == 0 ) /* released */
                {
                    if ( moko_debug ) g_debug( "stylus released" );
                }
                neod_buttonactions_powersave_reset();
                if ( power_state != NORMAL )
                {
                    neod_buttonactions_set_display( 100 );
                    power_state = NORMAL;
                }
            }
            else
            if ( event.type == 1 && event.code == CHARGER_INSERTION_BUTTON )
            {
                if ( event.value == 1 ) /* pressed */
                {
                    if ( moko_debug ) g_debug( "charger IN" );
                    neod_buttonactions_sound_play( "touchscreen" );
                    g_spawn_command_line_async( "dbus-send --system /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.ChargerConnected", NULL );
                }
                else if ( event.value == 0 ) /* released */
                {
                    if ( moko_debug ) g_debug( "charger OUT" );
                    g_spawn_command_line_async( "dbus-send --system /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.ChargerDisconnected", NULL );
                }
                neod_buttonactions_powersave_reset();
                if ( power_state != NORMAL )
                {
                    neod_buttonactions_set_display( 100 );
                    power_state = NORMAL;
                }
            }	 
	    else
	    if ( event.type == 5 && event.code == HEADPHONE_INSERTION_SWITCHCODE )
	    {
		if ( event.value == 1 ) /* inserted */
		{
		    if ( moko_debug ) g_debug( "headphones IN" );		    
#ifdef NEOD_PLATFORM_FIC_NEO1973
	            g_spawn_command_line_async( "dbus-send --session --type=signal /org/openmoko/PhoneKit/Headset org.openmoko.PhoneKit.Headset.HeadsetIn", NULL );
#else 
		    g_spawn_command_line_async( "amixer sset \"Amp Mode\" \"Headphones\"", NULL );
#endif
		}
		else if ( event.value == 0 ) /* released */
		{
		    if ( moko_debug ) g_debug( "headphones OUT" );
#ifdef NEOD_PLATFORM_FIC_NEO1973
		    g_spawn_command_line_async( "dbus-send --session --type=signal /org/openmoko/PhoneKit/Headset org.openmoko.PhoneKit.Headset.HeadsetOut", NULL );
#else 
		    g_spawn_command_line_async( "amixer sset \"Amp Mode\" \"Stereo Speakers\"", NULL );
#endif
		}
		neod_buttonactions_powersave_reset();
#if 0
		if ( power_state != NORMAL )
		{
		    neod_buttonactions_set_display( 100 );
		    power_state = NORMAL;
		}
#endif
	    }
	}
    }
    return TRUE;
}

gboolean neod_buttonactions_aux_timeout( guint timeout )
{
    if ( aux_timer == -1 )
        return FALSE;
    g_debug( "aux pressed for %d", timeout );

    neod_buttonactions_sound_play( "touchscreen" );
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );

    if ( aux_menu && GTK_WIDGET_MAPPED(aux_menu) )
    {
        g_debug( "aux menu already open -- closing." );
        gtk_widget_hide( aux_menu );
        return FALSE;
    }

    aux_timer = -1;
    if ( timeout < 1 )
    {
        // get status of desktop visibility
        Screen *screen = GDK_SCREEN_XSCREEN(gdk_screen_get_default());

        Atom type = 0;
        int format;
        gulong nitems, bytes_after, *num;
        int status = TRUE;

        gdk_error_trap_push ();
        int result = XGetWindowProperty (DisplayOfScreen(screen),
                                     RootWindowOfScreen(screen),
                                     gdk_x11_get_xatom_by_name("_NET_SHOWING_DESKTOP"),
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_CARDINAL,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &num);
        if (!gdk_error_trap_pop () && result == Success)
        {
            if (type == XA_CARDINAL && nitems > 0)
            {
                status = *num ? TRUE : FALSE;
            }
        }
        XFree (num);

        // toggle desktop visibility
        XEvent xev;

        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = DisplayOfScreen(screen);
        xev.xclient.window = RootWindowOfScreen(screen);
        xev.xclient.message_type = gdk_x11_get_xatom_by_name("_NET_SHOWING_DESKTOP");
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = !status;
        xev.xclient.data.l[1] = 0;
        xev.xclient.data.l[2] = 0;
        xev.xclient.data.l[3] = 0;
        xev.xclient.data.l[4] = 0;

        XSendEvent(DisplayOfScreen(screen), RootWindowOfScreen(screen), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    }
    else
    {
        neod_buttonactions_show_aux_menu();
    }
    return FALSE;
}

void neod_buttonactions_popup_selected_fullscreen( GtkWidget* button, gpointer user_data )
{
    static int is_fullscreen = 0;

    gtk_widget_hide( aux_menu );

    const char* title = get_text_property( last_active_window, gdk_x11_get_xatom_by_name("_NET_WM_NAME") );
    g_debug( "active Window = %d ('%s')", (int)last_active_window, title );

    Display* display = XOpenDisplay( NULL );

    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = display;
    xev.xclient.window = last_active_window;
    xev.xclient.message_type = gdk_x11_get_xatom_by_name( "_NET_WM_STATE" );
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1 - is_fullscreen; // ADD = 1, REMOVE = 0

    xev.xclient.data.l[1] = gdk_x11_get_xatom_by_name("_NET_WM_STATE_FULLSCREEN");
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    //TODO: add timeout checking for response
    XSendEvent (display, gdk_x11_get_default_root_xwindow (), False,
                SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XCloseDisplay( display );

    is_fullscreen = 1 - is_fullscreen;
}

void neod_buttonactions_set_orientation(gboolean new_o) {
    if(new_o)
        g_spawn_command_line_async( "xrandr -o 1", NULL );
    else
        g_spawn_command_line_async( "xrandr -o 0", NULL );
    orientation = new_o;
}

void neod_buttonactions_popup_selected_orientation( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( aux_menu );
    neod_buttonactions_set_orientation(!orientation);
}

void neod_buttonactions_popup_selected_screenshot( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( aux_menu );
    g_spawn_command_line_async( "gpe-scap", NULL );
}

void neod_buttonactions_popup_selected_lock( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( power_menu );

    if (getenv("DO_NOT_USE_APM") == NULL) {
           system("/usr/bin/apm -s");
           return;
    }

    int fd = open( "/sys/power/state", O_WRONLY );
    if ( fd != -1 )
    {
        char command[] = "mem\n";
        write(fd, &command, sizeof(command) );
        close( fd );
    }
}

void neod_buttonactions_popup_selected_lock_display(GtkWidget* button, 
													gpointer user_data) {
    gtk_widget_hide(power_menu);
	neod_buttonactions_lock_display();
}

void neod_buttonactions_popup_selected_restartUI( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( power_menu );
    //FIXME notify user
    system( "/etc/init.d/xserver-nodm restart");
}

void neod_buttonactions_popup_selected_reboot( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( power_menu );
    //moko_ui_banner_show_text( 4, "Rebooting System..." );
    system( "/sbin/reboot");
}

void neod_buttonactions_popup_selected_poweroff( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( power_menu );
    //moko_ui_banner_show_text( 4, "Shutting down System..." );
    system( "/sbin/poweroff");
}

void neod_buttonactions_popup_selected_pmprofile( GtkComboBox* combo, gpointer user_data )
{
    gtk_widget_hide( power_menu );
    int new_pmprofile = gtk_combo_box_get_active( combo );
    g_assert( FULL <= new_pmprofile && new_pmprofile <= NONE );
    g_debug( "switch pm profile to %d", new_pmprofile );
    gconf_client_set_int( gconfc, "/desktop/openmoko/neod/power_management", new_pmprofile, NULL );
}

static gboolean read_boolean_from_path( const gchar* path )
{
    int value;
    FILE* f = fopen( path, "r" );
    if ( f == NULL )
    {
        g_debug( "can't open file '%s': (%s), aborting.", path, strerror( errno ) );
        return FALSE;
    }
    fscanf( f, "%d", &value );
    fclose( f );
    g_debug( "read value from '%s' = '%d'", path, value );
    return value;
}

static void write_boolean_to_path( const gchar* path, gboolean b )
{
    FILE* f = fopen( path, "w" );
    if ( f == NULL )
    {
        g_debug( "can't open file '%s': (%s), aborting.", path, strerror( errno ) );
        return;
    }
    fprintf( f, b ? "1\n" : "0\n" );
    fclose( f );
}

static gboolean is_turned_on( int unit )
{
    switch( unit )
    {
        case GSM:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            return read_boolean_from_path( "/sys/bus/platform/devices/neo1973-pm-gsm.0/power_on" );
#endif
            return FALSE;
        case BLUETOOTH:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            return read_boolean_from_path( "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on" );
#endif
            return FALSE;
        case GPS:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            return read_boolean_from_path( "/sys/bus/platform/devices/neo1973-pm-gps.0/pwron" );
#endif
            return FALSE;
        case WIFI:
            return wifi_radio_is_on ( WIFI_IFACE );
        default:
            g_assert( FALSE ); // should never reach this
    }
}

static void peripheral_set_power( int unit, gboolean on )
{
    switch( unit )
    {
        case GSM:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            //write_boolean_to_path( "/sys/devices/platform/gta01-pm-gsm.0/power_on", on );
            system( on ? "/etc/init.d/gsmd restart" : "/etc/init.d/gsmd stop" );
#endif
            break;
        case BLUETOOTH:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            write_boolean_to_path( "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on", on );
	     write_boolean_to_path( "/sys/bus/platform/devices/neo1973-pm-bt.0/reset", 0 );
#endif
            break;
        case GPS:
#ifdef NEOD_PLATFORM_FIC_NEO1973
            write_boolean_to_path( "/sys/bus/platform/devices/neo1973-pm-gps.0/pwron", on );
#endif
            break;
        case WIFI:
            wifi_radio_control ( WIFI_IFACE, on );
            break;
        default:
            g_assert( FALSE ); // should never reach this
    }
}

void neod_buttonactions_popup_selected_switch_power( GtkWidget* button, gpointer user_data )
{
    gtk_widget_hide( power_menu );
    gboolean new_power_state = !is_turned_on( GPOINTER_TO_INT( user_data ) );
    g_debug( "switch power of unit %d to %d", GPOINTER_TO_INT( user_data ), new_power_state );
    peripheral_set_power ( GPOINTER_TO_INT( user_data ), new_power_state);
}

void neod_buttonactions_gconf_cb( GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data )
{
    g_debug( "gconf callback" );
    pm_value = gconf_client_get_int( client, "/desktop/openmoko/neod/power_management", NULL );
    if ( pm_value < 0 || pm_value > 2 ) pm_value = 0;
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );
    gtk_widget_destroy( power_menu );
    //gtk_widget_unref( power_menu );
    power_menu = 0;
}

void neod_buttonactions_show_aux_menu()
{
    // remember last active window before showing popup menu
    last_active_window = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );

    // show popup menu requesting for actions
    if ( !aux_menu )
    {
        aux_menu = gtk_dialog_new_with_buttons( "AUX Menu",
                                                NULL,
                                                GTK_DIALOG_MODAL,
                                                "Dismiss this menu", GTK_RESPONSE_OK,
                                                NULL );
        gtk_widget_set_name( GTK_WIDGET(aux_menu), "neod-dialog" );
        gtk_button_box_set_layout( GTK_BUTTON_BOX(GTK_DIALOG(aux_menu)->action_area), GTK_BUTTONBOX_SPREAD );
        GtkWidget* box = gtk_vbox_new( 0, 0 );
        gtk_widget_set_name( box, "neod-menu" );
//        GtkWidget* title = gtk_label_new( "AUX Button Menu" );
//        gtk_box_pack_start_defaults( GTK_BOX(box), title );

        GtkWidget* fullscreen = gtk_button_new_with_label( "Toggle Fullscreen" );
        g_signal_connect( G_OBJECT(fullscreen), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_fullscreen), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), fullscreen );

        GtkWidget* orientation = gtk_button_new_with_label( "Swap Orientation" );
        g_signal_connect( G_OBJECT(orientation), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_orientation), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), orientation );

        GtkWidget* scap = gtk_button_new_with_label( "Screenshot" );
        g_signal_connect( G_OBJECT(scap), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_screenshot), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), scap );

        gtk_widget_show_all( GTK_WIDGET(box) );

        // override, otherwise matchbox won't show it fullscreen
        gtk_window_set_type_hint( GTK_WINDOW(aux_menu), GDK_WINDOW_TYPE_HINT_NORMAL );
        //gtk_window_fullscreen( GTK_WINDOW(aux_menu) );
        g_signal_connect_swapped( aux_menu, "response", G_CALLBACK(gtk_widget_hide), aux_menu);
        gtk_box_pack_start_defaults( GTK_BOX(GTK_DIALOG(aux_menu)->vbox), box );
    }
    int response = gtk_dialog_run( GTK_DIALOG(aux_menu) );
    g_debug( "gtk_dialog_run completed, response = %d", response );
}

void neod_buttonactions_show_power_menu()
{
    static GtkWidget* gsmpower = 0;
    static GtkWidget* btpower = 0;
    static GtkWidget* gpspower = 0;
    static GtkWidget* wifipower = 0;
    static GtkWidget* pmprofile = 0;

    // remember last active window before showing popup menu
    last_active_window = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );


    // show popup menu requesting for actions
    if ( !power_menu )
    {
        power_menu = gtk_dialog_new_with_buttons( "POWER Menu",
                NULL,
                GTK_DIALOG_MODAL,
                "Dismiss this menu", GTK_RESPONSE_OK,
                NULL );
        gtk_widget_set_name( GTK_WIDGET(power_menu), "neod-dialog" );
        gtk_button_box_set_layout( GTK_BUTTON_BOX(GTK_DIALOG(power_menu)->action_area), GTK_BUTTONBOX_SPREAD );
        GtkWidget* box = gtk_vbox_new( 0, 0 );
        gtk_widget_set_name( box, "neod-menu" );
//        GtkWidget* title = gtk_label_new( "POWER Button Menu" );
//        gtk_box_pack_start_defaults( GTK_BOX(box), title );

        gsmpower = gtk_button_new();
        g_signal_connect( G_OBJECT(gsmpower), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_switch_power), GINT_TO_POINTER( GSM ) );
        gtk_box_pack_start_defaults( GTK_BOX(box), gsmpower );

        btpower = gtk_button_new();
        g_signal_connect( G_OBJECT(btpower), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_switch_power), GINT_TO_POINTER( BLUETOOTH ) );
        gtk_box_pack_start_defaults( GTK_BOX(box), btpower );

        gpspower = gtk_button_new();
        g_signal_connect( G_OBJECT(gpspower), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_switch_power), GINT_TO_POINTER( GPS ) );
        gtk_box_pack_start_defaults( GTK_BOX(box), gpspower );

        wifipower = gtk_button_new();
        g_signal_connect( G_OBJECT(wifipower), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_switch_power), GINT_TO_POINTER( WIFI ) );
        gtk_box_pack_start_defaults( GTK_BOX(box), wifipower );

        gtk_box_pack_start_defaults( GTK_BOX(box), gtk_hseparator_new() );

        pmprofile = gtk_combo_box_new_text();
        gtk_combo_box_append_text( GTK_COMBO_BOX(pmprofile), "Power Management:\nDim first, then lock" );
        gtk_combo_box_append_text( GTK_COMBO_BOX(pmprofile), "Power Management:\nDim only, don't lock" );
        gtk_combo_box_append_text( GTK_COMBO_BOX(pmprofile), "Power Management:\nDisabled" );
        gtk_combo_box_set_active( GTK_COMBO_BOX(pmprofile), pm_value );
        g_signal_connect( G_OBJECT(pmprofile), "changed", G_CALLBACK(neod_buttonactions_popup_selected_pmprofile), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), pmprofile );

        gtk_box_pack_start_defaults( GTK_BOX(box), gtk_hseparator_new() );

        GtkWidget* lock_display = gtk_button_new_with_label("Lock Display");
        g_signal_connect(G_OBJECT(lock_display), "clicked", 
			G_CALLBACK(neod_buttonactions_popup_selected_lock_display), NULL);
		gtk_box_pack_start_defaults(GTK_BOX(box), lock_display);
#if 0
        GtkWidget* lock = gtk_button_new_with_label("Lock Phone");
        g_signal_connect( G_OBJECT(lock), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_lock), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), lock );
#endif
        GtkWidget* poweroff = gtk_button_new_with_label( "Shutdown Now" );
        g_signal_connect( G_OBJECT(poweroff), "clicked", G_CALLBACK(neod_buttonactions_popup_selected_poweroff), NULL );
        gtk_box_pack_start_defaults( GTK_BOX(box), poweroff );

        gtk_widget_show_all( GTK_WIDGET(box) );

        // override, otherwise matchbox won't show it fullscreen
        gtk_window_set_type_hint( GTK_WINDOW(power_menu), GDK_WINDOW_TYPE_HINT_NORMAL );
        //gtk_window_fullscreen( GTK_WINDOW(power_menu) );
        g_signal_connect_swapped( power_menu, "response", G_CALLBACK(gtk_widget_hide), power_menu);
        gtk_box_pack_start_defaults( GTK_BOX(GTK_DIALOG(power_menu)->vbox), box );
    }

    gtk_button_set_label( GTK_BUTTON(gsmpower), g_strdup_printf( "Turn %s GSM", is_turned_on( GSM ) ? "off" : "on" ) );
    gtk_button_set_label( GTK_BUTTON(btpower), g_strdup_printf( "Turn %s Bluetooth", is_turned_on( BLUETOOTH ) ? "off" : "on" ) );
    gtk_button_set_label( GTK_BUTTON(gpspower), g_strdup_printf( "Turn %s GPS", is_turned_on( GPS ) ? "off" : "on" ) );
    gtk_button_set_label( GTK_BUTTON(wifipower), g_strdup_printf( "Turn %s Wifi", is_turned_on( WIFI ) ? "off" : "on" ) );

    int response = gtk_dialog_run( GTK_DIALOG(power_menu) );
    g_debug( "gtk_dialog_run completed, response = %d", response );
}

const gchar* authors[] = {
    "OpenMoko has been brought to you by:",
    " "
    "Sean Moss-Pultz",
    "Harald 'LaF0rge' Welte",
    "Michael 'Mickey' Lauer",
    "Werner Almesberger",
    "Alice Tang",
    "Allen Chang",
    "Dave Wu",
    "Wanda",
    "Jelan Hsu",
    "Miles Hsieh",
    "Nod Huang",
    "Paul Tian",
    "Sean Chiang",
    "Shawn Lin",
    "Timmy Huang",
    "Willie Chen",
    "Olv",
    "JServ",
    "Jollen",
    "Rasterman",
    "Matt Hsu",
    "John Lee",
    "Tick",
    "Roh",
    "Erin Yueh",
    "Jeremy",
    "Holger 'Zecke' Freyther",
    "Daniel 'Alphaone' Willmann",
    "Stefan Schmidt",
    "Jan 'Shoragan' Luebbe",
    "Soeren 'Abraxa' Apel",
    "Rod Whitby",
    "Chris @ O-Hand",
    "Ross @ O-Hand",
    "Thomas @ O-Hand",
    "Rob @ O-Hand",
    "Dodji @ O-Hand",
    "NJP @ O-Hand",
    " ",
    "@" __DATE__ ":" __TIME__,
    "gcc " __VERSION__,
};

gboolean neod_buttonactions_power_while_aux()
{
    g_debug( "aux and power pressed together" );
    if ( aux_menu )
        gtk_widget_hide( aux_menu );
    gtk_show_about_dialog( NULL,
        "authors", authors,
        "comments", "open. mobile. free.",
        "copyright", "2006-2007 OpenMoko, Inc.",
#if GTK_MINOR_VERSION < 12
        "name", "OpenMoko 2007.2", /* Gtk+ up to 2.10.x */
#else
        "program-name", "OpenMoko 2007.2", /* Gtk+ >= 2.11 */
#endif
        "website", "http://www.openmoko.org",
        "logo", gdk_pixbuf_new_from_file( PKGDATADIR "/openmoko-logo.jpg", NULL ),
        NULL );
    return FALSE;
}

gboolean neod_buttonactions_power_timeout( guint timeout )
{
    g_debug( "power pressed for %d", timeout );

    neod_buttonactions_sound_play( "touchscreen" );
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );

    if ( power_menu && GTK_WIDGET_MAPPED(power_menu) )
    {
        g_debug( "power menu already open -- closing." );
        gtk_widget_hide( power_menu );
        return TRUE;
    }

    power_timer = -1;

    // special case for power button being pressed while aux is held
/*    if ( aux_timer != -1 || ( aux_menu && GTK_WIDGET_MAPPED( aux_menu ) ) )
        return neod_buttonactions_power_while_aux(); */

    if ( timeout < 1 )
    {
        Window xwindow = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );
        const char* title = get_text_property( xwindow, gdk_x11_get_xatom_by_name("_NET_WM_NAME") );
        g_debug( "active Window = %d ('%s')", (int) xwindow, title );

        if ( is_desktop_window(xwindow) )
        {
            g_debug( "sorry, i'm not going to close the today window" );
            return FALSE;
        } else if(gtk_window_is_active(GTK_WINDOW(lock_display))) {
            g_debug("sorry, i'm not going to close the lock window");
            return FALSE;
		}

        Display* display = XOpenDisplay( NULL );

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
#if 1
        XSendEvent (display, gdk_x11_get_default_root_xwindow (), False,
                    SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        XCloseDisplay( display );
#endif
    }
    else
    {
        neod_buttonactions_show_power_menu();
    }
    return FALSE;
}

void neod_buttonactions_powersave_reset()
{
    g_debug( "mainmenu powersave reset" );
    if ( powersave_timer1 != -1 )
        g_source_remove( powersave_timer1 );
    if ( powersave_timer2 != -1 )
        g_source_remove( powersave_timer2 );
    if ( powersave_timer3 != -1 )
        g_source_remove( powersave_timer3 );

    if ( pm_value == NONE )
        return;

    //TODO load this from preferences
    powersave_timer1 = g_timeout_add_seconds( 10, (GSourceFunc) neod_buttonactions_powersave_timeout1, (gpointer)1 );
    powersave_timer2 = g_timeout_add_seconds( 50, (GSourceFunc) neod_buttonactions_powersave_timeout2, (gpointer)1 );
    powersave_timer3 = g_timeout_add_seconds( 60 * 5, (GSourceFunc) neod_buttonactions_powersave_timeout3, (gpointer)1 );
}

void neod_buttonactions_set_display( int brightness )
{
    g_debug( "requested to set display brightness %d", brightness );

    if ( !backlight_node )
    {
        GDir* backlight_class_dir = g_dir_open( SYS_CLASS_BACKLIGHT, 0, NULL );
        if ( backlight_class_dir )
        {
            const gchar* backlight_node_dir = g_dir_read_name( backlight_class_dir );
            if ( backlight_node_dir )
            {
                g_debug( "backlight_node_dir = %s", backlight_node_dir );
                backlight_node = g_strdup_printf( SYS_CLASS_BACKLIGHT "%s/brightness", backlight_node_dir );
                g_debug( "detected backlight in sysfs as '%s'", backlight_node );
                const gchar* backlight_node_max_brightness = g_strdup_printf( SYS_CLASS_BACKLIGHT "%s/max_brightness", backlight_node_dir );

                FILE* f = fopen( backlight_node_max_brightness, "r" );
                if ( f == NULL )
                {
                    g_debug( "can't open max brightness '%s': (%s), aborting.", backlight_node_max_brightness, strerror( errno ) );
                    g_dir_close( backlight_class_dir );
                    return;
                }
                fscanf( f, "%d", &backlight_max_brightness );
                fclose( f );
                g_debug( "scanned maximum brightness for '%s' = '%d'", backlight_node, backlight_max_brightness );
                g_dir_close( backlight_class_dir );
            }
        }
        else
        {
            g_debug( "can't open '%s': (%s), aborting.", SYS_CLASS_BACKLIGHT, strerror( errno ) );
            return;
        }
    }

    int fd = g_open( backlight_node, O_WRONLY, 0 );
    if ( fd == -1 )
        g_debug( "can't open backlight device '%s'(%s)", backlight_node, strerror( errno ) );
    else
    {
        char buf[10];
        int numbytes = g_sprintf(
			buf, "%d", backlight_max_brightness * brightness / 100);

        g_debug( "writing '%s' to '%s'", buf, backlight_node );
        write( fd, buf, numbytes );
        close( fd );
    }

	if(brightness == 0)
		neod_buttonactions_lock_display();
}

gboolean neod_buttonactions_powersave_timeout1( guint timeout )
{
    g_debug( "mainmenu powersave timeout 1" );
    //FIXME talk to peripheral device abstraction daemon
    power_state = DISPLAY_DIM;
    neod_buttonactions_set_display( 25 );
    return FALSE;
}

gboolean neod_buttonactions_powersave_timeout2( guint timeout )
{
    g_debug( "mainmenu powersave timeout 2" );
    //FIXME talk to peripheral device abstraction daemon
    neod_buttonactions_set_display( 0 );
    power_state = DISPLAY_OFF;
    return FALSE;
}

gboolean neod_buttonactions_powersave_timeout3( guint timeout )
{
    if ( pm_value != FULL )
        return FALSE;
    g_debug( "mainmenu powersave timeout 3" );
    //FIXME talk to peripheral device abstraction daemon
    power_state = SUSPEND;
    system( "/usr/bin/apm -s");
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );
    power_state = NORMAL;
    return FALSE;
}

static void _sound_play( const gchar* samplename )
{
    pa_context_play_sample( pac,
                            samplename,      // Name of my sample
                            NULL,            // Use default sink
                            PA_VOLUME_NORM,  // Full volume
                            NULL,            // Don't need a callback
                            NULL
                           );
}

void neod_buttonactions_sound_play( const gchar* samplename )
{
    g_return_if_fail( pac );

    pa_threaded_mainloop_lock( pam );
    _sound_play( samplename );
    pa_threaded_mainloop_unlock( pam );
}

/* this function is called within the pulseaudio event thread */
static void neod_buttonactions_sound_state_cb( pa_context* pac, void* userdata )
{
    g_debug( "mainmenu sound state callback. state = %d", pa_context_get_state( pac ) );
    if ( pa_context_get_state( pac ) == PA_CONTEXT_READY )
    {
        _sound_play ( "startup" );
    }
}

void neod_buttonactions_sound_init()
{
    g_debug( "panel mainmenu sound init" );
    
    g_thread_init( NULL );

    pam = pa_threaded_mainloop_new();
    if ( !pam )
    {
        printf( "couldn't create mainloop: %s", strerror( errno ) );
        return;
    }

    pa_mainloop_api* mapi = pa_threaded_mainloop_get_api( pam );

    pac = pa_context_new( mapi, "test client" );
    if ( !pac )
    {
        printf( "couldn't create pa_context: %s", strerror( errno ) );
        return;
    }

    pa_context_set_state_callback( pac, neod_buttonactions_sound_state_cb, NULL );
    pa_context_connect( pac, NULL, 0, NULL );
    pa_threaded_mainloop_start( pam );

    g_debug( "sound init ok. threaded mainloop started" );
}

gboolean neod_buttonactions_initial_update()
{
    // need a workaround until OM bug #991 has been fixed
    // http://bugzilla.openmoko.org/cgi-bin/bugzilla/show_bug.cgi?id=991
    g_debug( "neod_buttonactions_initial_update" );
#ifdef BUG_991_FIXED
    for ( int i = 0; i <= max_input_fd; ++i )
    {
        char name[256] = "Unknown";
        if( ioctl( input_fd[i].fd, EVIOCGNAME(sizeof(name)), name ) < 0)
        {
            perror("evdev ioctl");
            continue;
        }

        g_debug( "input node %d corresponds to %s", i, name );

        if ( strcmp( name, "FIC Neo1973 PMU events" ) != 0 )
            continue;

        BIT_MASK( keys, KEY_MAX );
        if( ioctl( input_fd[i].fd, EVIOCGKEY(sizeof(keys)), keys ) < 0)
        {
            perror("evdev ioctl");
            continue;
        }

        if ( BIT_TEST( keys, CHARGER_INSERTION_BUTTON ) )
#else
        apm_info info;
        memset (&info, 0, sizeof (apm_info));
        apm_read (&info);
        if ( info.battery_status == BATTERY_STATUS_CHARGING )
#endif
        {
            g_debug( "charger already inserted" );
            g_spawn_command_line_async( "dbus-send --system /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.ChargerConnected", NULL );
        }
        else
        {
            g_debug( "charger not yet inserted" );
            g_spawn_command_line_async( "dbus-send --system /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.ChargerDisconnected", NULL );
        }
#ifdef BUG_991_FIXED
    }
#endif

    return FALSE;
}

/**
 * This callback function gets triggered when the drop was successful. The
 * lock_display window gets hidden in this situation.
 */
void neod_buttonactions_lock_display_on_drag(GtkWidget *widget, 
										     GdkDragContext *context, 
											 int x, int y,
                        					 GtkSelectionData *seldata, 
											 guint info, 
											 guint time,
					                         gpointer userdata) {
	gtk_widget_hide(lock_display);
	neod_buttonactions_set_orientation(lock_display_last_orientation);
}

/**
 * This callback function sets the data which is send throug the
 * drag and drop. It is necassery to set something here to allow 
 * dragging.
 */
void neod_buttonactions_lock_display_data_get(GtkWidget *widget, 
											  GdkDragContext *context, 
                        					  GtkSelectionData *seldata, 
											  guint info, 
											  guint time,
					                          gpointer userdata) {
	guchar str[] = "LOCK";
	gtk_selection_data_set(seldata, seldata->target, 8, str, sizeof(str));
}

/**
 * This callback function should be triggered whenever the dragging
 * on the display lock begins. The drag image gets set as icon
 * and gets hidden as a picture in the window.
 */
void neod_buttonactions_lock_display_drag_begin(GtkWidget *widget, 
												GdkDragContext *context, 
												gpointer userdata) {
	gtk_widget_hide(GTK_WIDGET(userdata));
	
	g_debug("lock display drag begins");
}

/**
 * This callback function should be triggered whenever the dragging
 * on the display lock ends. 
 */
void neod_buttonactions_lock_display_drag_end(GtkWidget *widget, 
											  GdkDragContext *context, 
											  gpointer userdata) {
	gtk_widget_show(GTK_WIDGET(userdata));
	g_debug("lock display drag ends");
}

/**
 * Activates the display lock. An uncloseable window is implemented,
 * which can only be hidden if the user draws a specif motion
 * on the display.
 */
void neod_buttonactions_lock_display() {
    // remember last active window before showing display lock 
    last_active_window = get_window_property( 
		gdk_x11_get_default_root_xwindow(), 
		gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW"));

    if(!lock_display) {
		// lock display gui
        lock_display = gtk_window_new(GTK_WINDOW_TOPLEVEL);

		GtkWidget *fixed = gtk_fixed_new();
		gtk_fixed_put(GTK_FIXED(fixed), 
			gtk_image_new_from_file(PKGDATADIR "/south.png"), 0, 0);
		GtkWidget *drag = gtk_image_new_from_file(PKGDATADIR "/drag.png");
		gtk_fixed_put(GTK_FIXED(fixed), drag, 190, 0);
		
		GtkWidget *south = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(south), fixed);
		
		GtkWidget *north = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(north), 
			gtk_image_new_from_file(PKGDATADIR "/north.png"));
		
        GtkWidget *mid = gtk_image_new_from_file(PKGDATADIR "/mid.png");

        GtkWidget *box1 = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(box1), north, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(box1), mid, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(box1), south, 0, 0, 0);

		gtk_window_fullscreen(GTK_WINDOW(lock_display));
        gtk_window_set_keep_above( GTK_WINDOW(lock_display), TRUE );
		gtk_container_add(GTK_CONTAINER(lock_display), box1);

		static GtkTargetEntry targetentries[] = {{"text/plain", 0, 0}};

		gtk_drag_dest_set(north, GTK_DEST_DEFAULT_ALL, targetentries, 1,
			GDK_ACTION_COPY);
		gtk_drag_source_set(south, GDK_BUTTON1_MASK, targetentries, 1,
			GDK_ACTION_COPY);

		// drag and drop signals
		gtk_signal_connect(GTK_OBJECT(north), "drag_data_received",
			GTK_SIGNAL_FUNC(neod_buttonactions_lock_display_on_drag), NULL);
		gtk_signal_connect(GTK_OBJECT(south), "drag_data_get",
			GTK_SIGNAL_FUNC(neod_buttonactions_lock_display_data_get), NULL);
		gtk_signal_connect(GTK_OBJECT(south), "drag_begin",
			GTK_SIGNAL_FUNC(neod_buttonactions_lock_display_drag_begin), drag);
		gtk_signal_connect(GTK_OBJECT(south), "drag_end",
			GTK_SIGNAL_FUNC(neod_buttonactions_lock_display_drag_end), drag);
	
		// drag and drop icon
		GdkPixmap *pixmap = NULL;
		GdkBitmap *mask = NULL;

		GdkPixbuf *drag_gdk = 
			gdk_pixbuf_new_from_file(PKGDATADIR "/drag.png", NULL);

		gdk_pixbuf_render_pixmap_and_mask(drag_gdk,  &pixmap, &mask, 128);
		gtk_drag_set_default_icon(gdk_colormap_get_system(), pixmap, mask,
			gdk_pixbuf_get_width(drag_gdk) / 2,
			gdk_pixbuf_get_height(drag_gdk) / 2);
    }

	lock_display_last_orientation = orientation;
	neod_buttonactions_set_orientation(FALSE);
	gtk_widget_show_all(lock_display);

    g_debug("display locked");
}
