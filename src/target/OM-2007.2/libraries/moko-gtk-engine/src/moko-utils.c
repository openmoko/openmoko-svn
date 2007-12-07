/*
 * moko-utils.c
 * This file is part of moko-engine
 *
 * Copyright (C) 2006,2007 - OpenedHand Ltd
 *
 * Originally from OpenedHand's Sato GTK+ Engine
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


#include <gtk/gtk.h>
#include <endian.h>

/*
  Shading routines taken from clutter-color.c
 */

#define CFX_ONE    (1 << CFX_Q)	/* 1 */
#define CFX_Q      16		/* Decimal part size in bits */

#define CFX_360 CLUTTER_INT_TO_FIXED (360)
#define CFX_240 CLUTTER_INT_TO_FIXED (240)
#define CFX_180 CLUTTER_INT_TO_FIXED (180)
#define CFX_120 CLUTTER_INT_TO_FIXED (120)
#define CFX_60  CLUTTER_INT_TO_FIXED (60)

#define CLUTTER_FIXED_TO_INT(x)         ((x) >> CFX_Q)
#define CLUTTER_INT_TO_FIXED(x)         ((x) << CFX_Q)
#define CLUTTER_FIXED_DIV(x,y) ((((x) << 8)/(y)) << 8)
#define CLUTTER_FIXED_MUL(x,y) ((x) >> 8) * ((y) >> 8)


#define CFX_INT         CLUTTER_FIXED_TO_INT
#define CFX_MUL         CLUTTER_FIXED_MUL
#define CFX_DIV         CLUTTER_FIXED_DIV

typedef gint32 Fixed;

void color_to_hlsx (GdkColor *src, Fixed *hue, Fixed *luminance, Fixed *saturation);
void color_from_hlsx (GdkColor *dest, Fixed hue, Fixed luminance, Fixed saturation);

/* <private> */
const double _magic = 68719476736.0*1.5;
/* Where in the 64 bits of double is the mantisa */
#if (__FLOAT_WORD_ORDER == 1234)
#define _CFX_MAN			0
#elif (__FLOAT_WORD_ORDER == 4321)
#define _CFX_MAN			1
#else
#define CFX_NO_FAST_CONVERSIONS
#endif

/*
 * double_to_fixed :
 * @value: value to be converted
 *
 * A fast conversion from double precision floating to fixed point
 *
 * Return value: Fixed point representation of the value
 *
 * Since: 0.2
 */

Fixed
double_to_fixed (double val)
{
  union 
  {
    double d;
    unsigned int i[2];
  } dbl;

  dbl.d = val;
  dbl.d = dbl.d + _magic;
  return dbl.i[_CFX_MAN];
}

/* k is a percentage */
void
moko_shade_colour (GdkColor *src,
                   GdkColor *dest,
                   gdouble      k)
{
  Fixed h, l, s, shade;
  GdkColor _src;

  shade = double_to_fixed (k);

  /* convert to 8 bit per channel */
  _src = *src;
  _src.red = _src.red / 257;
  _src.blue = _src.blue / 257;
  _src.green = _src.green / 257;


  color_to_hlsx (&_src, &h, &l, &s);

  l = l + CFX_MUL (CFX_ONE - l, shade);
  if (l > CFX_ONE)
    l = CFX_ONE;
  else if (l < 0)
    l = 0;
/*
  s = s + CFX_MUL (CFX_ONE - s, shade);
  if (s > CFX_ONE)
    s = CFX_ONE;
  else if (s < 0)
    s = 0;
*/
  color_from_hlsx (dest, h, l, s);

  /* convert back to 16 bit per channel */
  dest->red = dest->red * 257;
  dest->blue = dest->blue * 257;
  dest->green = dest->green * 257;
}

/**
 * color_to_hlsx:
 * @src: a #GdkColor
 * @hue: return location for the hue value or %NULL
 * @luminance: return location for the luminance value or %NULL
 * @saturation: return location for the saturation value or %NULL
 *
 * Converts @src to the HLS format. Returned hue is in degrees (0 .. 360),
 * luminance and saturation from interval <0 .. 1>.
 */
