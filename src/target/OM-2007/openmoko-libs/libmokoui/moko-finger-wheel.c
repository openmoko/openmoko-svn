/*  moko-finger-wheel.c
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-finger-wheel.h"

#include "moko-application.h"
#include "moko-finger-tool-box.h"
#include "moko-finger-window.h"

#include <gtk/gtkbutton.h>
#include <gtk/gtkwindow.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

enum {
    PRESS_LEFT_UP,
    PRESS_RIGHT_DOWN,
    PRESS_BOTTOM,
    LONG_PRESS_LEFT_UP,
    LONG_PRESS_RIGHT_DOWN,
    LAST_SIGNAL
};

#define FINGER_WHEEL_LONG_PRESS_TIMEOUT 1000

G_DEFINE_TYPE (MokoFingerWheel, moko_finger_wheel, MOKO_TYPE_FIXED)

#define MOKO_FINGER_WHEEL_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WHEEL, MokoFingerWheelPrivate))

static MokoFixedClass *parent_class = NULL;
static guint wheel_signals[LAST_SIGNAL] = { 0 };

typedef struct _MokoFingerWheelPrivate
{
    GtkWindow* popup;
} MokoFingerWheelPrivate;

/* forward declarations */
static void moko_finger_wheel_realize(GtkWidget* widget);
static void moko_finger_wheel_show(GtkWidget* widget);
static void moko_finger_wheel_hide(GtkWidget* widget);
static gint moko_finger_wheel_button_press(GtkWidget* widget, GdkEventButton* event);
static gint moko_finger_wheel_motion_notify(GtkWidget* widget, GdkEventMotion* event);
static gint moko_finger_wheel_button_release(GtkWidget* widget, GdkEventButton* event);
static gboolean moko_finger_wheel_button_long_press(gpointer data);

