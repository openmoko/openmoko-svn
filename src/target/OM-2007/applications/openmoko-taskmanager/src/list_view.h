/**
 *  list_view.h
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
 
#ifndef _MOKO_TASK_MANAGER_LIST_VIEW_H 
#define _MOKO_TASK_MANAGER_LIST_VIEW_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libmokoui/moko-tree-view.h>

#include "misc.h"

#define COLUMN_NO  3
#define COLUMN_SPACE 20
#define ROW_SPACE  20
#define MARGIN  10

enum{
    TEXT_COL = 0,
    OBJECT_COL,
    PIXBUF_COL,
    MAX_COL
};
G_BEGIN_DECLS
/*MOKOTASKLIST property*/


/*Pango Font spec*/

#define MOKOTASKLIST_TYPE			(list_get_type())
#define MOKOTASKLIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKOTASKLIST_TYPE, MokoTaskList))
#define MOKOTASKLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MOKOTASKLIST_TYPE, MokoTaskListClass))
#define IS_MOKOTASKLIST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKOTASKLIST_TYPE))
#define IS_MOKOTASKLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MOKOTASKLIST_TYPE))

typedef struct _MokoTaskList		MokoTaskList;
typedef struct _MokoTaskListClass	MokoTaskListClass;

struct _MokoTaskList
{
	GtkVBox vbox;

	GtkHBox *hbox;
	GtkListStore *list_store;
	GtkWidget *list_view;
	GtkWidget *scrolled;
};

struct _MokoTaskListClass
{
	GtkVBoxClass parent_class;
	void(*list) (MokoTaskList *l);
};

GType list_get_type (void);

GtkWidget* 
list_new();

void 
moko_update_store_list (Display *dpy, GtkListStore *list_store);

void 
moko_set_list_highlight (Display *dpy, MokoTaskList *l) ;

G_END_DECLS

#endif /*_MOKO_TASK_MANAGER_LIST_VIEW_H*/
