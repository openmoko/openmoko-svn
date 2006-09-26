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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

#define MOKO_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_APPLICATION, MokoApplicationPriv));

typedef struct _MokoApplicationPriv
{
    gboolean killable;
    gboolean is_topmost;
    GdkWindow *group_leader;
    guint window_count;
    GtkWidget *common_application_menu;
    GtkWidget *common_filter_menu;
    GtkWidget *common_toolbar;
    GSList *windows;
    Window window_group;
    gchar *name;
} MokoApplicationPriv;

enum
{
    PROP_0,
    PROP_IS_TOPMOST,
    PROP_KILLABLE
};

static void moko_application_class_init (MokoApplicationClass *self);
static void moko_application_init (MokoApplication *self);
static void moko_application_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void moko_application_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

GType moko_application_get_type (void)
{
    static GType program_type = 0;

    if (!program_type)
    {
        static const GTypeInfo program_info =
        {
            sizeof(MokoApplicationClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) moko_application_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(MokoApplication),
            0,  /* n_preallocs */
            (GInstanceInitFunc) moko_application_init,
        };
        program_type = g_type_register_static(G_TYPE_OBJECT, "MokoApplication", &program_info, 0);
    }
    return program_type;
}

static void moko_application_init (MokoApplication *self)
{
    MokoApplicationPriv *priv = MOKO_APPLICATION_GET_PRIVATE (self);

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
    MokoApplicationPriv *priv = MOKO_APPLICATION_GET_PRIVATE (MOKO_APPLICATION(self));

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

    g_type_class_add_private (self, sizeof(MokoApplicationPriv));

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
    MokoApplicationPriv *priv = MOKO_APPLICATION_GET_PRIVATE (object);

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

/* Public methods */

/**
 * moko_application_get_instance:
 *
 * Return value: Returns the #MokoApplication for the current process.
 * The object is created on the first call.
 **/
MokoApplication* moko_application_get_instance (void)
{
    static MokoApplication *program = NULL;

    if (!program)
    {
        program = g_object_new (MOKO_TYPE_APPLICATION, NULL);
    }

    return program;
}

