/**
 *  @file detail-area.c
 *  @brief The detail area in the main window
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
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#include <libmokoui/moko-details-window.h>
#include <string.h>

#include "detail-area.h"
#include "navigation-area.h"
#include "package-list.h"

/**
 * @brief Create a detail area to the application manager data
 * @param appdata The application manager data
 * @return The toplevel widget of detail area
 */
GtkWidget *
detail_area_new (ApplicationManagerData *appdata)
{
  GtkWidget    *text;
  MokoDetailsWindow  *detail;
  GtkBox             *box;

  detail = moko_details_window_new ();
  box = moko_details_window_put_in_box (detail);

  text = gtk_text_view_new ();
  gtk_widget_show (text);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text), TRUE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_WORD);

  gtk_container_add (GTK_CONTAINER (detail), text);
  application_manager_data_set_tvdetail (appdata, text);

  return GTK_WIDGET (box);
}

/**
 * @brief Format the depends list of package.
 * @param depends The depends list
 * @param The dest string
 */
static void 
format_depends_list (char *dest, char *depends, int size)
{
  int i = 0;
  char *src;

  src = depends;
  dest[i++] = '\t';
  dest[i++] = '*';
  while (*src)
    {
      if (*src == ',')
        {
          dest[i++] = '\n';
          dest[i++] = '\t';
          dest[i++] = '*';
          src++;
        }
      dest[i++] = *src;
      src++;
    }
  dest[i] = 0;
}

/**
 * @brief Update the detail area infomation base on the package that selected
 * @param appdata The application manager data
 * @param pkg The package infomation
 */
void 
detail_area_update_info (ApplicationManagerData *appdata, 
                         gpointer pkg)
{
  GtkWidget      *textview;
  GtkTextBuffer  *buffer;
  GtkTextIter    start, end;
  GdkPixbuf      *pix;
  GtkTextTagTable   *tagtable;
  gint           pstart, pend;
  char           *depends;

  g_debug ("Update the info in the detail area");

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));

  textview = application_manager_get_tvdetail (appdata);
  g_return_if_fail (GTK_IS_TEXT_VIEW (textview));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
  gtk_text_buffer_set_text (buffer, "", -1);
  g_return_if_fail (pkg != NULL);

  /* Init tag */
  tagtable = gtk_text_buffer_get_tag_table (buffer);
  if (gtk_text_tag_table_lookup (tagtable, "bold") == NULL)
    {
      gtk_text_buffer_create_tag (buffer, "bold",
                                  "weight", PANGO_WEIGHT_BOLD,
                                  "scale", 1.1,
                                  NULL);
    }

  /* Insert the pixmap of the package
     FIXME It needs a way to lookup the picture */
  pix = create_pixbuf ("unkown.png");

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_insert_pixbuf (buffer, &start, pix);

  /* Insert package name */
  gtk_text_buffer_get_end_iter (buffer, &end);
  pstart = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, 
                          package_list_get_package_name (pkg),
                          -1);

  /* Insert the "\n" */
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_insert (buffer, &end, "\n", -1);

  /* Insert the maintainer */
  gtk_text_buffer_get_end_iter (buffer, &end);
  pend = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, 
                          package_list_get_package_maintainer (pkg),
                          -1);

  /* Set bold to the first line */
  gtk_text_buffer_get_iter_at_offset (buffer, &start, pstart);
  gtk_text_buffer_get_iter_at_offset (buffer, &end, pend);
  gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &end);

  /* Set the Version */
  gtk_text_buffer_get_end_iter (buffer, &end);
  pstart = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, "\nVersion\n\t", -1);

  gtk_text_buffer_get_end_iter (buffer, &end);
  pend = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, 
                          package_list_get_package_version (pkg),
                          -1);
  gtk_text_buffer_get_iter_at_offset (buffer, &start, pstart);
  gtk_text_buffer_get_iter_at_offset (buffer, &end, pend);
  gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &end);

  /* Set the descript */
  gtk_text_buffer_get_end_iter (buffer, &end);
  pstart = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, "\nDescription\n\t", -1);

  gtk_text_buffer_get_end_iter (buffer, &end);
  pend = gtk_text_iter_get_offset (&end);
  gtk_text_buffer_insert (buffer, &end, 
                          package_list_get_package_description (pkg),
                          -1);
  gtk_text_buffer_get_iter_at_offset (buffer, &start, pstart);
  gtk_text_buffer_get_iter_at_offset (buffer, &end, pend);
  gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &end);

  /* Set the depends */
  depends = package_list_get_package_depends (pkg);
  if (depends != NULL)
    {
      char *dep;
      int size = strlen (depends) *2 +8;
      dep = g_malloc (size);
      g_return_if_fail (dep != NULL);

      format_depends_list (dep, depends, size);

      gtk_text_buffer_get_end_iter (buffer, &end);
      pstart = gtk_text_iter_get_offset (&end);
      gtk_text_buffer_insert (buffer, &end, "\nDepends\n", -1);

      gtk_text_buffer_get_end_iter (buffer, &end);
      pend = gtk_text_iter_get_offset (&end);
      gtk_text_buffer_insert (buffer, &end, 
                              dep,
                              -1);
      gtk_text_buffer_get_iter_at_offset (buffer, &start, pstart);
      gtk_text_buffer_get_iter_at_offset (buffer, &end, pend);
      gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &end);

      g_free (dep);
    }

  g_object_unref (pix);
}

