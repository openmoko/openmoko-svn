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
cb_answer_button_clicked (GtkButton * button,
                          MokoDialerData * appdata) 
{
  DBG_ENTER ();
  appdata->g_state.callstate = STATE_TALKING;
  /* TODO: MokoGsmdConnection->answer
   * gsm_answer ();
   */

  gtk_widget_hide (appdata->window_incoming);

  //transfer the contact info
   window_talking_prepare (appdata);
  gtk_widget_show (appdata->window_talking);
  DBG_LEAVE ();
} 

void
cb_ignore_button_clicked (GtkButton * button,
                          MokoDialerData * appdata) 
{
  DBG_ENTER ();
  DBG_MESSAGE ("We will mute the phone for this call.");
  appdata->g_state.callstate = STATE_IGNORED;
  DBG_LEAVE ();
} void 

cb_reject_button_clicked (GtkButton * button,
                          MokoDialerData * appdata) 
{
  DBG_ENTER ();
  /* TODO: MokoGsmdConnection->hangup
   * gsm_hangup ();
   */
  appdata->g_state.callstate = STATE_REJECTED;
  gtk_widget_hide (appdata->window_incoming);
  DBG_LEAVE ();
}

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
window_incoming_fails (MokoDialerData * appdata) 
{
  DBG_ENTER ();
  DBG_LEAVE ();
} 

gint  
timer_incoming_time_out (MokoDialerData * appdata) 
{
  
//DBG_ENTER();
  //TIMER_DATA * timer_data = &(appdata->g_timer_data);
  moko_dialer_status_update_icon (appdata->status_incoming);
  
  //now that we have the incoming call status report correctly, the timeout 
  //mechanism is not needed.
/*
  if (event_get_keep_calling ())
  {
    event_reset_keep_calling ();
    timer_data->ticks = 0;
  }
  else
  { //we count 4 before we confirm that there are no calling at all.
    if (timer_data->ticks >= 3)
    {
      DBG_MESSAGE ("THE CALLER aborted, we quit.");
      gsm_hangup ();
      appdata->g_state.callstate = STATE_MISSED;
      appdata->g_state.historytype = MISSED;
      gdk_threads_enter ();
      gtk_widget_hide (appdata->window_incoming);
      gdk_threads_leave ();
      return 0;                 //don't lookout the timeout.
    }
    else
    {
      DBG_MESSAGE ("ticks=%d", timer_data->ticks);
      timer_data->ticks++;
    }
  }
*/ 
    return 1;
}
void 
on_window_incoming_hide (GtkWidget * widget, MokoDialerData * appdata) 
{
  DBG_ENTER ();
  if (appdata->g_timer_data.ptimer != 0)
    
  {
    g_source_remove (appdata->g_timer_data.ptimer);
    appdata->g_timer_data.ptimer = 0;
  }
  /*
  event_reset_clip_signaled ();
  event_reset_incoming_signaled ();
  event_reset_keep_calling ();
  */
  if (appdata->g_state.callstate != STATE_TALKING)
    
  {                             //     add_histroy_entry(g_state.historytype,g_state.contactinfo.name,g_state.contactinfo.number,g_state.contactinfo.picpath,g_state.starttime,0);
    /*add_histroy_entry (appdata, appdata->g_state.historytype,
                        appdata->g_peer_info.name,
                        appdata->g_peer_info.number,
                        appdata->g_peer_info.ID,
                        appdata->g_state.starttime,
                        appdata->g_state.startdate, 0);
                        */
  }
  DBG_LEAVE ();
}
void 
window_incoming_setup_timer (MokoDialerData * appdata) 
{
  time_t timep;
  struct tm *p;
  time (&timep);
  p = localtime (&timep);
  sprintf (appdata->g_state.starttime, "%02d:%02d:%02d", p->tm_hour,
             p->tm_min, p->tm_sec);
  sprintf (appdata->g_state.startdate, "%04d/%02d/%02d", p->tm_year,
            p->tm_mon, p->tm_mday);
  if (appdata->g_timer_data.ptimer != 0)
    
  {
    g_source_remove (appdata->g_timer_data.ptimer);
    appdata->g_timer_data.ptimer = 0;
  }
  memset (&(appdata->g_timer_data), 0, sizeof (appdata->g_timer_data));
  appdata->g_timer_data.stopsec = 0;
  appdata->g_timer_data.ptimer =
  g_timeout_add (1000, (GSourceFunc)timer_incoming_time_out, (gpointer)appdata);
}

void 
on_window_incoming_show (GtkWidget * widget, MokoDialerData * appdata) 
{
  DBG_ENTER ();
  appdata->g_state.callstate = STATE_INCOMING;
  window_incoming_setup_timer (appdata);
  DBG_LEAVE ();
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

void
window_incoming_show (MokoDialerData *data)
{
  if (!data->window_incoming)
  {
    window_incoming_init (data);
  }

  if (gtk_dialog_run (GTK_DIALOG (data->window_incoming)) == GTK_RESPONSE_OK)
  {
    moko_gsmd_connection_voice_accept (data->connection);
    /* dialer_window_talking_show (data); */
    if (!data->window_talking)
      window_talking_init (data);
    gtk_widget_show_all (data->window_talking);
  }
  else
  {
    moko_gsmd_connection_voice_hangup (data->connection);
  }

  gtk_widget_hide (data->window_incoming);
}

void
window_incoming_update_message (MokoDialerData *data, const gchar *clip)
{
  moko_message_dialog_set_message (MOKO_MESSAGE_DIALOG (data->window_incoming),
                                   "Incoming call from %s", clip);
}
