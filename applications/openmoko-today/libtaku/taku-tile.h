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

#ifndef _TAKU_TILE
#define _TAKU_TILE

#include <gtk/gtkeventbox.h>

G_BEGIN_DECLS

#define TAKU_TYPE_TILE taku_tile_get_type()

#define TAKU_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TAKU_TYPE_TILE, TakuTile))

#define TAKU_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TAKU_TYPE_TILE, TakuTileClass))

#define TAKU_IS_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TAKU_TYPE_TILE))

#define TAKU_IS_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TAKU_TYPE_TILE))

#define TAKU_TILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TAKU_TYPE_TILE, TakuTileClass))

typedef struct {
  GtkEventBox parent;
} TakuTile;

typedef struct {
  GtkEventBoxClass parent_class;

  void (* activate) (TakuTile *tile);
  void (* clicked) (TakuTile *tile);

  const char *(* get_sort_key) (TakuTile *tile);
  const char *(* get_search_key) (TakuTile *tile);

  gboolean (* matches_filter) (TakuTile *tile, gpointer filter);
} TakuTileClass;

GType taku_tile_get_type (void);

GtkWidget* taku_tile_new (void);

const char *taku_tile_get_sort_key (TakuTile *tile);

const char *taku_tile_get_search_key (TakuTile *tile);

gboolean taku_tile_matches_filter (TakuTile *tile, gpointer filter);

G_END_DECLS

#endif /* _TAKU_TILE */
