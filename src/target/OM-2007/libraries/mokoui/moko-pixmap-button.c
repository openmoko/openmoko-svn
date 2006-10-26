/*  moko-pixmap-button.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "moko-pixmap-button.h"

G_DEFINE_TYPE (MokoPixmapButton, moko_pixmap_button, GTK_TYPE_BUTTON);

#define PIXMAP_BUTTON_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_PIXMAP_BUTTON, MokoPixmapButtonPrivate))
#define CHILD_SPACING 1

typedef struct _MokoPixmapButtonPrivate MokoPixmapButtonPrivate;

struct _MokoPixmapButtonPrivate
{
};

static void
moko_pixmap_button_size_request (GtkWidget *widget, GtkRequisition *requisition);

static void
moko_pixmap_button_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_pixmap_button_parent_class)->dispose)
        G_OBJECT_CLASS (moko_pixmap_button_parent_class)->dispose (object);
}

static void
moko_pixmap_button_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_pixmap_button_parent_class)->finalize (object);
}

static void
moko_pixmap_button_class_init (MokoPixmapButtonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoPixmapButtonPrivate));

    /* hook virtual methods */
    widget_class->size_request = moko_pixmap_button_size_request;

    /* install properties */
    gtk_widget_class_install_style_property( widget_class, g_param_spec_boxed(
        "size-request",
        "Size Request",
        "Sets the widget size request to a fixed value",
        GTK_TYPE_BORDER, G_PARAM_READABLE) );

    object_class->dispose = moko_pixmap_button_dispose;
    object_class->finalize = moko_pixmap_button_finalize;
}

static void
moko_pixmap_button_init (MokoPixmapButton *self)
{
    g_debug( "moko_pixmap_button_init" );
    gtk_button_set_focus_on_click( GTK_BUTTON(self), FALSE );
}

MokoPixmapButton*
moko_pixmap_button_new (void)
{
    return g_object_new (MOKO_TYPE_PIXMAP_BUTTON, NULL);
}

static void
moko_pixmap_button_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    g_debug( "moko_pixmap_button_size_request" );
    GtkButton *button = GTK_BUTTON (widget);
    GtkBorder default_border;
    GtkBorder* size_request; // modified
    gint focus_width;
    gint focus_pad;

    //gtk_button_get_props (button, &default_border, NULL, NULL);
    gtk_widget_style_get (GTK_WIDGET (widget),
                          "focus-line-width", &focus_width,
                          "focus-padding", &focus_pad,
                          "size-request", &size_request, // modified
                          NULL);

    if ( size_request && size_request->left + size_request->right + size_request->top + size_request->bottom ) // new fixed thing
    {
        g_debug( "moko_pixmap_button_size_request: style requested size = '%d x %d'", size_request->right, size_request->bottom );
        requisition->width = size_request->right;
        requisition->height = size_request->bottom;
    }
    else // old dynamic routine
    {
        requisition->width = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
                GTK_WIDGET (widget)->style->xthickness) * 2;
        requisition->height = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
                GTK_WIDGET (widget)->style->ythickness) * 2;

        if (GTK_WIDGET_CAN_DEFAULT (widget))
        {
            requisition->width += default_border.left + default_border.right;
            requisition->height += default_border.top + default_border.bottom;
        }

        if (GTK_BIN (button)->child && GTK_WIDGET_VISIBLE (GTK_BIN (button)->child))
        {
            GtkRequisition child_requisition;

            gtk_widget_size_request (GTK_BIN (button)->child, &child_requisition);

            requisition->width += child_requisition.width;
            requisition->height += child_requisition.height;
        }

        requisition->width += 2 * (focus_width + focus_pad);
        requisition->height += 2 * (focus_width + focus_pad);
    }
}
