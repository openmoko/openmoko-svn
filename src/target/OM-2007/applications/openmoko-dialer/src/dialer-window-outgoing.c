/*   openmoko-dialer-window-outgoing.c
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

#include <string.h>

#include <libmokoui/moko-ui.h>
#
#include <gtk/gtk.h>

#include "contacts.h"
#include "dialer-main.h"
#include "moko-dialer-status.h"
#include "dialer-window-outgoing.h"
#include "dialer-window-history.h"
#include "dialer-window-talking.h"

static void
cb_speaker_button_clicked (GtkButton * button, MokoDialerData * data)
{
  if (GTK_WIDGET_VISIBLE (data->buttonSpeaker))
    gtk_widget_show (data->buttonHandset);
  else
    gtk_widget_show (data->buttonSpeaker);

  gtk_widget_hide (GTK_WIDGET (button));
  // do something else here too
}

static void
cb_redial_button_clicked (GtkButton * button, MokoDialerData * data)
{
  gchar *number = g_object_get_data (G_OBJECT (button), "current-number");

  gtk_widget_hide (GTK_WIDGET (button));
  gtk_widget_show (data->buttonCancel);


  moko_gsmd_connection_voice_hangup (data->connection);
  moko_gsmd_connection_voice_dial (data->connection, number);
}


gint
window_outgoing_init (MokoDialerData * p_dialer_data)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *status;
  GtkWidget *button;

  if (p_dialer_data->window_outgoing != 0)
    return 1;
  
  vbox = gtk_vbox_new (FALSE, 0);
  status = moko_dialer_status_new ();
  moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                      "outgoing_0.png");
  moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                      "outgoing_1.png");
  moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                      "outgoing_2.png");
  moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                      "outgoing_3.png");
  //moko_dialer_status_set_error_icon (MOKO_DIALER_STATUS (status),
  //                                   "failure.png");
  //moko_dialer_status_set_icon_by_index (MOKO_DIALER_STATUS (status), 0);

  gtk_box_pack_start (GTK_BOX (vbox), status, FALSE, FALSE, 0);

  /* Set up window */
  window = moko_message_dialog_new ();

  gtk_widget_realize (GTK_WIDGET (window));
  
  moko_message_dialog_set_image (MOKO_MESSAGE_DIALOG (window), 
       gtk_image_new_from_file (PKGDATADIR G_DIR_SEPARATOR_S "outgoing_1.png"));
  gtk_window_set_title (GTK_WINDOW (window), "Outgoing Call");

  /* Set up buttons */
  button = gtk_button_new_from_stock (MOKO_STOCK_SPEAKER);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_speaker_button_clicked), p_dialer_data);
  p_dialer_data->buttonSpeaker = button;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), button, 
                      FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (MOKO_STOCK_HANDSET);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_speaker_button_clicked), p_dialer_data);
  p_dialer_data->buttonHandset = button;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), button, 
                      FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (MOKO_STOCK_CALL_REDIAL);
  p_dialer_data->buttonRedial = button;
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_redial_button_clicked), p_dialer_data);

  p_dialer_data->buttonCancel = gtk_dialog_add_button (GTK_DIALOG (window),
                                                       GTK_STOCK_CANCEL,
                                                       GTK_RESPONSE_CANCEL);

  moko_dialer_status_set_title_label (MOKO_DIALER_STATUS (status),
                                      "Outgoing call");
  moko_dialer_status_set_status_label (MOKO_DIALER_STATUS (status),
                                       "Calling ... (00:00:00)");

  p_dialer_data->window_outgoing = GTK_WIDGET (window);
  p_dialer_data->status_outgoing = MOKO_DIALER_STATUS (status);

  return 1;
}

static void
call_progress_cb (MokoGsmdConnection *connection, int type, MokoDialerData *data)
{
  g_debug ("Outgoing Call Progress: %d", type);
  if (type == MOKO_GSMD_PROG_DISCONNECT)
  {
    gtk_dialog_response (GTK_DIALOG (data->window_outgoing), GTK_RESPONSE_CANCEL);
    return;
  }

  if (type ==  MOKO_GSMD_PROG_CONNECTED)
  {
    gtk_dialog_response (GTK_DIALOG (data->window_outgoing), GTK_RESPONSE_OK);
  }
}

void
window_outgoing_dial (MokoDialerData *data, gchar *number)
{
  MokoJournalEntry *entry = NULL;
  MokoJournalVoiceInfo *info = NULL;
  
  gulong progress_handler;
  
  /* create the journal entry for this call and add it to the journal */
  entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
  moko_journal_entry_set_direction (entry, DIRECTION_OUT);
  moko_journal_entry_get_voice_info (entry, &info);
  moko_journal_entry_set_dtstart (entry, moko_time_new_today ());
  moko_journal_voice_info_set_distant_number (info, number);
  moko_journal_add_entry (data->journal, entry);
  /* FIXME: We should be able to associate a number with a contact uid, and 
            add that info to the entry */
  
  /* connect our handler to track call progress */
  progress_handler = g_signal_connect (data->connection, "call-progress", 
                                       G_CALLBACK (call_progress_cb), data);
  g_object_set_data (G_OBJECT (data->window_outgoing), "current-number", number);
  moko_gsmd_connection_voice_dial (data->connection, number);

  moko_message_dialog_set_message (MOKO_MESSAGE_DIALOG (data->window_outgoing),
                                   "Calling %s", number);               

  
  if (gtk_dialog_run (GTK_DIALOG (data->window_outgoing)) == GTK_RESPONSE_OK)
  {
    g_print ("Outgoing: Preparing talking window\n");
    /* call has connected, so open the talking window */
    /* window_talking_show (); */
    window_talking_prepare (data);
     
    g_print ("Outgoing: Showing window\n");
    gtk_widget_show (data->window_talking);
  }
  else
  {
    /* call canceled */
    moko_gsmd_connection_voice_hangup (data->connection);
  }

  gtk_widget_hide (data->window_outgoing);
  g_object_steal_data (G_OBJECT (data->window_outgoing), "current-number");

  /* disconnect the call progress handler since we no longer need it */
  g_signal_handler_disconnect (data->connection, progress_handler);
  
  /* commit the journal entry */
  moko_journal_write_to_storage (data->journal);
}
