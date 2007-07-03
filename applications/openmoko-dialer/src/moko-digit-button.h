/*  moko-button-digit.h
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */

#ifndef _MOKO_DIGIT_BUTTON_H_
#define _MOKO_DIGIT_BUTTON_H_

#include <gtk/gtkbutton.h>

#include <glib-object.h>

G_BEGIN_DECLS
#define MOKO_TYPE_DIGIT_BUTTON                 (moko_digit_button_get_type())
#define MOKO_DIGIT_BUTTON(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIGIT_BUTTON, MokoDigitButton))
#define MOKO_DIGIT_BUTTON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_DIGIT_BUTTON, MokoDigitButtonClass))
#define MOKO_IS_DIGIT_BUTTON(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIGIT_BUTTON))
#define MOKO_IS_DIGIT_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIGIT_BUTTON))
#define MOKO_DIGIT_BUTTON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIGIT_BUTTON, MokoDigitButtonClass))
typedef struct _MokoDigitButtonClass MokoDigitButtonClass;

typedef struct _MokoDigitButton MokoDigitButton;
struct _MokoDigitButton
{
  GtkButton gtkbutton;
};

struct _MokoDigitButtonClass
{
  GtkButtonClass parent_class;
};


GType moko_digit_button_get_type (void);

gboolean moko_digit_button_set_numbers (GtkWidget * widget, gchar left,
                                        gchar right);

GtkWidget *moko_digit_button_new_with_labels (const gchar * label1,
                                              const gchar * label2);

GtkWidget *moko_digit_button_new ();

gchar moko_digit_button_get_right (MokoDigitButton * button);

gchar moko_digit_button_get_left (MokoDigitButton * button);

G_END_DECLS
#endif // _MOKO_FINGER_WHEEL_H_
