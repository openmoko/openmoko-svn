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

/* function declerations */
static void window_outgoing_setup_timer (MokoDialerData * appdata);

static void
cb_speaker_button_clicked (GtkButton * button, MokoDialerData * data)
{
  if (GTK_WIDGET_VISIBLE (data->buttonSpeaker))
    gtk_widget_show (data->buttonHandset);
  else
    gtk_widget_show (data->buttonSpeaker);

  gtk_widget_hide (button);
  // do something else here too
}

static void
cb_redial_button_clicked (GtkButton * button, MokoDialerData * data)
{
  gchar *number = g_object_get_data (G_OBJECT (button), "current-number");

  gtk_widget_hide (button);
  gtk_widget_show (data->buttonCancel);


  moko_gsmd_connection_voice_hangup (data->connection);
  moko_gsmd_connection_voice_dial (data->connection, number);
}

static void
cb_cancel_button_clicked (GtkButton * button, MokoDialerData * appdata)
{
  DBG_ENTER ();
  /* TODO: MokoGsmdConnection->hangup
   * gsm_hangup ();
   */
  appdata->g_state.callstate = STATE_FAILED;
  DBG_TRACE ();
  gtk_widget_hide (appdata->window_outgoing);
  DBG_LEAVE ();
}

gint
timer_outgoing_time_out (MokoDialerData * appdata)
{
//DBG_ENTER();
  TIMER_DATA *timer_data = &(appdata->g_timer_data);


  timer_data->ticks++;
  timer_data->hour = timer_data->ticks / 3600;
  timer_data->min = (timer_data->ticks - timer_data->hour * 3600) / 60;
  timer_data->sec = timer_data->ticks % 60;


  sprintf (timer_data->timestring, "Calling ... (%02d:%02d:%02d)",
           timer_data->hour, timer_data->min, timer_data->sec);

//ok,we update the label now.


  moko_dialer_status_set_status_label (appdata->status_outgoing,
                                       timer_data->timestring);
  moko_dialer_status_update_icon (appdata->status_outgoing);

  if (timer_data->stopsec != 0 && timer_data->ticks >= timer_data->stopsec)
  {

    timer_data->timeout = 1;
    g_source_remove (timer_data->ptimer);
    timer_data->ptimer = 0;
//maybe it failes
//    window_outgoing_fails (appdata);
    return 0;                   //0 stops the timer.
  }
  else
    return 1;
}



static void
on_window_outgoing_hide (GtkWidget * widget, MokoDialerData * appdata)
{
  if (appdata->g_timer_data.ptimer != 0)
  {
    g_source_remove (appdata->g_timer_data.ptimer);
    appdata->g_timer_data.ptimer = 0;
  }
  if (appdata->g_state.callstate != STATE_TALKING)
  {                             //     add_histroy_entry(g_state.historytype,g_state.contactinfo.name,g_state.contactinfo.number,g_state.contactinfo.picpath,g_state.starttime,0);
/*
    add_histroy_entry (appdata, appdata->g_state.historytype,
                       appdata->g_peer_info.name,
                       appdata->g_peer_info.number,
                       appdata->g_peer_info.picpath,
                       appdata->g_state.starttime,
                       appdata->g_state.startdate, 0);
*/
  }


}

static void
window_outgoing_setup_timer (MokoDialerData * appdata)
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
// 1:30 timeout
  appdata->g_timer_data.stopsec = 90;

  appdata->g_timer_data.ptimer =
    g_timeout_add (1000, (GSourceFunc) timer_outgoing_time_out, appdata);


}

static void
on_window_outgoing_show (GtkWidget * widget, MokoDialerData * appdata)
{

  window_outgoing_setup_timer (appdata);
  appdata->g_state.callstate = STATE_CALLING;
  /* TODO: MokoGsmdConnection->dial
   * int retv = gsm_dial (appdata->g_peer_info.number);
   * DBG_MESSAGE ("GSM_DIAL returns %d", retv);
   */
}


gint
window_outgoing_init (MokoDialerData * p_dialer_data)
{
  MokoFingerWindow *window;
  GtkWidget *vbox;
  GtkWidget *status;

  if (p_dialer_data->window_outgoing == 0)
  {

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
    moko_message_dialog_set_image (MOKO_MESSAGE_DIALOG (window), gtk_image_new_from_file (PKGDATADIR G_DIR_SEPARATOR_S "outgoing_1.png"));
    gtk_window_set_title (GTK_WINDOW (window), "Outgoing Call");


    /* Set up buttons */
    GtkWidget *button = gtk_button_new_from_stock (MOKO_STOCK_SPEAKER);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_speaker_button_clicked), p_dialer_data);
    p_dialer_data->buttonSpeaker = button;
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    button = gtk_button_new_from_stock (MOKO_STOCK_HANDSET);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_speaker_button_clicked), p_dialer_data);
    p_dialer_data->buttonHandset = button;
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), button, FALSE, FALSE, 0);


    button = gtk_button_new_from_stock (MOKO_STOCK_CALL_REDIAL);
    p_dialer_data->buttonRedial = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_redial_button_clicked), p_dialer_data);

    p_dialer_data->buttonCancel = gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);


    moko_dialer_status_set_title_label (MOKO_DIALER_STATUS (status),
                                        "Outgoing call");
    moko_dialer_status_set_status_label (MOKO_DIALER_STATUS (status),
                                         "Calling ... (00:00:00)");

    p_dialer_data->window_outgoing = GTK_WIDGET (window);
    p_dialer_data->status_outgoing = MOKO_DIALER_STATUS (status);

    g_signal_connect ((gpointer) window, "show",
                      G_CALLBACK (on_window_outgoing_show), p_dialer_data);
    g_signal_connect ((gpointer) window, "hide",
                      G_CALLBACK (on_window_outgoing_hide), p_dialer_data);

  }

  return 1;
}

void
call_progress_cb (MokoGsmdConnection *connection, int type, MokoDialerData *data)
{
  if (type == MOKO_GSMD_PROG_REJECT)
  {
    g_debug ("call rejected");
    return;
  }

  if (type ==  MOKO_GSMD_PROG_CONNECTED)
  {
    gtk_dialog_response (data->window_outgoing, GTK_RESPONSE_OK);
  }
}

void
window_outgoing_dial (MokoDialerData *data, gchar *number)
{
  g_signal_connect (data->connection, "call-progress", call_progress_cb, data);
  g_object_set_data (G_OBJECT (data->window_outgoing), "current-number", number);
  moko_gsmd_connection_voice_dial (data->connection, number);
  moko_message_dialog_set_message (MOKO_MESSAGE_DIALOG (data->window_outgoing), "Calling %s", number);
  if (gtk_dialog_run (data->window_outgoing) == GTK_RESPONSE_OK)
  {
    /* call has connected, so open the talking window */
    /* window_talking_show (); */
    window_talking_prepare (data);
    gtk_widget_show (data->window_talking);
  }
  else
  {
    /* call canceled */
    moko_gsmd_connection_voice_hangup (data->connection);
  }

  gtk_widget_hide (data->window_outgoing);
  g_object_steal_data (G_OBJECT (data->window_outgoing), "current-number");
}
