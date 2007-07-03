/*
 *  moko-keypad; The keypads keypad
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

#include <gtk/gtk.h>
#include <gtk/gtk.h>

#include <libmokoui/moko-stock.h>

#include "moko-keypad.h"

#include "moko-dialer-textview.h"
#include "moko-dialer-panel.h"

G_DEFINE_TYPE (MokoKeypad, moko_keypad, GTK_TYPE_VBOX)

#define MOKO_KEYPAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_KEYPAD, MokoKeypadPrivate))

struct _MokoKeypadPrivate
{
  GtkWidget     *textview;
  GtkWidget     *panel;
  GtkWidget     *delete;
  GtkWidget     *dial;
};

enum
{
  DIAL_NUMBER,

  LAST_SIGNAL
};

static guint keypad_signals[LAST_SIGNAL] = {0, };

/* Callbacks */

static void
on_dial_clicked (GtkWidget *button, MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  const gchar *number;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  number = moko_dialer_textview_get_input (
                                        MOKO_DIALER_TEXTVIEW (priv->textview), 
                                        TRUE);
  g_signal_emit (G_OBJECT (keypad), keypad_signals[DIAL_NUMBER], 0, number);
}

static void
on_delete_clicked (GtkWidget *button, MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  MokoDialerTextview *textview;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;
  
  textview = MOKO_DIALER_TEXTVIEW (priv->textview);

  moko_dialer_textview_delete (textview);
}

static void
on_panel_user_input (MokoDialerPanel *panel, 
                      const gchar      digit, 
                      MokoKeypad      *keypad)
{
  MokoKeypadPrivate *priv;
  gchar buf[2];

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  /* Create a string to insert into the textview */
  buf[0] = digit;
  buf[1] = '\0';

  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->textview), buf);
}

static void
on_panel_user_hold (MokoDialerPanel *panel, 
                     const gchar      digit, 
                     MokoKeypad      *keypad)
{
  g_print ("on_panel_user_hold: %c\n", digit);
}

/* GObject functions */
static void
moko_keypad_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_keypad_parent_class)->dispose (object);
}

static void
moko_keypad_finalize (GObject *keypad)
{
  G_OBJECT_CLASS (moko_keypad_parent_class)->finalize (keypad);
}


static void
moko_keypad_class_init (MokoKeypadClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_keypad_finalize;
  obj_class->dispose = moko_keypad_dispose;

  keypad_signals[DIAL_NUMBER] =
    g_signal_new ("dial_number", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoKeypadClass, dial_number),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  g_type_class_add_private (obj_class, sizeof (MokoKeypadPrivate)); 
}

static void
moko_keypad_init (MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  GtkWidget *hbox, *vbox;
  GtkStockItem stock_item;
  GtkWidget *bvbox, *icon, *label, *align;

  priv = keypad->priv = MOKO_KEYPAD_GET_PRIVATE (keypad);

  /* The textview */
  priv->textview = moko_dialer_textview_new ();
  gtk_box_pack_start (GTK_BOX (keypad), priv->textview, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (keypad), hbox, TRUE, TRUE, 0);

  /* Dialing pad */
  priv->panel = moko_dialer_panel_new ();
  g_signal_connect (G_OBJECT (priv->panel), "user_input",
                    G_CALLBACK (on_panel_user_input), (gpointer)keypad);
  g_signal_connect (G_OBJECT (priv->panel), "user_hold",
                    G_CALLBACK (on_panel_user_hold), (gpointer)keypad);

  gtk_box_pack_start (GTK_BOX (hbox), priv->panel, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  
  /* Delete button */
  priv->delete = gtk_button_new ();
  g_signal_connect (G_OBJECT (priv->delete), "clicked",
                    G_CALLBACK (on_delete_clicked), (gpointer)keypad); 
  bvbox = gtk_vbox_new (FALSE, 0);
  
  icon = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (bvbox), icon, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Delete");
  gtk_box_pack_start (GTK_BOX (bvbox), label, FALSE, FALSE, 0);
  
  gtk_container_add (GTK_CONTAINER (priv->delete), bvbox);
  gtk_widget_set_name (priv->delete, "mokofingerbutton-orange");
  gtk_box_pack_start (GTK_BOX (vbox), priv->delete, FALSE, FALSE, 0);
   /* Dial button */
  priv->dial = gtk_button_new ();
  g_signal_connect (G_OBJECT (priv->dial), "clicked",
                    G_CALLBACK (on_dial_clicked), (gpointer)keypad);
  bvbox = gtk_vbox_new (FALSE, 0);
  align = gtk_alignment_new (0.5, 0.5, 1, 0);
  
  icon = gtk_image_new_from_stock (MOKO_STOCK_CALL_DIAL, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (bvbox), icon, FALSE, FALSE, 0);
  
  gtk_stock_lookup (MOKO_STOCK_CALL_DIAL, &stock_item);
  label = gtk_label_new (stock_item.label);
  gtk_box_pack_start (GTK_BOX (bvbox), label, FALSE, FALSE, 0);
  
  gtk_container_add (GTK_CONTAINER (align), bvbox);
  gtk_container_add (GTK_CONTAINER (priv->dial), align);
  gtk_widget_set_name (priv->dial, "mokofingerbutton-black");
  gtk_box_pack_start (GTK_BOX (vbox), priv->dial, TRUE, TRUE, 0);
 }

GtkWidget*
moko_keypad_new (void)
{
  MokoKeypad *keypad = NULL;
  
  keypad = g_object_new (MOKO_TYPE_KEYPAD, 
                         "homogeneous", FALSE,
                         "spacing", 12,
                         NULL);

  return GTK_WIDGET (keypad);
}
