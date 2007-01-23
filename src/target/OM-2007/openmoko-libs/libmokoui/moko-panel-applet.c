/*  moko-panel-applet.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */

#include "moko-panel-applet.h"

#include <gtk/gtkmenu.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>

#include <gdk/gdkx.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoPanelApplet, moko_panel_applet, G_TYPE_OBJECT)

#define MOKO_PANEL_APPLET_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletPrivate))
#define MOKO_PANEL_APPLET_TAP_HOLD_TIMEOUT  750

typedef struct _MokoPanelAppletPrivate
{
    gboolean is_initialized;
    gboolean hold_timeout_triggered;
    gboolean scaling_requested;
} MokoPanelAppletPrivate;

enum {
    DUMMY_SIGNAL,
    CLICKED,
    TAP_HOLD,
    LAST_SIGNAL,
};

static guint moko_panel_applet_signals[LAST_SIGNAL] = { 0, };

/* parent class pointer */
static GObjectClass* parent_class = NULL;
static int* app_argc;
static char*** app_argv;
static gboolean init_ok = FALSE;

void moko_panel_system_init( int* argc, char*** argv )
{
    gtk_init( argc, argv );
    app_argc = argc;
    app_argv = argv;
    init_ok = TRUE;
}

/* forward declarations */
void moko_panel_applet_real_resize_callback(MokoPanelApplet* self, int w, int h);
void moko_panel_applet_real_paint_callback(MokoPanelApplet* self, Drawable drw);
void moko_panel_applet_real_button_press_callback(MokoPanelApplet* self, int x, int y);
void moko_panel_applet_real_button_release_callback(MokoPanelApplet* self, int x, int y);

void moko_panel_applet_signal_clicked(MokoPanelApplet* self);
void moko_panel_applet_signal_tap_hold(MokoPanelApplet* self);

static void _mb_applet_resize_callback(MBTrayApp* mb_applet, int w, int h);
static void _mb_applet_paint_callback(MBTrayApp* mb_applet, Drawable drw);
static void _mb_applet_button_callback(MBTrayApp* mb_applet, int x, int y, Bool released);

static GdkFilterReturn _moko_panel_applet_gdk_event_filter(GdkXEvent* xev, GdkEvent* gev, MokoPanelApplet* self);

static void
moko_panel_applet_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_panel_applet_parent_class)->dispose)
        G_OBJECT_CLASS (moko_panel_applet_parent_class)->dispose (object);
}

static void
moko_panel_applet_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_panel_applet_parent_class)->finalize (object);
}

static void
moko_panel_applet_class_init(MokoPanelAppletClass* klass)
{
    g_assert( init_ok ); // if this fails, you probably forgot to call moko_panel_init()
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoPanelAppletPrivate));

    /* hook destruction */
    object_class->dispose = moko_panel_applet_dispose;
    object_class->finalize = moko_panel_applet_finalize;

    /* virtual methods */
    klass->resize_callback = moko_panel_applet_real_resize_callback;
    klass->paint_callback = moko_panel_applet_real_paint_callback;
    klass->button_press_callback = moko_panel_applet_real_button_press_callback;
    klass->button_release_callback = moko_panel_applet_real_button_release_callback;

    klass->clicked = moko_panel_applet_signal_clicked;
    klass->tap_hold = moko_panel_applet_signal_tap_hold;

    /* install properties */

    /* install signals */
    moko_panel_applet_signals[CLICKED] = g_signal_new ("clicked",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoPanelAppletClass, clicked),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    moko_panel_applet_signals[TAP_HOLD] = g_signal_new ("tap-hold",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoPanelAppletClass, tap_hold),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

MokoPanelApplet*
moko_panel_applet_new()
{
    MokoPanelApplet* self = g_object_new(MOKO_TYPE_PANEL_APPLET, NULL);
    return self;
}

