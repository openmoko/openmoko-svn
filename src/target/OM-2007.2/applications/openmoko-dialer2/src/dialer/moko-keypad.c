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

#include "moko-keypad.h"

#include "moko-contacts.h"
#include "moko-dialer-textview.h"
#include "moko-dialer-panel.h"
#include "moko-tips.h"

G_DEFINE_TYPE (MokoKeypad, moko_keypad, GTK_TYPE_VBOX)

#define MOKO_KEYPAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_KEYPAD, MokoKeypadPrivate))

struct _MokoKeypadPrivate
{
  gboolean      pin_mode;

  GtkWidget     *tips;
  GtkWidget     *textview;
  GtkWidget     *panel;
  GtkWidget     *delete;
  GtkWidget     *dial;
  GtkWidget     *dial_label;

  GtkWidget     *dialbox;
};

enum
{
  DIAL_NUMBER,
  PIN_ENTRY,
  DIGIT_PRESSED,

  LAST_SIGNAL
};

static guint keypad_signals[LAST_SIGNAL] = {0, };

static GtkWidget*
_get_window (GtkWidget *widget)
{
  GtkWidget *parent = NULL;

  while ((parent = widget->parent))
  {
    if (GTK_IS_WINDOW (parent))
      break;
    widget = parent;
  }
  if (GTK_IS_WINDOW (parent))
    return parent;
  else
    return NULL;
}

void
moko_keypad_set_pin_mode (MokoKeypad *keypad, gboolean pin_mode)
{
  MokoKeypadPrivate *priv;
  GtkWidget *window;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  if (priv->pin_mode == pin_mode)
    return;

  priv->pin_mode = pin_mode;
  
  if (pin_mode)
  {
    window = _get_window (GTK_WIDGET (keypad));
    if (GTK_IS_WINDOW (window))
      gtk_window_set_title (GTK_WINDOW (window), "Enter Pin");
    gtk_label_set_markup (GTK_LABEL (priv->dial_label), "Send\nPin");
  }
  else
  {
    window = _get_window (GTK_WIDGET (keypad));
    if (GTK_IS_WINDOW (window))
      gtk_window_set_title (GTK_WINDOW (window), "Dialer");
    gtk_label_set_markup (GTK_LABEL (priv->dial_label), "Dial");
  }

}

void
moko_keypad_set_display_text (MokoKeypad *keypad, const gchar *text)
{
  MokoKeypadPrivate *priv;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  moko_dialer_textview_empty (MOKO_DIALER_TEXTVIEW (priv->textview));
  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->textview), text);
}

void
moko_keypad_set_talking (MokoKeypad *keypad, gboolean talking)
{
  MokoKeypadPrivate *priv;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  if (talking)
    gtk_widget_hide (priv->dialbox);
  else
    gtk_widget_show_all (priv->dialbox);
}

/* Callbacks */
static void
on_tip_selected (MokoTips *tips, MokoContactEntry *entry, MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  g_return_if_fail (entry);
  priv = keypad->priv;

  moko_dialer_textview_empty (MOKO_DIALER_TEXTVIEW (priv->textview));
  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->textview),
                               entry->number);

  g_debug ("%s", entry->number);
  g_signal_emit (G_OBJECT (keypad), keypad_signals[DIAL_NUMBER], 
                 0, entry->number);
}

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
 if (priv->pin_mode) {
   g_signal_emit (G_OBJECT (keypad), keypad_signals[PIN_ENTRY], 0, number);
   moko_dialer_textview_empty (MOKO_DIALER_TEXTVIEW (priv->textview));
 } else
   g_signal_emit (G_OBJECT (keypad), keypad_signals[DIAL_NUMBER], 0, number);
 
}

static gboolean
moko_keypad_hold_timeout (MokoDialerTextview *textview)
{
  moko_dialer_textview_empty (textview);
  return FALSE;
}

static gboolean
on_delete_event (GtkWidget *button, GdkEventButton *event, MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  MokoDialerTextview *textview;
  GList *matches;
  static gint hold_timeout_source = 0;

  g_return_val_if_fail (MOKO_IS_KEYPAD (keypad), FALSE);
  priv = keypad->priv;

  textview = MOKO_DIALER_TEXTVIEW (priv->textview);

  if (event->type == GDK_BUTTON_PRESS)
  {
    moko_dialer_textview_delete (textview);
    hold_timeout_source = g_timeout_add (800, (GSourceFunc) moko_keypad_hold_timeout, textview);
    return FALSE;
  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    g_source_remove (hold_timeout_source);
  }

  if (!priv->pin_mode)
  {
    /* Some autocomplete stuff */
    matches = moko_contacts_fuzzy_lookup (moko_contacts_get_default (),
                                          moko_dialer_textview_get_input (
                                          MOKO_DIALER_TEXTVIEW (priv->textview),
                                          TRUE));
    moko_tips_set_matches (MOKO_TIPS (priv->tips), matches);
  }
  
  return FALSE;
}

