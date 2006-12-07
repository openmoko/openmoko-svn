/**
 * @file callbacks.c
 * @brief openmoko-taskmanager callbacks functions based on misc.c.
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

#include "callbacks.h"

void 
om_update_store_list (Display *dpy, GtkListStore *list_store);

void 
om_set_list_highlight (Display *dpy, List *l);

GdkFilterReturn
om_window_filter (GdkXEvent *xev, GdkEvent *gev, List *l) {
    XEvent *ev = (XEvent *)xev;
    Display *dpy = ev->xany.display;

    if (ev->xany.type == PropertyNotify
      	 && ev->xproperty.window == DefaultRootWindow (dpy)) {
        if (ev->xproperty.atom == atoms[_NET_CLIENT_LIST])	{
		om_update_store_list(dpy, l->list_store);
      	 }
      	 else if (ev->xproperty.atom == atoms[_NET_ACTIVE_WINDOW]) {
		om_set_list_highlight (dpy, l);
      	 }
    }
    return GDK_FILTER_CONTINUE;
}

gboolean
om_wm_cmd (GtkWidget *w, GtkWidget *list_view, int task) {
    GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (list_view));
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (sel, &model, &iter)) {
    	  Window w;

    	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &w, -1);
    	  mbcommand(GDK_DISPLAY(), task, w, NULL);
    	  return TRUE;
    	  }
    else {
    	  /*GtkMessageDialog* dialog = gtk_message_dialog_new (GTK_WINDOW(gtk_widget_get_toplevel(w)),
 							GTK_DIALOG_DESTROY_WITH_PARENT,
		                                   GTK_MESSAGE_ERROR,      
		                                   GTK_BUTTONS_CLOSE,
		                                   "No Application selected..." );
	  gtk_dialog_run (GTK_DIALOG (dialog));
	  gtk_widget_destroy(GTK_WIDGET(dialog));
	  return TRUE;
	  */
	  g_debug ("send message to footer");
    	  }
    }


gboolean 
om_cursor_changed(GtkTreeView *treeview, GtkTreeModel *model) {
    g_debug ("Tab event");
    }

void
om_add_window (Display *dpy, Window w, GtkListStore *list_store){
    GtkTreeIter iter;
    gchar *name = NULL;
    GdkPixbuf *icon = NULL;
    Atom type;

    icon = om_get_window_icon (dpy, w);
    //name = om_get_window_name(dpy, w);
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 0, name, 1, w, -1);
    //gtk_list_store_set (list_store, &iter, 1, w, -1);

    if (icon) {
        GdkPixbuf *icons = gdk_pixbuf_scale_simple (icon, 160, 160, GDK_INTERP_BILINEAR);
        gdk_pixbuf_unref (icon);
        gtk_list_store_set (list_store, &iter, 2, icons, -1);
        }
    /*FIXME if no icon there should be a default icon to be added*/
    /*else {
        GtkWidget *def_icon = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_SMALL_TOOLBAR);
        GdkPixbuf *icons = gdk_pixbuf_scale_simple (def_icon, 160, 160, GDK_INTERP_BILINEAR);
        gdk_pixbuf_unref (def_icon);
        gtk_list_store_set (list_store, &iter, 2, icons, -1);
        }
   	*/
   }

void 
om_update_store_list (Display *dpy, GtkListStore *list_store) {
    Window *list;
    guint nr, i;
    GtkTreeIter iter;
    char *p;

    if (om_update_net_undocked_client_list (dpy, &list, &nr) == FALSE)
    	return;
    p = g_malloc0 (nr);

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter))	{
    	gboolean more;
    do{
    	gboolean found = FALSE;
    	Window w;
    	gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter, 1, &w, -1);
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
    	    om_add_window (dpy, list[i], list_store);
    	}

  	g_free (p);
}


void 
om_set_list_highlight (Display *dpy, List *l) {
    Window *wp;
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
  
    if (XGetWindowProperty (dpy, DefaultRootWindow (dpy), atoms[_NET_ACTIVE_WINDOW],
			  0, 4, False, XA_WINDOW, &type, &format, &nitems, &bytes_after, 
			  (unsigned char **)&wp) == Success)	{
        if (wp) {
            Window w;
            w = *wp;
            if (w != 0 && w != my_win) {
                GtkTreeIter iter;
                if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (l->list_store), &iter))	{
                    Window iw;
                    do {
                        gtk_tree_model_get (GTK_TREE_MODEL (l->list_store), &iter, 1, &iw, -1);
                        if (iw == w)	{
                            GtkTreePath *path;
                            path = gtk_tree_model_get_path (GTK_TREE_MODEL (l->list_store), &iter);
                            gtk_tree_view_set_cursor (GTK_TREE_VIEW (l->mokolist_view), path, NULL, FALSE);
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

void
om_tab_event_cb (GtkButton *btn, List *l) {
    g_debug ("tab event");
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeViewColumn *col;
    GtkTreeModel *model;

    //gtk_tree_view_get_cursor(l->list_view, &path, &col);
    gtk_tree_view_get_cursor(l->mokolist_view, &path, &col);

    model = GTK_TREE_MODEL (l->list_store);

    if (gtk_tree_model_get_iter (model, &iter, path)) {
        Window w;
        gtk_tree_model_get (model, &iter, 1, &w, -1);
        //om_print_win_list(GDK_DISPLAY(), &w, 1);
        //om_send_Xclimsgwm(GDK_DISPLAY (), w);
        mbcommand(GDK_DISPLAY(), MB_CMD_ACTIVAE_CLIENT, w, NULL);
        }
    if (path)
    	free (path);
    }

void        
om_hold_event_cb (GtkButton *btn, List *l) {
    g_debug ("tab with hold event");
    om_init_popup_menu(NULL, NULL, l);
    }
