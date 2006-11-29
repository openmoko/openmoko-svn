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

#include "moko-menu-box.h"

#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoFingerWindow, moko_finger_window, MOKO_TYPE_WINDOW)

#define MOKO_FINGER_WINDOW_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WINDOW, MokoFingerWindowPriv))

typedef struct _MokoFingerWindowPriv
{
    GtkVBox* vbox;
    GtkHBox* hbox;
    GtkLabel* label;
    GtkButton* scroller;
    MokoMenuBox* menubox;
} MokoFingerWindowPriv;

static void moko_finger_window_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose (object);
}

static void moko_finger_window_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_finger_window_parent_class)->finalize (object);
}

static void moko_finger_window_class_init (MokoFingerWindowClass *klass)
{
    moko_debug( "moko_finger_window_class_init" );
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (MokoFingerWindowPriv));

    object_class->dispose = moko_finger_window_dispose;
    object_class->finalize = moko_finger_window_finalize;
}

static void moko_finger_window_init (MokoFingerWindow *self)
{
    moko_debug( "moko_finger_window_init" );
    MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);
    priv->vbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->vbox) );
}

GtkWidget* moko_finger_window_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_finger_window_get_type(), NULL));
}

void moko_finger_window_set_application_menu(MokoFingerWindow* self, GtkMenu* menu)
{
    moko_debug( "moko_finger_window_set_application_menu" );
    MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);
    if (!priv->menubox )
    {
        priv->menubox = moko_menu_box_new();
        gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->menubox), FALSE, FALSE, 0 );
    }
    moko_menu_box_set_application_menu( priv->menubox, menu );
}

void moko_finger_window_set_contents(MokoFingerWindow* self, GtkWidget* child)
{
    moko_debug( "moko_finger_window_set_contents" );
    MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);

    gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(child), TRUE, TRUE, 0 );
}

void moko_finger_window_show_wheel(MokoFingerWindow* self)
{
}
void moko_finger_window_hide_wheel(MokoFingerWindow* self)
{
}
void moko_finger_window_show_tools(MokoFingerWindow* self)
{
}
void moko_finger_window_hide_tools(MokoFingerWindow* self)
{
}
