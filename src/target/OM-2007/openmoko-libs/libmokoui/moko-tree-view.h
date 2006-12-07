/*  moko-tree-view.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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

#ifndef _MOKO_TREE_VIEW_H_
#define _MOKO_TREE_VIEW_H_

#include <gtk/gtktreeview.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtkscrolledwindow.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_TREE_VIEW moko_tree_view_get_type()
#define MOKO_TREE_VIEW(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_TREE_VIEW, MokoTreeView))
#define MOKO_TREE_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_TREE_VIEW, MokoTreeViewClass))
#define MOKO_IS_TREE_VIEW(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_TREE_VIEW))
#define MOKO_IS_TREE_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_TREE_VIEW))
#define MOKO_TREE_VIEW_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_TREE_VIEW, MokoTreeViewClass))

typedef struct {
    GtkTreeView parent;
} MokoTreeView;

typedef struct {
    GtkTreeViewClass parent_class;
} MokoTreeViewClass;

GType moko_tree_view_get_type (void);
GtkWidget* moko_tree_view_new (void);
GtkWidget* moko_tree_view_new_with_model(GtkTreeModel* model);

void moko_tree_view_append_column(MokoTreeView* self, GtkTreeViewColumn* column);
GtkScrolledWindow* moko_tree_view_put_into_scrolled_window(MokoTreeView* self);

G_END_DECLS

#endif // _MOKO_TREE_VIEW_H_