void
color_to_hlsx (GdkColor *src,
	       Fixed       *hue,
	       Fixed       *luminance,
	       Fixed       *saturation)
{
  Fixed red, green, blue;
  Fixed min, max, delta;
  Fixed h, l, s;
  
  g_return_if_fail (src != NULL);

  red   = CLUTTER_INT_TO_FIXED (src->red)   / 255;
  green = CLUTTER_INT_TO_FIXED (src->green) / 255;
  blue  = CLUTTER_INT_TO_FIXED (src->blue)  / 255;

  if (red > green)
    {
      if (red > blue)
	max = red;
      else
	max = blue;

      if (green < blue)
	min = green;
      else
	min = blue;
    }
  else
    {
      if (green > blue)
	max = green;
      else
	max = blue;

      if (red < blue)
	min = red;
      else
	min = blue;
    }

  l = (max + min) / 2;
  s = 0;
  h = 0;

  if (max != min)
    {
      if (l <= CFX_ONE/2)
	s = CFX_DIV ((max - min), (max + min));
      else
	s = CFX_DIV ((max - min), (CLUTTER_INT_TO_FIXED (2) - max - min));

      delta = max - min;
      if (red == max)
	h = CFX_DIV ((green - blue), delta);
      else if (green == max)
	h = CLUTTER_INT_TO_FIXED (2) + CFX_DIV ((blue - red), delta);
      else if (blue == max)
	h = CLUTTER_INT_TO_FIXED (4) + CFX_DIV ((red - green), delta);

      h *= 60;
      if (h < 0)
	h += CLUTTER_INT_TO_FIXED (360);
    }

  if (hue)
    *hue = h;

  if (luminance)
    *luminance = l;

  if (saturation)
    *saturation = s;
}

/**
 * color_from_hlsx:
 * @dest: return location for a #GdkColor
 * @hue: hue value (0 .. 360)
 * @luminance: luminance value (0 .. 1)
 * @saturation: saturation value (0 .. 1)
 *
 * Converts a color expressed in HLS (hue, luminance and saturation)
 * values into a #GdkColor.
 */

void
color_from_hlsx (GdkColor *dest,
		 Fixed   hue,
		 Fixed   luminance,
		 Fixed   saturation)
{
  Fixed h, l, s;
  Fixed m1, m2;
  
  g_return_if_fail (dest != NULL);

  l = luminance;
  s = saturation;

  if (l <= CFX_ONE/2)
    m2 = CFX_MUL (l, (CFX_ONE + s));
  else
    m2 = l + s - CFX_MUL (l,s);

  m1 = 2 * l - m2;

  if (s == 0)
    {
      dest->red   = (guint8) CFX_INT (l * 255);
      dest->green = (guint8) CFX_INT (l * 255);
      dest->blue  = (guint8) CFX_INT (l * 255);
    }
  else
    {
      h = hue + CFX_120;
      while (h > CFX_360)
	h -= CFX_360;
      while (h < 0)
	h += CFX_360;

      if (h < CFX_60)
	dest->red = (guint8) CFX_INT((m1 + CFX_MUL((m2-m1), h) / 60) * 255);
      else if (h < CFX_180)
	dest->red = (guint8) CFX_INT (m2 * 255);
      else if (h < CFX_240)
	dest->red = (guint8)CFX_INT((m1+CFX_MUL((m2-m1),(CFX_240-h))/60)*255);
      else
	dest->red = (guint8) CFX_INT (m1 * 255);

      h = hue;
      while (h > CFX_360)
	h -= CFX_360;
      while (h < 0)
	h += CFX_360;

      if (h < CFX_60)
	dest->green = (guint8)CFX_INT((m1 + CFX_MUL((m2 - m1), h) / 60) * 255);
      else if (h < CFX_180)
        dest->green = (guint8) CFX_INT (m2 * 255);
      else if (h < CFX_240)
	dest->green =
	    (guint8) CFX_INT((m1 + CFX_MUL ((m2-m1), (CFX_240-h)) / 60) * 255);
      else
	dest->green = (guint8) CFX_INT (m1 * 255);

      h = hue - CFX_120;
      while (h > CFX_360)
	h -= CFX_360;
      while (h < 0)
	h += CFX_360;

      if (h < CFX_60)
	dest->blue = (guint8) CFX_INT ((m1 + CFX_MUL ((m2-m1), h) / 60) * 255);
      else if (h < CFX_180)
	dest->blue = (guint8) CFX_INT (m2 * 255);
      else if (h < CFX_240)
	dest->blue = (guint8)CFX_INT((m1+CFX_MUL((m2-m1),(CFX_240-h))/60)*255);
      else
	dest->blue = (guint8) CFX_INT(m1 * 255);
    }
}
