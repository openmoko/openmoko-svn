/*  moko-dialog-window.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/12/06 23:15:19 $) [$Author: mickey $]
 */

#include "moko-dialog-window.h"

#include <gtk/gtkeventbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoDialogWindow, moko_dialog_window, MOKO_TYPE_WINDOW)

#define MOKO_DIALOG_WINDOW_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DIALOG_WINDOW, MokoDialogWindowPrivate))

typedef struct _MokoDialogWindowPrivate MokoDialogWindowPrivate;

struct _MokoDialogWindowPrivate
{
    GtkVBox* vbox;
    GtkEventBox* eventbox;
    GtkLabel* label;
};

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

moko_dialog_window_class_init(MokoDialogWindowClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private (klass, sizeof(MokoDialogWindowPrivate));

    object_class->dispose = moko_dialog_window_dispose;
    object_class->finalize = moko_dialog_window_finalize;
}

MokoDialogWindow*
moko_dialog_window_new(void)
{
    return g_object_new(MOKO_TYPE_DIALOG_WINDOW, NULL);
}

static void
moko_dialog_window_init(MokoDialogWindow* self)
{
    moko_debug( "moko_dialog_window_init" );
    MokoWindow* parent = moko_application_get_main_window( moko_application_get_instance() );
    if ( parent )
    {
        gtk_window_set_transient_for( GTK_WINDOW(self), parent );
        gtk_window_set_modal( GTK_WINDOW(self), TRUE );
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
        gtk_widget_set_name( GTK_WIDGET(priv->label), "mokodialogwindow-title-label" );
        priv->eventbox = gtk_event_box_new();
        gtk_container_add( GTK_CONTAINER(priv->eventbox), GTK_WIDGET(priv->label) );
        gtk_widget_set_name( GTK_WIDGET(priv->eventbox), "mokodialogwindow-title-labelbox" );
        //FIXME get from theme
        gtk_misc_set_padding( GTK_MISC(priv->label), 0, 6 );
    }
    else
    {
        gtk_label_set_text( priv->label, title );
        gtk_window_set_title( GTK_WINDOW(self), title );
    }
    if ( !priv->vbox )
    {
        priv->vbox = gtk_vbox_new( FALSE, 0 );
        gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->eventbox), FALSE, FALSE, 0 );
        gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->vbox) );
    }
}
