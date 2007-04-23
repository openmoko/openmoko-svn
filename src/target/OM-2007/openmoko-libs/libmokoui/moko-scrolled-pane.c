/*  moko-scrolled-pane.c
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
 *  Current Version: $Rev$ ($Date: 2007/04/13 13:56:22 $) [$Author: mickey $]
 */

#include "moko-scrolled-pane.h"
#include "moko-pixmap-button.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoScrolledPane, moko_scrolled_pane, GTK_TYPE_HBOX)

#define SCROLLED_PANE_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_SCROLLED_PANE, MokoScrolledPanePrivate))

typedef struct _MokoScrolledPanePrivate
{
    GtkWidget* scrolledwindow; /* GtkScrolledWindow */
    GtkWidget* vbox; /* GtkVBox */
    GtkWidget* button; /* MokoPixmapButton */
    GtkWidget* scrollbar; /* GtkScrollBar */
} MokoScrolledPanePrivate;

/* parent class pointer */
static GtkHBoxClass* parent_class = NULL;

/* signals */
enum {
    FULLSCREEN_TOGGLED,
    LAST_SIGNAL,
};
static guint moko_scrolled_pane_signals[LAST_SIGNAL] = { 0 };

/* forward declarations */
static void _moko_scrolled_pane_fullscreen_clicked (GtkButton* button, MokoScrolledPane* self);

static void
moko_scrolled_pane_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_scrolled_pane_parent_class)->dispose)
        G_OBJECT_CLASS (moko_scrolled_pane_parent_class)->dispose (object);
}

static void
moko_scrolled_pane_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_scrolled_pane_parent_class)->finalize (object);
}

static void
moko_scrolled_pane_class_init(MokoScrolledPaneClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoScrolledPanePrivate));

    /* hook destruction */
    object_class->dispose = moko_scrolled_pane_dispose;
    object_class->finalize = moko_scrolled_pane_finalize;

    /* register signals */
    moko_scrolled_pane_signals[FULLSCREEN_TOGGLED] = g_signal_new ("fullscreen-toggled",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
            NULL,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__INT,
            G_TYPE_NONE,
            1,
            G_TYPE_INT,
            NULL);

    /* virtual methods */

    /* install properties */
}

GtkWidget*
moko_scrolled_pane_new(void)
{
    return g_object_new(MOKO_TYPE_SCROLLED_PANE, NULL);
}

static void
moko_scrolled_pane_init(MokoScrolledPane* self)
{
    /* Populate your instance here */
    moko_debug ("moko_scrolled_pane_init");

    MokoScrolledPanePrivate* priv = SCROLLED_PANE_GET_PRIVATE(self);
    priv->scrolledwindow = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( priv->scrolledwindow, GTK_POLICY_NEVER, GTK_POLICY_NEVER );

    priv->vbox = gtk_vbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(self), priv->scrolledwindow, TRUE, TRUE, 0 );
    priv->button = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(priv->button), "mokoscrolledpane-fullscreen-button-on" );
    gtk_box_pack_start( GTK_BOX(priv->vbox), priv->button, FALSE, FALSE, 0);
    priv->scrollbar = gtk_vscrollbar_new( gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(priv->scrolledwindow)));
    gtk_box_pack_start( GTK_BOX(priv->vbox), priv->scrollbar, TRUE, TRUE, 0);
    gtk_box_pack_start( GTK_BOX(self), priv->vbox, FALSE, FALSE, 0);
    g_signal_connect( G_OBJECT(priv->button), "clicked", G_CALLBACK(_moko_scrolled_pane_fullscreen_clicked), self );

}

static void _moko_scrolled_pane_fullscreen_clicked(GtkButton * button, MokoScrolledPane * self)
{
    static gboolean on = TRUE;
    if (on)
    {
        gtk_widget_set_name (GTK_WIDGET (button),
                             "mokoscrolledpane-fullscreen-button");
        gtk_widget_queue_draw (GTK_WIDGET (button));    //FIXME necessary?
    }
    else
    {
        gtk_widget_set_name (GTK_WIDGET (button),
                             "mokoscrolledpane-fullscreen-button-on");
        gtk_widget_queue_draw (GTK_WIDGET (button));    //FIXME necessary?
    }

    g_signal_emit( G_OBJECT(self), moko_scrolled_pane_signals[FULLSCREEN_TOGGLED], 0, on );
    on = !on;
}

GtkScrolledWindow* moko_scrolled_pane_get_scrolled_window(MokoScrolledPane* self)
{
    MokoScrolledPanePrivate* priv = SCROLLED_PANE_GET_PRIVATE(self);
    g_assert( priv->scrolledwindow );
    return priv->scrolledwindow;
}
