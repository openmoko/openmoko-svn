/*
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
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

#include <gconf/gconf-client.h>

#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>

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

#define SYS_CLASS_BACKLIGHT "/sys/class/backlight/"

static gchar* backlight_node = NULL;
static int backlight_max_brightness = 1;

#define AUX_BUTTON_KEYCODE 0x22
#define POWER_BUTTON_KEYCODE 0x23
#define TOUCHSCREEN_BUTTON_KEYCODE 0x14a

#ifdef NEOD_PLATFORM_FIC_NEO1973
    #define AUX_BUTTON_KEYCODE 169    /* aux */
    #define POWER_BUTTON_KEYCODE 116  /* power */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

#ifdef NEOD_PLATFORM_MOTOROLA_EZX
    #define AUX_BUTTON_KEYCODE 0xa7   /* voice */
    #define POWER_BUTTON_KEYCODE 0xd4 /* camera */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

#ifdef NEOD_PLATFORM_HTC
    #define AUX_BUTTON_KEYCODE 0xd4   /* camera */
    #define POWER_BUTTON_KEYCODE 0x74 /* power */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

#ifdef NEOD_PLATFORM_IPAQ
    #define AUX_BUTTON_KEYCODE 89   /* _KEY_RECORD */
    #define POWER_BUTTON_KEYCODE 0x74 /* KEY_POWER */
    #define TOUCHSCREEN_BUTTON_KEYCODE 0x14a
#endif

#define HEADPHONE_INSERTION_SWITCHCODE 0x02
#define CHARGER_INSERTION_BUTTON 0xa4

#define BIT_MASK( name, numbits )                                        \
    unsigned short  name[ ((numbits) - 1) / (sizeof( short ) * 8) + 1 ];    \
    memset( name, 0, sizeof( name ) )
#define BIT_TEST( bitmask, bit )    \
    ( bitmask[ (bit) / sizeof(short) / 8 ] & (1u << ( (bit) % (sizeof(short) * 8))) )

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

GConfClient* gconfc = 0;

enum PowerManagementMode
{
    FULL = 0,
    DIM_ONLY = 1,
    NONE = 2,
};

