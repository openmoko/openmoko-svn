/*
 *  Footer - Task item
 *
 *  Authored by Daniel Willmann <daniel@totalueberwachung.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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

#include "taskitem.h"

#include <gtk/gtk.h>
#include <glib.h>

void moko_task_item_init( MokoTaskItem *ti, gchar *name, GdkPixbuf *icon )
{
  GdkPixbuf *scaled_icon;

  ti->box = gtk_vbox_new( FALSE, 0 );
  ti->name = gtk_label_new( name );

  scaled_icon = gdk_pixbuf_scale_simple ( icon, 140, 140, GDK_INTERP_BILINEAR );
  ti->icon = gtk_image_new_from_pixbuf( scaled_icon );
  g_object_unref( G_OBJECT(scaled_icon) );

  gtk_box_pack_start( GTK_BOX(ti->box), GTK_WIDGET(ti->icon), TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX(ti->box), GTK_WIDGET(ti->name), TRUE, TRUE, 0 );
}

void moko_task_item_set_name( MokoTaskItem *ti, gchar *name )
{
  gtk_label_set_text( ti->name, name );

}

void moko_task_item_set_icon( MokoTaskItem *ti, GdkPixbuf *icon )
{
  GdkPixbuf *scaled_icon;
  scaled_icon = gdk_pixbuf_scale_simple ( icon, 140, 140, GDK_INTERP_BILINEAR );
  gtk_image_set_from_pixbuf( ti->icon, scaled_icon );
  g_object_unref( G_OBJECT(scaled_icon) );
}
