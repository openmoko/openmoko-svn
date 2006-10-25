
/*  moko_pixmap_container.c
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

#include "moko-pixmap-container.h"

G_DEFINE_TYPE (MokoPixmapContainer, moko_pixmap_container, GTK_TYPE_FIXED);

#define PIXMAP_CONTAINER_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_PIXMAP_CONTAINER, MokoPixmapContainerPrivate))

typedef struct _MokoPixmapContainerPrivate MokoPixmapContainerPrivate;

struct _MokoPixmapContainerPrivate
{
};

static GtkFixedClass *parent_class = NULL;

/* virtual methods */

static void
moko_pixmap_container_size_request(GtkWidget *widget, GtkRequisition *requisition);

static void
moko_pixmap_container_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_pixmap_container_parent_class)->dispose)
    G_OBJECT_CLASS (moko_pixmap_container_parent_class)->dispose (object);
}

static void
moko_pixmap_container_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_pixmap_container_parent_class)->finalize (object);
}

static void
moko_pixmap_container_class_init (MokoPixmapContainerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    /* get pointer to parent */
    parent_class = g_type_class_peek_parent(klass);

    /* add private data */
    g_type_class_add_private (klass, sizeof (MokoPixmapContainerPrivate));

    /* hook virtual methods */
    widget_class->size_request = moko_pixmap_container_size_request;

    /* install properties */
    gtk_widget_class_install_style_property( widget_class, g_param_spec_boxed(
        "size-request",
        "Size Request",
        "Sets the widget size request to a fixed value",
        GTK_TYPE_BORDER, G_PARAM_READABLE) );

    gtk_widget_class_install_style_property( widget_class, g_param_spec_boxed(
        "cargo-border",
        "Cargo Border",
        "Position and Size of the cargo element",
        GTK_TYPE_BORDER, G_PARAM_READABLE) );
}

static void
moko_pixmap_container_init (MokoPixmapContainer *self)
{
    g_debug( "moko_pixmap_container_init" );
    gtk_fixed_set_has_window( self, TRUE );
}

MokoPixmapContainer*
moko_pixmap_container_new (void)
{
  return g_object_new (MOKO_TYPE_PIXMAP_CONTAINER, NULL);
}

static void
moko_pixmap_container_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    g_debug( "moko_pixmap_container_size_request" );

    GtkBorder* size_request;
    GtkBorder* cargo_border;
    gtk_widget_style_get(widget, "size-request", &size_request,
                                 "cargo-border", &cargo_border, NULL );


    GtkFixed *fixed;
    GtkFixedChild *child;
    GList *children;
    GtkRequisition child_requisition;

    fixed = GTK_FIXED (widget);
    requisition->width = 0;
    requisition->height = 0;

    children = fixed->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        if ( cargo_border && cargo_border->left + cargo_border->right + cargo_border->top + cargo_border->bottom
             && !child->x && !child->y )
        {
            g_warning( "moko_pixmap_container_set_cargo: style requested cargo = '%d, %d x %d, %d'", size_request->left, size_request->top, size_request->right, size_request->bottom );
            gtk_widget_set_size_request( child->widget, cargo_border->right - cargo_border->left, cargo_border->bottom - cargo_border->top );
            child->x = cargo_border->left;
            child->y = cargo_border->top;
        }

        if (GTK_WIDGET_VISIBLE(child->widget))
        {
            gtk_widget_size_request(child->widget, &child_requisition);

            requisition->height = MAX (requisition->height,
                                        child->y +
                                        child_requisition.height);
            requisition->width = MAX (requisition->width,
                                        child->x +
                                        child_requisition.width);
        }
    }

    requisition->height += GTK_CONTAINER(fixed)->border_width * 2;
    requisition->width += GTK_CONTAINER(fixed)->border_width * 2;

    if ( size_request->left + size_request->right + size_request->top + size_request->bottom )
    {
        g_warning( "moko_pixmap_container_size_request: style requested size = '%d x %d'", size_request->right, size_request->bottom );
        requisition->height = MAX( requisition->height, size_request->bottom );
        requisition->width = MAX( requisition->height, size_request->right );
    }
}

void
moko_pixmap_container_set_cargo(MokoPixmapContainer* self, GtkWidget* child)
{
    gtk_fixed_put( GTK_FIXED(self), child, 0, 0 );
}
