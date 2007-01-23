/**
 *  @file pixbuf-list.c
 *  @brief The package list that get from the lib ipkg
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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

#include "pixbuf-list.h"

/**
 * @brief Create a pixbuf by the filename from the PKGDATADIR
 * @param filename The filename of the pixbuf file
 * @return The GdkPixbuf. If can not find the file, it will return NULL.
 */
GdkPixbuf *
create_pixbuf (const gchar *filename)
{
  gchar     *pathname;
  GdkPixbuf *pixbuf = NULL;
  GError    *error = NULL;

  pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S, 
                              filename);
  if (g_file_test (pathname, G_FILE_TEST_EXISTS))
    {
      pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
      if (!pixbuf)
        {
          fprintf (stderr, "Fail to load pixbuf file %s: %s\n", 
                   pathname, error->message);
          g_error_free (error);
        }
    }
  else
    {
      g_debug ("Can not find the file %s", pathname);
    }
  g_free (pathname);
  return pixbuf;
}

