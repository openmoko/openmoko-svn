/*  demo-panel-applet.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/12/21 18:03:04 $) [$Author: mickey $]
 */

#include "demo-panel-applet.h"

#include <libmb/mbpixbuf.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (DemoPanelApplet, demo_panel_applet, MOKO_TYPE_PANEL_APPLET);

#define PANEL_APPLET_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), DEMO_TYPE_PANEL_APPLET, DemoPanelAppletPrivate))

typedef struct _DemoPanelAppletPrivate
{
} DemoPanelAppletPrivate;

/* parent class pointer */
MokoPanelAppletClass* parent_class = NULL;

/* forward declarations */
void demo_panel_applet_clicked(DemoPanelApplet* self);
void demo_panel_applet_tap_hold(DemoPanelApplet* self);
void demo_panel_applet_paint(DemoPanelApplet* self, Drawable drw);

static void
demo_panel_applet_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (demo_panel_applet_parent_class)->dispose)
        G_OBJECT_CLASS (demo_panel_applet_parent_class)->dispose (object);
}

static void
demo_panel_applet_finalize(GObject* object)
{
    G_OBJECT_CLASS (demo_panel_applet_parent_class)->finalize (object);
}

static void
demo_panel_applet_class_init(DemoPanelAppletClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(DemoPanelAppletPrivate));

    /* hook destruction */
    object_class->dispose = demo_panel_applet_dispose;
    object_class->finalize = demo_panel_applet_finalize;

    /* virtual methods */
    MokoPanelAppletClass* applet_class = MOKO_PANEL_APPLET_CLASS(klass);
    applet_class->clicked = demo_panel_applet_clicked;
    applet_class->tap_hold = demo_panel_applet_tap_hold;
    applet_class->paint_callback = demo_panel_applet_paint;

    /* install properties */
}

DemoPanelApplet*
demo_panel_applet_new(void)
{
    return g_object_new(DEMO_TYPE_PANEL_APPLET, NULL);
}

static void
demo_panel_applet_init(DemoPanelApplet* self)
{
    /* Populate your instance here */
}

void demo_panel_applet_clicked(DemoPanelApplet* self)
{
    g_debug( "demo-panel-applet CLICKED" );
}

void demo_panel_applet_tap_hold(DemoPanelApplet* self)
{
    g_debug( "demo-panel-applet TAP-HOLD" );
}

void demo_panel_applet_paint(DemoPanelApplet* self, Drawable drw)
{
    MokoPanelApplet* panel = MOKO_PANEL_APPLET(self);
    MBPixbufImage* background = mb_tray_app_get_background( panel->mb_applet, panel->mb_pixbuf );

    for ( int y = 0; y < mb_tray_app_height( panel->mb_applet ); ++y )
    {
        for ( int x = 0; x < mb_tray_app_width( panel->mb_applet ); ++x )
        {
            mb_pixbuf_img_plot_pixel( panel->mb_pixbuf, background, x, y, y*x, x*3, y*2 );
        }
    }
    //mb_pixbuf_img_composite( self->mb_pixbuf, background, self->mb_pixbuf_image_scaled, 0, 0 );
    mb_pixbuf_img_render_to_drawable( panel->mb_pixbuf, background, drw, 0, 0 );
    mb_pixbuf_img_free( panel->mb_pixbuf, background );

    g_debug( "demo-panel-applet PAINT" );
}
