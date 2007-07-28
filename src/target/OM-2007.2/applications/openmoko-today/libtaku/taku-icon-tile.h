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

#ifndef _TAKU_ICON_TILE
#define _TAKU_ICON_TILE

#include "taku-tile.h"

G_BEGIN_DECLS

#define TAKU_TYPE_ICON_TILE taku_icon_tile_get_type()

#define TAKU_ICON_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TAKU_TYPE_ICON_TILE, TakuIconTile))

#define TAKU_ICON_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TAKU_TYPE_ICON_TILE, TakuIconTileClass))

#define TAKU_IS_ICON_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TAKU_TYPE_ICON_TILE))

#define TAKU_IS_ICON_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TAKU_TYPE_ICON_TILE))

#define TAKU_ICON_TILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TAKU_TYPE_ICON_TILE, TakuIconTileClass))

typedef struct _TakuIconTilePrivate TakuIconTilePrivate;

typedef struct {
  TakuTile parent;
  TakuIconTilePrivate *priv;
} TakuIconTile;

typedef struct {
  TakuTileClass parent_class;
} TakuIconTileClass;

GType taku_icon_tile_get_type (void);

GtkWidget* taku_icon_tile_new (void);

void taku_icon_tile_set_pixbuf (TakuIconTile *tile, GdkPixbuf *pixbuf);
void taku_icon_tile_set_icon_name (TakuIconTile *tile, const char *name);
void taku_icon_tile_set_primary (TakuIconTile *tile, const char *text);
const char *taku_icon_tile_get_primary (TakuIconTile *tile);
void taku_icon_tile_set_secondary (TakuIconTile *tile, const char *text);
const char *taku_icon_tile_get_secondary (TakuIconTile *tile);

G_END_DECLS

#endif /* _TAKU_ICON_TILE */
