/**
 *  list_view.c
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

#include "list_view.h"
#include <gdk/gdk.h>

static GdkPixbuf *default_icon;

enum {
    LIST_SIGNAL,
    LAST_SIGNAL
};

static void moko_task_list_class_init          (MokoTaskListClass *klass);
static void moko_task_list_init                (MokoTaskList *l);

static void moko_task_list_selection_changed (GtkIconView *self, MokoTaskList *l);

static guint list_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun MokoTaskList type.
*@param none
*@return GType
*/
GType moko_task_list_get_type (void) /* Typechecking */
{
    static GType list_type = 0;

    if (!list_type)
    {
        static const GTypeInfo list_info =
        {
            sizeof (MokoTaskListClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_task_list_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoTaskList),
            0,
            (GInstanceInitFunc) moko_task_list_init,
            NULL
        };

        list_type = g_type_register_static (GTK_TYPE_VBOX, "MokoTaskList", &list_info, 0);
    }

    return list_type;
}

/**
*@brief initialize MokoTaskList class.
*@param klass	MokoTaskList Class
*@return none
*/
static void moko_task_list_class_init(MokoTaskListClass * Klass) /* Class Initialization */
{
    list_signals[LIST_SIGNAL] = g_signal_new ("list",
            G_TYPE_FROM_CLASS (Klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoTaskListClass, list),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 
            0);
}

/**
*@brief initialize MokoTaskList UI.
*@param l	MokoTaskList instance
*@return none
*/
static void /* Instance Construction */
moko_task_list_init (MokoTaskList *l) 
{ 
  l->list_view = gtk_icon_view_new();
  gtk_widget_set_name (l->list_view, "gtktreeview-black");
  gtk_icon_view_set_columns (GTK_ICON_VIEW(l->list_view), COLUMN_NO);
  gtk_icon_view_set_margin (GTK_ICON_VIEW(l->list_view), MARGIN);
  gtk_icon_view_set_column_spacing (GTK_ICON_VIEW(l->list_view), COLUMN_SPACE);
  gtk_icon_view_set_row_spacing (GTK_ICON_VIEW(l->list_view), ROW_SPACE);
  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW(l->list_view), GTK_SELECTION_SINGLE);
  gtk_widget_show (l->list_view);
  l->list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_PIXBUF);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW(l->list_view), PIXBUF_COL);
  gtk_icon_view_set_text_column (l->list_view, TEXT_COL );
  gtk_icon_view_set_model (GTK_ICON_VIEW(l->list_view), GTK_TREE_MODEL(l->list_store));

  l->scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (l->scrolled);
  gtk_widget_set_size_request (GTK_WINDOW(l->scrolled), -1, 400);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (l->scrolled), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gtk_container_add (GTK_CONTAINER (l->scrolled), l->list_view);
   
  gtk_box_pack_start (GTK_BOX (l), l->scrolled, TRUE, TRUE, 0);
    
  default_icon = gdk_pixbuf_new_from_file_at_size (PKGDATADIR"/default-app-icon.xpm", 140, 140, NULL);
  if (!default_icon )
    g_error ("Failed to load default icon");

  g_signal_connect (l->list_view, "selection_changed", G_CALLBACK ( moko_task_list_selection_changed), l);
}
/* Construction */
GtkWidget* 
moko_task_list_new() 
{
    return GTK_WIDGET(g_object_new(moko_task_list_get_type(), NULL));
}

/* Destruction */
void 
moko_task_list_clear(MokoTaskList *l) 
{ 
    if (!l) g_free (l);
}

