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
#include "dbus-conn.h"

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
moko_wm_cmd (MokoTaskManager *tm, GtkWidget *list_view, int task)
{
  GList *path = gtk_icon_view_get_selected_items (GTK_ICON_VIEW(list_view));
  GtkTreeModel *model = gtk_icon_view_get_model (GTK_ICON_VIEW(list_view));
  GtkTreeIter iter; 

  if (gtk_tree_model_get_iter (model, &iter, path->data)) 
  {
    Window w;
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OBJECT_COL, &w, -1);
    mbcommand(GDK_DISPLAY(), task, w, NULL);
    return TRUE;
  }
  else 
  {
    moko_dbus_send_message ("No application selected");
    return FALSE;
  }
  
  if (path)
    g_list_free (path);
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
    moko_wm_cmd (tm, GTK_WIDGET (tm->l->list_view), CMD_ACTIVATE_WINDOW);
}

void
moko_kill_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{
    if (!tm)
    	return;
    moko_wm_cmd (tm, GTK_WIDGET (tm->l->list_view), CMD_CLOSE_WINDOW);
}

void
moko_kill_all_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{    
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_icon_view_get_model (GTK_ICON_VIEW (tm->l->list_view));
    Window *win;
    GSList *list = NULL;

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

   do
   {
        mbcommand (GDK_DISPLAY(), CMD_CLOSE_WINDOW, list->data, NULL); 
	}
    while (list = g_slist_next (list));
  
  if (list)
 	g_slist_free (list);
}
void
moko_quit_btn_cb (GtkButton *btn, MokoTaskManager *tm)
{
    if (!tm)
    	return;

    gtk_window_iconify (GTK_WINDOW (tm->window));
	moko_dbus_send_message ("");
}

void
moko_wheel_left_up_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
  if (!tm->l->list_view)
    return;
  
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_icon_view_get_model (GTK_ICON_VIEW (tm->l->list_view));
  GList *path = gtk_icon_view_get_selected_items (GTK_ICON_VIEW(tm->l->list_view));

  if (!path && gtk_tree_model_get_iter_first (model, &iter))
  { 
    GtkTreePath *new_path = NULL;
    for (;gtk_tree_model_iter_next (model, &iter);)
    {
      if (new_path)
	 gtk_tree_path_free (new_path);
      new_path = gtk_tree_model_get_path (model, &iter);
    }
    gtk_icon_view_select_path (GTK_ICON_VIEW (tm->l->list_view), new_path);
    gtk_tree_path_free (new_path);
  }
  else if (path && gtk_tree_path_prev (path->data))
  {
    gtk_icon_view_select_path (GTK_ICON_VIEW (tm->l->list_view), path->data);
  }
  else
    g_warning ("Can not find right path of icon view item");
  
  if (path)
  {
    gtk_tree_path_free (path->data);
    g_list_free (path);
  }
  return;
}

void
moko_wheel_right_down_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
  if (!tm->l->list_view)
    return;
  GList* path = NULL;
  GtkTreePath *new_path = NULL; 
  GtkTreeModel *model = gtk_icon_view_get_model (GTK_ICON_VIEW (tm->l->list_view));
  GtkTreeIter iter;
  
  path = gtk_icon_view_get_selected_items (GTK_ICON_VIEW(tm->l->list_view));

  if (path && gtk_tree_model_get_iter (model, &iter, path->data))
  {
    if (gtk_tree_model_iter_next (model, &iter))
      new_path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_path_free (path->data);//FIXME: If not use _GTK_SELECTION_SINGLE_ as icon view selection model, 
                                    //here should use g_list_foreach (path, gtk_tree_path_free, NULL) 
                                    //to free the tree path(es).
  }
  else if (gtk_tree_model_get_iter_first (model, &iter))
  { 
    new_path = gtk_tree_model_get_path (model, &iter);
  }
  else
    g_error ("Can not find right path of icon view item");
  
  if (new_path)
  {
    gtk_icon_view_select_path (GTK_ICON_VIEW (tm->l->list_view), new_path);
    gtk_tree_path_free (new_path);
  }
  g_list_free (path);
  
  return;
}

void
moko_wheel_bottom_press_cb (GtkWidget *self, MokoTaskManager *tm)
{
  GtkWidget* widget = GTK_WIDGET(self);
  Screen* screen = GDK_SCREEN_XSCREEN(gtk_widget_get_screen (widget));
  XEvent xev;

  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = DisplayOfScreen (screen);
  xev.xclient.window = RootWindowOfScreen (screen);
  xev.xclient.message_type = gdk_x11_get_xatom_by_name( "_NET_SHOWING_DESKTOP" );
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = TRUE;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (DisplayOfScreen (screen), RootWindowOfScreen (screen), False,
              SubstructureRedirectMask | SubstructureNotifyMask, &xev);

  return;
}
