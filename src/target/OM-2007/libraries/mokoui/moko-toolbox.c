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
#include "moko-toolbox.h"
#include "moko-search-bar.h"

#include <gtk/gtktoolbutton.h>
#include <gtk/gtkimage.h>

#define MOKO_TOOL_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_TOOL_BOX, MokoToolBoxPriv));

typedef struct _MokoToolBoxPriv
{
    GtkToolbar* toolbar;
    MokoSearchBar* searchbar;
} MokoToolBoxPriv;

/* add your signals here */
enum {
    MOKO_TOOL_BOX_SIGNAL,
    LAST_SIGNAL
};

static void moko_tool_box_class_init          (MokoToolBoxClass *klass);
static void moko_tool_box_init                (MokoToolBox      *self);

static guint moko_tool_box_signals[LAST_SIGNAL] = { 0 };

GType moko_tool_box_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo self_info =
        {
            sizeof (MokoToolBoxClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_tool_box_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoToolBox),
            0,
            (GInstanceInitFunc) moko_tool_box_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_VBOX, "MokoToolBox", &self_info, 0);
    }

    return self_type;
}

static void moko_tool_box_class_init (MokoToolBoxClass *klass) /* Class Initialization */
{
    g_type_class_add_private(klass, sizeof(MokoToolBoxPriv));

    moko_tool_box_signals[MOKO_TOOL_BOX_SIGNAL] = g_signal_new ("moko_tool_box",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoToolBoxClass, moko_tool_box),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_tool_box_init(MokoToolBox* self) /* Instance Construction */
{
    gboolean button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
    {
        g_assert( MOKO_IS_TOOL_BOX(widget) );
        MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(widget);
        static gboolean foo = FALSE;
        if ( foo )
        {
            gtk_widget_hide( GTK_WIDGET(priv->searchbar) );
            gtk_widget_show( GTK_WIDGET(priv->toolbar) );
        }
        else
        {
            gtk_widget_hide( GTK_WIDGET(priv->toolbar) );
            gtk_widget_show( GTK_WIDGET(priv->searchbar) );
        }
        g_debug( "hello world %d", foo );
        foo = !foo;
        return FALSE;
    }

    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    priv->toolbar = gtk_toolbar_new();
    gtk_widget_set_name( GTK_WIDGET(priv->toolbar), "moko_toolbar1" );
    gtk_widget_set_size_request( GTK_WIDGET(priv->toolbar), 50, 52 ); //FIXME get from style
    gtk_box_pack_start( GTK_BOX(self), priv->toolbar, TRUE, TRUE, 0 );

    priv->searchbar = moko_search_bar_new();
    gtk_widget_set_name( GTK_WIDGET(priv->searchbar), "moko_toolbar2" );
    gtk_box_pack_start( GTK_BOX(self), priv->searchbar, TRUE, TRUE, 0 );
    gtk_widget_set_size_request( GTK_WIDGET(priv->searchbar), 50, 52 ); //FIXME get from style

    g_signal_connect( GTK_WIDGET(self), "button-release-event", G_CALLBACK(button_release), NULL );

    gtk_widget_set_no_show_all( GTK_WIDGET(priv->searchbar), TRUE );
}

GtkWidget* moko_tool_box_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_tool_box_get_type(), NULL));
}

GtkWidget* moko_tool_box_new_with_search()
{
    MokoToolBox* toolbox = g_object_new(moko_tool_box_get_type(), NULL);
    g_assert( toolbox );
    moko_tool_box_add_search_button( toolbox );
    return GTK_WIDGET(toolbox);
}

void moko_tool_box_clear(MokoToolBox* self) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */

void moko_tool_box_add_search_button(MokoToolBox* self )
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    GtkToolButton* tool_search = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "" ));
    GtkImage* icon = gtk_image_new_from_file( "/local/pkg/openmoko/OM-2007/artwork/themes/openmoko-standard/gtk-2.0/openmoko-search-button.png" );
    gtk_tool_button_set_icon_widget( tool_search, icon );
    gtk_widget_set_name( GTK_WIDGET(tool_search), "moko_search_button" );
    gtk_toolbar_insert( priv->toolbar, tool_search, 0 );
}

GtkToolbar* moko_tool_box_get_tool_bar(MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    return priv->toolbar;
}
