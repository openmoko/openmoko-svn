/*  common.h
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#ifndef _DIALER_COMMON_H
#define _DIALER_COMMON_H

#include <gtk/gtk.h>

GdkPixbuf *create_pixbuf (const gchar * filename);
gchar *file_create_data_path_for_the_file (const gchar * filename);
gboolean file_load_person_image_from_relative_path (GtkWidget * widget,
                                                    char *rela_path);
GtkWidget *file_new_image_from_relative_path (char *rela_path);
#endif
