/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
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

#include "matchbox-keyboard.h"

struct MBKeyboardLayout
{
  MBKeyboard       *kbd;  
  char             *id;
  List             *rows;

  /* background image and changer background */
  char             *background;

  char             *changerground;
  int               changer_x, changer_y, changer_w, changer_h;

  /* real size */
  Bool              realsize;
  int               width;
  int               height;

  /* layout type */
  MBKeyboardLayoutType type;
};


MBKeyboardLayout*
mb_kbd_layout_new(MBKeyboard *kbd, const char *id)
{
  MBKeyboardLayout *layout = NULL;

  layout = util_malloc0(sizeof(MBKeyboardLayout));

  layout->kbd = kbd;
  layout->id  = strdup(id);

  return layout;
}

void
mb_kbd_layout_append_row(MBKeyboardLayout *layout,
			 MBKeyboardRow    *row)
{
  layout->rows = util_list_append(layout->rows, (pointer)row);
}

List*
mb_kbd_layout_rows(MBKeyboardLayout *layout)
{
  return util_list_get_first(layout->rows);
}

/* background image */
void
mb_kbd_layout_set_background(MBKeyboardLayout *layout,
                             const char  *background)
{
  layout->background = strdup(background);
}

const char *
mb_kbd_layout_get_background(MBKeyboardLayout *layout)
{
  return layout->background;
}

/* changer image */
void
mb_kbd_layout_set_changerground(MBKeyboardLayout *layout,
                                const char       *changerground)
{
  layout->changerground = strdup(changerground);
}

const char *
mb_kbd_layout_get_changerground(MBKeyboardLayout *layout)
{
  return layout->changerground;
}

void
mb_kbd_layout_set_changerground_x(MBKeyboardLayout *layout,
                                  int               x)
{
  layout->changer_x = x;
}

int
mb_kbd_layout_get_changerground_x(MBKeyboardLayout *layout)
{
  return layout->changer_x;
}

void
mb_kbd_layout_set_changerground_y(MBKeyboardLayout *layout,
                                  int               y)
{
  layout->changer_y = y;
}

int
mb_kbd_layout_get_changerground_y(MBKeyboardLayout *layout)
{
  return layout->changer_y;
}

void
mb_kbd_layout_set_changerground_w(MBKeyboardLayout *layout,
                                  int               w)
{
  layout->changer_w = w;
}

int
mb_kbd_layout_get_changerground_w(MBKeyboardLayout *layout)
{
  return layout->changer_w;
}

void
mb_kbd_layout_set_changerground_h(MBKeyboardLayout *layout,
                                  int               h)
{
  layout->changer_h = h;
}

int
mb_kbd_layout_get_changerground_h(MBKeyboardLayout *layout)
{
  return layout->changer_h;
}

/* real size */
void
mb_kbd_layout_set_realsize(MBKeyboardLayout *layout,
                           int realsize)
{
  layout->realsize = realsize;
}

int
mb_kbd_layout_realsize(MBKeyboardLayout *layout)
{
  return layout->realsize;
}

void
mb_kbd_layout_set_width(MBKeyboardLayout *layout,
                        int width)
{
  layout->width = width;
}

int
mb_kbd_layout_get_width(MBKeyboardLayout *layout)
{
  return layout->width;
}

void
mb_kbd_layout_set_height(MBKeyboardLayout *layout,
                         int height)
{
  layout->height = height;
}

int
mb_kbd_layout_get_height(MBKeyboardLayout *layout)
{
  return layout->height;
}

/* layout type */
void
mb_kbd_layout_set_type(MBKeyboardLayout     *layout,
                       MBKeyboardLayoutType  type)
{
  layout->type = type;
}

MBKeyboardLayoutType
mb_kbd_layout_get_type(MBKeyboardLayout *layout)
{
  return layout->type;
}
