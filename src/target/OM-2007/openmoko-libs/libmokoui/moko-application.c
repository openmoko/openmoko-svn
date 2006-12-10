/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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

#include <gtk/gtkwidget.h>
#include <gtk/gtkiconfactory.h>

#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdarg.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoApplication, moko_application, G_TYPE_OBJECT)

#define MOKO_APPLICATION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_APPLICATION, MokoApplicationPrivate));

typedef struct _MokoApplicationPrivate
{
    //FIXME hildon cruft, do we need all this?
    gboolean killable;
    gboolean is_topmost;
    guint window_count;
    GtkWidget *common_application_menu;
    GtkWidget *common_filter_menu;
    GtkWidget *common_toolbar;
    GSList *windows;
    Window window_group;
    gchar *name;

    // our stuff
    MokoWindow* main_window;
    GtkIconFactory* icon_factory;

} MokoApplicationPrivate;

enum
{
    PROP_0,
    PROP_IS_TOPMOST,
    PROP_KILLABLE
};

static void moko_application_class_init(MokoApplicationClass *self);
static void moko_application_init(MokoApplication *self);
static void moko_application_set_property(GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void moko_application_get_property(GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

static void moko_application_init(MokoApplication *self)
{
    moko_debug( "moko_application_init" );
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE (self);

    /* create our own icon factory and add some defaults to it */
    priv->icon_factory = gtk_icon_factory_new();
    gtk_icon_factory_add_default( priv->icon_factory );

#if 0
    moko_application_add_stock_icons( self,
                                      "openmoko-default-application",
                                      NULL );
#endif

    // cruft necessary?
    priv->killable = FALSE;
    priv->window_count = 0;
    priv->is_topmost = FALSE;
    priv->window_group = GDK_WINDOW_XID(gdk_display_get_default_group(gdk_display_get_default()));
    priv->common_application_menu = NULL;
    priv->common_toolbar = NULL;
    priv->common_toolbar = NULL;
    priv->name = NULL;
}

static void moko_application_finalize (GObject *self)
{
    MokoApplicationPrivate *priv = MOKO_APPLICATION_GET_PRIVATE (MOKO_APPLICATION(self));

    if (priv->common_toolbar)
    {
        g_object_unref (priv->common_toolbar);
        priv->common_toolbar = NULL;
    }

    if (priv->common_application_menu)
    {
        g_object_unref (priv->common_application_menu);
        priv->common_application_menu = NULL;
    }

    if (priv->common_filter_menu)
    {
        g_object_unref (priv->common_filter_menu);
        priv->common_filter_menu = NULL;
    }

    g_free (priv->name);
}

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
        case PROP_KILLABLE:
            g_value_set_boolean (value, priv->killable);
            break;
        case PROP_IS_TOPMOST:
            g_value_set_boolean (value, priv->is_topmost);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }

}

/* Event filter */

/*
 * We keep track of the _MB_CURRENT_APP_WINDOW property on the root window,
 * to detect when a window belonging to this program was is_topmost. This
 * is based on the window group WM hint.
 */
static GdkFilterReturn moko_application_root_window_event_filter(
        GdkXEvent *xevent,
        GdkEvent *event,
        gpointer data)
{
    XAnyEvent *eventti = xevent;
    MokoApplication *program = MOKO_APPLICATION (data);
    Atom active_app_atom =
            XInternAtom (GDK_DISPLAY (), "_MB_CURRENT_APP_WINDOW", False);

    if (eventti->type == PropertyNotify)
    {
        XPropertyEvent *pevent = xevent;

        if (pevent->atom == active_app_atom)
        {
            g_warning( "NYI: moko_application_update_top_most(program)" );
            //moko_application_update_top_most( program );
        }
    }

    return GDK_FILTER_CONTINUE;
}

/* Public API */

/**
 * moko_application_get_instance:
 *
 * @return the #MokoApplication for the current process.
 * The object is created on the first call.
 **/
MokoApplication* moko_application_get_instance (void)
{
    static MokoApplication *program = NULL;

    if (!program)
    {
        program = g_object_new(MOKO_TYPE_APPLICATION, NULL);
    }

    return program;
}

/** moko_application_get_main_window
 *
 * @return the main #MokoWindow for the current #MokoApplication.
 **/
MokoWindow* moko_application_get_main_window(MokoApplication* self)
{
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);
    return priv->main_window;
}
/** moko_application_set_main_window
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

/** moko_application_get_style_pixmap_dir
 *
 * @return the style pixmap directory
 * @note this is not necessarily $GTK_DATA_DIR/themes/$THEME_NAME/gtk-2.0/
 **/
gchar* moko_application_get_style_pixmap_dir()
{
    GtkStyle* style = gtk_rc_get_style_by_paths( gtk_settings_get_default(), "GtkWidget", "GtkWidget", GTK_TYPE_WIDGET );
    return g_path_get_dirname( style->rc_style->bg_pixmap_name[GTK_STATE_NORMAL] );
}

void moko_application_add_stock_icons(MokoApplication* self, ...)
{
    moko_debug( "moko_application_add_stock_icon" );
    MokoApplicationPrivate* priv = MOKO_APPLICATION_GET_PRIVATE(self);

    va_list valist;
    gchar* name;
    va_start(valist, self);

    while ( name = va_arg(valist, gchar*) )
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

MokoDialogWindow* moko_application_execute_dialog(MokoApplication* self, const gchar* title, GtkWidget* contents)
{
    MokoDialogWindow* dialog = moko_dialog_window_new();
    moko_dialog_window_set_title( dialog, title );
    moko_dialog_window_set_contents( dialog, contents );
    moko_dialog_window_run( dialog );
    return dialog;
}