int pm_value = 0;
gboolean orientation = TRUE;

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
            if ( getenv( "MOKO_DEBUG" ) )
            {
                g_debug( "read %d bytes from fd %d", size, input_fd[i].fd );
                g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
            }
            if ( event.type == 1 && event.code == AUX_BUTTON_KEYCODE )
            {
                if ( event.value == 1 ) /* pressed */
                {
                    g_debug( "triggering aux timer" );
                    aux_timer = g_timeout_add( 1 * 1000, (GSourceFunc) neod_buttonactions_aux_timeout, (gpointer)1 );
                }
                else if ( event.value == 0 ) /* released */
                {
                    g_debug( "resetting aux timer" );
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
                    g_debug( "triggering power timer" );
                    power_timer = g_timeout_add( 1 * 1000, (GSourceFunc) neod_buttonactions_power_timeout, (gpointer)1 );
                }
                else if ( event.value == 0 ) /* released */
                {
                    g_debug( "resetting power timer" );
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
                    g_debug( "stylus pressed" );
                    neod_buttonactions_sound_play( "touchscreen" );
                }
                else if ( event.value == 0 ) /* released */
                {
                    g_debug( "stylus released" );
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
                    g_debug( "charger IN" );
                    neod_buttonactions_sound_play( "touchscreen" );
                    g_spawn_command_line_async( "dbus-send /org/freedesktop/PowerManagement org.freedesktop.powermanagement.ChargerConnected", NULL );
                }
                else if ( event.value == 0 ) /* released */
                {
                    g_debug( "charger OUT" );
                    g_spawn_command_line_async( "dbus-send /org/freedesktop/PowerManagement org.freedesktop.powermanagement.ChargerDisconnected", NULL );
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
                if ( event.value == 0 ) /* inserted */
                {
                    g_debug( "headphones IN" );
                    g_spawn_command_line_async( "amixer sset \"Amp Mode\" \"Headphones\"", NULL );
                }
                else if ( event.value == 1 ) /* released */
                {
                    g_debug( "headphones OUT" );
                    g_spawn_command_line_async( "amixer sset \"Amp Mode\" \"Stereo Speakers\"", NULL );
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
    g_debug( "aux pressed for %d", timeout );

    neod_buttonactions_sound_play( "touchscreen" );
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );

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

// this is hardcoded to the Neo1973
void neod_buttonactions_popup_positioning_cb( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data )
{
    GtkRequisition req;
    gtk_widget_size_request( GTK_WIDGET(menu), &req );
    gint screen_width = gdk_screen_width();
    gint screen_height = gdk_screen_height();
    gboolean landscape_mode = gdk_screen_width() >= gdk_screen_height();

    if ( GTK_WIDGET(menu) == aux_menu )
    {
        if (landscape_mode)
            *x = screen_width - req.width;
        else
            *x = 0;
        *y = 0;
    }
    else if ( GTK_WIDGET(menu) == power_menu )
    {
        if (landscape_mode)
            *x = 0;
        else
            *x = screen_width - req.width;
        *y = screen_height - req.height;
    }
    else
        g_assert( FALSE ); // fail here if called for unknown menu
}

void neod_buttonactions_popup_selected_fullscreen( GtkMenuItem* menu, gpointer user_data )
{
    static int is_fullscreen = 0;

    gtk_widget_hide( aux_menu );
    Window xwindow = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );
    const char* title = get_text_property( xwindow, gdk_x11_get_xatom_by_name("_NET_WM_NAME") );
    g_debug( "active Window = %d ('%s')", (int) xwindow, title );

    Display* display = XOpenDisplay( NULL );

    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = display;
    xev.xclient.window = xwindow;
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

void neod_buttonactions_popup_selected_orientation( GtkMenuItem* menu, gpointer user_data )
{
    gtk_widget_hide( aux_menu );
    if ( orientation )
        g_spawn_command_line_async( "xrandr -o 1", NULL );
    else
        g_spawn_command_line_async( "xrandr -o 0", NULL );
    orientation = !orientation;
}

void neod_buttonactions_popup_selected_screenshot( GtkMenuItem* menu, gpointer user_data )
{
    gtk_widget_hide( aux_menu );
    g_spawn_command_line_async( "gpe-scap", NULL );
}

void neod_buttonactions_popup_selected_fullPM( GtkMenuItem* menu, gpointer user_data )
{
    gconf_client_set_int( gconfc, "/desktop/openmoko/neod/power_management", FULL, NULL );
}

void neod_buttonactions_popup_selected_dimOnly( GtkMenuItem* menu, gpointer user_data )
{
    gconf_client_set_int( gconfc, "/desktop/openmoko/neod/power_management", DIM_ONLY, NULL );
}

void neod_buttonactions_popup_selected_noPM( GtkMenuItem* menu, gpointer user_data )
{
    gconf_client_set_int( gconfc, "/desktop/openmoko/neod/power_management", NONE, NULL );
}

void neod_buttonactions_popup_selected_lock( GtkMenuItem* menu, gpointer user_data )
{
    int fd = open( "/sys/power/state", O_WRONLY );
    if ( fd != -1 )
    {
        char command[] = "mem\n";
        write(fd, &command, sizeof(command) );
        close( fd );
    }
}

void neod_buttonactions_popup_selected_restartUI( GtkMenuItem* menu, gpointer user_data )
{
    //FIXME notify user
    system( "/etc/init.d/xserver-nodm restart");
}

void neod_buttonactions_popup_selected_reboot( GtkMenuItem* menu, gpointer user_data )
{
    //moko_ui_banner_show_text( 4, "Rebooting System..." );
    system( "/sbin/reboot");
}

void neod_buttonactions_popup_selected_poweroff( GtkMenuItem* menu, gpointer user_data )
{
    //moko_ui_banner_show_text( 4, "Shutting down System..." );
    system( "/sbin/poweroff");
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
    // show popup menu requesting for actions
    if ( !aux_menu )
    {
        aux_menu = gtk_menu_new();

        GtkWidget* fullscreen = gtk_menu_item_new_with_label( "Toggle Fullscreen" );
        g_signal_connect( G_OBJECT(fullscreen), "activate", G_CALLBACK(neod_buttonactions_popup_selected_fullscreen), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(aux_menu), fullscreen );

        GtkWidget* orientation = gtk_menu_item_new_with_label( "Swap Orientation" );
        g_signal_connect( G_OBJECT(orientation), "activate", G_CALLBACK(neod_buttonactions_popup_selected_orientation), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(aux_menu), orientation );

        GtkWidget* scap = gtk_menu_item_new_with_label( "Screenshot" );
        g_signal_connect( G_OBJECT(scap), "activate", G_CALLBACK(neod_buttonactions_popup_selected_screenshot), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(aux_menu), scap );

        gtk_widget_show_all( GTK_WIDGET(aux_menu) );
    }
    gtk_menu_popup( GTK_MENU(aux_menu), NULL, NULL, neod_buttonactions_popup_positioning_cb, 0, 0, GDK_CURRENT_TIME );
}

void neod_buttonactions_show_power_menu()
{
    // show popup menu requesting for actions
    if ( !power_menu )
    {
        power_menu = gtk_menu_new();

        GtkWidget* bluetooth = gtk_check_menu_item_new_with_label( "Bluetooth Power" );
        gtk_widget_set_state( bluetooth, GTK_STATE_INSENSITIVE );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), bluetooth );

        GtkWidget* gps = gtk_check_menu_item_new_with_label( "GPS Power" );
        gtk_widget_set_state( gps, GTK_STATE_INSENSITIVE );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), gps );

        GtkWidget* gsm = gtk_check_menu_item_new_with_label( "GSM Power" );
        gtk_widget_set_state( gsm, GTK_STATE_INSENSITIVE );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), gsm );

        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), gtk_separator_menu_item_new() );

        // add profiles
        // TODO build profile list dynamically from database
        GtkWidget* profile = 0;
        profile = gtk_check_menu_item_new_with_label( "Profile: Full PM" );
        gtk_check_menu_item_set_draw_as_radio( GTK_CHECK_MENU_ITEM(profile), TRUE );
        gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(profile), pm_value == FULL );
        g_signal_connect( G_OBJECT(profile), "activate", G_CALLBACK(neod_buttonactions_popup_selected_fullPM), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), profile );
        profile = gtk_check_menu_item_new_with_label( "Profile: Dim Only" );
        gtk_check_menu_item_set_draw_as_radio( GTK_CHECK_MENU_ITEM(profile), TRUE );
        gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(profile), pm_value == DIM_ONLY );
        g_signal_connect( G_OBJECT(profile), "activate", G_CALLBACK(neod_buttonactions_popup_selected_dimOnly), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), profile );
        profile = gtk_check_menu_item_new_with_label( "Profile: No PM" );
        gtk_check_menu_item_set_draw_as_radio( GTK_CHECK_MENU_ITEM(profile), TRUE );
        gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(profile), pm_value == NONE );
        g_signal_connect( G_OBJECT(profile), "activate", G_CALLBACK(neod_buttonactions_popup_selected_noPM), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), profile );

        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), gtk_separator_menu_item_new() );

        GtkWidget* lock = gtk_menu_item_new_with_label( "Lock Phone" );
        g_signal_connect( G_OBJECT(lock), "activate", G_CALLBACK(neod_buttonactions_popup_selected_lock), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), lock );
            //GtkWidget* flightmode = gtk_menu_item_new_with_label( "Flight Mode" );
            //gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), flightmode );
            //GtkWidget* profilelist = gtk_menu_item_new_with_label( "<Profile List>" );
            //gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), profilelist );
            //GtkWidget* restartUI = gtk_menu_item_new_with_label( "Restart UI" );
            //g_signal_connect( G_OBJECT(restartUI), "activate", G_CALLBACK(neod_buttonactions_popup_selected_restartUI), NULL );
            //gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), restartUI );
        GtkWidget* reboot = gtk_menu_item_new_with_label( "Reboot Phone" );
        g_signal_connect( G_OBJECT(reboot), "activate", G_CALLBACK(neod_buttonactions_popup_selected_reboot), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), reboot );
        GtkWidget* poweroff = gtk_menu_item_new_with_label( "Power Off" );
        g_signal_connect( G_OBJECT(poweroff), "activate", G_CALLBACK(neod_buttonactions_popup_selected_poweroff), NULL );
        gtk_menu_shell_append( GTK_MENU_SHELL(power_menu), poweroff );
        gtk_widget_show_all( GTK_WIDGET(power_menu) );
    }
    gtk_menu_popup( GTK_MENU(power_menu), NULL, NULL, neod_buttonactions_popup_positioning_cb, 0, 0, GDK_CURRENT_TIME );
}

