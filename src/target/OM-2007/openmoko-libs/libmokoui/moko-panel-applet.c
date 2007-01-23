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

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoPanelApplet, moko_panel_applet, GTK_TYPE_ALIGNMENT)

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

/* forward declarations */
static gboolean cb_moko_panel_applet_button_release_event( GtkWidget* widget, GdkEventButton* event, MokoPanelApplet* self);
void moko_panel_applet_signal_clicked(MokoPanelApplet* self);
void moko_panel_applet_signal_tap_hold(MokoPanelApplet* self);

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

    /* default signal handlers */
    klass->clicked = moko_panel_applet_signal_clicked;
    klass->tap_hold = moko_panel_applet_signal_tap_hold;

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoPanelAppletPrivate));

    /* hook destruction */
    object_class->dispose = moko_panel_applet_dispose;
    object_class->finalize = moko_panel_applet_finalize;

    /* install properties */

    /* install signals */
    moko_panel_applet_signals[CLICKED] = g_signal_new("clicked",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoPanelAppletClass, clicked),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    moko_panel_applet_signals[TAP_HOLD] = g_signal_new("tap-hold",
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
    return g_object_new(MOKO_TYPE_PANEL_APPLET, NULL);
}

static void
moko_panel_applet_init(MokoPanelApplet* self)
{
    moko_debug( "moko_panel_applet_init" );

    MokoPanelAppletClass* klass = MOKO_PANEL_APPLET_GET_CLASS(self);
    MokoPanelAppletPrivate* priv = MOKO_PANEL_APPLET_GET_PRIVATE( self );
    priv->hold_timeout_triggered = FALSE;

    self->eventbox = gtk_event_box_new();
    gtk_event_box_set_visible_window( self->eventbox, FALSE );
    gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(self->eventbox) );
    gtk_widget_show( self->eventbox );

    g_signal_connect( G_OBJECT(self->eventbox), "button-release-event", cb_moko_panel_applet_button_release_event, self );
}

static gboolean _moko_panel_applet_window_clicked(GtkWidget* widget, GdkEventButton* event, MokoPanelApplet* self)
{
    gdk_pointer_ungrab( event->time );
    gtk_widget_hide( self->toplevelwindow );
}

void moko_panel_applet_get_positioning_hint(MokoPanelApplet* self, GtkWidget* popup, int* x, int* y)
{
    int win_w;
    int win_h;
    gdk_window_get_geometry( GTK_WIDGET(self->toplevelwindow)->window, NULL, NULL, &win_w, &win_h, NULL );
    moko_debug( "-- popup geom = %d, %d", win_w, win_h );
    GtkAllocation* allocation = &GTK_WIDGET(self->toplevelwindow)->allocation;
    moko_debug( "-- popup alloc = %d, %d", allocation->width, allocation->height );

    GtkAllocation* applet_alloc = &GTK_WIDGET(self)->allocation;
    moko_debug( "-- applet alloc = %d, %d", applet_alloc->width, applet_alloc->height );

    int x_abs;
    int y_abs;

    //FIXME this doesn't work w/ matchbox-panel 2 yet
    gdk_window_get_root_origin( GTK_WIDGET(self->eventbox)->window, &x_abs, &y_abs );

    moko_debug( "-- abs position = %d, %d", x_abs, y_abs );

    *x = x_abs;
    *y = y_abs + applet_alloc->height + 4;

    if ( *x + win_w > gdk_screen_width() )
            *x = gdk_screen_width() - win_w - 2;
    if ( *y + win_h > gdk_screen_height() )
            *y = gdk_screen_height - win_h - applet_alloc->height - 2;

    moko_debug( "-- final position = %d, %d", *x, *y );
}

