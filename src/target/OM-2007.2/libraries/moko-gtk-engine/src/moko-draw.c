/*
 * moko-draw.c
 * This file is part of moko-engine
 *
 * Originally from OpenedHand's Sato GTK+ Engine
 *
 * Copyright (C) 2006,2007 - OpenedHand Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "moko-draw.h"
#include "moko-style.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#define DEBUG(func) // g_printf ("%s: detail = '%s'; state = %d; x:%d; y:%d; w:%d; h:%d;\n", func, detail, state_type, x, y, width, height);
#define DETAIL(foo) (detail && strcmp (foo, detail) == 0)

GtkStyleClass *parent_style_class;

/**
 * Prepare a new GC with the additional Moko style values
 */
static GdkGC*
moko_gc_new (GdkGC* old_gc, GdkDrawable *d)
{
  GdkGC *new_gc;
  new_gc = gdk_gc_new (d);
  gdk_gc_copy (new_gc, old_gc);
  gdk_gc_set_line_attributes (new_gc, 2, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
  return new_gc;
}

static void
moko_gradient (GtkStyle * style, GdkWindow * window, GtkStateType state_type,
	       gint x, gint y, gint width, gint height)
{
  gint i, rd, gd, bd;		/* rd, gd, bd - change in r g and b for gradient */
  GdkColor c1, c2, c3, c4;
  GdkGC *gc;
  gc = gdk_gc_new (window);

  /* get the start and end colours */
  moko_shade_colour (&style->bg[state_type], &c1, 1.6);
  moko_shade_colour (&style->bg[state_type], &c2, 1.4);
  moko_shade_colour (&style->bg[state_type], &c3, 1.2);
  moko_shade_colour (&style->bg[state_type], &c4, 1.0);

  /* set line for 1px */
  gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, GDK_CAP_BUTT,
			      GDK_JOIN_MITER);


  /*** First Gradient ***/
  /* calculate the delta values */

  rd = (c1.red - c2.red) / MAX (height / 2, 1);
  gd = (c1.green - c2.green) / MAX (height / 2, 1);
  bd = (c1.blue - c2.blue) / MAX (height / 2, 1);

  i = 0;
  while (i < height / 2)
  {
    gdk_gc_set_rgb_fg_color (gc, &c1);
    gdk_draw_line (window, gc, x, y + i, x + width, y + i);
    c1.red -= rd;
    c1.blue -= bd;
    c1.green -= gd;
    i++;
  }

  /*** Second Gradient ***/

  rd = (c3.red - c4.red) / MAX (height / 2, 1);
  gd = (c3.green - c4.green) / MAX (height / 2, 1);
  bd = (c3.blue - c4.blue) / MAX (height / 2, 1);

  i = height / 2;
  while (i < height)
  {
    gdk_gc_set_rgb_fg_color (gc, &c3);
    gdk_draw_line (window, gc, x, y + i, x + width, y + i);
    c3.red -= rd;
    c3.blue -= bd;
    c3.green -= gd;
    i++;
  }


}

