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

#define DETAIL(foo) (detail && strcmp (foo, detail) == 0)

GtkStyleClass *parent_style_class;

static void
moko_draw_box (DRAW_ARGS)
{
  GdkGC *gc;

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

  /* hack to remove prelight */
  if (state_type == GTK_STATE_PRELIGHT
      && !DETAIL ("menuitem") && !DETAIL ("bar"))
  {
    if (widget && GTK_IS_TOGGLE_BUTTON (widget)
        && shadow_type == GTK_SHADOW_IN)
    {
      state_type = GTK_STATE_ACTIVE;
    }
    else
    {
      if (widget && GTK_IS_BUTTON (widget)
          && (gtk_button_get_relief (GTK_BUTTON (widget)) == GTK_RELIEF_NONE))
      {
        /* none relief buttons shouldn't draw anything for "normal" state */
        return;
      }
      else
      {
        state_type = GTK_STATE_NORMAL;
      }
    }
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
  /* no focus indicator for the moment ... */
}

static void
moko_draw_check (GtkStyle * style, GdkWindow * window,
		 GtkStateType state_type, GtkShadowType shadow_type,
		 GdkRectangle * area, GtkWidget * widget,
		 const gchar * detail, gint x, gint y, gint width,
		 gint height)
{

  gdk_draw_rectangle (window, style->black_gc, TRUE,
                      x, y, width, height);

  gdk_draw_rectangle (window, style->base_gc[GTK_STATE_NORMAL], TRUE,
                      x + 2, y + 2, width - 4, height - 4);

  if (shadow_type == GTK_SHADOW_IN)
  {
    gdk_draw_rectangle (window, style->base_gc[GTK_STATE_SELECTED], TRUE,
                        x + 4, y + 4, width - 8, height - 8);
  }

}

static void
moko_draw_option (GtkStyle * style, GdkWindow * window,
		  GtkStateType state_type, GtkShadowType shadow_type,
		  GdkRectangle * area, GtkWidget * widget,
		  const gchar * detail, gint x, gint y, gint width,
		  gint height)
{
  cairo_t *cr;

  /* X arc drawing code is really ugly, so we use cairo here */
  gdk_draw_arc (window, style->black_gc, TRUE,
   x, y, width, height, 0, 360 * 64);

  x += 2; y += 2;
  width -= 4; height -=4;

  gdk_draw_arc (window, style->base_gc[state_type], TRUE,
   x, y, width, height, 0, 360 * 64);

  if (shadow_type == GTK_SHADOW_IN)
  {
    x += 2; y += 2;
    width -= 4; height -=4;

    gdk_draw_arc (window, style->base_gc[GTK_STATE_SELECTED], TRUE,
     x, y, width, height, 0, 360 * 64);
  }
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


static void
moko_draw_layout (GtkStyle *style,  GdkWindow *window,
                  GtkStateType state_type, gboolean use_text,
                  GdkRectangle *area, GtkWidget *widget,
                  const char *detail, int x, int y, PangoLayout *layout)
{
  GdkGC *gc;

  gc = use_text ? style->text_gc[state_type] : style->fg_gc[state_type];

  if (area)
  {
    gdk_gc_set_clip_rectangle (gc, area);
  }

  gdk_draw_layout (window, gc, x, y, layout);

  if (area)
  {
    gdk_gc_set_clip_rectangle (gc, NULL);
  }
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
  style_class->draw_layout = moko_draw_layout;

}
