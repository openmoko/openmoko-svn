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
#include "moko-menubox.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkmenuitem.h>

#define MOKO_MENU_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_MENU_BOX, MokoMenuBoxPriv));

typedef struct _MokoMenuBoxPriv
{
    GtkMenuBar* menubar_l;
    GtkMenuItem* appitem;
    GtkMenu* appmenu;
    GtkMenuBar* menubar_r;
    GtkMenuItem* filteritem;
    GtkMenu* filtermenu;
} MokoMenuBoxPriv;

/* add your signals here */
enum {
    MOKO_MENU_BOX_SIGNAL,
    LAST_SIGNAL
};

static void moko_menu_box_class_init          (MokoMenuBoxClass *klass);
static void moko_menu_box_init                (MokoMenuBox      *f);

static guint moko_menu_box_signals[LAST_SIGNAL] = { 0 };

GType moko_menu_box_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoMenuBoxClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_menu_box_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoMenuBox),
            0,
            (GInstanceInitFunc) moko_menu_box_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_HBOX, "MokoMenuBox", &f_info, 0);
    }

    return self_type;
}

static void moko_menu_box_class_init (MokoMenuBoxClass *klass) /* Class Initialization */
{
    g_type_class_add_private(klass, sizeof(MokoMenuBoxPriv));

    moko_menu_box_signals[MOKO_MENU_BOX_SIGNAL] = g_signal_new ("moko_menu_box",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoMenuBoxClass, moko_menu_box),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_menu_box_init (MokoMenuBox *self) /* Instance Construction */
{
    g_debug( "moko_paned_window_init" );
    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    priv->menubar_l = NULL;
    priv->menubar_r = NULL;

}

GtkWidget* moko_menu_box_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_menu_box_get_type(), NULL));
}

void moko_menu_box_clear(MokoMenuBox *f) /* Destruction */
{
    /* destruct your widgets here */
}

static gboolean cb_button_release(GtkWidget *widget, GdkEventButton *event, GtkMenu* menu)
{
    //FIXME don't open menu when it is already opened
    g_debug( "menu open forwarder activated..." );

    g_debug( "clicked on %f, %f", event->x, event->y );

    if ( !GTK_WIDGET_VISIBLE(menu) )
    {
        g_debug( "menu open forwarder: not yet open -- popping up" );
        gtk_menu_shell_select_first( GTK_MENU_SHELL(widget), FALSE );
        return FALSE;
    }
    {
        g_debug( "menu open forwarder: already open -- ignoring" );
        gtk_menu_popdown( menu );
        return TRUE;
    }
}

static void cb_filter_menu_update( GtkMenu* menu, GtkMenuItem* filtitem )
{
    gchar* text;
    GtkMenuItem* item = gtk_menu_get_active( menu );
    if (GTK_BIN(item)->child)
    {
        GtkWidget *child = GTK_BIN(item)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_get(GTK_LABEL (child), &text);
        g_debug(" selection done. menu item text: %s", text );
    }
    if (GTK_BIN(filtitem)->child)
    {
        GtkWidget *child = GTK_BIN(filtitem)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_set(GTK_LABEL (child), text);
        g_debug(" selection done. menu label updated." );
    }
}

void moko_menu_box_set_application_menu(MokoMenuBox* self, GtkMenu* menu)
{
    g_debug( "moko_menu_box_set_application_menu" );

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->menubar_l )
    {
        priv->menubar_l = gtk_menu_bar_new();
        gtk_widget_set_name( GTK_WIDGET(priv->menubar_l), "mokomenubox-application-menubar" );
        gtk_box_pack_start( GTK_BOX(self), GTK_WIDGET(priv->menubar_l), TRUE, TRUE, 0 );

    }
    GtkMenuItem* appitem = gtk_menu_item_new_with_label( g_get_application_name() );
    priv->appitem = appitem;
    priv->appmenu = menu;
    gtk_menu_item_set_submenu( appitem, menu );
    gtk_menu_shell_append( GTK_MENU_BAR(priv->menubar_l), appitem );

    //FIXME hack to popup the first menu if user clicks on menubar
    //g_signal_connect( GTK_WIDGET(priv->menubar_l), "button-release-event", G_CALLBACK(cb_button_release), menu );
}

void moko_menu_box_set_filter_menu(MokoMenuBox* self, GtkMenu* menu)
{
    g_debug( "moko_menu_box_set_filter_menu" );

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->menubar_r )
    {
        priv->menubar_r = gtk_menu_bar_new();
        gtk_widget_set_name( GTK_WIDGET(priv->menubar_r), "mokomenubox-filter-menubar" );
        gtk_box_pack_end( GTK_BOX(self), GTK_WIDGET(priv->menubar_r), TRUE, TRUE, 0 );
    }
    GtkMenuItem* filtitem = gtk_menu_item_new_with_label( "Filter Menu" );
    priv->filteritem = filtitem;
    priv->filtermenu = menu;
    g_signal_connect (G_OBJECT(menu), "selection_done", G_CALLBACK(cb_filter_menu_update), filtitem );
    gtk_menu_item_set_submenu( filtitem, menu );
    gtk_menu_shell_append( GTK_MENU_BAR(priv->menubar_r), filtitem );

    //FIXME hack to popup the first menu if user clicks on menubar
    //g_signal_connect( GTK_WIDGET(priv->menubar_r), "button-release-event", G_CALLBACK(cb_button_release), menu );
}

