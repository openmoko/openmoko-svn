/*   openmoko-dialer-window-incoming.c
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
  
#include <libmokoui/moko-ui.h>
  
#include <gtk/gtk.h>
#include <string.h>
#include "contacts.h"
#include "dialer-main.h"
#include "moko-dialer-status.h"
#include "dialer-window-incoming.h"
#include "dialer-window-talking.h"
#include "dialer-window-history.h"

void
window_incoming_prepare (MokoDialerData * appdata) 
{
  if (!appdata)
    
  {
    DBG_WARN ("appdata=NULL!");
    return;
  }
  if (appdata->window_incoming == 0)
    
  {
    window_incoming_init (appdata);
  }
  moko_dialer_status_set_person_number (appdata->status_incoming,
                                          appdata->g_peer_info.number);
  if (appdata->g_peer_info.hasname)
    
  {
    moko_dialer_status_set_person_image (appdata->status_incoming,
                                           appdata->g_peer_info.ID);
    moko_dialer_status_set_person_name (appdata->status_incoming,
                                         appdata->g_peer_info.name);
  }
  
  else
    
  {
    moko_dialer_status_set_person_image (appdata->status_incoming, "");
    moko_dialer_status_set_person_name (appdata->status_incoming, "");
  }
}


void
window_incoming_init (MokoDialerData * data) 
{
  GtkWidget * window;

  if (data->window_incoming)
     return;

  window = moko_message_dialog_new ();

  gtk_dialog_add_button (GTK_DIALOG (window), MOKO_STOCK_CALL_ANSWER, GTK_RESPONSE_OK);
  gtk_dialog_add_button (GTK_DIALOG (window), MOKO_STOCK_CALL_REJECT, GTK_RESPONSE_CANCEL);
  moko_message_dialog_set_message (MOKO_MESSAGE_DIALOG (window), "Incoming call");

  data->window_incoming = window;

}

static void
call_progress_cb (MokoGsmdConnection *connection, int type, MokoDialerData *data)
{
  g_debug ("Incoming Call Progress: %d", type);
  if (type == MOKO_GSMD_PROG_DISCONNECT)
  {
    /* call was disconnected before it was answered */
    gtk_dialog_response (GTK_DIALOG (data->window_incoming), GTK_RESPONSE_CANCEL);
  }
}


void
window_incoming_show (MokoDialerData *data)
{
  MokoJournalEntry *entry = NULL;
  MokoJournalVoiceInfo *info = NULL;
  gulong progress_handler;

  if (!data->window_incoming)
  {
    window_incoming_init (data);
  }

  /* create the journal entry for this call and add it to the journal */
  entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
  moko_journal_entry_set_direction (entry, DIRECTION_IN);
  moko_journal_entry_get_voice_info (entry, &info);
  moko_journal_entry_set_dtstart (entry, moko_time_new_today ());
  moko_journal_add_entry (data->journal, entry);

  /* connect our handler to track call progress */
  progress_handler = g_signal_connect (data->connection, "call-progress", 
                    G_CALLBACK (call_progress_cb), data);


  if (gtk_dialog_run (GTK_DIALOG (data->window_incoming)) == GTK_RESPONSE_OK)
  {
    moko_gsmd_connection_voice_accept (data->connection);
    /* dialer_window_talking_show (data); */
    if (!data->window_talking)
      window_talking_init (data);
    gtk_widget_show_all (data->window_talking);
    moko_journal_voice_info_set_was_missed (info, FALSE);
  }
  else
  {
    moko_gsmd_connection_voice_hangup (data->connection);
    /* mark the call as misssed
     * FIXME: this is not strictly true if the call was rejected
     */
    moko_journal_voice_info_set_was_missed (info, TRUE);
  }

  gtk_widget_hide (data->window_incoming);

  /* disconnect the call progress handler since we no longer need it */
  g_signal_handler_disconnect (data->connection, progress_handler);
}

void
window_incoming_update_message (MokoDialerData *data, const gchar *clip)
{
  g_debug ("Incoming Call CLIP: %s", clip);
  moko_message_dialog_set_message (MOKO_MESSAGE_DIALOG (data->window_incoming),
                                   "Incoming call from %s", clip);
  // moko_journal_voice_info_set_distant_number ();
}
