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
#include "taku-tile.h"

G_DEFINE_ABSTRACT_TYPE (TakuTile, taku_tile, GTK_TYPE_EVENT_BOX);

#define TILE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TAKU_TYPE_TILE, TakuTilePrivate))

typedef struct _TakuTilePrivate TakuTilePrivate;

struct _TakuTilePrivate
{
};

enum {
  ACTIVATE,
  CLICKED,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL];

static void
taku_tile_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
taku_tile_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
taku_tile_dispose (GObject *object)
{
  G_OBJECT_CLASS (taku_tile_parent_class)->dispose (object);
}

static void
taku_tile_finalize (GObject *object)
{
  G_OBJECT_CLASS (taku_tile_parent_class)->finalize (object);
}

static gboolean
taku_tile_expose (GtkWidget      *widget,
                  GdkEventExpose *event)
{
  if (GTK_WIDGET_DRAWABLE (widget)) {
    gint x, y, width, height;
    GtkStateType state;

    x = widget->allocation.x;
    y = widget->allocation.y;
    width = widget->allocation.width;
    height = widget->allocation.height;

    state = GTK_WIDGET_STATE (widget);
    /* If this isn't isn't being drawn active and it's focused, highlight it */
    if (state != GTK_STATE_ACTIVE && GTK_WIDGET_HAS_FOCUS (widget)) {
      state = GTK_STATE_SELECTED;
    }

    gtk_paint_flat_box (widget->style, widget->window,
                        state, GTK_SHADOW_NONE,
                        &event->area, widget, NULL,
                        x, y, width, height);
    
    (* GTK_WIDGET_CLASS (taku_tile_parent_class)->expose_event) (widget, event);
  }
  
  return FALSE;
}

/*
 * Timeout callback to restore the state of the widget after the clicked state
 * change.
 */
static gboolean
reset_state (gpointer data)
{
  gtk_widget_set_state (GTK_WIDGET (data), GTK_STATE_NORMAL);
  return FALSE;
}

/*
 * The tile was clicked, so start the state animation and fire the signal.
 */
static void
taku_tile_clicked (TakuTile *tile)
{
  g_return_if_fail (TAKU_IS_TILE (tile));

  gtk_widget_set_state (GTK_WIDGET (tile), GTK_STATE_ACTIVE);
  g_timeout_add (500, reset_state, tile);

  g_signal_emit (tile, signals[CLICKED], 0);
}

/*
 * This is called by GtkWidget when the tile is activated.  Generally this means
 * the user has focused the tile and pressed [return] or [space].
 */
static void
taku_tile_real_activate (TakuTile *tile)
{
  taku_tile_clicked (tile);
}

/*
 * Callback when a button is released inside the tile.  As we're targetting
 * stylus devices we don't need to worry about clicks that started outside the
 * tile just yet, though we should.
 */
static gboolean
taku_tile_button_release (GtkWidget *widget, GdkEventButton *event)
{
  if (event->button == 1) {
    gtk_widget_grab_focus (widget);
    taku_tile_clicked (TAKU_TILE (widget));
  }
  
  return TRUE;
}

static void
taku_tile_class_init (TakuTileClass *klass)
{
  /* TODO: remove useless gobject-fu */
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TakuTilePrivate));

  object_class->get_property = taku_tile_get_property;
  object_class->set_property = taku_tile_set_property;
  object_class->dispose = taku_tile_dispose;
  object_class->finalize = taku_tile_finalize;

  widget_class->expose_event = taku_tile_expose;
  widget_class->button_release_event = taku_tile_button_release;

  klass->activate = taku_tile_real_activate;

  /* Hook up the activate signal so we get keyboard handling for free */
  signals[ACTIVATE] = g_signal_new ("activate",
                                    G_OBJECT_CLASS_TYPE (object_class),
                                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                                    G_STRUCT_OFFSET (TakuTileClass, activate),
                                    NULL, NULL,
                                    g_cclosure_marshal_VOID__VOID,
                                    G_TYPE_NONE, 0);
  widget_class->activate_signal = signals[ACTIVATE];

  signals[CLICKED] = g_signal_new ("clicked",
                                    G_OBJECT_CLASS_TYPE (object_class),
                                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                                    G_STRUCT_OFFSET (TakuTileClass, clicked),
                                    NULL, NULL,
                                    g_cclosure_marshal_VOID__VOID,
                                    G_TYPE_NONE, 0);
}

static void
taku_tile_init (TakuTile *self)
{
  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (self), GTK_CAN_FOCUS);
  gtk_widget_set_app_paintable (GTK_WIDGET (self), TRUE);
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (self), 6);
}

GtkWidget *
taku_tile_new (void)
{
  return g_object_new (TAKU_TYPE_TILE, NULL);
}

const char *
taku_tile_get_sort_key (TakuTile *tile)
{
  TakuTileClass *class;

  g_return_val_if_fail (TAKU_IS_TILE (tile), NULL);

  class = TAKU_TILE_GET_CLASS (tile);
  if (class->get_sort_key != NULL)
    return class->get_sort_key (tile);
  else
    return NULL;
}

const char *
taku_tile_get_search_key (TakuTile *tile)
{
  TakuTileClass *class;

  g_return_val_if_fail (TAKU_IS_TILE (tile), NULL);

  class = TAKU_TILE_GET_CLASS (tile);
  if (class->get_search_key != NULL)
    return class->get_search_key (tile);
  else
    return NULL;
}

gboolean
taku_tile_matches_filter (TakuTile *tile, gpointer filter)
{
  TakuTileClass *class;

  g_return_val_if_fail (TAKU_IS_TILE (tile), FALSE);
  g_return_val_if_fail (filter != NULL, FALSE);

  class = TAKU_TILE_GET_CLASS (tile);
  if (class->matches_filter != NULL)
    return class->matches_filter (tile, filter);
  else
    return TRUE;
}
