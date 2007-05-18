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

#ifndef _TASKITEM_H_
#define _TASKITEM_H_

#include <gtk/gtk.h>

typedef struct _MokoTaskItem
{
  GtkWidget *name;
  GtkWidget *icon;
  GtkWidget *box;
} MokoTaskItem;


void moko_task_item_init( MokoTaskItem *ti, gchar *name, GdkPixbuf *icon );
void moko_task_item_set_name( MokoTaskItem *ti, gchar *name );
void moko_task_item_set_icon( MokoTaskItem *ti, GdkPixbuf *icon );

#endif
