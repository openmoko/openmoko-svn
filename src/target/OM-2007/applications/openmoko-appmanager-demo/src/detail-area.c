/**
 *  @file detail-area.c
 *  @brief The detail area in the main window
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include "detail-area.h"
#include "navigation-area.h"

/**
 * @brief Create a detail area to the application manager data
 * @param appdata The application manager data
 * @return The toplevel widget of detail area
 */
GtkWidget *
detail_area_new (ApplicationManagerData *appdata)
{
  GtkWidget    *scrollwindow;
  GtkWidget    *text;

  scrollwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrollwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow), 
                                  GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text = gtk_text_view_new ();
  gtk_widget_show (text);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text), TRUE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_WORD);

  gtk_container_add (GTK_CONTAINER (scrollwindow), text);
  application_manager_data_set_tvdetail (appdata, text);

  return scrollwindow;
}

/**
 * @brief Update the detail area infomation base on the package that selected
 * @param appdata The application manager data
 */
void 
detail_area_update_info (ApplicationManagerData *appdata)
{
  GtkWidget      *textview;
  GtkTextBuffer  *buffer;
  GtkTextIter    iter;
  GdkPixbuf      *pix;
  gchar          *name = NULL;
  gchar          str[256];

  g_debug ("Update the info in the detail area");

  g_return_if_fail (appdata != NULL);

  textview = application_manager_get_tvdetail (appdata);
  if (textview == NULL)
    {
      g_debug ("Textview is NULL");
      return;
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
  if (buffer == NULL)
    {
      g_debug ("Textview not init correctly, textbuffer is NULL");
      return;
    }

  name = treeview_get_selected_name (application_manager_get_tvpkglist (appdata));

  pix = create_pixbuf ("unkown.png");

  sprintf (str, "The selected package name is:%s", name);
  gtk_text_buffer_set_text (buffer, str, -1);

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_insert_pixbuf (buffer, &iter, pix);

  g_object_unref (pix);
}
