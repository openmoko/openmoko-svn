/*  moko-dialer-pannel.c
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
#include "moko-dialer-panel.h"

#include "moko-digit-button.h"

G_DEFINE_TYPE (MokoDialerPanel, moko_dialer_panel, GTK_TYPE_VBOX)

#define NOVALUE '\0'

enum
{
  CLICKED_SIGNAL,
  HOLD_SIGNAL,
  LAST_SIGNAL
};

//forward definition
     static gboolean moko_dialer_panel_pressed (MokoDigitButton * button,
                                                GdkEventButton * event,
                                                MokoDialerPanel *panel);

     static gint moko_dialer_panel_signals[LAST_SIGNAL] = { 0 };


typedef struct
{
  gchar value;
  MokoDialerPanel *panel;
} HoldTimeoutData;

static void
moko_dialer_panel_class_init (MokoDialerPanelClass * class)
{
/*
  GtkVBoxClass* vbox_class;

  vbox_class= (GtkVBoxClass*) class;
*/

  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) class;
  class->moko_dialer_panel_input = NULL;
  class->moko_dialer_panel_hold = NULL;

  moko_dialer_panel_signals[CLICKED_SIGNAL] =
    g_signal_new ("user_input",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerPanelClass,
                                   moko_dialer_panel_input), NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR, G_TYPE_NONE, 1,
                  g_type_from_name ("gchar"));

  moko_dialer_panel_signals[HOLD_SIGNAL] =
    g_signal_new ("user_hold",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerPanelClass,
                                   moko_dialer_panel_hold), NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR, G_TYPE_NONE, 1,
                  g_type_from_name ("gchar"));

}


static void
moko_dialer_panel_init (MokoDialerPanel * moko_dialer_panel)
{
  gchar *left[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
  };

  gchar *right[4][3] = {
    {"", "ABC", "DEF"},
    {"GHI", "JKL", "MNO"},
    {"PQRS", "TUV", "WXYZ"},
    {"+", "p", "w"}
  };

  gchar leftchar[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
  };

  gchar rightchar[4][3] = {
    {NOVALUE, NOVALUE, NOVALUE},
    {NOVALUE, NOVALUE, NOVALUE},
    {NOVALUE, NOVALUE, NOVALUE},
    {'+', 'p', 'w'}
  };


  GtkWidget *table;
  gint i, j;

  table = gtk_table_new (4, 3, TRUE);
  gtk_container_add (GTK_CONTAINER (moko_dialer_panel), table);
  gtk_widget_show (table);



  for (i = 0; i < 4; i++)
    for (j = 0; j < 3; j++)
    {

      moko_dialer_panel->buttons[i][j] =
        moko_digit_button_new_with_labels (left[i][j], right[i][j]);

      moko_digit_button_set_numbers (moko_dialer_panel->buttons[i][j],
                                     leftchar[i][j], rightchar[i][j]);

      gtk_table_attach_defaults (GTK_TABLE (table),
                                 moko_dialer_panel->buttons[i][j], j, j + 1,
                                 i, i + 1);

      g_signal_connect ((gpointer) moko_dialer_panel->buttons[i][j],
                        "button_press_event", 
                        G_CALLBACK (moko_dialer_panel_pressed),
                        moko_dialer_panel);
      g_signal_connect ((gpointer) moko_dialer_panel->buttons[i][j],
                        "button_release_event",
                        G_CALLBACK (moko_dialer_panel_pressed),
                        moko_dialer_panel);

      gtk_widget_show (moko_dialer_panel->buttons[i][j]);
    }

}

static gboolean
moko_dialer_panel_hold_timeout (HoldTimeoutData *data)
{
  g_signal_emit (data->panel, moko_dialer_panel_signals[HOLD_SIGNAL], 0, data->value);

  return FALSE;
}

static gboolean
moko_dialer_panel_pressed (MokoDigitButton *button, 
                           GdkEventButton *event,
                           MokoDialerPanel *panel)
{
  static gint hold_timeout_source = 0;

  if (event->type == GDK_BUTTON_PRESS)
  {
    HoldTimeoutData *timeout_data;
    gchar value = NOVALUE;

    /* Normal 'clicked' event */
    value = moko_digit_button_get_left (button);
    g_signal_emit (panel, moko_dialer_panel_signals[CLICKED_SIGNAL], 0, value);

    /* Set up for a tap-and-hold event */
    value = moko_digit_button_get_right (button);

    /* this button doesn't have a "hold" value */
    if (value == NOVALUE)
      return FALSE;

    timeout_data = g_new0 (HoldTimeoutData, 1);

    timeout_data->panel = panel;
    timeout_data->value = value;

    hold_timeout_source = g_timeout_add_full (G_PRIORITY_DEFAULT, 800,
                                  (GSourceFunc) moko_dialer_panel_hold_timeout,
                                  timeout_data,
                                  (GDestroyNotify) g_free);

  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    if (hold_timeout_source != 0)
      g_source_remove (hold_timeout_source);
  }
  return FALSE;
}

GtkWidget *
moko_dialer_panel_new ()
{
  MokoDialerPanel *dp;

  dp = (MokoDialerPanel *) g_object_new (MOKO_TYPE_DIALER_PANEL, NULL);
  return GTK_WIDGET (dp);

}
