/*  moko-dialog-window.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date: 2006/12/06 23:15:19 $) [$Author: mickey $]
 */

#include "moko-dialog-window.h"
#include "moko-pixmap-button.h"
#include "moko-application.h"

#include <gtk/gtkeventbox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>

#include <glib/gmain.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(...)
#endif

G_DEFINE_TYPE (MokoDialogWindow, moko_dialog_window, MOKO_TYPE_WINDOW)

#define MOKO_DIALOG_WINDOW_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DIALOG_WINDOW, MokoDialogWindowPrivate))

typedef struct _MokoDialogWindowPrivate MokoDialogWindowPrivate;

struct _MokoDialogWindowPrivate
{
    GtkWidget* vbox;        /* GtkVBox */
    GtkWidget* hbox;        /* GtkHBox */
    GtkWidget* eventbox;    /* GtkEventBox */
    GtkWidget* label;       /* GtkLabel */
    GtkWidget* closebutton; /* MokoPixmapButton */
};

typedef struct _MokoDialogRunInfo
{
    GtkWidget *dialog; /* MokoDialogWindow */
    gint response_id;
    GMainLoop *loop;
    gboolean destroyed;
} MokoDialogRunInfo;

static void moko_dialog_window_close(MokoDialogWindow* self);

static void
shutdown_loop (MokoDialogRunInfo *ri)
{
    if (g_main_loop_is_running (ri->loop))
        g_main_loop_quit (ri->loop);
}

static void
run_unmap_handler (MokoDialogWindow* dialog, gpointer data)
{
    MokoDialogRunInfo *ri = data;

    shutdown_loop (ri);
}

static void
run_response_handler (MokoDialogWindow* dialog,
                              gint response_id,
                              gpointer data)
{
    MokoDialogRunInfo *ri;

    ri = data;

    ri->response_id = response_id;

    shutdown_loop (ri);
}

static gint
        run_delete_handler (MokoDialogWindow* dialog,
                            GdkEventAny *event,
                            gpointer data)
{
    MokoDialogRunInfo *ri = data;

    shutdown_loop (ri);

    return TRUE; /* Do not destroy */
}

static void
        run_destroy_handler (MokoDialogWindow* dialog, gpointer data)
{
    MokoDialogRunInfo *ri = data;

    /* shutdown_loop will be called by run_unmap_handler */

    ri->destroyed = TRUE;
}


static void
moko_dialog_window_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_dialog_window_parent_class)->dispose)
        G_OBJECT_CLASS (moko_dialog_window_parent_class)->dispose (object);
}

static void
moko_dialog_window_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_dialog_window_parent_class)->finalize (object);
}

static void
moko_dialog_window_class_init(MokoDialogWindowClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private (klass, sizeof(MokoDialogWindowPrivate));

    object_class->dispose = moko_dialog_window_dispose;
    object_class->finalize = moko_dialog_window_finalize;
}

GtkWidget*
moko_dialog_window_new(void)
{
    return GTK_WIDGET (g_object_new(MOKO_TYPE_DIALOG_WINDOW, NULL));
}

static void
moko_dialog_window_init(MokoDialogWindow* self)
{
    moko_debug( "moko_dialog_window_init" );
    GtkWidget* parent = moko_application_get_main_window( moko_application_get_instance() );
    if ( parent )
    {
        gtk_window_set_transient_for( GTK_WINDOW(self), GTK_WINDOW (parent) );
#ifndef DEBUG_THIS_FILE
        gtk_window_set_modal( GTK_WINDOW(self), TRUE );
#endif
        gtk_window_set_destroy_with_parent( GTK_WINDOW(self), TRUE );
    }
}