static void
moko_finger_wheel_dispose(GObject *object)
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
moko_finger_wheel_class_init(MokoFingerWheelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoFingerWheelPrivate));

    /* hook virtual methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->realize = moko_finger_wheel_realize;
    widget_class->show = moko_finger_wheel_show;
    widget_class->hide = moko_finger_wheel_hide;
    widget_class->button_press_event = moko_finger_wheel_button_press;
    widget_class->motion_notify_event = moko_finger_wheel_motion_notify;
    widget_class->button_release_event = moko_finger_wheel_button_release;

    /* install properties */
    /* ... */

    object_class->dispose = moko_finger_wheel_dispose;
    object_class->finalize = moko_finger_wheel_finalize;

    /** Init the moko finger wheel signal to null */
    klass->press_left_up = NULL;
    klass->press_right_down = NULL;
    klass->press_bottom = NULL;
    klass->long_press_left_up = NULL;
    klass->long_press_right_down = NULL;

    /** Press the left up area */
    wheel_signals[PRESS_LEFT_UP] =
                 g_signal_new ("press_left_up",
                 G_OBJECT_CLASS_TYPE (object_class),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET (MokoFingerWheelClass, press_left_up),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

    /** Press the right down area */
    wheel_signals[PRESS_RIGHT_DOWN] =
                 g_signal_new ("press_right_down",
                 G_OBJECT_CLASS_TYPE (object_class),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET (MokoFingerWheelClass, press_right_down),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

    /** Press the bottom area */
    wheel_signals[PRESS_BOTTOM] =
                 g_signal_new ("press_bottom",
                 G_OBJECT_CLASS_TYPE (object_class),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET (MokoFingerWheelClass, press_bottom),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

    /** Long press the left up area */
    wheel_signals[LONG_PRESS_LEFT_UP] =
                 g_signal_new ("long_press_left_up",
                 G_OBJECT_CLASS_TYPE (object_class),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET (MokoFingerWheelClass, long_press_left_up),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

    /** Long press the right down area */
    wheel_signals[LONG_PRESS_RIGHT_DOWN] =
                 g_signal_new ("long_press_right_down",
                 G_OBJECT_CLASS_TYPE (object_class),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET (MokoFingerWheelClass, long_press_right_down),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

}

static void
moko_finger_wheel_init(MokoFingerWheel *self)
{
    self->area_id = LAST_SIGNAL;
    gtk_widget_set_name( GTK_WIDGET(self), "mokofingerwheel" );
}

GtkWidget*
moko_finger_wheel_new(void)
{
    return GTK_WIDGET(g_object_new(moko_finger_wheel_get_type(), NULL));
}

static void
moko_finger_wheel_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void moko_finger_wheel_show(GtkWidget* widget)
{
    gtk_widget_ensure_style( widget ); //FIXME needed here?
    moko_debug( "moko_finger_wheel_show" );
    GTK_WIDGET_CLASS(parent_class)->show(widget);
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(widget);
    if ( !priv->popup )
    {
        priv->popup = gtk_window_new(GTK_WINDOW_POPUP);
        //FIXME Setting it to transparent is probably not necessary since we issue a mask anyway, right?
        //gtk_widget_set_name( GTK_WIDGET(priv->popup), "transparent" );
        gtk_container_add( GTK_CONTAINER(priv->popup), widget );
        MokoWindow* window = moko_application_get_main_window( moko_application_get_instance() );
        //FIXME check if it's a finger window

        GtkAllocation geometry;
        gboolean valid = moko_finger_window_get_geometry_hint( window, widget, &geometry );
        gtk_window_move( GTK_WIDGET(priv->popup), geometry.x, geometry.y );

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

    /* resize FingerToolBox, if visible */
    MokoWindow* window = moko_application_get_main_window( moko_application_get_instance() );
    if ( MOKO_IS_FINGER_WINDOW(window) )
    {
        MokoFingerToolBox* toolbox = moko_finger_window_get_toolbox( MOKO_FINGER_WINDOW(window) );
        if ( GTK_WIDGET_VISIBLE(toolbox) )
        {
            moko_debug( "moko_finger_wheel: toolbox is visible, sending resize" );
            gtk_widget_hide( GTK_WIDGET(toolbox) );
            gtk_widget_show( GTK_WIDGET(toolbox) );
        }
        else
        {
            moko_debug( "moko_finger_wheel: toolbox not visible, doing nothing" );
        }
    }
    else
    {
        g_warning( "moko_finger_wheel: main window is not a finger window" );
    }
}

static void moko_finger_wheel_hide(GtkWidget* widget)
{
    moko_debug( "moko_finger_wheel_hide" );
    GTK_WIDGET_CLASS(parent_class)->hide(widget);
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(widget);
    gtk_widget_hide( priv->popup );

    /* resize FingerToolBox, if visible */
    MokoWindow* window = moko_application_get_main_window( moko_application_get_instance() );
    if ( MOKO_IS_FINGER_WINDOW(window) )
    {
        MokoFingerToolBox* toolbox = moko_finger_window_get_toolbox( MOKO_FINGER_WINDOW(window) );
        if ( GTK_WIDGET_VISIBLE(toolbox) )
        {
            moko_debug( "moko_finger_wheel: toolbox is visible, sending resize" );
            gtk_widget_hide( GTK_WIDGET(toolbox) );
            gtk_widget_show( GTK_WIDGET(toolbox) );
        }
        else
        {
            moko_debug( "moko_finger_wheel: toolbox not visible, doing nothing" );
        }
    }
    else
    {
        g_warning( "moko_finger_wheel: main window is not a finger window" );
    }
}

void moko_finger_wheel_raise(MokoFingerWheel* self)
{
    moko_debug( "moko_finger_wheel_raise" );
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(self);
    g_return_if_fail(priv->popup);
    gdk_window_raise( GTK_WIDGET(priv->popup)->window );
}

/**
 * @brief Caculate the area that user checked
 */
static void moko_finger_wheel_button_check_area (GtkWidget* widget, GdkEventButton* event)
{
    GtkRequisition req;

    g_return_if_fail (MOKO_IS_FINGER_WHEEL (widget));

    gtk_widget_size_request( widget, &req );
    moko_debug ("moko_finger_wheel_button_check_area");
    moko_debug ("The event x=%d, y=%d", (int)event->x, (int)event->y);
    moko_debug ("The req width=%d, height=%d", req.width, req.height);
    if ( ((int)event->x) < ((int)req.width/2))
    {
        if ( ((int)event->y) < ((int)req.height/2))
        {
            MOKO_FINGER_WHEEL (widget)->area_id = PRESS_LEFT_UP;
        }
        else
        {
            MOKO_FINGER_WHEEL (widget)->area_id = PRESS_BOTTOM;
        }
    }
    else
    {
        if ( ((int)event->y) < ((int)req.height/2))
        {
            MOKO_FINGER_WHEEL (widget)->area_id = LAST_SIGNAL;
        }
        else
        {
            MOKO_FINGER_WHEEL (widget)->area_id = PRESS_RIGHT_DOWN;
        }
    }
}

/**
 * @brief Emit the signal
 */
static void moko_finger_wheel_button_emit_signal (GtkWidget* widget, GdkEventButton* event)
{
    g_return_if_fail (MOKO_IS_FINGER_WHEEL (widget));

    if ((MOKO_FINGER_WHEEL (widget)->area_id <PRESS_LEFT_UP) ||
        (MOKO_FINGER_WHEEL (widget)->area_id >= LAST_SIGNAL))
    {
        return;
    }

    g_signal_emit (widget, wheel_signals[MOKO_FINGER_WHEEL (widget)->area_id], 0);
}

static gint moko_finger_wheel_button_press(GtkWidget* widget, GdkEventButton* event)
{
    moko_debug( "moko_finger_wheel_button_press" );
    
    MokoFingerWheelPrivate* priv = MOKO_FINGER_WHEEL_GET_PRIVATE(widget);
    
    gtk_grab_add( widget );
    gtk_widget_set_state( widget, GTK_STATE_ACTIVE );
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

    moko_finger_wheel_button_check_area (widget, event);
    
    g_source_remove_by_user_data((gpointer) widget);
    g_timeout_add (FINGER_WHEEL_LONG_PRESS_TIMEOUT, (GSourceFunc) moko_finger_wheel_button_long_press, (gpointer) widget);
    
    return TRUE;
}

//FIXME Right now this is hardcoded to relative mode. Implement absolute mode as well
static gint moko_finger_wheel_motion_notify(GtkWidget* widget, GdkEventMotion* event)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint)
        gdk_window_get_pointer (event->window, &x, &y, &state);
    else
    {
        x = event->x;
        y = event->y;
        state = event->state;
    }

    if (state & GDK_BUTTON1_MASK)
        moko_debug( "FIXME: emit scroll values here..." );

    return TRUE;
}

static gint moko_finger_wheel_button_release(GtkWidget* widget, GdkEventButton* event)
{
    moko_debug( "moko_finger_wheel_button_release" );
    
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
    gtk_widget_set_state( widget, GTK_STATE_NORMAL );
    gtk_grab_remove( widget );

    moko_finger_wheel_button_emit_signal (widget, event);
    
    g_source_remove_by_user_data((gpointer) widget);
    return TRUE;
}


static gboolean moko_finger_wheel_button_long_press(gpointer data)
{
    
    GtkWidget* widget = (GtkWidget*) data;
    
    if (MOKO_FINGER_WHEEL (widget)->area_id == PRESS_LEFT_UP)
    {
        moko_debug( "moko_finger_wheel_button_long_press_left_up" );
        g_source_remove_by_user_data((gpointer) widget);
        g_timeout_add (FINGER_WHEEL_LONG_PRESS_TIMEOUT/4, (GSourceFunc) moko_finger_wheel_button_long_press, (gpointer) widget);
        g_signal_emit (widget, wheel_signals[LONG_PRESS_LEFT_UP], 0);
        return TRUE;
    }
    else if (MOKO_FINGER_WHEEL (widget)->area_id == PRESS_RIGHT_DOWN)
    {
        moko_debug( "moko_finger_wheel_button_long_press_right_down" );
        g_source_remove_by_user_data((gpointer) widget);
        g_timeout_add (FINGER_WHEEL_LONG_PRESS_TIMEOUT/4, (GSourceFunc) moko_finger_wheel_button_long_press, (gpointer) widget);

        g_signal_emit (widget, wheel_signals[LONG_PRESS_RIGHT_DOWN], 0);
        return TRUE;
    }
    
    return FALSE;
}
