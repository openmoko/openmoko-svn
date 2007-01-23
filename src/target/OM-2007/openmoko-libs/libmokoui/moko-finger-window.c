/*  moko-finger-window.c
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-finger-window.h"

#include "moko-menu-box.h"

#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>

//#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#undef DEBUG_THIS_FILE
#endif

#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
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
    MokoMenuBox* menubox;
    MokoFingerWheel* wheel;
    MokoFingerToolBox* tools;
} MokoFingerWindowPriv;

static void moko_finger_window_dispose(GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose (object);
}

static void moko_finger_window_finalize(GObject *object)
{
    G_OBJECT_CLASS (moko_finger_window_parent_class)->finalize (object);
}

static void moko_finger_window_class_init(MokoFingerWindowClass *klass)
{
    moko_debug( "moko_finger_window_class_init" );
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (MokoFingerWindowPriv));

    object_class->dispose = moko_finger_window_dispose;
    object_class->finalize = moko_finger_window_finalize;
}

static void moko_finger_window_init(MokoFingerWindow *self)
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

MokoFingerWheel* moko_finger_window_get_wheel(MokoFingerWindow* self)
{
    moko_debug( "moko_finger_window_get_wheel" );
    MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);
    if (!priv->wheel) priv->wheel = moko_finger_wheel_new(self);
    return priv->wheel;
}

MokoFingerToolBox* moko_finger_window_get_toolbox(MokoFingerWindow* self)
{
    moko_debug( "moko_finger_window_get_toolbox" );
    MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);
    if (!priv->tools) priv->tools = moko_finger_tool_box_new(self);
    return priv->tools;
}

gboolean moko_finger_window_get_geometry_hint(MokoFingerWindow* self, GtkWidget* hintee, GtkAllocation* allocation)
{
    //FIXME get geometry hints from theme
    moko_debug( "moko_finger_window_geometry_hint" );

    GtkRequisition req;
    gtk_widget_size_request( hintee, &req );
    int x, y, w, h;
    gdk_window_get_geometry( GTK_WIDGET(self)->window, &x, &y, &w, &h, NULL );
    int absx;
    int absy;
    gdk_window_get_origin( GTK_WIDGET(self)->window, &absx, &absy );

    moko_debug( "hintee requisition is %d, %d", req.width, req.height );
    moko_debug( "finger window geometry is %d, %d * %d, %d", x, y, w, h );

    if ( MOKO_IS_FINGER_WHEEL(hintee) )
    {
        allocation->x = absx;
        allocation->y = absy + h - req.height;
        allocation->width = w;
        allocation->height = h;
        return TRUE;
    }
    else if ( MOKO_IS_FINGER_TOOL_BOX(hintee) )
    {
        MokoFingerWindowPriv* priv = MOKO_FINGER_WINDOW_PRIVATE(self);
        if ( priv->wheel && GTK_WIDGET_VISIBLE(priv->wheel) )
        {
            moko_debug( "-- wheel is visible" );
            GtkAllocation* wheelalloc = &(GTK_WIDGET(priv->wheel)->allocation);
            moko_debug( "-- wheel alloc is %d, %d, %d, %d", wheelalloc->x, wheelalloc->y, wheelalloc->width, wheelalloc->height );
            //FIXME get from theme: 22 is the overlap factor for wheel + toolbox
#define WHEEL_TOOL_BOX_OVERLAP 22
#define TOOL_BOX_HEIGHT 104
            allocation->x = absx + wheelalloc->x + wheelalloc->width - WHEEL_TOOL_BOX_OVERLAP;
            allocation->y = absy + h - req.height;
            //allocation->width = w - allocation->x;
            allocation->width = w - wheelalloc->width + WHEEL_TOOL_BOX_OVERLAP;
            //FIXME compute
            allocation->height = TOOL_BOX_HEIGHT;
            return TRUE;
        }
        else
        {
            moko_debug( "-- wheel not visible" );
            allocation->x = absx;
            allocation->y = absy + h - req.height;
            allocation->width = w;
            //FIXME compute
            allocation->height = TOOL_BOX_HEIGHT;
        }
        return TRUE;
    }
    return FALSE;
}
