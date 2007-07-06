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

#include <gtk/gtk.h>
#include "taku-icon-tile.h"

G_DEFINE_TYPE (TakuIconTile, taku_icon_tile, TAKU_TYPE_TILE);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TAKU_TYPE_ICON_TILE, TakuIconTilePrivate))

struct _TakuIconTilePrivate
{
  GtkWidget *icon;
  GtkWidget *primary;
  GtkWidget *secondary;
};

/* TODO: properties for the icon and strings */

static void
taku_icon_tile_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
taku_icon_tile_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
taku_icon_tile_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (taku_icon_tile_parent_class)->dispose)
    G_OBJECT_CLASS (taku_icon_tile_parent_class)->dispose (object);
}

static void
taku_icon_tile_finalize (GObject *object)
{
  G_OBJECT_CLASS (taku_icon_tile_parent_class)->finalize (object);
}

static const char *
taku_icon_tile_get_key (TakuTile *tile)
{
  return taku_icon_tile_get_primary (TAKU_ICON_TILE (tile));
}

static void
taku_icon_tile_class_init (TakuIconTileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  TakuTileClass *tile_class = TAKU_TILE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TakuIconTilePrivate));

  object_class->get_property = taku_icon_tile_get_property;
  object_class->set_property = taku_icon_tile_set_property;
  object_class->dispose = taku_icon_tile_dispose;
  object_class->finalize = taku_icon_tile_finalize;

  tile_class->get_sort_key = taku_icon_tile_get_key;
  tile_class->get_search_key = taku_icon_tile_get_key;
}

static void
taku_icon_tile_init (TakuIconTile *self)
{
  GtkWidget *vbox, *hbox;
  PangoAttribute *attr;
  PangoAttrList *list;

  self->priv = GET_PRIVATE (self);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox);

  self->priv->icon = gtk_image_new ();
  gtk_widget_show (self->priv->icon);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->icon, FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbox);

  self->priv->primary = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (self->priv->primary), PANGO_ELLIPSIZE_END);
  gtk_widget_show (self->priv->primary);
  gtk_misc_set_alignment (GTK_MISC (self->priv->primary), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), self->priv->primary, TRUE, TRUE, 0);
  
  list = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (list, attr);
  attr = pango_attr_scale_new (1.2);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (list, attr);
  gtk_label_set_attributes (GTK_LABEL (self->priv->primary), list);
  pango_attr_list_unref (list);
  
  self->priv->secondary = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (self->priv->secondary), PANGO_ELLIPSIZE_END);
  gtk_widget_show (self->priv->secondary);
  gtk_misc_set_alignment (GTK_MISC (self->priv->secondary), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), self->priv->secondary, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (self), hbox);
}

GtkWidget *
taku_icon_tile_new (void)
{
  return g_object_new (TAKU_TYPE_ICON_TILE, NULL);
}

void
taku_icon_tile_set_pixbuf (TakuIconTile *tile, GdkPixbuf *pixbuf)
{
  g_return_if_fail (TAKU_IS_ICON_TILE (tile));

  gtk_image_set_from_pixbuf (GTK_IMAGE (tile->priv->icon), pixbuf);
}

void
taku_icon_tile_set_icon_name (TakuIconTile *tile, const char *name)
{
  g_return_if_fail (TAKU_IS_ICON_TILE (tile));

  gtk_image_set_from_icon_name (GTK_IMAGE (tile->priv->icon),
                                name, gtk_icon_size_from_name ("TakuIcon"));
}

void
taku_icon_tile_set_primary (TakuIconTile *tile, const char *text)
{
  g_return_if_fail (TAKU_IS_ICON_TILE (tile));

  gtk_label_set_text (GTK_LABEL (tile->priv->primary), text);
}

const char *
taku_icon_tile_get_primary (TakuIconTile *tile)
{
  return gtk_label_get_text (GTK_LABEL (tile->priv->primary));
}

void
taku_icon_tile_set_secondary (TakuIconTile *tile, const char *text)
{
  g_return_if_fail (TAKU_IS_ICON_TILE (tile));

  gtk_label_set_text (GTK_LABEL (tile->priv->secondary), text);
}

const char *
taku_icon_tile_get_secondary (TakuIconTile *tile)
{
  return gtk_label_get_text (GTK_LABEL (tile->priv->secondary));
}