void moko_dialog_window_set_title(MokoDialogWindow* self, const gchar* title)
{
    moko_debug( "moko_dialog_window_set_title" );
    MokoDialogWindowPrivate* priv = MOKO_DIALOG_WINDOW_GET_PRIVATE(self);
    if ( !priv->label )
    {
        priv->label = gtk_label_new( title );
        gtk_window_set_title( GTK_WINDOW(self), title );
        gtk_widget_set_name( priv->label, "mokodialogwindow-title-label" );
        priv->hbox = gtk_hbox_new( FALSE, 0 );
        priv->eventbox = gtk_event_box_new();
        gtk_box_pack_start( GTK_BOX(priv->hbox), priv->eventbox, TRUE, TRUE, 0 );
        priv->closebutton = moko_pixmap_button_new();
        gtk_widget_set_name( priv->closebutton, "mokodialogwindow-closebutton" );
        g_signal_connect_swapped( G_OBJECT(priv->closebutton), "clicked", G_CALLBACK(moko_dialog_window_close), self );
        gtk_box_pack_start( GTK_BOX(priv->hbox), priv->closebutton, FALSE, FALSE, 0 );
        gtk_container_add( GTK_CONTAINER(priv->eventbox), priv->label );
        gtk_widget_set_name( priv->eventbox, "mokodialogwindow-title-labelbox" );
        //FIXME get from theme
        gtk_misc_set_padding( GTK_MISC(priv->label), 0, 6 );
        gtk_widget_show( priv->hbox );
        gtk_widget_show( priv->label );
        gtk_widget_show( priv->closebutton );
        gtk_widget_show( priv->eventbox );
    }
    else
    {
        gtk_label_set_text( GTK_LABEL (priv->label), title );
        gtk_window_set_title( GTK_WINDOW(self), title );
    }
    if ( !priv->vbox )
    {
        priv->vbox = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start( GTK_BOX(priv->vbox), priv->hbox, FALSE, FALSE, 0 );
        gtk_container_add( GTK_CONTAINER(self), priv->vbox );
        gtk_widget_show( priv->vbox );
    }
}

void moko_dialog_window_set_contents(MokoDialogWindow* self, GtkWidget* contents)
{
    moko_debug( "moko_dialog_window_set_contents" );
    MokoDialogWindowPrivate* priv = MOKO_DIALOG_WINDOW_GET_PRIVATE(self);
    g_return_if_fail( priv->vbox );
    gtk_box_pack_start( GTK_BOX(priv->vbox), contents, TRUE, TRUE, 0 );
}

static void moko_dialog_window_close(MokoDialogWindow* self)
{
    /* Synthesize delete_event to close dialog. */

    GtkWidget *widget = GTK_WIDGET(self);
    GdkEvent *event;

    event = gdk_event_new( GDK_DELETE );

    event->any.window = g_object_ref(widget->window);
    event->any.send_event = TRUE;

    gtk_main_do_event( event );
    gdk_event_free( event );
}

guint moko_dialog_window_run(MokoDialogWindow* dialog)
{
    moko_debug( "moko_dialog_window_run" );

    MokoDialogRunInfo ri = { NULL, GTK_RESPONSE_NONE, NULL, FALSE };
    gboolean was_modal;
    gulong response_handler;
    gulong unmap_handler;
    gulong destroy_handler;
    gulong delete_handler;

    g_return_val_if_fail (MOKO_IS_DIALOG_WINDOW(dialog), -1);

    g_object_ref (dialog);

    was_modal = GTK_WINDOW (dialog)->modal;
#ifndef DEBUG_THIS_FILE
    if (!was_modal)
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
#endif

    if (!GTK_WIDGET_VISIBLE (dialog))
        gtk_widget_show (GTK_WIDGET (dialog));

    response_handler =
            g_signal_connect (dialog,
                              "response",
                              G_CALLBACK (run_response_handler),
                              &ri);

    unmap_handler =
            g_signal_connect (dialog,
                              "unmap",
                              G_CALLBACK (run_unmap_handler),
                              &ri);

    delete_handler =
            g_signal_connect (dialog,
                              "delete-event",
                              G_CALLBACK (run_delete_handler),
                              &ri);

    destroy_handler =
            g_signal_connect (dialog,
                              "destroy",
                              G_CALLBACK (run_destroy_handler),
                              &ri);

    ri.loop = g_main_loop_new (NULL, FALSE);

    GDK_THREADS_LEAVE ();
    g_main_loop_run (ri.loop);
    GDK_THREADS_ENTER ();

    g_main_loop_unref (ri.loop);
    ri.loop = NULL;

    if (!ri.destroyed)
    {
        if (!was_modal)
            gtk_window_set_modal (GTK_WINDOW(dialog), FALSE);

        g_signal_handler_disconnect (dialog, response_handler);
        g_signal_handler_disconnect (dialog, unmap_handler);
        g_signal_handler_disconnect (dialog, delete_handler);
        g_signal_handler_disconnect (dialog, destroy_handler);
    }

    g_object_unref (dialog);

    return ri.response_id;
}
