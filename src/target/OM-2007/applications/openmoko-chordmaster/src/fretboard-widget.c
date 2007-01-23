/*  fretboard-widget.c
 *
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
 *
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "fretboard-widget.h"

#include <pango/pango.h>

G_DEFINE_TYPE (FretboardWidget, fretboard_widget, GTK_TYPE_DRAWING_AREA);

#define FRETBOARD_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHORD_TYPE_FRETBOARD, FretboardWidgetPrivate))

typedef struct _FretboardWidgetPrivate
{
} FretboardWidgetPrivate;

/* forward declarations */
gboolean
_expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    g_debug( "expose event callback" );
    GError* error = NULL;

    GdkGC* gc = gdk_gc_new( widget->window );

    gchar* frets = NULL;
    if ( CHORD_FRETBOARD(widget)->frets )
        frets = g_strdup( CHORD_FRETBOARD(widget)->frets );

    gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_MITER );

    // background
    gdk_draw_pixbuf( widget->window, gc,
            CHORD_FRETBOARD(widget)->texture,
            0,
            0,
            30,
            0,
            -1,
            -1,
            GDK_RGB_DITHER_MAX,
            0,
            0);

    // frets
    for ( int i = 0; i < 6; ++i )
    {
        GdkColor fretcolor1 = { 0, 0xff <<8, 0xfb <<8, 0xf7 <<8 };
        GdkColor fretcolor2 = { 0, 0xc6 <<8, 0xb7 <<8, 0xcd <<8 };
        GdkColor fretcolor3 = { 0, 0x90 <<8, 0x81 <<8, 0x80 <<8 };

        gdk_gc_set_rgb_fg_color( gc, &fretcolor1 );
        gdk_draw_line( widget->window, gc, 30 + i*80, 0, 30 + i*80, 256 );
        gdk_gc_set_rgb_fg_color( gc, &fretcolor2 );
        gdk_draw_line( widget->window, gc, 32 + i*80, 0, 32 + i*80, 256 );
        gdk_gc_set_rgb_fg_color( gc, &fretcolor3 );
        gdk_draw_line( widget->window, gc, 34 + i*80, 0, 34 + i*80, 256 );
    }

    if ( !frets) return TRUE;

    const gchar strings[] = "EBGDAE";

    PangoContext* context = gtk_widget_get_pango_context( widget );
    PangoLayout* layout = pango_layout_new( context );

    // strings
    for ( int i = 0; i < 6; ++i )
    {
        GdkColor labeloffcolor = { 0, 0xee <<8, 0xee <<8, 0xee <<8 };
        GdkColor labeloncolor = { 0, 0x44 <<8, 0x44 <<8, 0x44 <<8 };
        GdkColor stringcolor1 = { 0, 0xff <<8, 0xfb <<8, 0xa7 <<8 };
        GdkColor stringcolor2 = { 0, 0xe6 <<8, 0xb7 <<8, 0x2d <<8 };
        GdkColor stringcolor3 = { 0, 0x90 <<8, 0x41 <<8, 0x00 <<8 };

        if ( frets[5-i] == 'x' || frets[5-i] == 'X' )
        {
            gdk_gc_set_rgb_fg_color( gc, &labeloffcolor );
            pango_layout_set_text( layout, "x", 1 );
            gdk_gc_set_line_attributes(gc, 2, GDK_LINE_ON_OFF_DASH, GDK_CAP_ROUND, GDK_JOIN_MITER );
        }
        else
        {
            gdk_gc_set_rgb_fg_color( gc, &labeloncolor );
            pango_layout_set_text( layout, &strings[i], 1 );
            gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_MITER );
        }
        //FIXME take font size into account
        gdk_draw_layout( widget->window, gc, 0, 20 + 40*i, layout );

        gdk_gc_set_rgb_fg_color( gc, &stringcolor1 );
        gdk_draw_line( widget->window, gc, 30, 30 + i*40, 470, 30 + i*40 );
        gdk_gc_set_rgb_fg_color( gc, &stringcolor2 );
        gdk_draw_line( widget->window, gc, 30, 32 + i*40, 470, 32 + i*40 );
        gdk_gc_set_rgb_fg_color( gc, &stringcolor3 );
        gdk_draw_line( widget->window, gc, 30, 34 + i*40, 470, 34 + i*40 );
    }

    // barree
    gint maxfinger = 0;
    gint minfinger = 10;

    for ( int i = 0; i < 6; ++i )
    {
        if ( frets[i] == 'x' || frets[i] == 'X' || frets[i] == '0' ) continue;
        gint position = frets[i] - 0x30;
        if ( position > maxfinger ) maxfinger = position;
        if ( position < minfinger ) minfinger = position;
    }
    if ( maxfinger > 5 )
    {
        for ( int i = 0; i < 6; ++i )
        {
            if ( frets[i] == 'x' || frets[i] == 'X' || frets[i] == '0' ) continue;
            frets[i] = ( frets[i]-= minfinger);
        }
        gchar barree = 0x30 + minfinger;
        GdkColor barreecolor = { 0, 0xee <<8, 0x22 <<8, 0x44 <<8 };
        pango_layout_set_text( layout, &barree, 1 );
        gdk_gc_set_rgb_fg_color( gc, &barreecolor );
        //FIXME take font size into account
        gdk_draw_layout( widget->window, gc, 13, 0, layout );
    }

    // finger positions
    for ( int i = 0; i < 6; ++i )
    {
        if ( frets[5-i] == 'x' || frets[5-i] == 'X' ) continue;
        gint position = ( frets[5-i] - 0x30 );

        g_debug( "finger position '%d' = '%d'", i, position );

        if ( position )
            gdk_draw_pixbuf( widget->window, gc, CHORD_FRETBOARD(widget)->fingerpoint,
                0, 0,
                50 + 80*(position-1), 10+40*i,
                -1, -1,
                GDK_RGB_DITHER_MAX, 0, 0);
    }
    g_free( frets );
    return TRUE;
}

static void
fretboard_widget_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (fretboard_widget_parent_class)->dispose)
        G_OBJECT_CLASS (fretboard_widget_parent_class)->dispose (object);
}

static void
fretboard_widget_finalize (GObject *object)
{
    G_OBJECT_CLASS (fretboard_widget_parent_class)->finalize (object);
}

static void
fretboard_widget_class_init (FretboardWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (FretboardWidgetPrivate));

    /* hook virtual methods */
    /* ... */

    /* install properties */
    /* ... */

    object_class->dispose = fretboard_widget_dispose;
    object_class->finalize = fretboard_widget_finalize;
}

static void
fretboard_widget_init (FretboardWidget *self)
{
    GError* error = NULL;
    self->texture = gdk_pixbuf_new_from_file( PKGDATADIR "/wood.png", &error );
    self->fingerpoint = gdk_pixbuf_new_from_file( PKGDATADIR "/ball.png", &error );

    gtk_widget_set_size_request(self, 450, 270);
    g_signal_connect (G_OBJECT(self), "expose_event",
                      G_CALLBACK(_expose_event_callback), NULL);

    //FIXME find out how to make the background of the GdkDrawingArea transparent...
    gtk_widget_set_name( GTK_WIDGET(self), "mokopanedwindow-lower-enclosing" );
}

FretboardWidget*
fretboard_widget_new (void)
{
    return g_object_new (CHORD_TYPE_FRETBOARD, NULL);
}

void fretboard_widget_set_frets(FretboardWidget* self, const gchar* frets)
{
    //g_free( self->frets ); self->frets = g_strdup( frets );
    self->frets = frets;
}
