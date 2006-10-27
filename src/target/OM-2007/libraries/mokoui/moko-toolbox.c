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
#include "moko-pixmap-container.h"
#include "moko-pixmap-button.h"

#include <gtk/gtkentry.h>
#include <gtk/gtkvbox.h>

#define MOKO_TOOL_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_TOOL_BOX, MokoToolBoxPriv));

typedef struct _MokoToolBoxPriv
{
    MokoPixmapContainer* toolbar_page;
    GtkHBox* buttonbox;
    MokoPixmapContainer* searchbar_page;
    GtkEntry* entry;
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
        self_type = g_type_register_static(GTK_TYPE_NOTEBOOK, "MokoToolBox", &self_info, 0);
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
    g_debug( "moko_tool_box_init" );
    gtk_notebook_set_show_border( GTK_NOTEBOOK(self), FALSE );
    gtk_notebook_set_show_tabs( GTK_NOTEBOOK(self), FALSE );
}

GtkWidget* moko_tool_box_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_tool_box_get_type(), NULL));
}

GtkWidget* moko_tool_box_new_with_search()
{
    void button_release(GtkWidget* w, MokoToolBox* self)
    {
        static int current_page = 1;
        gtk_notebook_set_current_page( GTK_NOTEBOOK(self), current_page );
        g_debug( "button_release: current_page is now: %d", current_page );
        current_page = 1 - current_page;
    }
    MokoToolBox* self = MOKO_TOOL_BOX(moko_tool_box_new());
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);

    priv->toolbar_page = moko_pixmap_container_new();
    gtk_widget_set_name( GTK_WIDGET(priv->toolbar_page), "mokotoolbox-normal-mode" );

    MokoPixmapButton* search = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(search), "mokotoolbox-search-button" );
    gtk_fixed_put( GTK_FIXED(priv->toolbar_page), search, 0, 0 );
    priv->buttonbox = gtk_hbox_new( FALSE, 17 ); //FIXME need to get from style
    gtk_fixed_put( GTK_FIXED(priv->toolbar_page), GTK_WIDGET(priv->buttonbox), 84, 7 ); //FIXME need to get from style
    //gtk_widget_set_size_request( GTK_WIDGET(priv->buttonbox), 400, 52 ); //FIXME need to get from style

    gtk_notebook_append_page( GTK_NOTEBOOK(self), priv->toolbar_page, NULL );

    g_signal_connect( G_OBJECT(search), "clicked", G_CALLBACK(button_release), self );

    priv->searchbar_page = moko_pixmap_container_new();
    gtk_widget_set_name( GTK_WIDGET(priv->searchbar_page), "mokotoolbox-search-mode" );
    gtk_notebook_append_page( GTK_NOTEBOOK(self), priv->searchbar_page, NULL );

    MokoPixmapButton* back = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(back), "mokotoolbox-back-button" );
    gtk_fixed_put( GTK_FIXED(priv->searchbar_page), back, 400, 0 ); //FIXME need to get from style
    g_signal_connect( G_OBJECT(back), "clicked", G_CALLBACK(button_release), self );

    priv->entry = gtk_entry_new();
    gtk_entry_set_has_frame( priv->entry, FALSE );
    gtk_entry_set_inner_border( priv->entry, FALSE );
    gtk_widget_set_name( GTK_WIDGET(priv->entry), "mokotoolbox-search-entry" );
    moko_pixmap_container_set_cargo( priv->searchbar_page, GTK_WIDGET(priv->entry) );

    return GTK_WIDGET(self);
}

void moko_tool_box_clear(MokoToolBox* self) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */

void moko_tool_box_add_search_button(MokoToolBox* self )
{
#if 0
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    GtkToolButton* tool_search = GTK_TOOL_BUTTON(gtk_tool_button_new( NULL, "" ));
    GtkImage* icon = gtk_image_new_from_file( "/local/pkg/openmoko/OM-2007/artwork/themes/openmoko-standard/gtk-2.0/openmoko-search-button.png" );
    gtk_tool_button_set_icon_widget( tool_search, icon );
    gtk_widget_set_name( GTK_WIDGET(tool_search), "moko_search_button" );
    gtk_toolbar_insert( priv->toolbar, tool_search, 0 );
#endif
}

GtkHBox* moko_tool_box_get_button_box(MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    return priv->buttonbox;
}

void moko_tool_box_add_action_button(MokoToolBox* self)
{
    g_debug( "moko_tool_box_add_action_button" );
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);

    MokoPixmapButton* button = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(button), "mokotoolbox-action-button" );

    gtk_box_pack_start( GTK_BOX(priv->buttonbox), GTK_WIDGET(button), FALSE, FALSE, 0 );
}