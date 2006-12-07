/*  moko-navigation-list.h
 *
 *  Authored By Ken Zhao <ken_zhao@fic-sh.com.cn>
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
 *  Current Version: $Rev$ ($Date: 2006/11/28 17:38:14 $) [$Author: Ken $]
 */

#ifndef _MOKO_NAVIGATION_LIST_H_
#define _MOKO_NAVIGATION_LIST_H_

#include "moko-tree-view.h"

#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkviewport.h>
#include <gtk/gtkfixed.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkliststore.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_NAVIGATION_LIST moko_navigation_list_get_type()
#define MOKO_NAVIGATION_LIST(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_NAVIGATION_LIST, MokoTreeView))
#define MOKO_NAVIGATION_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_NAVIGATION_LIST, MokoTreeViewClass))
#define MOKO_IS_NAVIGATION_LIST(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_NAVIGATION_LIST))
#define MOKO_IS_NAVIGATION_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_NAVIGATION_LIST))
#define MOKO_NAVIGATION_LIST_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_NAVIGATION_LIST, MokoTreeViewClass))

typedef struct {
    GtkViewport parent;
} MokoNavigationList;

typedef struct {
    GtkViewportClass parent_class;
} MokoNavigationListClass;

GType moko_navigation_list_get_type (void);
MokoNavigationList* moko_navigation_list_new (void);
MokoNavigationList* moko_navigation_list_new_with_model (GtkTreeModel *model);

void moko_navigation_list_append_column (MokoNavigationList* self, GtkTreeViewColumn* column);
MokoTreeView* moko_navigation_list_get_tree_view(MokoNavigationList* self);

G_END_DECLS

#endif // _MOKO_NAVIGATION_LIST_H_

