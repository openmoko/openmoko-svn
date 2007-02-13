/*
 *  libmokoui -- OpenMoko Application Framework UI Library
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#include "moko-tool-box.h"
#include "moko-fixed.h"

#include <gtk/gtkentry.h>
#include <gtk/gtkvbox.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoToolBox, moko_tool_box, GTK_TYPE_NOTEBOOK)

#define MOKO_TOOL_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_TOOL_BOX, MokoToolBoxPriv));

typedef struct _MokoToolBoxPriv
{
    MokoFixed* toolbar_page;
    GtkHBox* buttonbox;
    MokoFixed* searchbar_page;
    GtkEntry* entry;
} MokoToolBoxPriv;

/* add your signals here */
enum {
    SEARCHBOX_VISIBLE,
    SEARCHBOX_INVISIBLE,
    LAST_SIGNAL,
};

static void moko_tool_box_class_init          (MokoToolBoxClass *klass);
static void moko_tool_box_init                (MokoToolBox      *self);

static guint moko_tool_box_signals[LAST_SIGNAL] = { 0 };

static void _button_release(GtkWidget* w, MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    static int current_page = 1;
    gtk_notebook_set_current_page( GTK_NOTEBOOK(self), current_page );
    moko_debug( "moko_tool_box_button_release: current_page is now: %d", current_page );

    if( current_page == 1 )
        gtk_widget_grab_focus (GTK_WIDGET (priv->entry));

    current_page = 1 - current_page;
    g_signal_emit( G_OBJECT(self), current_page ? moko_tool_box_signals[SEARCHBOX_INVISIBLE] : moko_tool_box_signals[SEARCHBOX_VISIBLE], 0, NULL );

}

static gboolean _entry_focus_in(GtkWidget *widget, GdkEventFocus *event, MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    gtk_widget_set_name( widget, "mokotoolbox-search-entry" );
    gtk_widget_set_name( GTK_WIDGET(priv->searchbar_page), "mokotoolbox-search-mode" );
    return FALSE;
}

static gboolean _entry_focus_out(GtkWidget *widget, GdkEventFocus *event, MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    gtk_widget_set_name( widget, "mokotoolbox-search-entry-focusout" );
    gtk_widget_set_name( GTK_WIDGET(priv->searchbar_page), "mokotoolbox-search-mode-focusout" );
    return FALSE;
}

static void moko_tool_box_class_init (MokoToolBoxClass *klass) /* Class Initialization */
{
    g_type_class_add_private(klass, sizeof(MokoToolBoxPriv));

    moko_tool_box_signals[SEARCHBOX_VISIBLE] = g_signal_new ("searchbox_visible",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoToolBoxClass, searchbox_visible),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    moko_tool_box_signals[SEARCHBOX_INVISIBLE] = g_signal_new ("searchbox_invisible",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoToolBoxClass, searchbox_invisible),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_tool_box_init(MokoToolBox* self) /* Instance Construction */
{
    moko_debug( "moko_tool_box_init" );
    gtk_notebook_set_show_border( GTK_NOTEBOOK(self), FALSE );
    gtk_notebook_set_show_tabs( GTK_NOTEBOOK(self), FALSE );
}

//FIXME 1st: rewrite moko_tool_box_new / moko_tool_box_new_with_search for using g_object properties
//FIXME 2nd: support enabling/disabling search mode on-the-fly
GtkWidget* moko_tool_box_new() /* Construction */
{
    MokoToolBox* self = MOKO_TOOL_BOX(g_object_new(MOKO_TYPE_TOOL_BOX, NULL));
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);

    priv->toolbar_page = moko_fixed_new();
    gtk_widget_set_name( GTK_WIDGET(priv->toolbar_page), "mokotoolbox-normal-mode" );
    priv->buttonbox = gtk_hbox_new( FALSE, 17 ); //FIXME need to get from style
    gtk_fixed_put( GTK_FIXED(priv->toolbar_page), GTK_WIDGET(priv->buttonbox), 1, 7 ); //FIXME need to get from style

    gtk_notebook_append_page( GTK_NOTEBOOK(self), priv->toolbar_page, NULL );

    return GTK_WIDGET(self);
}

GtkWidget* moko_tool_box_new_with_search()
{
    MokoToolBox* self = MOKO_TOOL_BOX(g_object_new(MOKO_TYPE_TOOL_BOX, NULL));
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);

    priv->toolbar_page = moko_fixed_new();
    gtk_widget_set_name( GTK_WIDGET(priv->toolbar_page), "mokotoolbox-normal-mode" );

    MokoPixmapButton* search = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(search), "mokotoolbox-search-button" );
    gtk_fixed_put( GTK_FIXED(priv->toolbar_page), search, 0, 0 );
    priv->buttonbox = gtk_hbox_new( FALSE, 17 ); //FIXME need to get from style
    gtk_fixed_put( GTK_FIXED(priv->toolbar_page), GTK_WIDGET(priv->buttonbox), 84, 7 ); //FIXME need to get from style

    gtk_notebook_append_page( GTK_NOTEBOOK(self), priv->toolbar_page, NULL );

    g_signal_connect( G_OBJECT(search), "clicked", G_CALLBACK(_button_release), self );

    priv->searchbar_page = moko_fixed_new();
    gtk_widget_set_name( GTK_WIDGET(priv->searchbar_page), "mokotoolbox-search-mode" );
    gtk_notebook_append_page( GTK_NOTEBOOK(self), priv->searchbar_page, NULL );

    MokoPixmapButton* back = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(back), "mokotoolbox-back-button" );
    gtk_fixed_put( GTK_FIXED(priv->searchbar_page), back, 400, 0 ); //FIXME need to get from style
    g_signal_connect( G_OBJECT(back), "clicked", G_CALLBACK(_button_release), self );

    priv->entry = gtk_entry_new();
    gtk_entry_set_has_frame( priv->entry, FALSE );
    // gtk_entry_set_inner_border( priv->entry, FALSE );
    gtk_widget_set_name( GTK_WIDGET(priv->entry), "mokotoolbox-search-entry" );
    moko_fixed_set_cargo( priv->searchbar_page, GTK_WIDGET(priv->entry) );
    g_signal_connect ((gpointer) priv->entry, "focus_in_event",
                      G_CALLBACK (_entry_focus_in),
                      self);
    g_signal_connect ((gpointer) priv->entry, "focus_out_event",
                      G_CALLBACK (_entry_focus_out),
                      self);


    return GTK_WIDGET(self);
}

void moko_tool_box_clear(MokoToolBox* self) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */

void moko_tool_box_add_search_button(MokoToolBox* self)
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

GtkWidget* moko_tool_box_get_button_box(MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    return GTK_WIDGET (priv->buttonbox);
}

GtkWidget* moko_tool_box_add_action_button(MokoToolBox* self)
{
    moko_debug( "moko_tool_box_add_action_button" );
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);

    MokoPixmapButton* button = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(button), "mokotoolbox-action-button" );

    //gtk_box_pack_start( GTK_BOX(priv->buttonbox), GTK_WIDGET(button), FALSE, FALSE, 0 );
    gtk_box_pack_end( GTK_BOX(priv->buttonbox), GTK_WIDGET(button), FALSE, FALSE, 0 );

    return GTK_WIDGET (button);
}

GtkWidget* moko_tool_box_get_entry(MokoToolBox* self)
{
    MokoToolBoxPriv* priv = MOKO_TOOL_BOX_GET_PRIVATE(self);
    return GTK_WIDGET (priv->entry);
}
