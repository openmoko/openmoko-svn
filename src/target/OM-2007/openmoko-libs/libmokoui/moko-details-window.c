/*  moko-details-window.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */

#include "moko-details-window.h"

#include "moko-application.h"
#include "moko-paned-window.h"
#include "moko-pixmap-button.h"

#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoDetailsWindow, moko_details_window, GTK_TYPE_SCROLLED_WINDOW)

#define DETAILS_WINDOW_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DETAILS_WINDOW, MokoDetailsWindowPrivate))

typedef struct _MokoDetailsWindowPrivate
{
} MokoDetailsWindowPrivate;

/* parent class pointer */
static GtkScrolledWindowClass* parent_class = NULL;

/* forward declarations */
static void _cb_fullscreen_clicked(GtkButton* button, MokoDetailsWindow* self);

static void
moko_details_window_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_details_window_parent_class)->dispose)
        G_OBJECT_CLASS (moko_details_window_parent_class)->dispose (object);
}

static void
moko_details_window_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_details_window_parent_class)->finalize (object);
}

static void
moko_details_window_class_init (MokoDetailsWindowClass *klass)
{
    /* hook parent */
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoDetailsWindowPrivate));

    /* hook virtual methods */
    /* ... */

    /* install properties */
    /* ... */

    object_class->dispose = moko_details_window_dispose;
    object_class->finalize = moko_details_window_finalize;
}

static void
moko_details_window_init (MokoDetailsWindow *self)
{
    moko_debug( "moko_details_window_init" );
    gtk_scrolled_window_set_policy( self, GTK_POLICY_NEVER, GTK_POLICY_NEVER );
}

MokoDetailsWindow*
moko_details_window_new (void)
{
    return g_object_new(MOKO_TYPE_DETAILS_WINDOW, NULL );
}

GtkBox* moko_details_window_put_in_box(MokoDetailsWindow* self)
{
    moko_debug( "moko_details_window_put_in_box" );
    //FIXME perfect place to add portrait/landscape switch
    GtkHBox* hbox = gtk_hbox_new( FALSE, 0 );
    GtkVBox* vbox = gtk_vbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(self), TRUE, TRUE, 0 );
    MokoPixmapButton* fullscreen = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(fullscreen), "mokodetailswindow-fullscreen-button-on" );
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(fullscreen), FALSE, FALSE, 0 );
    GtkVScrollbar* bar = gtk_vscrollbar_new( gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(self) ) );
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(bar), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(vbox), FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT(fullscreen), "clicked", G_CALLBACK(_cb_fullscreen_clicked), self );
    return hbox;
}

static void _cb_fullscreen_clicked(GtkButton* button, MokoDetailsWindow* self)
{
    g_debug( "openmoko-paned-demo: fullscreen clicked" );
    static gboolean on = TRUE;
    if ( on )
    {
        gtk_widget_set_name( GTK_WIDGET(button), "mokodetailswindow-fullscreen-button" );
        gtk_widget_queue_draw( GTK_WIDGET(button) ); //FIXME necessary?
    }
    else
    {
        gtk_widget_set_name( GTK_WIDGET(button), "mokodetailswindow-fullscreen-button-on" );
        gtk_widget_queue_draw( GTK_WIDGET(button) ); //FIXME necessary?
    }
    MokoPanedWindow* mainwindow = moko_application_get_main_window( moko_application_get_instance() );
    g_return_if_fail( MOKO_IS_PANED_WINDOW(mainwindow) );
    moko_paned_window_set_fullscreen( mainwindow, on );
    on = !on;
}
