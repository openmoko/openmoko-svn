/*  moko-finger-wheel.c
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

#include "moko-application.h"
#include "moko-finger-wheel.h"

#include <gtk/gtkbutton.h>
#include <gtk/gtkwindow.h>

G_DEFINE_TYPE (MokoFingerWheel, moko_finger_wheel, MOKO_TYPE_FIXED)

#define MOKO_FINGER_WHEEL_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WHEEL, MokoFingerWheelPrivate))

static MokoFixedClass *parent_class = NULL;

typedef struct _MokoFingerWheelPrivate
{
    GtkWindow* popup;
} MokoFingerWheelPrivate;

/* forward declarations */
static void moko_finger_wheel_show (GtkWidget* widget);
static void moko_finger_wheel_hide (GtkWidget* widget);

static void
moko_finger_wheel_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_wheel_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_wheel_parent_class)->dispose (object);
}

static void
moko_finger_wheel_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_finger_wheel_parent_class)->finalize (object);
}

static void
moko_finger_wheel_class_init (MokoFingerWheelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoFingerWheelPrivate));

    /* hook virtual methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->show = moko_finger_wheel_show;
    widget_class->hide = moko_finger_wheel_hide;
    //widget_class->show_all = moko_finger_wheel_show;
    //widget_class->hide_all = moko_finger_wheel_hide;

    /* install properties */
    /* ... */

    object_class->dispose = moko_finger_wheel_dispose;
    object_class->finalize = moko_finger_wheel_finalize;
}

static void
moko_finger_wheel_init (MokoFingerWheel *self)
{
    gtk_widget_set_name( GTK_WIDGET(self), "mokofingerwheel" );
}

GtkWidget*
moko_finger_wheel_new (void)
{
    return GTK_WIDGET(g_object_new(moko_finger_wheel_get_type(), NULL));
}

static void moko_finger_wheel_show (GtkWidget* widget)
{
    gtk_widget_ensure_style( widget ); //FIXME needed here?
    g_debug( "moko_finger_wheel_show" );
    GTK_WIDGET_CLASS(parent_class)->show(widget);
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(widget);
    if ( !priv->popup )
    {
        priv->popup = gtk_window_new(GTK_WINDOW_POPUP);
        gtk_widget_set_name( GTK_WIDGET(priv->popup), "transparent" );
        gtk_container_add( GTK_CONTAINER(priv->popup), widget );
        MokoWindow* window = moko_application_get_main_window( moko_application_get_instance() );
        GtkRequisition req;
        gtk_widget_size_request( widget, &req );
        //g_debug( "My requisition is %d, %d", req.width, req.height );
        int x, y, w, h;
        gdk_window_get_geometry( GTK_WIDGET(window)->window, &x, &y, &w, &h, NULL );
        //g_debug( "WINDOW geometry is %d, %d * %d, %d", x, y, w, h );
        int absx;
        int absy;
        gdk_window_get_origin( GTK_WIDGET(window)->window, &absx, &absy );
        GtkAllocation* alloc = &GTK_WIDGET(window)->allocation;
        //g_debug( "WINDOW allocation is %d, %d * %d, %d", alloc->x, alloc->y, alloc->width, alloc->height );
        gtk_window_move( priv->popup, absx, absy + h - req.height );

        //FIXME Isn't there a way to get this as a mask directly from the style without having to reload it?
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(GTK_WIDGET(widget)->style->rc_style->bg_pixmap_name[GTK_STATE_NORMAL], NULL);
        GdkPixmap* pixmap;
        GdkBitmap* mask;
        gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &mask, 128);
        g_object_unref(G_OBJECT(pixbuf));

        //GdkPixmap* mask = GTK_WIDGET(widget)->style->bg_pixmap[GTK_STATE_NORMAL];
        gtk_widget_shape_combine_mask(priv->popup, mask, 0, 0);
    }
    gtk_widget_show( priv->popup );
}

static void moko_finger_wheel_hide (GtkWidget* widget)
{
    g_debug( "moko_finger_wheel_hide" );
    GTK_WIDGET_CLASS(parent_class)->hide(widget);
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(widget);
    gtk_widget_hide( priv->popup );
}

