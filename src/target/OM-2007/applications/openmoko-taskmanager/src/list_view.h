/**
 * @file list_view.h
 * @brief list_view.h based on gtk+-2.0.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
 #ifndef OPENMOKO_LIST_VIEW_H
 #define OPENMOKO_LIST_VIEW_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libmokoui/moko-tree-view.h>


#include "misc.h"

G_BEGIN_DECLS
/*LIST property*/

/*Pango Font spec*/

#define LIST_TYPE			(list_get_type())
#define LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), LIST_TYPE, List))
#define LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), LIST_TYPE, ListClass))
#define IS_LIST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), LIST_TYPE))
#define IS_LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), LIST_TYPE))

typedef struct _List		List;
typedef struct _ListClass	ListClass;

struct _List
{
	GtkVBox vbox;

	GtkHBox *hbox;
	GtkButton *btn_close;
	GtkListStore *list_store;
	GtkWidget *list_view;
	GtkWidget *scrolled;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	//MokoTreeView *mokolist_view;

	/*temporary */
	GtkButton *tab, *tabhold;
};

struct _ListClass
{
	GtkVBoxClass parent_class;
	void(*list) (List *l);
};

GType list_get_type (void);

G_END_DECLS
 #endif /*list_view.h*/
