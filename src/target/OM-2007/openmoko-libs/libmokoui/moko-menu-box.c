/*
 * libmokoui -- OpenMoko Application Framework UI Library
 *
 * Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 * Copyright (C) 2006-2007 OpenMoko Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 * Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "moko-menu-box.h"

#include <gtk/gtk.h>
#include <string.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

#define MOKO_MENU_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_MENU_BOX, MokoMenuBoxPriv));

typedef struct _MokoMenuBoxPriv
{
    GtkWidget* menubar_l;  /* GtkMenuBar       */
    GtkWidget* appitem;    /* GtkImageMenuItem */
    GtkWidget* appmenu;    /* GtkMenu          */
    GtkWidget* menubar_r;  /* GtkMenuBar       */
    GtkWidget* filteritem; /* GtkImageMenuItem */
    GtkWidget* filtermenu; /* GtkMenu          */
} MokoMenuBoxPriv;

/* add your signals here */
enum {
    FILTER_CHANGED,
    LAST_SIGNAL
};

static void moko_menu_box_class_init          (MokoMenuBoxClass *klass);
static void moko_menu_box_init                (MokoMenuBox      *f);

static guint moko_menu_box_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (MokoMenuBox, moko_menu_box, GTK_TYPE_HBOX)

static void moko_menu_box_class_init (MokoMenuBoxClass *klass) /* Class Initialization */
{
    g_type_class_add_private(klass, sizeof(MokoMenuBoxPriv));

    moko_menu_box_signals[FILTER_CHANGED] =
            g_signal_new ("filter-changed",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET(MokoMenuBoxClass, filter_changed),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__STRING,
            G_TYPE_NONE, 1, G_TYPE_STRING );
}

static void moko_menu_box_init (MokoMenuBox *self) /* Instance Construction */
{
    moko_debug( "moko_menu_box_init" );
    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    priv->menubar_l = NULL;
    priv->menubar_r = NULL;

}

GtkWidget* moko_menu_box_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_menu_box_get_type(), NULL));
}

void moko_menu_box_clear(MokoMenuBox* f) /* Destruction */
{
    /* destruct your widgets here */
}

static gboolean cb_button_release(GtkWidget *widget, GdkEventButton *event, GtkMenu* menu)
{
    moko_debug( "menu open forwarder: clicked on %f, %f", event->x, event->y );
    moko_debug( "menu open forwarder: clicked on window %p, whereas our window is %p", event->window, widget->window );

    if ( event->window != widget->window ) return FALSE;

    if ( !GTK_WIDGET_VISIBLE(menu) )
    {
        moko_debug( "menu open forwarder: not yet open -- popping up" );
        /* this is kind of funny, if you don't add the grab manually,
           then Gtk+ won't recognize the next click (selection) */
        /* FIXME: check this is still needed ... */
        gtk_grab_add( widget );
        gtk_menu_shell_select_first( GTK_MENU_SHELL(widget), TRUE );
        return TRUE;
    }
    else
    {
        moko_debug( "menu open forwarder: already open -- ignoring" );
        gtk_menu_popdown( menu );
        return FALSE;
    }
    moko_debug( "menu open forwarder: out of bounds" );
    return FALSE;
}

static void cb_filter_menu_update( GtkMenu* menu, MokoMenuBox* self )
{
    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    gchar* text;
    GtkWidget* item = gtk_menu_get_active( menu );
    if (GTK_BIN(item)->child)
    {
        GtkWidget *child = GTK_BIN(item)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_get(GTK_LABEL (child), &text);
        moko_debug(" selection done. menu item text: %s", text );
    }
    if (GTK_BIN(priv->filteritem)->child)
    {
        GtkWidget *child = GTK_BIN(priv->filteritem)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_set(GTK_LABEL (child), text);
        moko_debug(" selection done. menu label updated." );
    }

    g_signal_emit( G_OBJECT(self), moko_menu_box_signals[FILTER_CHANGED], 0, text );
}

static void cb_menu_size_request( GtkWidget* widget, GtkRequisition* requisition, MokoMenuBox* self )
{
    // force popup menus to open with a certain width as per designer's request. See bug #130
    GtkAllocation* a = &( GTK_WIDGET(self) )->allocation;
    moko_debug( "size request of menu = %d / %d -- forcing width to %d", requisition->width, requisition->height, a->width / 2.5 );
    if ( requisition->width != a->width / 2.5 )
        requisition->width = a->width / 2.5;
}

