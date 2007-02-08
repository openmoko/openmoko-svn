/*  common.c
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#include "moko-dialer-declares.h"
#include "common.h"
#include "error.h"
/**
 * @brief Create a pixbuf by the filename from the PKGDATADIR
 * @param filename The filename of the pixbuf file
 * @return The GdkPixbuf. If can not find the file, it will return NULL.
 */
GdkPixbuf *
create_pixbuf (const gchar * filename)
{
  gchar *pathname;
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;

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

/**
 * @brief Create a filepath by the filename from the PKGDATADIR
 * @param filename The filename of the pixbuf file
 * @return Newly allocated string if the file exists, otherwise returns NULL
 */

gchar *
file_create_data_path_for_the_file (const gchar * filename)
{
  gchar *pathname;

  pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S,
                              filename);

  if (g_file_test (pathname, G_FILE_TEST_EXISTS))
  {
    return pathname;
  }
  else
  {
    g_debug ("Can not find the file %s", pathname);
    g_free (pathname);
    return NULL;
  }


}


/**
 * @brief load the person's image file by the filename from the PKGDATADIR
 * @param rela_path The filename of the pixbuf file
 * @param widget, the gtkImage to load the file.
 * @return TURE, FALSE
 */
gboolean
file_load_person_image_from_relative_path (GtkWidget * widget,
                                           char *rela_path)
{

  gchar *pathname;
  GtkImage *image = GTK_IMAGE (widget);

  pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S,
                              rela_path);


  if (g_file_test (pathname, G_FILE_TEST_EXISTS))
  {
    gtk_image_set_from_file (image, pathname);
    g_free (pathname);
    return TRUE;
  }
  else
  {
    //first we load the default picture 
    pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S,
                                MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH);
    if (g_file_test (pathname, G_FILE_TEST_EXISTS))
    {
      gtk_image_set_from_file (image, pathname);
      g_free (pathname);
      return TRUE;
    }
    else
    {
      g_debug ("Can not find the file %s", pathname);
      gtk_image_set_from_stock (image, "gtk-yes",
                                GTK_ICON_SIZE_LARGE_TOOLBAR);
      g_free (pathname);
      return FALSE;
    }
  }
}


GtkWidget *
file_new_image_from_relative_path (char *rela_path)
{


  gchar *pathname;
  GtkWidget *image = 0;

  pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S,
                              rela_path);


  if (g_file_test (pathname, G_FILE_TEST_EXISTS))
  {
    image = gtk_image_new_from_file (pathname);
  }
  else
  {
//            g_debug ("Can not find the file %s", pathname);
    image = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_LARGE_TOOLBAR);

  }
  g_free (pathname);
  return image;
}


/**
 * @brief load the person's image file by the filename from the PKGDATADIR, and strech it.
 * @param rela_path The filename of the pixbuf file
 * @param widget, the gtkImage to load the file.
 * @return TURE, FALSE
 */
gboolean
file_load_person_image_scalable_from_relative_path (GtkWidget * widget,
                                                    char *rela_path)
{

  gchar *pathname;
  GtkImage *image = GTK_IMAGE (widget);

  pathname = g_strdup_printf ("%s%s%s", PKGDATADIR, G_DIR_SEPARATOR_S,
                              rela_path);


  if (g_file_test (pathname, G_FILE_TEST_EXISTS))
  {
    //    gtk_image_set_from_file(image,pathname);          
    GError *err = NULL;
    GdkPixbuf *src_pixbuf, *dest_pixbuf;
    src_pixbuf = gdk_pixbuf_new_from_file (pathname, &err);
    DBG_MESSAGE
      ("file_load_person_image_scalable_from_relative_path,width=%d,height=%d",
       widget->allocation.width, widget->allocation.height);

//          dest_pixbuf = gdk_pixbuf_scale_simple (src_pixbuf, widget->requisition.width, widget->requisition.height, GDK_INTERP_NEAREST);
    dest_pixbuf =
      gdk_pixbuf_scale_simple (src_pixbuf, widget->allocation.width,
                               widget->allocation.height, GDK_INTERP_NEAREST);
    gtk_image_set_from_pixbuf (image, dest_pixbuf);
    g_free (pathname);
    return TRUE;
  }
  else
  {
    g_debug ("Can not find the file %s", pathname);
    gtk_image_set_from_stock (image, "gtk-yes", GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_free (pathname);
    return FALSE;
  }


}