gboolean cb_moko_panel_applet_button_release_event( GtkWidget* widget, GdkEventButton* event, MokoPanelApplet* self)
{
    moko_debug( "cb_moko_panel_applet_button_release_event" );
    if ( event->button == 1 )
        g_signal_emit( G_OBJECT(self), moko_panel_applet_signals[CLICKED], 0, NULL );
    else if ( event->button == 3 )
        g_signal_emit( G_OBJECT(self), moko_panel_applet_signals[TAP_HOLD], 0, NULL );
    return TRUE;
}

void moko_panel_applet_signal_clicked(MokoPanelApplet* self)
{
    moko_debug( __FUNCTION__ );
    if ( self->toplevelwindow && GTK_WIDGET_VISIBLE( self->toplevelwindow ) )
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
    if ( !self->icon )
    {
        self->icon = gtk_image_new_from_file( filename );
        g_return_if_fail( self->icon );
        gtk_container_add( GTK_CONTAINER(self->eventbox), GTK_WIDGET(self->icon) );
        gtk_widget_show( GTK_WIDGET(self->icon) );
    }
    else
        gtk_image_set_from_file( self->icon, filename );
}

void moko_panel_applet_set_pixbuf(MokoPanelApplet* self, GdkPixbuf* pixbuf)
{
    if ( !self->icon )
    {
        self->icon = gtk_image_new_from_pixbuf( pixbuf );
        g_return_if_fail( self->icon );
        gtk_container_add( GTK_CONTAINER(self->eventbox), GTK_WIDGET(self->icon) );
        gtk_widget_show( GTK_WIDGET(self->icon) );
    }
    else
        gtk_image_set_from_pixbuf( self->icon, pixbuf );
}

void moko_panel_applet_set_widget(MokoPanelApplet* self, GtkWidget* widget)
{
    gtk_container_add( GTK_CONTAINER(self->eventbox), GTK_WIDGET(widget) );
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

    if ( self->toplevelwindow && GTK_WIDGET_VISIBLE( self->toplevelwindow ) )
        moko_panel_applet_close_popup( self );

    if ( GTK_IS_MENU(self->popup[type]) )
    {
        self->toplevelwindow = self->popup[type];
        gtk_menu_popup( GTK_MENU( self->toplevelwindow ), NULL, NULL, NULL, NULL, 0, CurrentTime );
    }
    else
    {
        self->toplevelwindow = gtk_window_new( GTK_WINDOW_POPUP );
        gtk_container_add( GTK_CONTAINER(self->toplevelwindow), self->popup[type] );
        g_signal_connect( G_OBJECT(self->toplevelwindow), "button-press-event", G_CALLBACK(_moko_panel_applet_window_clicked), self );
        gtk_widget_add_events( self->toplevelwindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
        gtk_widget_realize( self->toplevelwindow );
        gtk_widget_show_all( self->toplevelwindow );
        int x = 0;
        int y = 0;
        moko_panel_applet_get_positioning_hint( self, self->popup, &x, &y );
        gtk_window_move( self->toplevelwindow, x, y );
        gdk_pointer_grab( GTK_WIDGET(self->toplevelwindow)->window, TRUE, GDK_BUTTON_PRESS_MASK, NULL, NULL, CurrentTime );
    }
}

void moko_panel_applet_close_popup(MokoPanelApplet* self)
{
    g_return_if_fail( self->toplevelwindow || !GTK_WIDGET_VISIBLE(self->toplevelwindow) );
    moko_debug( "moko_panel_applet_close_popup" );

    if ( GTK_IS_MENU( self->toplevelwindow ) )
    {
        gtk_menu_popdown( GTK_MENU( self->toplevelwindow) );
    }
    else
    {
        gdk_pointer_ungrab( CurrentTime );
        gtk_widget_hide( self->toplevelwindow );
        gtk_container_remove( GTK_CONTAINER(self->toplevelwindow), gtk_bin_get_child( GTK_BIN(self->toplevelwindow) ) );
        gtk_widget_destroy( self->toplevelwindow );
        self->toplevelwindow = 0L;
    }
}