static void
moko_draw_box (DRAW_ARGS)
{
  GdkGC *gc;

  DEBUG ("draw_box");
  SANITIZE_SIZE;

  /*** spin buttons ***/
  if (DETAIL ("spinbutton_down") || DETAIL ("spinbutton_up"))
    return;

  if (DETAIL ("spinbutton"))
  {
    /* FIXME: for RTL */
    width += 10;
    x -= 10;
  }

  /*** combo boxes ***/
  if (DETAIL ("button") && widget && GTK_IS_COMBO_BOX_ENTRY (widget->parent))
  {
    GtkWidget *entry;

    entry = g_object_get_data (G_OBJECT (widget->parent), "moko-combo-entry");
    if (GTK_IS_ENTRY (entry))
    {
      gtk_widget_queue_draw_area (entry, entry->allocation.x, entry->allocation.y, entry->allocation.width,entry->allocation.height);

    }

    g_object_set_data (G_OBJECT (widget->parent), "moko-combo-button", widget);

    /* FIXME: RTL */
    width += 10;
    x -= 10;
  }

  gc = moko_gc_new (style->text_gc[state_type], window);


  /* "fix" for prelight active toggle buttons */
  if (DETAIL ("button") && state_type == GTK_STATE_PRELIGHT && shadow_type == GTK_SHADOW_IN)
  {
    state_type = GTK_STATE_ACTIVE;
  }

  /*** draw the gradient ***/
  if (!DETAIL ("menu") && !DETAIL ("trough"))
   moko_gradient (style, window, state_type, x+1, y+1, width-2, height-2);



   /*** treeview headers ***/
  if (widget && GTK_IS_TREE_VIEW (widget->parent))
  {
    goto exit;
  }


  if (DETAIL ("trough"))
  {
    if (widget && GTK_IS_HSCALE (widget))
    {
      gdk_draw_line (window, gc, x, y + height / 2, x + width, y + height / 2);
      goto exit;
    }
    else if (widget && GTK_IS_VSCALE (widget))
    {
      gdk_draw_line (window, gc, x + width / 2, y, x + width / 2, y + height);
      goto exit;
    }
    else
      gdk_draw_rectangle (window, style->base_gc[state_type], TRUE, x, y, width, height);

  }

  /*** draw the border ***/
  if (!DETAIL ("bar"))
  {
    gdk_draw_rectangle (window, gc, FALSE, x + 1, y + 1, width - 2, height - 2);
  }

exit:
  g_object_unref (gc);

}

static void
moko_draw_shadow (DRAW_ARGS)
{
 GdkGC *gc;

  DEBUG ("draw_shadow");

  if (shadow_type == GTK_SHADOW_NONE)
    return;

  SANITIZE_SIZE;

  /* FIXME: for RTL */
  if (widget && (GTK_IS_SPIN_BUTTON (widget) || GTK_IS_COMBO_BOX_ENTRY (widget->parent)))
      width += 10;

  if (widget && GTK_IS_COMBO_BOX_ENTRY (widget->parent))
  {
    GtkWidget *button;
    g_object_set_data (G_OBJECT (widget->parent), "moko-combo-entry", widget);

    button = g_object_get_data (G_OBJECT (widget->parent), "moko-combo-button");
    if (GTK_IS_BUTTON (button))
      gtk_widget_queue_draw_area (button, button->allocation.x, button->allocation.y, button->allocation.width,button->allocation.height);
  }

  /* draw a hilight shadow on focused widgets (i.e. entry widgets) */
  if (widget && GTK_WIDGET_HAS_FOCUS (widget))
    gc = moko_gc_new (style->base_gc[GTK_STATE_SELECTED], window);
  else
    gc = moko_gc_new (style->text_gc[state_type], window);

  gdk_draw_rectangle (window, gc, FALSE, x + 1, y + 1, width - 2, height - 2);

  g_object_unref (gc);
}

static void
moko_draw_focus (GtkStyle *style, GdkWindow *window, GtkStateType state_type,
                 GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                 gint x, gint y, gint width, gint height)
{
  GdkGC *gc;
  DEBUG ("draw_focus");

  /* no focus indicator for the moment ... */
  return;
/*
  gc = moko_gc_new (style->fg_gc[GTK_STATE_SELECTED], window);
  gdk_draw_rectangle (window, gc, FALSE, x, y, width, height);
  g_object_unref (gc);
*/
}

static void
moko_draw_check (GtkStyle * style, GdkWindow * window,
		 GtkStateType state_type, GtkShadowType shadow_type,
		 GdkRectangle * area, GtkWidget * widget,
		 const gchar * detail, gint x, gint y, gint width,
		 gint height)
{
  GdkGC *gc;

  DEBUG ("draw_check");
  gc = moko_gc_new (style->text_gc[state_type], window);

  /* clear the background */
  gdk_draw_rectangle (window, style->base_gc[GTK_STATE_NORMAL], TRUE, x+1, y+1, width-2, height-2);

  if (shadow_type == GTK_SHADOW_IN)
  {
    GdkGC *mark_gc;
    mark_gc = moko_gc_new (style->base_gc[GTK_STATE_SELECTED], window);
    gdk_draw_rectangle (window, mark_gc, TRUE, x + 3, y + 3, width - 6, height - 6);
    g_object_unref (mark_gc);
  }

  gdk_draw_rectangle (window, gc, FALSE, x, y, width, height);

  g_object_unref (gc);
}

