/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Inspired by hildon-program.c (C) 2006 Nokia Corporation.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#include "moko-application.h"
#include "moko-stock.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkiconfactory.h>

#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdarg.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(...)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#endif

G_DEFINE_TYPE (MokoApplication, moko_application, G_TYPE_OBJECT)

#define MOKO_APPLICATION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_APPLICATION, MokoApplicationPrivate));

typedef struct _MokoApplicationPrivate
{
    /* Window Manager Collaboration */
    Atom mb_current_app_window_atom; // _MB_CURRENT_APP_WINDOW
    Atom net_active_window_atom; // _NET_ACTIVE_WINDOW
    gboolean seen_matchbox_atom; // TRUE, if we received _MB_CURRENT_APP_WINDOW at least once
    Window window_group; // X11 Window Group (one group per application)
    gboolean is_topmost; // whether one of the application windows is topmost
    GSList* windows; // contains all windows belonging to this application
    gchar* name;

    /* Common Utilities */
    MokoWindow* main_window;
    GtkIconFactory* icon_factory;

} MokoApplicationPrivate;

enum
{
    PROP_0,
    PROP_IS_TOPMOST,
    PROP_KILLABLE,
};

/* forwards */
static void moko_application_class_init(MokoApplicationClass *self);
static void moko_application_init(MokoApplication *self);
static void moko_application_finalize (GObject *self);
static void moko_application_set_property(GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void moko_application_get_property(GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

/* private API */
static void moko_application_class_init (MokoApplicationClass *self)
{
    GObjectClass *object_class = G_OBJECT_CLASS(self);

    g_type_class_add_private (self, sizeof(MokoApplicationPrivate));

    /* Set up object virtual functions */
    object_class->finalize = moko_application_finalize;
    object_class->set_property = moko_application_set_property;
    object_class->get_property = moko_application_get_property;

    /* Install properties */
    g_object_class_install_property (object_class, PROP_IS_TOPMOST,
                                     g_param_spec_boolean ("is-topmost",
                                             "Is top-most",
                                             "Whether one of the program's window or dialog currently "
                                                     "is activated by window manager",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (object_class, PROP_KILLABLE,
                                     g_param_spec_boolean ("can-hibernate",
                                             "Can hibernate",
                                             "Whether the program should be set to hibernate by the Task "
                                                     "Navigator in low memory situation",
                                             FALSE,
                                             G_PARAM_READWRITE));
    return;
}

static void moko_application_init(MokoApplication *self)
{
    moko_debug_minder( self && MOKO_IS_APPLICATION(self) );
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE (self);

    /* create our own icon factory and add some defaults to it */
    priv->icon_factory = gtk_icon_factory_new();
    gtk_icon_factory_add_default( priv->icon_factory );

    priv->windows = NULL;
    priv->is_topmost = FALSE;
    priv->window_group = GDK_WINDOW_XID(gdk_display_get_default_group(gdk_display_get_default()));
    priv->name = NULL;

    priv->mb_current_app_window_atom = XInternAtom( GDK_DISPLAY(), "_MB_CURRENT_APP_WINDOW", False );
    priv->net_active_window_atom = XInternAtom( GDK_DISPLAY(), "_NET_ACTIVE_WINDOW", False );
    priv->seen_matchbox_atom = FALSE;

    moko_stock_register();
}

static void moko_application_finalize (GObject *self)
{
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE(MOKO_APPLICATION(self));
    g_free( priv->name );
}

static void moko_application_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec)
{
    switch (property_id){
        case PROP_KILLABLE:
            g_warning( "NYI: moko_application_set_can_hibernate()" );
            //moko_application_set_can_hibernate (MOKO_APPLICATION (object),
            //                                  g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }

}

static void moko_application_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec)
{
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE (object);

    switch (property_id)
    {
        case PROP_IS_TOPMOST:
            g_value_set_boolean (value, priv->is_topmost);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }

}

/* Utilities */

/*
 * Checks the root window to know which is the topmost window
 */
Window
moko_application_get_active_window(MokoApplication* self)
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

    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE (MOKO_APPLICATION(self));
    Atom active_app_atom = priv->seen_matchbox_atom ? priv->mb_current_app_window_atom : priv->net_active_window_atom;

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

static gint
moko_application_window_list_compare(gconstpointer window_a, gconstpointer window_b)
{
    g_return_val_if_fail( 1, MOKO_IS_WINDOW(window_a) && MOKO_IS_WINDOW(window_b) );
    return window_a != window_b;
}
/*
 * foreach function, checks if a window is topmost and acts consequently
 */
static void
moko_application_list_is_is_topmost(gpointer data, gpointer window_id_)
{
    if ( data && MOKO_IS_WINDOW (data) )
    {
        MokoWindow *window = MOKO_WINDOW(data);
        Window window_id = * (Window*)window_id_;
        moko_window_update_topmost(window, window_id);
    }
}

/*
 * Check the _MB_CURRENT_APP_WINDOW / _NET_ACTIVE_WINDOW on the root window, and update
 * the topmost status accordingly
 */
static void
moko_application_update_top_most(MokoApplication* self)
{
    moko_debug( "moko_application_update_top_most '%s'", g_get_application_name() );

    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);
    Window active_window = moko_application_get_active_window(self);

    if (active_window)
    {
        XWMHints *wm_hints = XGetWMHints(GDK_DISPLAY(), active_window);

        if (wm_hints)
        {
            if (wm_hints->window_group == priv->window_group)
            {
                if (!priv->is_topmost)
                {
                    moko_debug( "-- '%s' is now topmost :)", g_get_application_name() );
                    priv->is_topmost = TRUE;
                    g_object_notify(G_OBJECT(self), "is-topmost");
                }
            }
            else if (priv->is_topmost)
            {
                moko_debug( "-- '%s' is no longer topmost :(", g_get_application_name() );
                priv->is_topmost = FALSE;
                g_object_notify(G_OBJECT(self), "is-topmost");
            }
        }
        XFree (wm_hints);
    }

    /* Update topmost status for every window */
#if 1
    g_slist_foreach(priv->windows, (GFunc)moko_application_list_is_is_topmost, &active_window);
#endif
}

/* Event filter */
/*
 * We keep track of the _MB_CURRENT_APP_WINDOW property on the root window,
 * to detect when a window belonging to this program was is_topmost. This
 * is based on the window group WM hint.
 */
static GdkFilterReturn
moko_application_root_window_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    moko_debug( "moko_application_root_window_event_filter" );

    XAnyEvent *eventti = xevent;
    MokoApplication* program = MOKO_APPLICATION(data);
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(program);

    if (eventti->type == PropertyNotify)
    {
        XPropertyEvent* pevent = xevent;
        moko_debug( "-- got PropertyNotify for Atom '%s'", XGetAtomName( GDK_DISPLAY(), pevent->atom ) );

        if (pevent->atom == priv->mb_current_app_window_atom)
        {
            moko_debug( "-- got _MB_CURRENT_APP_WINDOW atom" );
            priv->seen_matchbox_atom = TRUE;
            moko_application_update_top_most( program );
            return GDK_FILTER_CONTINUE;
        }
        else
        if ( (pevent->atom == priv->net_active_window_atom) && (!priv->seen_matchbox_atom) )
        {
            moko_debug( "-- got _NET_ACTIVE_WINDOW atom" );
            moko_application_update_top_most( program );
            return GDK_FILTER_CONTINUE;
        }
    }
    return GDK_FILTER_CONTINUE;
}

/* public API */

/**
 * moko_application_get_instance:
 *
 * @return the #MokoApplication for the current process.
 * @note The object is created on the first call.
 **/
MokoApplication* moko_application_get_instance (void)
{
    static MokoApplication* program = NULL;

    if (!program)
    {
        program = g_object_new(MOKO_TYPE_APPLICATION, NULL);
    }

    return program;
}

/**
 * moko_application_add_window:
 *
 * add #MokoWindow to the list of windows for this application
 * @note usually there's no need to call this explicitly, since it's called automatically in the constructor of #MokoWindow
 * @note the first #MokoWindow constructed in an application is treated as the main window
 **/
void
moko_application_add_window(MokoApplication* self, MokoWindow* window)
{
    moko_debug_minder( self && MOKO_IS_APPLICATION(self) );
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE(self);

    if ( g_slist_find_custom(priv->windows, window, moko_application_window_list_compare) )
    {
        /* We already have that window */
        g_warning( "moko_application_add_window: window added twice, fix your program" );
        return;
    }

    if (!priv->windows)
    {
        /* Now that we have a window we should start keeping track of the root window */
        gdk_window_set_events( gdk_get_default_root_window(), gdk_window_get_events(gdk_get_default_root_window ()) | GDK_PROPERTY_CHANGE_MASK);
        gdk_window_add_filter( gdk_get_default_root_window(), moko_application_root_window_event_filter, self );
    }

    priv->windows = g_slist_append( priv->windows, window );
}

/**
 * moko_application_remove_window:
 *
 * remove #MokoWindow from the list of windows for this application
 * @note usually there's no need to call this explicitly, since it's called automatically in the destructor of #MokoWindow
 **/
void
moko_application_remove_window(MokoApplication* self, MokoWindow* window)
{
    moko_debug_minder( self && MOKO_IS_APPLICATION(self) );
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE(self);

    priv->windows = g_slist_remove( priv->windows, window );
}
/**
 * moko_application_get_main_window:
 *
 * @return the main #MokoWindow for the current #MokoApplication.
 **/
GtkWidget* moko_application_get_main_window(MokoApplication* self)
{
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);
    return GTK_WIDGET (priv->main_window);
}
/**
 * moko_application_set_main_window:
 *
 * set the main #MokoWindow for this application
 * @note usually there is no need to call this explicitly since
 * it happens automatically when you create the first #MokoWindow
 **/
