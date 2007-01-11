/**
 * @file list_view.c
 * @brief list_view.c based on misc.c and gtk+-2.0.
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

#include "list_view.h"
#include <gdk/gdk.h>

static GdkPixbuf *default_icon;

enum {
    LIST_SIGNAL,
    LAST_SIGNAL
};

static void moko_task_list_class_init          (MokoTaskListClass *klass);
static void moko_task_list_init                (MokoTaskList *l);

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
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
   // GtkWidget *ico;
    GtkWidget *align;

    align=gtk_alignment_new(0, 0, 1, 1);    
    gtk_alignment_set_padding(GTK_ALIGNMENT (align), 0, 150, 0, 0);
    l->list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_PIXBUF);
    l->list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (l->list_store));
    gtk_widget_set_name (l->list_view, "gtktreeview-black");
    gtk_widget_show (l->list_view);
    //l->mokolist_view = moko_tree_view_new_with_model (GTK_TREE_MODEL (l->list_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (l->list_view), FALSE);
    //gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (l->mokolist_view), FALSE);
    
   /* l->renderer = gtk_cell_renderer_pixbuf_new ();
    l->column = gtk_tree_view_column_new_with_attributes ("Icon", l->renderer, "pixbuf", 2, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (l->list_view), l->column);
    //moko_tree_view_append_column (GTK_TREE_VIEW (l->mokolist_view), l->column);

    l->renderer = gtk_cell_renderer_text_new ();
    l->column = gtk_tree_view_column_new_with_attributes ("Running programs", l->renderer, 
    													"text", 0, NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (l->list_view), l->column);
  		*/

   column = gtk_tree_view_column_new();
   gtk_tree_view_column_set_title (column, ("Task list"));
   gtk_tree_view_column_set_resizable (column, TRUE);
  
    renderer = gtk_cell_renderer_pixbuf_new ();
   gtk_tree_view_column_pack_start (column,  renderer , FALSE);
   gtk_tree_view_column_set_attributes (column,  renderer , 
  				"pixbuf", PIXBUF_COL, NULL);

   renderer = gtk_cell_renderer_text_new ();
   gtk_tree_view_column_pack_start (column,  renderer , FALSE);
   gtk_tree_view_column_set_attributes(column,  renderer , 
   				"text", TEXT_COL, NULL);
  
   gtk_tree_view_append_column (GTK_TREE_VIEW (l->list_view), column);
		
    l->scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (l->scrolled);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (l->scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_container_add (GTK_CONTAINER (align), l->list_view);
    gtk_container_add (GTK_CONTAINER (l->scrolled), align);
    gtk_widget_set_size_request (l->scrolled, -1, 400);
   
    //l->scrolled = moko_tree_view_put_into_scrolled_window (l->mokolist_view);

    //gtk_box_pack_start (GTK_BOX (l), l->hbox, FALSE, FALSE, 0);
    //gtk_box_pack_end (l->hbox, l->btn_close, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (l), l->scrolled, TRUE, TRUE, 0);
    
    default_icon = gdk_pixbuf_new_from_file_at_size (PKGDATADIR"/default-app-icon.xpm",
		    					160, 160, NULL);
    if (!default_icon )
	    g_error ("Failed to load default icon");
}

/* Construction */
GtkWidget* 
moko_task_list_new() 
{
    return GTK_WIDGET(g_object_new(moko_task_list_get_type(), NULL));
}

/* Destruction */
void 
moko_task_list_clear(MokoTaskList *l) { 
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
    gtk_list_store_set (list_store, &iter, TEXT_COL, name, OBJECT_COL, w, -1);

    if (icon) {
        GdkPixbuf *icons = gdk_pixbuf_scale_simple (icon, 160, 160, GDK_INTERP_BILINEAR);
        gtk_list_store_set (list_store, &iter, PIXBUF_COL, icons, -1);
	gdk_pixbuf_unref (icons);
        }
    else if (default_icon) 
        gtk_list_store_set (list_store, &iter, PIXBUF_COL, default_icon, -1);
    else
	    g_error ("Failed to load %s's icon", name);

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

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter))	{
    	gboolean more;
    do{
    	gboolean found = FALSE;
    	Window w;
    	gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter, OBJECT_COL, &w, -1);
    	for (i=0; i<nr; i++) {
    	    if (list[i] == w) {
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
    
    for (i=0; i<nr; i++) {
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
                            gtk_tree_view_set_cursor (GTK_TREE_VIEW (l->list_view), path, NULL, FALSE);
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