static void
moko_add_window (Display *dpy, Window w, GtkListStore *list_store)
{
  GtkTreeIter iter;
  gchar *name = NULL;
  GdkPixbuf *icon = NULL;

  name = moko_get_window_name(dpy, w);
  if (!strcmp (name, "Openmoko-taskmanager"))
  {
    g_free (name);
    return;
  }

  icon = moko_get_window_icon (dpy, w);
  gtk_list_store_append (list_store, &iter);
  g_debug ("add widnow %s", name);
  gtk_list_store_set (list_store, &iter, TEXT_COL, name, OBJECT_COL, w, -1);

  if (icon) 
  {
    GdkPixbuf *icons = gdk_pixbuf_scale_simple (icon, 140, 140, GDK_INTERP_BILINEAR);
    gtk_list_store_set (list_store, &iter, PIXBUF_COL, icons, -1);
    gdk_pixbuf_unref (icons);
  }
  else if (default_icon) 
    gtk_list_store_set (list_store, &iter, PIXBUF_COL, default_icon, -1);
  else
    g_warning ("Failed to load %s's icon", name);
  
  if (icon)
    gdk_pixbuf_unref (icon);
  g_free (name);
}

void 
moko_update_store_list (Display *dpy, GtkListStore *list_store)
{
    Window *list;
    guint nr, i;
    GtkTreeIter iter;
    char *p;

    if (moko_update_net_undocked_client_list (dpy, &list, &nr) == FALSE)
    	return;
    p = g_malloc0 (nr);

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter))
    {
    	gboolean more;
    do{
    	gboolean found = FALSE;
    	Window w;
    	gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter, OBJECT_COL, &w, -1);
    	for (i=0; i<nr; i++) 
	{
    	    if (list[i] == w) 
			{
    	    	p[i] = 1;
    	    	found = TRUE;
    	    	break;
    	    }
    	}
    	if (found)
    	    more = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
    	else
    	    more = gtk_list_store_remove (list_store, &iter);
    	}
    while (more);
    }
    
    for (i=0; i<nr; i++) 
	{
    	if (p[i] == 0 && list[i] != my_win)
    	    moko_add_window (dpy, list[i], list_store);
   	}

  	g_free (p);
}

void 
moko_set_list_highlight (Display *dpy, MokoTaskList *l) 
{
    Window *wp;
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
  
    if (XGetWindowProperty (dpy, DefaultRootWindow (dpy), atoms[_NET_ACTIVE_WINDOW],
			  0, 4, False, XA_WINDOW, &type, &format, &nitems, &bytes_after, 
			  (unsigned char **)&wp) == Success)	
    {
        if (wp)
        {
            Window w;
            w = *wp;
            if (w != 0 && w != my_win) 
            {
                GtkTreeIter iter;
                if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (l->list_store), &iter))	
                {
                    Window iw;
                    do 
                    	{
                        gtk_tree_model_get (GTK_TREE_MODEL (l->list_store), &iter, OBJECT_COL, &iw, -1);
                        if (iw == w)
                        {
                            GtkTreePath *path;
                            path = gtk_tree_model_get_path (GTK_TREE_MODEL (l->list_store), &iter);
				gtk_icon_view_select_path (GTK_ICON_VIEW (l->list_view), path);
                            gtk_tree_path_free (path);
                            break;
                        }
                      }while (gtk_tree_model_iter_next (GTK_TREE_MODEL (l->list_store), &iter));
                   }
              }
          XFree (wp);
         }
    }
}

static void 
moko_task_list_selection_changed (GtkIconView *self, MokoTaskList *l)
{
  char *name = NULL;
  Window w;
  GtkTreeIter iter;
  GList *path = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (self));
  GtkTreeModel *model = gtk_icon_view_get_model (GTK_ICON_VIEW(self));
  
  if ( !path)
    return;
		    
  gtk_tree_model_get_iter (model, &iter, path->data);
  gtk_tree_model_get (GTK_TREE_MODEL (l->list_store), &iter, OBJECT_COL, &w, -1);
  name = moko_get_window_name(GDK_DISPLAY(), w);

  if (name)
    moko_dbus_send_message (name);

  g_list_foreach (path, gtk_tree_path_free, NULL);
  g_list_free (path);
  g_free (name);
  return;
}
