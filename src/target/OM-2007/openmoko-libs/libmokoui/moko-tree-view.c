/*  moko-tree-view.c
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

#include "moko-tree-view.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(...)
#endif

G_DEFINE_TYPE (MokoTreeView, moko_tree_view, GTK_TYPE_TREE_VIEW)

#define TREE_VIEW_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_TREE_VIEW, MokoTreeViewPrivate))

/* forward declarations */
void moko_tree_view_size_request(GtkWidget* widget, GtkRequisition* requisition);

static void
moko_tree_view_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_tree_view_parent_class)->dispose)
        G_OBJECT_CLASS (moko_tree_view_parent_class)->dispose (object);
}

static void
moko_tree_view_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_tree_view_parent_class)->finalize (object);
}

static void
moko_tree_view_class_init (MokoTreeViewClass *klass)
{
    /* register private data */

    /* hook virtual methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->size_request = moko_tree_view_size_request;

    /* install properties */
    /* ... */

    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = moko_tree_view_dispose;
    object_class->finalize = moko_tree_view_finalize;
}

static void
moko_tree_view_init (MokoTreeView *self)
{
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(self), TRUE );
    gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(self), TRUE );
}

GtkWidget*
moko_tree_view_new (void)
{
    return GTK_WIDGET(g_object_new(moko_tree_view_get_type(), NULL));
}

GtkWidget*
moko_tree_view_new_with_model (GtkTreeModel *model)
{
    return GTK_WIDGET(g_object_new(moko_tree_view_get_type(), "model", model, NULL));
}

/* reimplemented to enforce entry-height granularity */
void moko_tree_view_size_request(GtkWidget* widget, GtkRequisition* requisition)
{
    moko_debug( "moko_tree_view_size_request" );
    GTK_WIDGET_CLASS(moko_tree_view_parent_class)->size_request( widget, requisition );
#if 0 /* it doesn't work with size_request... should we try overwriting size_alloc? */
    MokoTreeView* self = MOKO_TREE_VIEW(widget);
    moko_debug( "-- [old] requesting %d, %d", requisition->width, requisition->height );

    // compute height as a whole-number factor of the cell height
    GtkTreeViewColumn* first_column = gtk_tree_view_get_column( GTK_TREE_VIEW(self), 0 );
    g_assert( first_column ); // fail here if no columns added yet
    gint cr_x_offset;
    gint cr_y_offset;
    gint cr_width;
    gint cr_height;
    gtk_tree_view_column_cell_get_size( first_column, NULL, &cr_x_offset, &cr_y_offset, &cr_width, &cr_height );

    // prevent half cells to be visible
    requisition->height -= requisition->height % (cr_height+2);
    moko_debug( "-- [old] requesting %d, %d", requisition->width, requisition->height );
#endif
}

void moko_tree_view_append_column(MokoTreeView* self, GtkTreeViewColumn* column)
{
    gtk_tree_view_column_set_alignment( column, 0.5 );
    gtk_tree_view_column_set_spacing( column, 4 );
    gtk_tree_view_column_set_sizing( column, GTK_TREE_VIEW_COLUMN_FIXED );
    //FIXME grab from state information
    gtk_tree_view_column_set_fixed_width( column, 100 );
    gtk_tree_view_append_column( GTK_TREE_VIEW(self), column );

    g_object_set( G_OBJECT(column),
                  "resizable", TRUE,
                  "reorderable", FALSE,
                  "sort-indicator", TRUE,
                  NULL );

}

void _moko_tree_view_adjustment_changed(GtkAdjustment* adj, MokoTreeView* self)
{
    // compute value for vadjustment
    gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(self), TRUE );
    GtkTreeViewColumn* first_column = gtk_tree_view_get_column( GTK_TREE_VIEW(self), 0 );
    g_assert( first_column ); // fail here if no columns added yet
    gint cr_x_offset;
    gint cr_y_offset;
    gint cr_width;
    gint cr_height;
    gtk_tree_view_column_cell_get_size( first_column, NULL, &cr_x_offset, &cr_y_offset, &cr_width, &cr_height );

    //FIXME why does it need the +2 ?
    if ( adj->step_increment != cr_height+2 )
        adj->step_increment = cr_height+2;
    gint pageinc = cr_height * ( ( adj->page_size / cr_height ) -1 );
    if ( adj->page_increment != pageinc )
        adj->page_increment = pageinc;

    g_debug( "new vadjustment step increment = %f", adj->step_increment );
    g_debug( "new vadjustment page increment = %f", adj->page_increment );
    g_debug( "new vadjustment page size = %f", adj->page_size );

}

GtkWidget* moko_tree_view_put_into_scrolled_window(MokoTreeView* self)
{
    GtkWidget* scrolledwindow = gtk_scrolled_window_new( NULL, NULL );
    //FIXME get from style or (even better) set as initial size hint in MokoPanedWindow (also via style sheet of course)
    gtk_widget_set_size_request( scrolledwindow, 0, 170 );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
    gtk_container_add( GTK_CONTAINER(scrolledwindow), GTK_WIDGET(self) );
    GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(scrolledwindow) );
    g_signal_connect( G_OBJECT(adj), "changed", G_CALLBACK(_moko_tree_view_adjustment_changed), self );
    return scrolledwindow;
}
