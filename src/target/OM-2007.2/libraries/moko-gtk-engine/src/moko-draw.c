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

  /* "fix" for prelight active toggle buttons */
  if (DETAIL ("button") && state_type == GTK_STATE_PRELIGHT && shadow_type == GTK_SHADOW_IN)
  {
    state_type = GTK_STATE_ACTIVE;
  }


  if (DETAIL ("trough"))
    gc = style->base_gc[state_type];
  else if (DETAIL ("bar"))
    gc = style->base_gc[GTK_STATE_SELECTED];
  else
    gc = style->bg_gc[state_type];

  if (MOKO_RC_STYLE (style->rc_style)->has_border)
  {
    gdk_draw_rectangle (window, style->dark_gc[state_type], TRUE,
        x, y, width, height);
    x += 2;
    y += 2;
    width -= 4;
    height -= 4;
  }
  gdk_draw_rectangle (window, gc, TRUE, x, y, width, height);

}

static void
moko_draw_shadow (DRAW_ARGS)
{
  GdkGC* gc;
  DEBUG ("draw_shadow");

  if (shadow_type == GTK_SHADOW_NONE)
    return;

  SANITIZE_SIZE;

  gc = gdk_gc_new (window);

  /* draw a hilight shadow on focused widgets (i.e. entry widgets) */
  if (widget && GTK_WIDGET_HAS_FOCUS (widget))
    gdk_gc_copy (gc, style->base_gc[GTK_STATE_SELECTED]);
  else
    gdk_gc_copy (gc, style->base_gc[state_type]);

  gdk_gc_set_line_attributes (gc, 2, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

  gdk_draw_rectangle (window, gc, FALSE, x + 1, y + 1, width - 2, height - 2);

  g_object_unref (gc);
}

static void
moko_draw_focus (GtkStyle *style, GdkWindow *window, GtkStateType state_type,
                 GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                 gint x, gint y, gint width, gint height)
{
  DEBUG ("draw_focus");

  /* no focus indicator for the moment ... */
}

static void
moko_draw_check (GtkStyle * style, GdkWindow * window,
		 GtkStateType state_type, GtkShadowType shadow_type,
		 GdkRectangle * area, GtkWidget * widget,
		 const gchar * detail, gint x, gint y, gint width,
		 gint height)
{
  GdkGC *gc, *border;

  if (state_type == GTK_STATE_PRELIGHT)
    state_type = GTK_STATE_NORMAL;

  if (shadow_type == GTK_SHADOW_IN)
    gc = style->base_gc[GTK_STATE_SELECTED];
  else
    gc = style->base_gc[state_type];

  gdk_draw_rectangle (window, style->black_gc, TRUE,
                      x, y, width, height);

  gdk_draw_rectangle (window, gc, TRUE,
                      x + 2, y + 2, width - 4, height - 4);

}

static void
moko_draw_option (GtkStyle * style, GdkWindow * window,
		  GtkStateType state_type, GtkShadowType shadow_type,
		  GdkRectangle * area, GtkWidget * widget,
		  const gchar * detail, gint x, gint y, gint width,
		  gint height)
{
  DEBUG ("draw_option");

  GdkGC *gc;
  if (state_type == GTK_STATE_PRELIGHT)
    state_type = GTK_STATE_NORMAL;
  if (shadow_type == GTK_SHADOW_IN)
    gc = style->base_gc[GTK_STATE_SELECTED];
  else
    gc = style->base_gc[state_type];


  gdk_draw_arc (window, style->black_gc, TRUE,
   x, y, width, height, 0, 360 * 64);

  gdk_draw_arc (window, gc, TRUE,
   x + 2, y + 2, width - 4, height - 4, 0, 360 * 64);

}

static void
moko_draw_box_gap (GtkStyle * style, GdkWindow * window,
		   GtkStateType state_type, GtkShadowType shadow_type,
		   GdkRectangle * area, GtkWidget * widget, const gchar * detail,
		   gint x, gint y, gint width, gint height,
		   GtkPositionType gap_side, gint gap_x, gint gap_width)
{
  gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE, x, y, width, height);
}


static void
moko_draw_extension (GtkStyle * style, GdkWindow * window,
		     GtkStateType state_type, GtkShadowType shadow_type,
		     GdkRectangle * area, GtkWidget * widget,const gchar * detail,
		     gint x, gint y, gint width, gint height,
		     GtkPositionType gap_side)
{
  gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE, x, y, width, height);
}

void
moko_draw_vline (GtkStyle *style, GdkWindow *window, GtkStateType state_type,
    GdkRectangle *area, GtkWidget *widget, const gchar *detail, gint x1, gint
    x2, gint y)
{
  /* don't paint anything for the moment */
}

void
moko_draw_hline (GtkStyle *style, GdkWindow *window, GtkStateType state_type,
    GdkRectangle *area, GtkWidget *widget, const gchar *detail, gint x1, gint
    x2, gint y)
{
  /* don't paint anything for the moment */
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
  style_class->draw_vline = moko_draw_vline;
  style_class->draw_hline = moko_draw_hline;

}