static void
moko_panel_applet_init(MokoPanelApplet* self)
{
    moko_debug( "moko_panel_applet_init" );

    MokoPanelAppletClass* klass = MOKO_PANEL_APPLET_GET_CLASS(self);
    //self->applet = mb_tray_app_new_with_display (unsigned char *app_name, MBTrayAppResizeCB resize_cb, MBTrayAppPaintCB paint_cb, int *argc, char ***argv, Display *display)
    self->mb_applet = mb_tray_app_new_with_display( "testing", _mb_applet_resize_callback, _mb_applet_paint_callback, app_argc, app_argv, GDK_DISPLAY() );
    mb_tray_app_set_user_data( self->mb_applet, self );
    mb_tray_app_set_button_callback( self->mb_applet, _mb_applet_button_callback );

    self->mb_pixbuf = mb_pixbuf_new( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) );

    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    priv->is_initialized = FALSE;
    priv->scaling_requested = FALSE;
    priv->hold_timeout_triggered = FALSE;
}

static GdkFilterReturn
_moko_panel_applet_gdk_event_filter(GdkXEvent* xev, GdkEvent* gev, MokoPanelApplet* self)
{
    XEvent* ev = (XEvent*)xev;
    Display *dpy = ev->xany.display;

    mb_tray_handle_xevent(self->mb_applet, ev);

    return GDK_FILTER_CONTINUE;
}

static gboolean _moko_panel_applet_window_clicked(GtkWidget* widget, GdkEventButton* event, MokoPanelApplet* self)
{
    gdk_pointer_ungrab( event->time );
    gtk_widget_hide( self->window );
}

static gboolean _moko_panel_applet_tap_hold_timeout(MokoPanelApplet* self)
{
    moko_debug( "_moko_panel_applet_tap_hold_timeout" );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    g_signal_emit( G_OBJECT(self), moko_panel_applet_signals[TAP_HOLD], 0, NULL );
    priv->hold_timeout_triggered = TRUE;
    return FALSE;
}

static void _mb_applet_resize_callback(MBTrayApp* mb_applet, int w, int h)
{
    MokoPanelApplet* self = mb_tray_app_get_user_data( mb_applet );
    MOKO_PANEL_APPLET_GET_CLASS( self )->resize_callback( self, w, h );
}

static void _mb_applet_paint_callback(MBTrayApp* mb_applet, Drawable drw)
{
    MokoPanelApplet* self = mb_tray_app_get_user_data( mb_applet );
    MOKO_PANEL_APPLET_GET_CLASS( self )->paint_callback( self, drw );
}

static void _mb_applet_button_callback(MBTrayApp* mb_applet, int x, int y, Bool released)
{
    MokoPanelApplet* self = mb_tray_app_get_user_data( mb_applet );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    if ( !released )
        MOKO_PANEL_APPLET_GET_CLASS( self )->button_press_callback( self, x, y );
    else if ( !priv->hold_timeout_triggered )
        MOKO_PANEL_APPLET_GET_CLASS( self )->button_release_callback( self, x, y );
    else
        moko_debug( "_mb_applet_button_callback: surpressing release callback, because timeout was triggered" );
}

void moko_panel_applet_real_resize_callback(MokoPanelApplet* self, int w, int h)
{
    moko_debug( "moko_panel_applet_resize_callback" );
    moko_debug( "-- size = %d, %d", w, h );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    if ( !self->mb_pixbuf_image )
    {
        g_warning( "no valid icon for panel application during resize callback" );
        return;
    }
    if ( self->mb_pixbuf_image_scaled && self->mb_pixbuf_image_scaled->width == w && self->mb_pixbuf_image_scaled->height == h )
        return;
    if ( !priv->scaling_requested )
        return;
    moko_debug( "-- new size, scaling pixbuf" );
    MBPixbufImage* scaled = mb_pixbuf_img_scale( self->mb_pixbuf, self->mb_pixbuf_image, w, h );
    if ( self->mb_pixbuf_image_scaled )
        mb_pixbuf_img_free( self->mb_pixbuf, self->mb_pixbuf_image_scaled );
    self->mb_pixbuf_image_scaled = scaled;
}