gboolean neod_buttonactions_power_timeout( guint timeout )
{
    g_debug( "power pressed for %d", timeout );

    neod_buttonactions_sound_play( "touchscreen" );
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );

    power_timer = -1;
    if ( timeout < 1 )
    {
        Window xwindow = get_window_property( gdk_x11_get_default_root_xwindow(), gdk_x11_get_xatom_by_name("_NET_ACTIVE_WINDOW") );
        const char* title = get_text_property( xwindow, gdk_x11_get_xatom_by_name("_NET_WM_NAME") );
        g_debug( "active Window = %d ('%s')", (int) xwindow, title );

        if ( is_desktop_window(xwindow) )
        {
            g_debug( "sorry, i'm not going to close the today window" );
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
    powersave_timer1 = g_timeout_add( 10 * 1000, (GSourceFunc) neod_buttonactions_powersave_timeout1, (gpointer)1 );
    powersave_timer2 = g_timeout_add( 20 * 1000, (GSourceFunc) neod_buttonactions_powersave_timeout2, (gpointer)1 );
    powersave_timer3 = g_timeout_add( 60 * 5 * 1000, (GSourceFunc) neod_buttonactions_powersave_timeout3, (gpointer)1 );
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
        int numbytes = g_sprintf( buf, "%d\0", backlight_max_brightness * brightness / 100 );
        g_debug( "writing '%s' to '%s'", buf, backlight_node );
        write( fd, buf, numbytes );
        close( fd );
    }
}

