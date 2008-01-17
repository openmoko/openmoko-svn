/*
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 */

#include <gtk/gtk.h>

#include "moko-pin.h"
#include "moko-dialer-textview.h"
#include "moko-dialer-panel.h"

#include <string.h>

typedef struct
{
  GtkWidget *display;
  GtkWidget *dialog;
  gchar *pin;
} MokoPinData;

static void
on_pad_user_input (MokoDialerPanel *panel, const gchar digit,
                   MokoPinData *data)
{
  gchar buf[2];
  gchar *new_pin;

  /* Phones use '#' for PIN 'entered' signal */
  if (digit == '#')
  {
    gtk_dialog_response (GTK_DIALOG (data->dialog), GTK_RESPONSE_OK);
    return;
  }
  
  /* "*" acts as delete for now */
  if (digit == '*')
  {
    moko_dialer_textview_delete (MOKO_DIALER_TEXTVIEW (data->display));
    data->pin[strlen(data->pin)-1] = '\0';
    return;
  }

  /* Create a string from the new digit */
  buf[0] = digit;
  buf[1] = '\0';
  
  if (!data->pin)
  {
    new_pin = g_strdup (buf);
  }
  else
  {
    new_pin = g_strconcat (data->pin, buf, NULL);
    g_free (data->pin);
  }
  data->pin = new_pin;

  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (data->display), "*");
}



char*
get_pin_from_user (const char *message)
{
  GtkWidget *pad;
  gchar *pin;
  MokoPinData data;
  
  data.dialog = gtk_dialog_new_with_buttons (message, NULL, 0,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  gtk_dialog_set_has_separator (GTK_DIALOG (data.dialog), FALSE);

  data.display = moko_dialer_textview_new ();
  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (data.dialog)->vbox), data.display);
  
  pad = moko_dialer_panel_new ();
  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (data.dialog)->vbox), pad);
  g_signal_connect (pad, "user_input", G_CALLBACK (on_pad_user_input), &data);
  
  gtk_widget_show_all (GTK_DIALOG (data.dialog)->vbox);
  
  data.pin = NULL;
  if (gtk_dialog_run (GTK_DIALOG (data.dialog)) == GTK_RESPONSE_OK)
  {
    pin = data.pin;
  }
  else
  {
    pin = NULL;
    g_free (data.pin);
  }
  gtk_widget_destroy (data.dialog);

  return pin;
}

void
display_pin_error (const char *message)
{
  GtkWidget *dlg;
  
  dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                message);
  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);
}

