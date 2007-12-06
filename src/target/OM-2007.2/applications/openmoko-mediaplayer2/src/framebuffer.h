/*
 *  pslash - a lightweight framebuffer splashscreen for embedded devices.
 *
 *  Copyright (c) 2006 Matthew Allum <mallum@o-hand.com>
 *
 *  Adapted for the OpenMoko Media Player by Soeren Apel (abraxa@dar-clan.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef _HAVE_FRAMEBUFFER_H
#define _HAVE_FRAMEBUFFER_H

#include <glib.h>
#include <termios.h>

typedef struct
{
  gint fd;
  struct termios save_termios;
  gint type;
  gint visual;
  gint width, height;
  gint bpp;
  gint stride;
  gchar *data;
  gchar *base;

  gint angle;
  gint real_width, real_height;
}  framebuffer;

framebuffer* fb_new(int angle);

void fb_free(framebuffer *fb);

inline void
psplash_fb_plot_pixel(framebuffer *fb,
	gint x, gint y,
	guint8 red, guint8 green, guint8 blue);

void
fb_draw_rect(framebuffer *fb,
	gint x, gint y,
	gint width, gint height,
	guint8 red, guint8 green, guint8 blue);

void
fb_draw_image(framebuffer *fb,
	gint x, gint y,
	gint img_width, gint img_height,
	gint img_bytes_per_pixel,
	guint8 *rle_data);

#endif
