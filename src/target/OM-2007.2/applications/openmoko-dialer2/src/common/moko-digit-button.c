/*  moko-button-digit.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */

#include "moko-digit-button.h"
#include <gtk/gtkwindow.h>
#include <gtk/gtk.h>
#include <gtk/gtkbutton.h>


G_DEFINE_TYPE (MokoDigitButton, moko_digit_button, GTK_TYPE_BUTTON)

#define MOKO_DIGIT_BUTTON_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DIGIT_BUTTON, MokoDigitButtonPrivate))
struct _MokoDigitButtonPrivate
{
  GtkWidget *labelDigit;
  GtkWidget *labelAcrobat;
  gchar leftclickdigit;
  gchar rightclickdigit;
};



typedef struct _MokoDigitButtonPrivate MokoDigitButtonPrivate;

GtkWidget
*moko_digit_button_new ()
{
  return moko_digit_button_new_with_labels ("1", "ABC");
}


/**
 * @brief new a MokoDigitButton with the 2 strings.
 * @param string_digit  the left digit part of the button. such as '1','2'...'0'
 * @param string_acrobat  the right acrobat part of the button, such as 'ABC' etc.
 */
GtkWidget *
moko_digit_button_new_with_labels (const gchar * string_digit,
                                   const gchar * string_acrobat)
{
  gchar *str;
  MokoDigitButton *digitbutton =
    (MokoDigitButton *) g_object_new (MOKO_TYPE_DIGIT_BUTTON, NULL);

  gtk_widget_show (GTK_WIDGET (digitbutton));
  GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (digitbutton), GTK_CAN_FOCUS);

  GtkWidget *alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (digitbutton), alignment);
  gtk_widget_show (alignment);

  GtkWidget *hbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (alignment), hbox);


  GtkWidget *labelDigit = gtk_label_new (NULL);
  str = g_markup_printf_escaped ("<span size=\"xx-large\">%s</span>", string_digit);
  gtk_label_set_markup (GTK_LABEL (labelDigit), str);
  gtk_box_pack_start (GTK_BOX (hbox), labelDigit, TRUE, TRUE, 0);
  g_free (str);

  GtkWidget *labelAcrobat = gtk_label_new (NULL);
  str = g_markup_printf_escaped ("<small>%s</small>", string_acrobat);
  gtk_label_set_markup (GTK_LABEL (labelAcrobat), str);
  g_free (str);
  gtk_container_add (GTK_CONTAINER (hbox), labelAcrobat);

  gtk_widget_set_name (GTK_WIDGET (digitbutton), "mokodialerdigitbutton");

  MokoDigitButtonPrivate *priv =
    (MokoDigitButtonPrivate *) MOKO_DIGIT_BUTTON_GET_PRIVATE (digitbutton);

  priv->labelDigit = labelDigit;
  priv->labelAcrobat = labelAcrobat;

  gtk_widget_show_all (alignment);

  return GTK_WIDGET (digitbutton);

}

gboolean
moko_digit_button_set_numbers (GtkWidget * widget, gchar left, gchar right)
{

  g_return_val_if_fail (MOKO_IS_DIGIT_BUTTON (widget), FALSE);
  MokoDigitButtonPrivate *priv =
    (MokoDigitButtonPrivate *) MOKO_DIGIT_BUTTON_GET_PRIVATE (widget);
  g_return_val_if_fail (priv != NULL, FALSE);

  priv->leftclickdigit = left;
  priv->rightclickdigit = right;
  return TRUE;
}

static void
moko_digit_button_class_init (MokoDigitButtonClass * klass)
{
  g_type_class_add_private (klass, sizeof (MokoDigitButtonPrivate));
  return;
}

/**
 * @brief  set the digit button digit field to be -1.
 */
static void
moko_digit_button_init (MokoDigitButton * self)
{

  MokoDigitButtonPrivate *priv = MOKO_DIGIT_BUTTON_GET_PRIVATE (self);
  priv->labelDigit = 0;
  priv->labelAcrobat = 0;
  priv->leftclickdigit = -1;
  priv->rightclickdigit = -1;
  return;
}

gchar
moko_digit_button_get_left (MokoDigitButton * button)
{
  MokoDigitButtonPrivate *priv = MOKO_DIGIT_BUTTON_GET_PRIVATE (button);
  return (priv->leftclickdigit);
}

gchar
moko_digit_button_get_right (MokoDigitButton * button)
{
  MokoDigitButtonPrivate *priv = MOKO_DIGIT_BUTTON_GET_PRIVATE (button);
  return (priv->rightclickdigit);
}

