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

/**
 * @brief Create a detail area to the main window
 * @param window The main window
 * @return The toplevel widget of detail area
 */
GtkWidget *
detail_area_new_for_window (MokoPanedWindow *window)
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

  return scrollwindow;
}
