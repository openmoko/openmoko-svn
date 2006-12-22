/*  moko-panel-applet.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/12/07 17:20:16 $) [$Author: mickey $]
 */

#include "moko-panel-applet.h"

#include <gdk/gdkx.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoPanelApplet, moko_panel_applet, G_TYPE_OBJECT)

#define PANEL_APPLET_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletPrivate))

typedef struct _MokoPanelAppletPrivate
{
} MokoPanelAppletPrivate;

/* parent class pointer */
static GObjectClass* parent_class = NULL;
static int* app_argc;
static char*** app_argv;

/* forward declarations */
void moko_panel_applet_real_resize_callback(MokoPanelApplet* self, int w, int h);
void moko_panel_applet_real_paint_callback(MokoPanelApplet* self, Drawable drw);
static void _mb_applet_resize_callback(MBTrayApp* mb_applet, int w, int h);
static void _mb_applet_paint_callback(MBTrayApp* mb_applet, Drawable drw);
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

    /* install properties */
}

MokoPanelApplet*
moko_panel_applet_new(int* argc, char*** argv)
{
    app_argc = argc;
    app_argv = argv;
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

    self->mb_pixbuf = mb_pixbuf_new( mb_tray_app_xdisplay( self->mb_applet ), mb_tray_app_xscreen( self->mb_applet ) );

    mb_tray_app_main_init( self->mb_applet );

    gdk_window_add_filter( NULL, _moko_panel_applet_gdk_event_filter, self );


}

static GdkFilterReturn
_moko_panel_applet_gdk_event_filter(GdkXEvent* xev, GdkEvent* gev, MokoPanelApplet* self)
{
    XEvent* ev = (XEvent*)xev;
    Display *dpy = ev->xany.display;

    mb_tray_handle_xevent(self->mb_applet, ev);

    return GDK_FILTER_CONTINUE;
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

void moko_panel_applet_real_resize_callback(MokoPanelApplet* self, int w, int h)
{
    moko_debug( "moko_panel_applet_resize_callback" );
    moko_debug( "-- size = %d, %d", w, h );
    if ( !self->mb_pixbuf_image )
    {
        g_warning( "no valid icon for panel application during resize callback" );
        return;
    }
    if ( self->mb_pixbuf_image_scaled && self->mb_pixbuf_image_scaled->width == w && self->mb_pixbuf_image_scaled->height == h )
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
    if ( !self->mb_pixbuf_image_scaled )
    {
        g_warning( "no valid icon for panel application during paint callback" );
        return;
    }

    MBPixbufImage* background = mb_tray_app_get_background( self->mb_applet, self->mb_pixbuf );
    mb_pixbuf_img_composite( self->mb_pixbuf, background, self->mb_pixbuf_image_scaled, 0, 0 );
    mb_pixbuf_img_render_to_drawable( self->mb_pixbuf, background, drw, 0, 0 );
    mb_pixbuf_img_free( self->mb_pixbuf, background );
}

void moko_panel_applet_set_icon(MokoPanelApplet* self, const gchar* filename)
{
    moko_debug( "moko_panel_applet_set_icon" );
    g_assert( self->mb_pixbuf );
    if ( self->mb_pixbuf_image ) mb_pixbuf_img_free( self->mb_pixbuf, self->mb_pixbuf_image );
    self->mb_pixbuf_image = mb_pixbuf_img_new_from_file( self->mb_pixbuf, filename );
}
