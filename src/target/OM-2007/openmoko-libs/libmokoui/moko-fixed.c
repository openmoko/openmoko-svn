
/*  moko_fixed.c
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-fixed.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoFixed, moko_fixed, GTK_TYPE_FIXED)

#define PIXMAP_CONTAINER_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FIXED, MokoFixedPrivate))

typedef struct _MokoFixedPrivate MokoFixedPrivate;

struct _MokoFixedPrivate
{
};

static GtkFixedClass *parent_class = NULL;

/* declare virtual methods */
static void moko_fixed_realize(GtkWidget *widget);
static void moko_fixed_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void moko_fixed_size_allocate(GtkWidget *widget, GtkAllocation *allocation);

static void
moko_fixed_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_fixed_parent_class)->dispose)
    G_OBJECT_CLASS (moko_fixed_parent_class)->dispose (object);
}

static void
moko_fixed_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_fixed_parent_class)->finalize (object);
}

static void
moko_fixed_class_init (MokoFixedClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    /* get pointer to parent */
    parent_class = g_type_class_peek_parent(klass);

    /* add private data */
    g_type_class_add_private (klass, sizeof (MokoFixedPrivate));

    /* hook virtual methods */
    widget_class->realize = moko_fixed_realize;
    widget_class->size_request = moko_fixed_size_request;
    widget_class->size_allocate = moko_fixed_size_allocate;

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
moko_fixed_init(MokoFixed *self)
{
    moko_debug( "moko_fixed_init" );
    gtk_fixed_set_has_window( GTK_FIXED(self), TRUE );
}

GtkWidget*
moko_fixed_new (void)
{
    return GTK_WIDGET(g_object_new(moko_fixed_get_type(), NULL));
}

static void
moko_fixed_realize(GtkWidget *widget)
{
    moko_debug( "moko_fixed_realize" );

    GdkWindowAttr attributes;
    gint attributes_mask;

    if (GTK_WIDGET_NO_WINDOW (widget))
        GTK_WIDGET_CLASS (parent_class)->realize (widget);
    else
    {
        GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.x = widget->allocation.x;
        attributes.y = widget->allocation.y;
        attributes.width = widget->allocation.width;
        attributes.height = widget->allocation.height;
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = gtk_widget_get_events (widget);
        attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK;

        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

        widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes,
                                        attributes_mask);
        gdk_window_set_user_data (widget->window, widget);

        widget->style = gtk_style_attach (widget->style, widget->window);
        gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
    }
}

static void
moko_fixed_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    moko_debug( "moko_fixed_size_request" );

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
             && child->x == -1 && child->y == -1 )
        {
            moko_debug( "moko_fixed_set_cargo: style requested cargo = '%d, %d x %d, %d'", size_request->left, size_request->top, size_request->right, size_request->bottom );
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

    if ( size_request && size_request->left + size_request->right + size_request->top + size_request->bottom )
    {
        moko_debug( "moko_fixed_size_request: style requested size = '%d x %d'", size_request->right, size_request->bottom );
        requisition->height = MAX( requisition->height, size_request->bottom );
        requisition->width = MAX( requisition->height, size_request->right );
    }
}

static void
moko_fixed_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    moko_debug( "moko_fixed_size_allocate" );
    GtkFixed *fixed;
    GtkFixedChild *child;
    GtkAllocation child_allocation;
    GtkRequisition child_requisition;
    GList *children;
    guint16 border_width;

    fixed = GTK_FIXED (widget);

    widget->allocation = *allocation;

    moko_debug( "widget allocation is: %d %d, %d %d", allocation->x,
                                    allocation->y,
                                    allocation->width,
                                    allocation->height);

    if (!GTK_WIDGET_NO_WINDOW (widget))
    {
        if (GTK_WIDGET_REALIZED (widget))
            gdk_window_move_resize (widget->window,
                                    allocation->x,
                                    allocation->y,
                                    allocation->width,
                                    allocation->height);
    }

    border_width = GTK_CONTAINER (fixed)->border_width;

    children = fixed->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        if (GTK_WIDGET_VISIBLE (child->widget))
        {
            gtk_widget_get_child_requisition (child->widget, &child_requisition);
            child_allocation.x = child->x + border_width;
            child_allocation.y = child->y + border_width;

            if (GTK_WIDGET_NO_WINDOW (widget))
            {
                child_allocation.x += widget->allocation.x;
                child_allocation.y += widget->allocation.y;
            }

            child_allocation.width = child_requisition.width;
            child_allocation.height = child_requisition.height;
            gtk_widget_size_allocate (child->widget, &child_allocation);
        }
    }
}

void
moko_fixed_set_cargo(MokoFixed* self, GtkWidget* child)
{
    gtk_fixed_put( GTK_FIXED(self), child, -1, -1 );
}
