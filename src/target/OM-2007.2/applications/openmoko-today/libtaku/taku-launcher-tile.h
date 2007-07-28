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

#ifndef _TAKU_LAUNCHER_TILE
#define _TAKU_LAUNCHER_TILE

#include "taku-icon-tile.h"

G_BEGIN_DECLS

typedef struct {
  char *name;
  char **matches;
} TakuLauncherCategory;

TakuLauncherCategory * taku_launcher_category_new (void);
void taku_launcher_category_free (TakuLauncherCategory *launcher);


#define TAKU_TYPE_LAUNCHER_TILE taku_launcher_tile_get_type()

#define TAKU_LAUNCHER_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TAKU_TYPE_LAUNCHER_TILE, TakuLauncherTile))

#define TAKU_LAUNCHER_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TAKU_TYPE_LAUNCHER_TILE, TakuLauncherTileClass))

#define TAKU_IS_LAUNCHER_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TAKU_TYPE_LAUNCHER_TILE))

#define TAKU_IS_LAUNCHER_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TAKU_TYPE_LAUNCHER_TILE))

#define TAKU_LAUNCHER_TILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TAKU_TYPE_LAUNCHER_TILE, TakuLauncherTileClass))

typedef struct _TakuLauncherTilePrivate TakuLauncherTilePrivate;

typedef struct {
  TakuIconTile parent;
  TakuLauncherTilePrivate *priv;
} TakuLauncherTile;

typedef struct {
  TakuIconTileClass parent_class;
} TakuLauncherTileClass;

GType taku_launcher_tile_get_type (void);

GtkWidget* taku_launcher_tile_new (void);

GtkWidget * taku_launcher_tile_for_desktop_file (const char *filename);

const char ** taku_launcher_tile_get_categories (TakuLauncherTile *tile);

void taku_launcher_tile_add_group (TakuLauncherTile *tile, TakuLauncherCategory *category);

G_END_DECLS

#endif /* _TAKU_LAUNCHER_TILE */
