/* 
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _TAKU_TABLE
#define _TAKU_TABLE

#include <gtk/gtktable.h>
#include "taku-tile.h"

G_BEGIN_DECLS

#define TAKU_TYPE_TABLE taku_table_get_type()

#define TAKU_TABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TAKU_TYPE_TABLE, TakuTable))

#define TAKU_TABLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TAKU_TYPE_TABLE, TakuTableClass))

#define TAKU_IS_TABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TAKU_TYPE_TABLE))

#define TAKU_IS_TABLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TAKU_TYPE_TABLE))

#define TAKU_TABLE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TAKU_TYPE_TABLE, TakuTableClass))

typedef struct _TakuTablePrivate TakuTablePrivate;

typedef struct {
  GtkTable parent;
  TakuTablePrivate *priv;
} TakuTable;

typedef struct {
  GtkTableClass parent_class;
} TakuTableClass;

GType taku_table_get_type (void);

GtkWidget* taku_table_new (void);

void taku_table_set_filter (TakuTable *table, gpointer filter);

gpointer taku_table_get_filter (TakuTable *table);

G_END_DECLS

#endif /* _TAKU_TABLE */
