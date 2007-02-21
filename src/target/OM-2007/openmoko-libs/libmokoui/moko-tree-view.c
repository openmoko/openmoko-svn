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

G_DEFINE_TYPE (MokoTreeView, moko_tree_view, GTK_TYPE_TREE_VIEW);

#define TREE_VIEW_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_TREE_VIEW, MokoTreeViewPrivate))

typedef struct _MokoTreeViewPrivate
{
} MokoTreeViewPrivate;

/* forward declarations */
/* ... */

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
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoTreeViewPrivate));

    /* hook virtual methods */
    /* ... */

    /* install properties */
    /* ... */

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

void moko_tree_view_append_column(MokoTreeView* self, GtkTreeViewColumn* column)
{
    gtk_tree_view_column_set_alignment( column, 0.5 );
    gtk_tree_view_column_set_spacing( column, 4 );
    gtk_tree_view_append_column( GTK_TREE_VIEW(self), column );

    g_object_set( G_OBJECT(column),
                  "resizable", TRUE,
                  "reorderable", TRUE,
                  "sort-indicator", TRUE,
                  NULL );
}

GtkWidget* moko_tree_view_put_into_scrolled_window(MokoTreeView* self)
{
    GtkWidget* scrolledwindow = gtk_scrolled_window_new( NULL, NULL );
    //FIXME get from style or (even better) set as initial size hint in MokoPanedWindow (also via style sheet of course)
    gtk_widget_set_size_request( scrolledwindow, 0, 170 );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
    gtk_container_add( GTK_CONTAINER (scrolledwindow), GTK_WIDGET (self) );
    return scrolledwindow;
}
