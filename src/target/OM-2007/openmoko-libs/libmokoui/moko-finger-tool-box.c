/*  moko-finger-tool-box.c
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

#include "moko-finger-tool-box.h"

#include "moko-pixmap-button.h"

#include <gtk/gtkscrolledwindow.h>

G_DEFINE_TYPE (MokoFingerToolBox, moko_finger_tool_box, GTK_TYPE_HBOX)

#define MOKO_FINGER_TOOL_BOX_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBoxPrivate))

typedef struct _MokoFingerToolBoxPrivate
{
    MokoPixmapButton* leftarrow;
    GtkScrolledWindow* toolwindow;
    GtkHBox* hbox;
    MokoPixmapButton* rightarrow;
} MokoFingerToolBoxPrivate;

static void moko_finger_tool_box_size_request  (GtkWidget      *widget,
                    GtkRequisition *requisition);
static void moko_finger_tool_box_size_allocate (GtkWidget      *widget,
                    GtkAllocation  *allocation);

static void
moko_finger_tool_box_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->dispose (object);
}

static void
moko_finger_tool_box_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->finalize (object);
}

static void
moko_finger_tool_box_class_init (MokoFingerToolBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoFingerToolBoxPrivate));

    GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;

    widget_class->size_request = moko_finger_tool_box_size_request;
    widget_class->size_allocate = moko_finger_tool_box_size_allocate;

    /* install properties */
    /* ... */

    object_class->dispose = moko_finger_tool_box_dispose;
    object_class->finalize = moko_finger_tool_box_finalize;
}

static void
moko_finger_tool_box_init (MokoFingerToolBox *self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    priv->leftarrow = MOKO_PIXMAP_BUTTON( moko_pixmap_button_new() );
    gtk_widget_set_name( GTK_WIDGET(priv->leftarrow), "mokofingertoolbox-leftarrow" );
    priv->rightarrow = MOKO_PIXMAP_BUTTON( moko_pixmap_button_new() );
    gtk_widget_set_name( GTK_WIDGET(priv->rightarrow), "mokofingertoolbox-rightarrow" );

    gtk_box_pack_start( GTK_BOX(self), priv->leftarrow, FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(self), priv->toolwindow, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX(self), priv->rightarrow, FALSE, FALSE, 0 );

    gtk_widget_show_all( priv->toolwindow );
}

static void
moko_finger_tool_box_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    GtkBox *box;
    GtkBoxChild *child;
    GList *children;
    gint nvis_children;
    gint width;

    box = GTK_BOX (widget);
    requisition->width = 0;
    requisition->height = 0;
    nvis_children = 0;

    children = box->children;
    while (children)
    {
        g_debug( "current req width = %d", requisition->width );
        child = children->data;
        children = children->next;

        if (GTK_WIDGET_VISIBLE (child->widget))
        {
            GtkRequisition child_requisition;

            gtk_widget_size_request (child->widget, &child_requisition);

            if (box->homogeneous)
            {
                width = child_requisition.width + child->padding * 2;
                requisition->width = MAX (requisition->width, width);
            }
            else
            {
                requisition->width += child_requisition.width + child->padding * 2;
            }

            requisition->height = MAX (requisition->height, child_requisition.height);

            nvis_children += 1;
        }
    }

    if (nvis_children > 0)
    {
        if (box->homogeneous)
            requisition->width *= nvis_children;
        requisition->width += (nvis_children - 1) * box->spacing;
    }

    requisition->width += GTK_CONTAINER (box)->border_width * 2;
    requisition->height += GTK_CONTAINER (box)->border_width * 2;
}