gboolean neod_buttonactions_powersave_timeout1( guint timeout )
{
    g_debug( "mainmenu powersave timeout 1" );
    //FIXME talk to neod
    power_state = DISPLAY_DIM;
    neod_buttonactions_set_display( 25 );
    return FALSE;
}

gboolean neod_buttonactions_powersave_timeout2( guint timeout )
{
    g_debug( "mainmenu powersave timeout 2" );
    //FIXME talk to neod
    neod_buttonactions_set_display( 0 );
    power_state = DISPLAY_OFF;
    return FALSE;
}

gboolean neod_buttonactions_powersave_timeout3( guint timeout )
{
    if ( pm_value != FULL )
        return FALSE;
    g_debug( "mainmenu powersave timeout 3" );
    //FIXME talk to neod
    power_state = SUSPEND;
    system( "/usr/bin/apm -s");
    neod_buttonactions_powersave_reset();
    neod_buttonactions_set_display( 100 );
    power_state = NORMAL;
    return FALSE;
}

void neod_buttonactions_sound_state_cb( pa_context* pac, void* userdata )
{
    g_debug( "mainmenu sound state callback. state = %d", pa_context_get_state( pac ) );
    if ( pa_context_get_state( pac ) == PA_CONTEXT_READY )
    {
        neod_buttonactions_sound_play( "startup" );
    }
}

void neod_buttonactions_sound_init()
{
    g_debug( "panel mainmenu sound init" );
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

    pa_context_set_state_callback( pac, neod_buttonactions_sound_state_cb, NULL );
    pa_context_connect( pac, NULL, 0, NULL );
    pa_threaded_mainloop_start( mainloop );

    g_debug( "sound init ok. threaded mainloop started" );
}

void neod_buttonactions_sound_play( const gchar* samplename )
{
    g_return_if_fail( pac );
    pa_context_play_sample( pac,
                            samplename,      // Name of my sample
                            NULL,            // Use default sink
                            PA_VOLUME_NORM,  // Full volume
                            NULL,            // Don't need a callback
                            NULL
                           );

}

gboolean neod_buttonactions_initial_update()
{
    g_debug( "neod_buttonactions_initial_update" );
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
        {
            g_debug( "charger already inserted" );
            g_spawn_command_line_async( "dbus-send /org/freedesktop/PowerManagement org.freedesktop.powermanagement.ChargerConnected", NULL );
        }
        else
        {
            g_debug( "charger not yet inserted" );
            g_spawn_command_line_async( "dbus-send /org/freedesktop/PowerManagement org.freedesktop.powermanagement.ChargerDisconnected", NULL );
        }
    }

    return FALSE;
}