void moko_application_set_main_window(MokoApplication* self, MokoWindow* window)
{
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);
    priv->main_window = window;
    //FIXME g_object_ref the window?
}
/**
 * moko_application_get_is_topmost:
 *
 * @returns whether one of the program's windows or dialogs is currently
 * activated by the window manager.
 **/
gboolean
moko_application_get_is_topmost(MokoApplication* self)
{
    g_return_val_if_fail(self && MOKO_IS_APPLICATION(self), FALSE);
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);
    return priv->is_topmost;
}

/**
 * moko_application_add_stock_icons:
 *
 * register a number of stock icons given by name
 * @note name of stock icon must match filename in pixmap directory
 **/
void moko_application_add_stock_icons(MokoApplication* self, ...)
{
    moko_debug_minder( self && MOKO_IS_APPLICATION(self) );
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);

    va_list valist;
    gchar* name;
    va_start(valist, self);

    while ( (name = va_arg(valist, gchar*)) )
    {
        gchar* filename = g_strconcat( name, ".png", NULL );
        moko_debug( "-- adding stock icon '%s' from pixmap %s", name, g_build_filename( DATADIR "icons", filename, NULL ) );
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file( g_build_filename( DATADIR "icons", filename, NULL ), NULL );
        GtkIconSet* iconset = gtk_icon_set_new_from_pixbuf( pixbuf );
        gtk_icon_factory_add( priv->icon_factory, name, iconset);
        gtk_icon_set_unref( iconset );
        g_free( filename );
        g_object_unref( pixbuf );
    };

    va_end(valist);
}

/**
 * moko_application_execute_dialog:
 *
 * create a modal dialog window with @a title and @a contents
 **/
GtkWidget* moko_application_execute_dialog(MokoApplication* self, const gchar* title, GtkWidget* contents)
{
    GtkWidget* dialog = moko_dialog_window_new();
    moko_dialog_window_set_title( MOKO_DIALOG_WINDOW (dialog), title );
    moko_dialog_window_set_contents( MOKO_DIALOG_WINDOW (dialog), contents );
    moko_dialog_window_run( MOKO_DIALOG_WINDOW (dialog) );
    return GTK_WIDGET (dialog);
}