static void
on_panel_user_input (MokoDialerPanel *panel, 
                      const gchar      digit, 
                      MokoKeypad      *keypad)
{
  MokoKeypadPrivate *priv;
  gchar buf[2];
  GList *matches = NULL;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  /* Phones use '#' for PIN 'entered' signal */
  if (priv->pin_mode && digit == '#')
  { 
    on_dial_clicked (NULL, keypad);
    return;
  }   

  /* Create a string to insert into the textview */
  buf[0] = digit;
  buf[1] = '\0';

  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->textview), buf);

  if (!priv->pin_mode)
  {
    /* Some autocomplete stuff */
    gchar *text = moko_dialer_textview_get_input (
                                          MOKO_DIALER_TEXTVIEW (priv->textview), 
                                          TRUE);
    
    matches = moko_contacts_fuzzy_lookup (moko_contacts_get_default (), text);
    moko_tips_set_matches (MOKO_TIPS (priv->tips), matches);
    g_signal_emit (G_OBJECT (keypad), keypad_signals[DIGIT_PRESSED], 0, digit);
    g_free (text);
 }
}

static void
on_panel_user_hold (MokoDialerPanel *panel, 
                     const gchar      digit, 
                     MokoKeypad      *keypad)
{
  MokoKeypadPrivate *priv;
  gchar buf[3];
  GList *matches = NULL;

  g_return_if_fail (MOKO_IS_KEYPAD (keypad));
  priv = keypad->priv;

  /* Phones use '#' for PIN 'entered' signal */
  if (priv->pin_mode && digit == '#')
  {
    on_dial_clicked (NULL, keypad);
    return;
  }

  /* Create a string to insert into the textview */
  buf[0] = digit;
  buf[1] = '\0';

  moko_dialer_textview_delete (MOKO_DIALER_TEXTVIEW (priv->textview));
  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->textview), buf);

  if (!priv->pin_mode)
  {
    /* Some autocomplete stuff */
    gchar *text = moko_dialer_textview_get_input (
                                          MOKO_DIALER_TEXTVIEW (priv->textview), 
                                          TRUE);
    matches = moko_contacts_fuzzy_lookup (moko_contacts_get_default (),
                                          text);
    moko_tips_set_matches (MOKO_TIPS (priv->tips), matches);
    g_signal_emit (G_OBJECT (keypad), keypad_signals[DIGIT_PRESSED], 0, digit);
    g_free (text);
  }
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

 keypad_signals[PIN_ENTRY] =
    g_signal_new ("pin_entry", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoKeypadClass, pin_entry),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  keypad_signals[DIGIT_PRESSED] =
    g_signal_new ("digit_pressed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoKeypadClass, digit_pressed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR,
                  G_TYPE_NONE, 
                  1, G_TYPE_CHAR);

  g_type_class_add_private (obj_class, sizeof (MokoKeypadPrivate)); 
}

static void
moko_keypad_init (MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  GtkWidget *hbox, *vbox;
  GtkWidget *bvbox, *icon, *label, *align;

  priv = keypad->priv = MOKO_KEYPAD_GET_PRIVATE (keypad);

  /* The autocomplete tips */
  priv->tips = moko_tips_new ();
  g_signal_connect (priv->tips, "selected", 
                    G_CALLBACK (on_tip_selected), (gpointer)keypad);
  gtk_box_pack_start (GTK_BOX (keypad), priv->tips, FALSE, FALSE, 0);

  /* The textview */
  priv->textview = moko_dialer_textview_new ();
  gtk_box_pack_start (GTK_BOX (keypad), priv->textview, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (keypad), hbox, TRUE, TRUE, 0);

  /* Dialing pad */
  priv->panel = moko_dialer_panel_new ();
  g_signal_connect (G_OBJECT (priv->panel), "user_input",
                    G_CALLBACK (on_panel_user_input), (gpointer)keypad);
  g_signal_connect (G_OBJECT (priv->panel), "user_hold",
                    G_CALLBACK (on_panel_user_hold), (gpointer)keypad);

  gtk_box_pack_start (GTK_BOX (hbox), priv->panel, TRUE, TRUE, 0);

  priv->dialbox = vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  
  /* Delete button */
  priv->delete = gtk_button_new ();
  g_signal_connect (priv->delete, "button-press-event",
                    G_CALLBACK (on_delete_event), (gpointer)keypad);
  g_signal_connect (priv->delete, "button-release-event",
                    G_CALLBACK (on_delete_event), (gpointer)keypad);  
  
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
  
  icon = gtk_image_new_from_icon_name ("moko-call-dial", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (bvbox), icon, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Dial");
  priv->dial_label = label;
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
                         "spacing", 0,
                         NULL);

  return GTK_WIDGET (keypad);
}