static void
moko_finger_tool_box_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    GtkBox *box;
    GtkBoxChild *child;
    GList *children;
    GtkAllocation child_allocation;
    gint nvis_children;
    gint nexpand_children;
    gint child_width;
    gint width;
    gint extra;
    gint x;
    GtkTextDirection direction;

    box = GTK_BOX (widget);
    widget->allocation = *allocation;

    direction = gtk_widget_get_direction (widget);

    nvis_children = 0;
    nexpand_children = 0;
    children = box->children;

    while (children)
    {
        child = children->data;
        children = children->next;

        if (GTK_WIDGET_VISIBLE (child->widget))
        {
            nvis_children += 1;
            if (child->expand)
                nexpand_children += 1;
        }
    }

    if (nvis_children > 0)
    {
        if (box->homogeneous)
        {
            width = (allocation->width -
                    GTK_CONTAINER (box)->border_width * 2 -
                    (nvis_children - 1) * box->spacing);
            extra = width / nvis_children;
        }
        else if (nexpand_children > 0)
        {
            width = (gint) allocation->width - (gint) widget->requisition.width;
            extra = width / nexpand_children;
        }
        else
        {
            width = 0;
            extra = 0;
        }

        x = allocation->x + GTK_CONTAINER (box)->border_width;
        child_allocation.y = allocation->y + GTK_CONTAINER (box)->border_width;
        child_allocation.height = MAX (1, (gint) allocation->height - (gint) GTK_CONTAINER (box)->border_width * 2);

        children = box->children;
        while (children)
        {
            child = children->data;
            children = children->next;

            if ((child->pack == GTK_PACK_START) && GTK_WIDGET_VISIBLE (child->widget))
            {
                if (box->homogeneous)
                {
                    if (nvis_children == 1)
                        child_width = width;
                    else
                        child_width = extra;

                    nvis_children -= 1;
                    width -= extra;
                }
                else
                {
                    GtkRequisition child_requisition;

                    gtk_widget_get_child_requisition (child->widget, &child_requisition);

                    child_width = child_requisition.width + child->padding * 2;

                    if (child->expand)
                    {
                        if (nexpand_children == 1)
                            child_width += width;
                        else
                            child_width += extra;

                        nexpand_children -= 1;
                        width -= extra;
                    }
                }

                if (child->fill)
                {
                    child_allocation.width = MAX (1, (gint) child_width - (gint) child->padding * 2);
                    child_allocation.x = x + child->padding;
                }
                else
                {
                    GtkRequisition child_requisition;

                    gtk_widget_get_child_requisition (child->widget, &child_requisition);
                    child_allocation.width = child_requisition.width;
                    child_allocation.x = x + (child_width - child_allocation.width) / 2;
                }

                if (direction == GTK_TEXT_DIR_RTL)
                    child_allocation.x = allocation->x + allocation->width - (child_allocation.x - allocation->x) - child_allocation.width;

                gtk_widget_size_allocate (child->widget, &child_allocation);

                x += child_width + box->spacing;
            }
        }

        x = allocation->x + allocation->width - GTK_CONTAINER (box)->border_width;

        children = box->children;
        while (children)
        {
            child = children->data;
            children = children->next;

            if ((child->pack == GTK_PACK_END) && GTK_WIDGET_VISIBLE (child->widget))
            {
                GtkRequisition child_requisition;
                gtk_widget_get_child_requisition (child->widget, &child_requisition);

                if (box->homogeneous)
                {
                    if (nvis_children == 1)
                        child_width = width;
                    else
                        child_width = extra;

                    nvis_children -= 1;
                    width -= extra;
                }
                else
                {
                    child_width = child_requisition.width + child->padding * 2;

                    if (child->expand)
                    {
                        if (nexpand_children == 1)
                            child_width += width;
                        else
                            child_width += extra;

                        nexpand_children -= 1;
                        width -= extra;
                    }
                }

                if (child->fill)
                {
                    child_allocation.width = MAX (1, (gint)child_width - (gint)child->padding * 2);
                    child_allocation.x = x + child->padding - child_width;
                }
                else
                {
                    child_allocation.width = child_requisition.width;
                    child_allocation.x = x + (child_width - child_allocation.width) / 2 - child_width;
                }

                if (direction == GTK_TEXT_DIR_RTL)
                    child_allocation.x = allocation->x + allocation->width - (child_allocation.x - allocation->x) - child_allocation.width;

                gtk_widget_size_allocate (child->widget, &child_allocation);

                x -= (child_width + box->spacing);
            }
        }
    }
}

/* public API */

GtkWidget*
moko_finger_tool_box_new (void)
{
    return GTK_WIDGET(g_object_new(moko_finger_tool_box_get_type(), NULL));
}

GtkButton*
moko_finger_tool_box_add_button(MokoFingerToolBox* self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    MokoPixmapButton* b = moko_pixmap_button_new();
    gtk_widget_set_name( GTK_WIDGET(b), "mokofingertoolbox-toolbutton" );

    gtk_box_pack_start( GTK_BOX(self), b, FALSE, FALSE, 10 );
    gtk_widget_show( GTK_WIDGET(b) );
    return b;
}
