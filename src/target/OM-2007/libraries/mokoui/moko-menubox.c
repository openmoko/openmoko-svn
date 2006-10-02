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

#include <gtk/gtkmenubar.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkcombobox.h>

#define MOKO_MENU_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_MENUBOX, MokoMenuBoxPriv));

typedef struct _MokoMenuBoxPriv
{
    GtkMenuBar* menubar;
    GtkComboBox* combobox;
} MokoMenuBoxPriv;

/* add your signals here */
enum {
    MOKO_MENUBOX_SIGNAL,
    LAST_SIGNAL
};

static void moko_menubox_class_init          (MokoMenuBoxClass *klass);
static void moko_menubox_init                (MokoMenuBox      *f);

static guint moko_menubox_signals[LAST_SIGNAL] = { 0 };

GType moko_menubox_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoMenuBoxClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_menubox_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoMenuBox),
            0,
            (GInstanceInitFunc) moko_menubox_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_HBOX, "MokoMenuBox", &f_info, 0);
    }

    return self_type;
}

static void moko_menubox_class_init (MokoMenuBoxClass *klass) /* Class Initialization */
{
    g_type_class_add_private(klass, sizeof(MokoMenuBoxPriv));

    moko_menubox_signals[MOKO_MENUBOX_SIGNAL] = g_signal_new ("moko_menubox",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoMenuBoxClass, moko_menubox),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_menubox_init (MokoMenuBox *self) /* Instance Construction */
{
    g_debug( "moko_paned_window_init" );
    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    priv->menubar = NULL;
    priv->combobox = NULL;

}

GtkWidget* moko_menubox_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_menubox_get_type(), NULL));
}

void moko_menubox_clear(MokoMenuBox *f) /* Destruction */
{
    /* destruct your widgets here */
}

void moko_menubox_set_application_menu(MokoMenuBox* self, GtkMenu* menu)
{
    g_debug( "moko_menubox_set_application_menu" );

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->menubar )
    {
        priv->menubar = gtk_menu_bar_new();
        gtk_box_pack_start( GTK_BOX(self), GTK_WIDGET(priv->menubar), TRUE, TRUE, 0 );
    }
    GtkMenuItem* appitem = gtk_menu_item_new_with_label( g_get_application_name() );
    gtk_menu_item_set_submenu( appitem, menu );
    gtk_menu_shell_append( GTK_MENU_BAR(priv->menubar), appitem );
}

void moko_menubox_set_filter_menu(MokoMenuBox* self, GSList* entries)
{
    void _populate_filter_menu(gpointer data, gpointer user_data)
    {
        g_debug( "_populate_filter_menu %s", (gchar*) data );
        GtkComboBox* cb = GTK_COMBO_BOX(user_data);
        gtk_combo_box_append_text( cb, data );

    }

    g_debug( "moko_menubox_set_filter_menu" );

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->combobox )
    {
        priv->combobox = gtk_combo_box_new_text();
        gtk_box_pack_end( GTK_BOX(self), GTK_WIDGET(priv->combobox), TRUE, TRUE, 0 );
    }
    g_assert( entries );
    g_slist_foreach( entries, &_populate_filter_menu, priv->combobox );
    g_slist_free( entries );

    gtk_combo_box_set_active( priv->combobox, 0 );
}

