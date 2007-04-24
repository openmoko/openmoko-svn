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
#include "error.h"
G_DEFINE_TYPE (MokoDialerPanel, moko_dialer_panel, GTK_TYPE_VBOX)
     enum
     {
       CLICKED_SIGNAL,
       HOLD_SIGNAL,
       LAST_SIGNAL
     };

//forward definition
     static gboolean moko_dialer_panel_pressed (MokoDigitButton * button,
                                                GdkEventButton * event,
                                                gpointer data);

     static gint moko_dialer_panel_signals[LAST_SIGNAL] = { 0 };

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

  g_print ("moko_dialer_panel:start signal register\n");

  moko_dialer_panel_signals[CLICKED_SIGNAL] =
    g_signal_new ("user_input",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerPanelClass,
                                   moko_dialer_panel_input), NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR, G_TYPE_NONE, 1,
                  g_type_from_name ("gchar"));

  g_print ("moko_dialer_panel:signal register end,got the id :%d\n",
           moko_dialer_panel_signals[CLICKED_SIGNAL]);

  moko_dialer_panel_signals[HOLD_SIGNAL] =
    g_signal_new ("user_hold",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerPanelClass,
                                   moko_dialer_panel_hold), NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR, G_TYPE_NONE, 1,
                  g_type_from_name ("gchar"));

  g_print ("moko_dialer_panel:signal register end,got the id :%d\n",
           moko_dialer_panel_signals[HOLD_SIGNAL]);

}


static void
moko_dialer_panel_init (MokoDialerPanel * moko_dialer_panel)
{
  DBG_ENTER ();

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
    {-1, -1, -1},
    {-1, -1, -1},
    {-1, -1, -1},
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
                        "button_release_event",
                        G_CALLBACK (moko_dialer_panel_pressed),
                        moko_dialer_panel);

      gtk_widget_set_size_request (moko_dialer_panel->buttons[i][j], 20, 20);
      gtk_widget_show (moko_dialer_panel->buttons[i][j]);
    }

}



static gboolean
moko_dialer_panel_pressed (MokoDigitButton * button, GdkEventButton * event,
                           gpointer data)
{

  MokoDialerPanel *moko_dialer_panel;

//here! check it tomorrow!
  moko_dialer_panel = (MokoDialerPanel *) data;

  gchar value = -1;

  if (event->button == 3)
  {                             //right button
    value = moko_digit_button_get_right (button);
    if (value == -1)
    {                           //button 1-9 pressed with hold , we emit another signal: HOLD_SIGNAL, with the para of the number
      value = moko_digit_button_get_left (button);
      if (value != -1)
      {
        g_signal_emit (moko_dialer_panel,
                       moko_dialer_panel_signals[HOLD_SIGNAL], 0, value);
      }
    }
    else
    {                           //*, 0, # buttons are right clicked or pressed with hold
      if (value != -1)
        g_signal_emit (moko_dialer_panel,
                       moko_dialer_panel_signals[CLICKED_SIGNAL], 0, value);

    }

  }
  else if (event->button == 1)
  {                             //left button
    value = moko_digit_button_get_left (button);
    if (value != -1)
      g_signal_emit (moko_dialer_panel,
                     moko_dialer_panel_signals[CLICKED_SIGNAL], 0, value);

  }

  /* allow the signal to propagate the event further */
  return FALSE;
}




GtkWidget *
moko_dialer_panel_new ()
{
  DBG_ENTER ();

  MokoDialerPanel *dp;

  dp = (MokoDialerPanel *) g_object_new (MOKO_TYPE_DIALER_PANEL, NULL);
  return GTK_WIDGET (dp);

}
