/*
 *  moko-keypad; The keypads keypad.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef _HAVE_MOKO_KEYPAD_H
#define _HAVE_MOKO_KEYPAD_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_KEYPAD (moko_keypad_get_type ())

#define MOKO_KEYPAD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_KEYPAD, MokoKeypad))

#define MOKO_KEYPAD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_KEYPAD, MokoKeypadClass))

#define MOKO_IS_KEYPAD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_KEYPAD))

#define MOKO_IS_KEYPAD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_KEYPAD))

#define MOKO_KEYPAD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_KEYPAD, MokoKeypadClass))

typedef struct _MokoKeypad MokoKeypad;
typedef struct _MokoKeypadClass MokoKeypadClass;
typedef struct _MokoKeypadPrivate MokoKeypadPrivate;

struct _MokoKeypad
{
  GtkVBox         parent;

  /*< private >*/
  MokoKeypadPrivate   *priv;
};

struct _MokoKeypadClass 
{
  /*< private >*/
  GtkVBoxClass    parent_class;
  
  /* signals */
  void (*dial_number) (MokoKeypad *keypad, const gchar *number);
  void (*pin_entry) (MokoKeypad *keypad, const gchar *number);
  void (*digit_pressed) (MokoKeypad *keypad, const gchar digit);

  /* future padding */
  void (*_moko_keypad_1) (void);
  void (*_moko_keypad_2) (void);
  void (*_moko_keypad_3) (void);
  void (*_moko_keypad_4) (void);
}; 

GType moko_keypad_get_type (void) G_GNUC_CONST;

GtkWidget* moko_keypad_new (void);

void moko_keypad_set_pin_mode (MokoKeypad *keypad, gboolean pin_mode);
void moko_keypad_set_talking (MokoKeypad *keypad, gboolean talking);
void moko_keypad_set_display_text (MokoKeypad *keypad, const gchar *text);

G_END_DECLS

#endif /* _HAVE_MOKO_KEYPAD_H */
