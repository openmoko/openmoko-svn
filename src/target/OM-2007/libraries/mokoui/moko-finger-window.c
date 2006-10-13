/*  moko-finger-window.c
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "moko-finger-window.h"
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>

G_DEFINE_TYPE (MokoFingerWindow, moko_finger_window, MOKO_TYPE_WINDOW);

#define FINGER_WINDOW_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WINDOW, MokoFingerWindowPrivate))

typedef struct _MokoFingerWindowPrivate MokoFingerWindowPrivate;

struct _MokoFingerWindowPrivate
{
    GtkVBox* vbox;
    GtkHBox* hbox;
    GtkLabel* label;
    GtkButton* scroller;
};

static void
moko_finger_window_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose)
    G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose (object);
}

static void
moko_finger_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_finger_window_parent_class)->finalize (object);
}

static void
moko_finger_window_class_init (MokoFingerWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoFingerWindowPrivate));

  object_class->dispose = moko_finger_window_dispose;
  object_class->finalize = moko_finger_window_finalize;
}

static void
moko_finger_window_init (MokoFingerWindow *self)
{
    g_debug( "moko_finger_window_init" );

    MokoFingerWindowPrivate* priv = FINGER_WINDOW_PRIVATE(self);
    priv->vbox = gtk_vbox_new( FALSE, 0 );
    priv->hbox = gtk_hbox_new( FALSE, 0 );
    priv->scroller = gtk_button_new_with_label( "Hello" );
    priv->label = gtk_label_new( "Yo Yo" );
    gtk_box_pack_end( GTK_BOX(priv->vbox), GTK_WIDGET(priv->hbox), FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(priv->hbox), GTK_WIDGET(priv->scroller), FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(priv->hbox), GTK_WIDGET(priv->label), FALSE, FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->vbox) );
}

MokoFingerWindow*
moko_finger_window_new (void)
{
  return g_object_new (MOKO_TYPE_FINGER_WINDOW, NULL);
}

static void
moko_finger_window_set_contents (MokoFingerWindow* self, GtkWidget* child )
{
    g_debug( "moko_finger_window_init" );
    MokoFingerWindowPrivate* priv = FINGER_WINDOW_PRIVATE(self);

    gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(child), TRUE, TRUE, 0 );
}
