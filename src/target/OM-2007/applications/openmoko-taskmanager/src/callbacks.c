/**
 *  callbacks.c
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

#include "callbacks.h"

GdkFilterReturn
moko_window_filter (GdkXEvent *xev, GdkEvent *gev, MokoTaskList*l) 
{
    XEvent *ev = (XEvent *)xev;
    Display *dpy = ev->xany.display;

    if (ev->xany.type == PropertyNotify
      	 && ev->xproperty.window == DefaultRootWindow (dpy)) {
        if (ev->xproperty.atom == atoms[_NET_CLIENT_LIST])	{
		moko_update_store_list(dpy, l->list_store);
      	 }
      	 else if (ev->xproperty.atom == atoms[_NET_ACTIVE_WINDOW]) {
		moko_set_list_highlight (dpy, l);
      	 }
    }
    return GDK_FILTER_CONTINUE;
}

gboolean
moko_wm_cmd (GtkWidget *w, GtkWidget *list_view, int task)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (list_view));
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (sel, &model, &iter)) 
    	{
    	  Window w;

    	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OBJECT_COL, &w, -1);
    	  mbcommand(GDK_DISPLAY(), task, w, NULL);
    	  return TRUE;
    	  }
    else 
    	{
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
    	  return FALSE;
    	  }
}

gboolean 
moko_cursor_changed(GtkTreeView *treeview, GtkTreeModel *model)
{
    g_debug ("Tab event");
    
}


void
moko_go_to_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{
    if (!tm)
    	return;
    moko_wm_cmd (GTK_WIDGET (tm), 
    			GTK_WIDGET (tm->l->list_view), MB_CMD_ACTIVATE_CLIENT);
}

void
moko_kill_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{
    if (!tm)
    	return;
    moko_wm_cmd (GTK_WIDGET (tm), 
    			GTK_WIDGET (tm->l->list_view), MB_CMD_REMOVE_CLIENT);
}

void
moko_kill_all_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{    
    GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tm->l->list_view));
    GtkTreeIter iter;
    GtkTreeModel *model;
    Window *win;
    GSList *list = NULL;
    
    gtk_tree_selection_get_selected (sel, &model, &iter);

    if (!gtk_tree_model_get_iter_first (model, &iter))
    	return;
    
    do 
    	{
    	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OBJECT_COL, &win, -1);
    	list = g_slist_append (list, win);
    	if (!gtk_tree_model_iter_next (model, &iter))
    		break;
    	}
    while (1);

   do{
   	mbcommand (GDK_DISPLAY(), MB_CMD_REMOVE_CLIENT, list->data, NULL); 
   	g_debug ("%d", list->data);
  
	}
    while (list = g_slist_next (list));
    	  /*Window w;

    	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OBJECT_COL, &w, -1);
    	  mbcommand(GDK_DISPLAY(), task, w, NULL);
    	  return TRUE;
    	  }
*/
  if (list)
 	g_slist_free (list);
}
void
moko_quit_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{
    if (!tm)
    	return;

    gtk_window_iconify (GTK_WINDOW (tm->window));
}

void
moko_tab_event_cb (GtkButton *btn, MokoTaskList *l) 
{
    g_debug ("tab event");
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeViewColumn *col;
    GtkTreeModel *model;

    gtk_tree_view_get_cursor(GTK_TREE_VIEW (l->list_view), &path, &col);
    //gtk_tree_view_get_cursor(l->mokolist_view, &path, &col);

    model = GTK_TREE_MODEL (l->list_store);

    if (gtk_tree_model_get_iter (model, &iter, path)) {
        Window w;
        gtk_tree_model_get (model, &iter, OBJECT_COL, &w, -1);
        //moko_print_win_list(GDK_DISPLAY(), &w, 1);
        //moko_send_Xclimsgwm(GDK_DISPLAY (), w);
        mbcommand(GDK_DISPLAY(), MB_CMD_ACTIVATE_CLIENT, w, NULL);
        }
    if (path)
    	free (path);
}

void        
moko_hold_event_cb (GtkButton *btn, MokoTaskList *l) 
{
    moko_init_popup_menu(NULL, NULL, l);
 }

void
moko_wheel_left_up_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
   GtkTreeSelection    *selection;
   GtkTreeModel        *model;
   GtkTreeIter         iter;
   GtkTreePath* path;

   selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tm->l->list_view));

   if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	 return;
   
   path = gtk_tree_model_get_path (model, &iter);
   if (!gtk_tree_path_prev (path))
   {
	gtk_tree_path_free (path);
	return;
   }
   
   gtk_tree_view_set_cursor (GTK_TREE_VIEW (tm->l->list_view), path, 0, 0);
   gtk_tree_path_free (path);
   return;
}

void
moko_wheel_right_down_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
   GtkTreeSelection    *selection;
   GtkTreeModel        *model;
   GtkTreeIter         iter;
   GtkTreePath* path;

   if (!tm->l->list_view)
 	return;
	 
   selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tm->l->list_view));

   if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	 return;
   
   path = gtk_tree_model_get_path (model, &iter);
   gtk_tree_path_next (path);
   
   if (!path)
	return;
     
   gtk_tree_view_set_cursor (GTK_TREE_VIEW (tm->l->list_view), path, 0, 0);
   gtk_tree_path_free (path);
   return;
 }

void
moko_wheel_bottom_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
    if (!tm)
    	return;
    gtk_main_quit ();
    g_free (tm);
}