void moko_menu_box_set_application_menu(MokoMenuBox* self, GtkMenu* menu)
{
    moko_debug( "moko_menu_box_set_application_menu" );

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->menubar_l )
    {
        priv->menubar_l = gtk_menu_bar_new();
        gtk_widget_set_name( priv->menubar_l, "mokomenubox-application-menubar" );
        gtk_box_pack_start( GTK_BOX(self), priv->menubar_l, TRUE, TRUE, 0 );

    }
    GtkWidget* appitem = gtk_image_menu_item_new_with_label( g_get_application_name() );
    GtkWidget* appicon = gtk_image_new_from_stock( "openmoko-application-menu-icon", GTK_ICON_SIZE_MENU );
    gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(appitem), appicon );
    gtk_widget_set_name( GTK_WIDGET(appitem), "transparent" );
    priv->appitem = appitem;
    priv->appmenu = GTK_WIDGET(menu);
    g_signal_connect( G_OBJECT(menu), "size-request", G_CALLBACK(cb_menu_size_request), self );
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(appitem), GTK_WIDGET(menu) );
    gtk_menu_shell_append( GTK_MENU_SHELL(priv->menubar_l), appitem );

    //FIXME hack to popup the first menu if user clicks on menubar
    g_signal_connect( priv->menubar_l, "button-press-event", G_CALLBACK(cb_button_release), menu );
}

void moko_menu_box_set_filter_menu(MokoMenuBox* self, GtkMenu* menu)
{
    moko_debug( "moko_menu_box_set_filter_menu" );
    GtkWidget* filtitem;

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);
    if (!priv->menubar_r )
    {
        priv->menubar_r = gtk_menu_bar_new();
        gtk_widget_set_name( priv->menubar_r, "mokomenubox-filter-menubar" );
        gtk_box_pack_end( GTK_BOX(self), priv->menubar_r, TRUE, TRUE, 0 );

        filtitem = gtk_image_menu_item_new_with_label( "Filter Menu" );
        GtkWidget* filticon = gtk_image_new_from_stock( "openmoko-filter-menu-icon", GTK_ICON_SIZE_MENU );
        gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(filtitem), filticon );
        gtk_widget_set_name( filtitem, "transparent" );
        priv->filteritem = filtitem;
        priv->filtermenu = GTK_WIDGET(menu);
        g_signal_connect( G_OBJECT(menu), "size-request", G_CALLBACK(cb_menu_size_request), self );
        gtk_menu_shell_append( GTK_MENU_SHELL(priv->menubar_r), priv->filteritem );
        gtk_menu_item_set_submenu( GTK_MENU_ITEM(priv->filteritem), priv->filtermenu );
    }
    priv->filtermenu = GTK_WIDGET (menu);
    gtk_menu_item_set_submenu( GTK_MENU_ITEM (priv->filteritem), priv->filtermenu );
    g_signal_connect (G_OBJECT(menu), "selection_done", G_CALLBACK(cb_filter_menu_update), self );
    //FIXME hack to popup the first menu if user clicks on menubar
    g_signal_connect( priv->menubar_r, "button-press-event", G_CALLBACK(cb_button_release), menu );
}

void moko_menu_box_set_active_filter(MokoMenuBox* self, gchar* text)
{
    //FIXME this only works with text labels
    moko_debug( "moko_menu_box_set_active_filter" );

    // wander through all filter menu items, check their labels
    // if one is matching, then select it

    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    guint index = 0;
    GList* child = gtk_container_get_children( GTK_CONTAINER(priv->filtermenu) );
    while (child && GTK_IS_MENU_ITEM(child->data) )
    {
        GtkWidget *label = GTK_BIN(child->data)->child;
        if ( !label )
        {
            ++index;
            child = g_list_next(child);
            continue;
        }
        g_assert( GTK_IS_LABEL(label) );
        gchar* ltext;
        gtk_label_get( GTK_LABEL(label), &ltext );
        moko_debug( "moko_menu_box_set_active_filter: comparing '%s' with '%s'", ltext, text );
        if ( ltext && text && strcmp( ltext, text ) == 0 )
        {
            moko_debug( "moko_menu_box_set_active_filter: match found" );
            //FIXME this is a bit hackish or is it?
            gtk_menu_set_active( GTK_MENU(priv->filtermenu), index );
            cb_filter_menu_update( GTK_MENU (priv->filtermenu), self ); //need to sync. manually, since we it didn't go through popupmenu
            break;
        }
        ++index;
        child = g_list_next(child);
    }
    if (!child)
        g_warning( "moko_menu_box_set_active_filter: filter menu entry '%s' not found", text );
}

GtkWidget* moko_menu_box_get_filter_item (MokoMenuBox* self)
{
    MokoMenuBoxPriv* priv = MOKO_MENU_BOX_GET_PRIVATE(self);

    return priv->filteritem;
}

