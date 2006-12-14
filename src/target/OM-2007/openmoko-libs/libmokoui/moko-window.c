/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Based on hildon-window.c (C) 2006 Nokia Corporation.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "moko-application.h"
#include "moko-window.h"

#include <gtk/gtkentry.h>
#include <gtk/gtktextview.h>

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

/* add your signals here */
enum {
    MOKO_WINDOW_SIGNAL,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_IS_TOPMOST,
};

static void moko_window_class_init(MokoWindowClass *klass);
static void moko_window_init(MokoWindow *self);
static void moko_window_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);
static void moko_window_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);
static void moko_window_notify(GObject* gobject, GParamSpec* param);
static void moko_window_is_topmost_notify(MokoWindow* self);

static guint moko_window_signals[LAST_SIGNAL] = { 0 };
static GtkWindowClass* parent_class = NULL;

G_DEFINE_TYPE (MokoWindow, moko_window, GTK_TYPE_WINDOW)

#define MOKO_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_WINDOW, MokoWindowPrivate));

typedef struct _MokoWindowPrivate
{
    gboolean is_fullscreen;
    gboolean is_topmost;
} MokoWindowPrivate;

static void moko_window_class_init(MokoWindowClass *klass) /* Class Initialization */
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Set the global parent_class here */
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private( klass, sizeof(MokoWindowPrivate) );

    /* hook virtual methods */
    object_class->set_property = moko_window_set_property;
    object_class->get_property = moko_window_get_property;
    object_class->notify = moko_window_notify;

    /* install signals */
    moko_window_signals[MOKO_WINDOW_SIGNAL] = g_signal_new ("moko_window",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoWindowClass, moko_window),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    /* install properties */
    g_object_class_install_property (object_class, PROP_IS_TOPMOST,
                g_param_spec_boolean ("is-topmost",
                "Is top-most",
                "Whether the window is currently activated by the window "
                "manager",
                FALSE,
                G_PARAM_READABLE));
}

static void moko_window_init(MokoWindow *self) /* Instance Construction */
{
    moko_debug( "moko_window_init" );
    gtk_widget_set_size_request( GTK_WIDGET(self), 480, 640 ); //FIXME get from style
    MokoApplication* app = moko_application_get_instance();
    if ( !moko_application_get_main_window( app ) )
        moko_application_set_main_window( app, self );
    moko_application_add_window( app, self );
}

GtkWidget* moko_window_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_window_get_type(), NULL));
}

void moko_window_clear(MokoWindow *self) /* Destruction */
{
    /* destruct your widgets here */
}

static void
moko_window_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec)
{
    switch (property_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
moko_window_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
    MokoWindowPrivate* priv = MOKO_WINDOW_GET_PRIVATE(object);

    switch (property_id) {

    case PROP_IS_TOPMOST:
            g_value_set_boolean (value, priv->is_topmost);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
moko_window_notify(GObject* gobject, GParamSpec* param)
{
    moko_debug( "moko_window_notify" );
    MokoWindow* window = MOKO_WINDOW(gobject);

    if (strcmp(param->name, "title") == 0)
    {
        moko_debug( "update window title" );
        //moko_window_update_title(window);
    }
    else if (strcmp(param->name, "is-topmost"))
    {
        moko_window_is_topmost_notify(window);
    }

    if (G_OBJECT_CLASS(parent_class)->notify)
        G_OBJECT_CLASS(parent_class)->notify (gobject, param);
}

static void
moko_window_is_topmost_notify(MokoWindow* self)
{
    moko_debug( "moko_window_is_topmost_notify" );
    MokoWindowPrivate* priv = MOKO_WINDOW_GET_PRIVATE(self);
    if (priv->is_topmost)
    {
        moko_debug( "-- I am topmost now :D" );
    }
    else
    {
        moko_debug( "-- I am no longer topmost :(" );
    }
}

/*
 * Compare the window that was last topped, and act consequently
 */
void
moko_window_update_topmost(MokoWindow* self, Window window_id)
{
    moko_debug( "moko_window_update_topmost" );
    MokoWindowPrivate* priv = MOKO_WINDOW_GET_PRIVATE(self);
    Window my_window;

    my_window = GDK_WINDOW_XID (GTK_WIDGET (self)->window);

    if (window_id == my_window)
    {
        if (!priv->is_topmost)
        {
            priv->is_topmost = TRUE;
            moko_window_is_topmost_notify( self );
            g_object_notify( G_OBJECT(self), "is-topmost" );
        }
    }
    else if (priv->is_topmost)
    {
        /* Should this go in the signal handler? */
        GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(self));

        if (GTK_IS_ENTRY(focus))
            gtk_im_context_focus_out(GTK_ENTRY(focus)->im_context);
        if (GTK_IS_TEXT_VIEW (focus))
            gtk_im_context_focus_out(GTK_TEXT_VIEW(focus)->im_context);

        priv->is_topmost = FALSE;
        moko_window_is_topmost_notify(self);
        g_object_notify( G_OBJECT(self), "is-topmost" );
    }
}

/*
 * Checks the root window to know which is the topmost window
 */
Window
moko_window_get_active_window()
{
    Atom realtype;
    int format;
    int status;
    Window ret;
    unsigned long n;
    unsigned long extra;
    union
    {
        Window *win;
        unsigned char *char_pointer;
    } win;
    Atom active_app_atom = XInternAtom( GDK_DISPLAY(), "_MB_CURRENT_APP_WINDOW", False );
    win.win = NULL;

    status = XGetWindowProperty( GDK_DISPLAY(),
                                 GDK_ROOT_WINDOW(),
                                 active_app_atom,
                                 0L,
                                 16L,
                                 0,
                                 XA_WINDOW,
                                 &realtype,
                                 &format,
                                 &n,
                                 &extra,
                                 &win.char_pointer);
    if ( !(status == Success && realtype == XA_WINDOW && format == 32 && n == 1 && win.win != NULL) )
    {
        if (win.win != NULL) XFree (win.char_pointer);
        return None;
    }

    ret = win.win[0];

    if (win.win != NULL)
        XFree(win.char_pointer);

    return ret;
}