void moko_panel_applet_real_paint_callback(MokoPanelApplet* self, Drawable drw)
{
    moko_debug( "moko_panel_applet_paint_callback" );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    MBPixbufImage* icon = priv->scaling_requested ? self->mb_pixbuf_image_scaled : self->mb_pixbuf_image;

    if ( !icon )
    {
        g_warning( "no valid icon for panel application during paint callback" );
        return;
    }

    MBPixbufImage* background = mb_tray_app_get_background( self->mb_applet, self->mb_pixbuf );
    mb_pixbuf_img_composite( self->mb_pixbuf, background, icon, 0, 0 );
    mb_pixbuf_img_render_to_drawable( self->mb_pixbuf, background, drw, 0, 0 );
#if 0
    mb_pixbuf_img_free( self->mb_pixbuf, background );
#endif
}

void moko_panel_applet_real_button_press_callback(MokoPanelApplet* self, int x, int y)
{
    moko_debug( "moko_panel_applet_real_button_press_callback" );
    moko_debug( "-- at %d, %d", x, y );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    priv->hold_timeout_triggered = FALSE;
    g_timeout_add( MOKO_PANEL_APPLET_TAP_HOLD_TIMEOUT, (GSourceFunc) _moko_panel_applet_tap_hold_timeout, (gpointer) self );
}

void moko_panel_applet_real_button_release_callback(MokoPanelApplet* self, int x, int y)
{
    moko_debug( "moko_panel_applet_real_button_release_callback" );
    moko_debug( "-- at %d, %d", x, y );

    if ( x > 0 && x < mb_tray_app_width( self->mb_applet ) && y > 0 && y < mb_tray_app_height( self->mb_applet ) )
    {
        g_signal_emit( G_OBJECT(self), moko_panel_applet_signals[CLICKED], 0, NULL );
        g_source_remove_by_user_data( (gpointer) self );
    }
}

void moko_panel_applet_get_positioning_hint(MokoPanelApplet* self, GtkWidget* popup, int* x, int* y)
{
    int win_w;
    int win_h;
    gdk_window_get_geometry( GTK_WIDGET(self->window)->window, NULL, NULL, &win_w, &win_h, NULL );
    moko_debug( "-- popup geom = %d, %d", win_w, win_h );
    GtkAllocation* allocation = &GTK_WIDGET(self->window)->allocation;
    moko_debug( "-- popup alloc = %d, %d", allocation->width, allocation->height );

    int x_abs;
    int y_abs;
    mb_tray_app_get_absolute_coords( self->mb_applet, &x_abs, &y_abs );
    moko_debug( "-- abs position = %d, %d", x_abs, y_abs );

    *x = x_abs;
    *y = y_abs + mb_tray_app_height( self->mb_applet ) + 4;

    if ( *x + win_w > DisplayWidth( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) ) )
            *x = DisplayWidth( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) ) - win_w - 2;

    if ( *y + win_h > DisplayHeight( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) ) )
            *y = DisplayHeight( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) ) - win_h - mb_tray_app_height( self->mb_applet ) - 2;

    moko_debug( "-- final position = %d, %d", *x, *y );
}

void moko_panel_applet_signal_clicked(MokoPanelApplet* self)
{
    moko_debug( "moko_panel_applet_signal_clicked" );
    if ( self->window && GTK_WIDGET_VISIBLE( self->window ) )
        moko_panel_applet_close_popup( self );
    else
        moko_panel_applet_open_popup( self, MOKO_PANEL_APPLET_CLICK_POPUP );
}

void moko_panel_applet_signal_tap_hold(MokoPanelApplet* self)
{
    moko_debug( __FUNCTION__ );
    moko_panel_applet_open_popup( self, MOKO_PANEL_APPLET_TAP_HOLD_POPUP );
}

////////////////
// PUBLIC API //
////////////////
void moko_panel_applet_set_icon(MokoPanelApplet* self, const gchar* filename, gboolean request_scaling)
{
    moko_debug_minder( self->mb_pixbuf );
    if ( self->mb_pixbuf_image ) mb_pixbuf_img_free( self->mb_pixbuf, self->mb_pixbuf_image );
    self->mb_pixbuf_image = mb_pixbuf_img_new_from_file( self->mb_pixbuf, filename );
    g_assert( self->mb_pixbuf_image );
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    priv->scaling_requested = request_scaling;
    if ( !request_scaling )
    {
        moko_panel_applet_request_size( self, mb_pixbuf_img_get_width( self->mb_pixbuf_image ), mb_pixbuf_img_get_height( self->mb_pixbuf_image ) );
    }
}

