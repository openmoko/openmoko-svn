/*  moko-alignment.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Based on GtkAlignment which is (C) 1997-2000 GTK+ Team
 *
 *  Copyright (C) 2006 Vanille-Media
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-alignment.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoAlignment, moko_alignment, GTK_TYPE_ALIGNMENT)

#define MOKO_ALIGNMENT_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_ALIGNMENT, MokoAlignmentPrivate))

static GtkAlignmentClass* parent_class = NULL;

//FIXME this is a bit hackish
#define GTK_ALIGNMENT_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_ALIGNMENT, GtkAlignmentPrivate))
typedef struct _GtkAlignmentPrivate
{
  guint padding_top;
  guint padding_bottom;
  guint padding_left;
  guint padding_right;
};

typedef struct _MokoAlignmentPrivate
{
} MokoAlignmentPrivate;

/* forward declarations */
static void moko_alignment_realize(GtkWidget* widget);
static void moko_alignment_size_request(GtkWidget* widget, GtkRequisition* requisition);
static void moko_alignment_size_allocate(GtkWidget* widget, GtkAllocation* allocation);


static void
moko_alignment_class_init (MokoAlignmentClass *klass)
{
    /* hook parent */
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoAlignmentPrivate));

    /* hook virtual methods */
    widget_class->realize = moko_alignment_realize;
    widget_class->size_request = moko_alignment_size_request;
    widget_class->size_allocate = moko_alignment_size_allocate;

    /* install properties */
    gtk_widget_class_install_style_property( widget_class, g_param_spec_boxed(
    "padding",
    "Alignment Padding",
    "Sets the padding to a fixed value",
    GTK_TYPE_BORDER, G_PARAM_READABLE) );
}

static void
moko_alignment_init (MokoAlignment *self)
{
    //FIXME do we need this?
    //gtk_widget_set_redraw_on_allocate( GTK_WIDGET(self), TRUE );
    GTK_WIDGET_UNSET_FLAGS( GTK_WIDGET(self), GTK_NO_WINDOW );
}

GtkWidget*
moko_alignment_new (void)
{
    return GTK_WIDGET(g_object_new(moko_alignment_get_type(), NULL));
}

static void moko_alignment_size_request(GtkWidget* widget, GtkRequisition* requisition)
{
    moko_debug( "moko_alignment_size_request" );
    GtkBorder* padding = NULL;

    gtk_widget_style_get(widget,
                         "padding", &padding,
                         NULL);

    GtkAlignmentPrivate *priv = GTK_ALIGNMENT_GET_PRIVATE(widget);
    if ( padding )
    {
        priv->padding_top = padding->top;
        priv->padding_bottom = padding->bottom;
        priv->padding_left = padding->left;
        priv->padding_right = padding->right;
    }

    GtkBin* bin = GTK_BIN(widget);

    requisition->width = GTK_CONTAINER (widget)->border_width * 2;
    requisition->height = GTK_CONTAINER (widget)->border_width * 2;

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        GtkRequisition child_requisition;

        gtk_widget_size_request (bin->child, &child_requisition);

        requisition->width += child_requisition.width;
        requisition->height += child_requisition.height;

        /* Request extra space for the padding: */
        requisition->width += (priv->padding_left + priv->padding_right);
        requisition->height += (priv->padding_top + priv->padding_bottom);
    }
}

static void
moko_alignment_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    moko_debug( "moko_alignment_size_allocate" );
    GtkAlignment *alignment;
    GtkBin *bin;
    GtkAllocation child_allocation;
    GtkRequisition child_requisition;
    gint width, height;
    gint border_width;
    gint padding_horizontal, padding_vertical;
    GtkAlignmentPrivate *priv;

    padding_horizontal = 0;
    padding_vertical = 0;

    widget->allocation = *allocation;

    // <sync. with gdk window>
    if (GTK_WIDGET_REALIZED (widget) &!GTK_WIDGET_NO_WINDOW (widget))
        gdk_window_move_resize (widget->window,
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    // <sync with GdkWindow>

    alignment = GTK_ALIGNMENT (widget);
    bin = GTK_BIN (widget);

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        gtk_widget_get_child_requisition (bin->child, &child_requisition);

        border_width = GTK_CONTAINER (alignment)->border_width;

        priv = GTK_ALIGNMENT_GET_PRIVATE (widget);
        padding_horizontal = priv->padding_left + priv->padding_right;
        padding_vertical = priv->padding_top + priv->padding_bottom;

        width = allocation->width - padding_horizontal - 2 * border_width;
        height = allocation->height - padding_vertical - 2 * border_width;

        if (width > child_requisition.width)
            child_allocation.width = (child_requisition.width *
                    (1.0 - alignment->xscale) +
                    width * alignment->xscale);
        else
            child_allocation.width = width;

        if (height > child_requisition.height)
            child_allocation.height = (child_requisition.height *
                    (1.0 - alignment->yscale) +
                    height * alignment->yscale);
        else
            child_allocation.height = height;

        if (GTK_WIDGET_NO_WINDOW (widget))
        {
            if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
                child_allocation.x = (1.0 - alignment->xalign) * (width - child_allocation.width) + allocation->x + border_width + priv->padding_right;
            else
                child_allocation.x = alignment->xalign * (width - child_allocation.width) + allocation->x + border_width + priv->padding_left;

            child_allocation.y = alignment->yalign * (height - child_allocation.height) + allocation->y + border_width + priv->padding_top;
        }
        else
        {
            if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
                child_allocation.x = (1.0 - alignment->xalign) * (width - child_allocation.width) + border_width + priv->padding_right;
            else
                child_allocation.x = alignment->xalign * (width - child_allocation.width) + border_width + priv->padding_left;

            child_allocation.y = alignment->yalign * (height - child_allocation.height) + border_width + priv->padding_top;
        }
        gtk_widget_size_allocate (bin->child, &child_allocation);
    }
}

static void
moko_alignment_realize(GtkWidget* widget)
{
    moko_debug( "moko_alignment_realize" );

    GdkWindowAttr attributes;
    gint attributes_mask;

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

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    //FIXME find out why a pixmap engine doesn't want to draw on an Alignment even though it has a GdkWindow
    //Answer: because the background pixmap gets drawn by the X server, no one calls gtk_paint_flat_box or so
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}