static void
moko_draw_option (GtkStyle * style, GdkWindow * window,
		  GtkStateType state_type, GtkShadowType shadow_type,
		  GdkRectangle * area, GtkWidget * widget,
		  const gchar * detail, gint x, gint y, gint width,
		  gint height)
{
  GdkGC *gc;

  DEBUG ("draw_option");

  gc = moko_gc_new (style->text_gc[state_type], window);


  /* clear the background */
  gdk_draw_arc (window, style->base_gc[GTK_STATE_NORMAL], TRUE, x+1, y+1, width-2, height-2, 0, 360 * 64);

  if (shadow_type == GTK_SHADOW_IN)
  {
    GdkGC *mark_gc;
    mark_gc = moko_gc_new (style->base_gc[GTK_STATE_SELECTED], window);
    gdk_draw_arc (window, mark_gc, TRUE, x + 3, y + 3, width - 6, height - 6, 0, 360 * 64);
    g_object_unref (mark_gc);
  }
  gdk_draw_arc (window, gc, FALSE, x, y, width, height, 0, 360 * 64);

  g_object_unref (gc);
}

static void
moko_draw_box_gap (GtkStyle * style, GdkWindow * window,
		   GtkStateType state_type, GtkShadowType shadow_type,
		   GdkRectangle * area, GtkWidget * widget, const gchar * detail,
		   gint x, gint y, gint width, gint height,
		   GtkPositionType gap_side, gint gap_x, gint gap_width)
{
  GdkGC *gc;
  GdkRectangle rect;

  /* lets try without a border */
  return;

  if (shadow_type == GTK_SHADOW_NONE)
    return;

  gc = moko_gc_new (style->fg_gc [state_type], window);

  /* start off with a rectangle... */
  gdk_draw_rectangle (window, gc, FALSE, x, y, width, height);


  switch (gap_side)
  {
    case GTK_POS_TOP:

      rect.x = x + gap_x;
      rect.y = y;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_BOTTOM:
      rect.x = x + gap_x;
      rect.y = y + height - 1;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_LEFT:
      rect.x = x;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
    case GTK_POS_RIGHT:
      rect.x = x + width - 2;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
  }

  /* and finally blank out the gap */
  gtk_style_apply_default_background (style, window, TRUE, state_type, area,
				      rect.x, rect.y, rect.width,
				      rect.height);


  g_object_unref (gc);
}


static void
moko_draw_extension (GtkStyle * style, GdkWindow * window,
		     GtkStateType state_type, GtkShadowType shadow_type,
		     GdkRectangle * area, GtkWidget * widget,const gchar * detail,
		     gint x, gint y, gint width, gint height,
		     GtkPositionType gap_side)
{

  GdkGC *gc;
  gc = moko_gc_new (style->fg_gc[state_type], window);
  
  moko_gradient (style, window, state_type, x, y, width, height);

  gdk_draw_rectangle (window, gc, FALSE, x, y, width - 1, height - 1);
  g_object_unref (gc);

}


void
moko_draw_style_class_init (GtkStyleClass * style_class)
{

  parent_style_class = g_type_class_peek_parent (style_class);

  style_class->draw_shadow = moko_draw_shadow;
  style_class->draw_box = moko_draw_box;
  style_class->draw_check = moko_draw_check;
  style_class->draw_option = moko_draw_option;
  style_class->draw_box_gap = moko_draw_box_gap;
  style_class->draw_shadow_gap = moko_draw_box_gap;
  style_class->draw_extension = moko_draw_extension;
  style_class->draw_focus = moko_draw_focus;
}