void moko_panel_applet_set_popup(MokoPanelApplet* self, GtkWidget* popup, MokoPanelAppletPopupType type)
{
    if ( popup == self->popup[type] )
        return;
    if ( self->popup[type] )
    {
        gtk_widget_destroy( self->popup[type] );
        //FIXME necessary here or does gtk_widget_destroy removes all references?
        g_object_unref( self->popup[type] );
    }
    self->popup[type] = popup;
    g_object_ref( popup );
}

void moko_panel_applet_open_popup(MokoPanelApplet* self, MokoPanelAppletPopupType type)
{
    moko_debug( "moko_panel_applet_open_popup [type=%d]", type );
    GtkWidget* popup = self->popup[type];
    if ( !popup )
    {
        moko_debug( "-- no popup for type %d : return", type );
        return;
    }

    if ( self->window && GTK_WIDGET_VISIBLE( self->window ) )
        moko_panel_applet_close_popup( self );

    if ( GTK_IS_MENU(self->popup[type]) )
    {
        self->window = self->popup[type];
        gtk_menu_popup( GTK_MENU( self->window ), NULL, NULL, NULL, NULL, 0, CurrentTime );
    }
    else
    {
        self->window = gtk_window_new( GTK_WINDOW_POPUP );
        gtk_container_add( GTK_CONTAINER(self->window), self->popup[type] );
        g_signal_connect( G_OBJECT(self->window), "button-press-event", G_CALLBACK(_moko_panel_applet_window_clicked), self );
        gtk_widget_add_events( self->window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
        gtk_widget_realize( self->window );
        gtk_widget_show_all( self->window );
        int x = 0;
        int y = 0;
        moko_panel_applet_get_positioning_hint( self, self->popup, &x, &y );
        gtk_window_move( self->window, x, y );
        //gtk_widget_set_uposition( self->window, x, y );
        gdk_pointer_grab( GTK_WIDGET(self->window)->window, TRUE, GDK_BUTTON_PRESS_MASK, NULL, NULL, CurrentTime );
    }

}

void moko_panel_applet_close_popup(MokoPanelApplet* self)
{
    g_return_if_fail( self->window || !GTK_WIDGET_VISIBLE(self->window) );
    moko_debug( "moko_panel_applet_close_popup" );

    if ( GTK_IS_MENU( self->window ) )
    {
        gtk_menu_popdown( GTK_MENU( self->window) );
    }
    else
    {
        gdk_pointer_ungrab( CurrentTime );
        gtk_widget_hide( self->window );
        gtk_container_remove( GTK_CONTAINER(self->window), gtk_bin_get_child( GTK_BIN(self->window) ) );
        gtk_widget_destroy( self->window );
        self->window = 0L;
    }
}

void moko_panel_applet_request_size(MokoPanelApplet* self, int x, int y)
{
    moko_debug( "moko_panel_applet_request_size: %d, %d", x, y );
    mb_tray_app_request_size( self->mb_applet, x, y );
}

void moko_panel_applet_request_offset(MokoPanelApplet* self, int offset)
{
    mb_tray_app_request_offset( self->mb_applet, offset );
}

void moko_panel_applet_show(MokoPanelApplet* self)
{
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    if ( !priv->is_initialized )
    {
        mb_tray_app_main_init( self->mb_applet );
        gdk_window_add_filter( NULL, _moko_panel_applet_gdk_event_filter, self );
        priv->is_initialized = TRUE;
    }
    else
    {
        mb_tray_app_unhide( self->mb_applet );
    }
}

void moko_panel_applet_hide(MokoPanelApplet* self)
{
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    if ( priv->is_initialized )
    {
        mb_tray_app_hide( self->mb_applet );
    }
}