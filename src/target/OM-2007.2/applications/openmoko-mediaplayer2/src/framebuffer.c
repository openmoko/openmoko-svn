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

#include <fcntl.h>
#include <glib.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <bits/types.h>
#include <termios.h>
#include <unistd.h>

#include "framebuffer.h"

/**
 * Opens and returns a new handle to the default framebuffer device
 * @param angle Angle of screen rotation (0, 90, 180 or 270)
 * @return The framebuffer handle
 */
framebuffer*
fb_new(gint angle)
{
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;

	gint off;
	gchar *fbdev = NULL;
	framebuffer *fb = NULL;

	fbdev = getenv("FBDEV");
	if (!fbdev) fbdev = "/dev/fb0";

	if (!(fb = malloc(sizeof(framebuffer))))
	{
		g_printerr("Out of memory while trying to allocate framebuffer handle\n");
		goto fail;
	}

	memset (fb, 0, sizeof(framebuffer));
	fb->fd = -1;

	if ((fb->fd = open(fbdev, O_RDWR)) < 0)
	{
		g_printerr("Error opening %s\n", fbdev);
		goto fail;
	}

	if ( (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb_fix) == -1) ||
	     (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb_var) == -1) )
	{
		g_printerr("Error obtaining framebuffer info\n");
		goto fail;
	}

	if (fb_var.bits_per_pixel < 16)
	{
		g_printerr("Error: %d bpp framebuffers are not supported\n", fb_var.bits_per_pixel);
		goto fail;
	}

	fb->real_width  = fb->width  = fb_var.xres;
	fb->real_height = fb->height = fb_var.yres;
	fb->bpp    = fb_var.bits_per_pixel;
	fb->stride = fb_fix.line_length;
	fb->type   = fb_fix.type;
	fb->visual = fb_fix.visual;

	fb->base = (gchar*) mmap((__caddr_t) NULL,
		/*fb_fix.smem_len */
		fb->stride * fb->height,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fb->fd, 0);

	if (fb->base == (char*)-1)
	{
		g_printerr("Error: cannot mmap() the framebuffer\n");
		goto fail;
	}

	off = (unsigned long) fb_fix.smem_start % (unsigned long) sysconf(_SC_PAGESIZE);
	fb->data = fb->base + off;

	fb->angle = angle;

	if ( (fb->angle == 90) || (fb->angle == 270) )
	{
			fb->width  = fb->real_height;
			fb->height = fb->real_width;
	} else {
			fb->width  = fb->real_width;
			fb->height = fb->real_height;
	}

	return fb;

fail:

	if (fb) fb_free(fb);
	return NULL;
}

/**
 * Closes a framebuffer handle
 */
void
fb_free(framebuffer *fb)
{
	if (!fb) return;

	if (fb->fd >= 0) close (fb->fd);

	free(fb);
}

/**
 * Sets one pixel of the framebuffer to a specific color
 * @param fb Framebuffer handle
 * @param x X coordinate
 * @param y Y coordinate
 * @param red R component (0..255)
 * @param green G component (0..255)
 * @param blue B component (0..255)
 */
inline void
psplash_fb_plot_pixel(framebuffer *fb,
	gint x, gint y,
	guint8 red, guint8 green, guint8 blue)
{
	#define OFFSET(fb,x,y) (((y) * (fb)->stride) + ((x) * ((fb)->bpp >> 3)))

	gint off;

	if ( (x < 0) || (x > fb->width-1) ||
	     (y < 0) || (y > fb->height-1) ) return;

	switch (fb->angle)
	{
		case 270:
			off = OFFSET(fb, fb->height-y-1, x);
			break;

		case 180:
			off = OFFSET(fb, fb->width-x-1, fb->height-y-1);
			break;

		case 90:
			off = OFFSET(fb, y, fb->width-x-1);
			break;

		case 0:
		default:
			off = OFFSET(fb, x, y);
	}

	/* FIXME: handle no RGB orderings */
	switch (fb->bpp)
	{
		case 24:
		case 32:
			*(fb->data + off)     = red;
			*(fb->data + off + 1) = green;
			*(fb->data + off + 2) = blue;
			break;

		case 16:
			*(volatile guint16*) (fb->data + off) = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			break;

		default:
			/* depth not supported yet */
			break;
	}
}

/**
 * Fills a rectangular area with one color
 * @param fb Framebuffer handle
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param red R component (0..255)
 * @param green G component (0..255)
 * @param blue B component (0..255)
 */
void
fb_draw_rect(framebuffer *fb,
	gint x, gint y,
	gint width, gint height,
	guint8 red, guint8 green, guint8 blue)
{
	gint dx, dy;

	for (dy=0; dy<height; dy++)
		for (dx=0; dx<width; dx++)
			psplash_fb_plot_pixel(fb, x+dx, y+dy, red, green, blue);
}

void
fb_draw_image(framebuffer *fb,
	gint x, gint y,
	gint img_width, gint img_height,
	gint img_bytes_per_pixel,
	guint8 *rle_data)
{
	guint8 *p = rle_data;
	gint dx = 0, dy = 0, total_len;
	guint len;

	total_len = img_width * img_height * img_bytes_per_pixel;

	/* FIXME: Optimise, check for over runs ... */
	while ((p-rle_data) < total_len)
	{
		len = *(p++);

		if (len & 128)
		{
			len -= 128;

			if (!len) break;

			do
			{
				psplash_fb_plot_pixel(fb, x+dx, y+dy, *p, *(p+1), *(p+2));
				if (++dx >= img_width) { dx=0; dy++; }
			}
			while (--len && (p-rle_data) < total_len);

			p += img_bytes_per_pixel;

		} else {

			if (!len) break;

			do
			{
				psplash_fb_plot_pixel (fb, x+dx, y+dy, *p, *(p+1), *(p+2));
				if (++dx >= img_width) { dx=0; dy++; }
				p += img_bytes_per_pixel;
			}
			while (--len && (p - rle_data) < total_len);
		}
	}
}
