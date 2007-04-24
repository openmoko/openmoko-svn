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
  
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-pixmap-button.h>
  
#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkvbox.h>
  
#include "contacts.h"
#include "openmoko-dialer-main.h"
#include "moko-dialer-status.h"
#include "openmoko-dialer-window-incoming.h"
#include "openmoko-dialer-window-talking.h"
#include "openmoko-dialer-window-history.h"
#include "dialergsm.h"
#include "history.h"
void 
cb_answer_button_clicked (GtkButton * button,
                          MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  appdata->g_state.callstate = STATE_TALKING;
  gsm_answer ();
  gtk_widget_hide (appdata->window_incoming);
  
//transfer the contact info
    window_talking_prepare (appdata);
  gtk_widget_show (appdata->window_talking);
  DBG_LEAVE ();
} void 

cb_ignore_button_clicked (GtkButton * button,
                          MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  DBG_MESSAGE ("We will mute the phone for this call.");
  appdata->g_state.callstate = STATE_IGNORED;
  DBG_LEAVE ();
} void 

cb_reject_button_clicked (GtkButton * button,
                          MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  gsm_hangup ();
  appdata->g_state.callstate = STATE_REJECTED;
  gtk_widget_hide (appdata->window_incoming);
  DBG_LEAVE ();
} void 

window_incoming_prepare (MOKO_DIALER_APP_DATA * appdata) 
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
window_incoming_fails (MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  DBG_LEAVE ();
} gint  timer_incoming_time_out (MOKO_DIALER_APP_DATA * appdata) 
{
  
//DBG_ENTER();
    TIMER_DATA * timer_data = &(appdata->g_timer_data);
  moko_dialer_status_update_icon (appdata->status_incoming);
  
//now that we have the incoming call status report correctly, the timeout mechanism is not needed.
/*
  if (event_get_keep_calling ())
  {
    event_reset_keep_calling ();
    timer_data->ticks = 0;
  }
  else
  {                             //we count 4 before we confirm that there are no calling at all.
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
on_window_incoming_hide (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  if (appdata->g_timer_data.ptimer != 0)
    
  {
    g_source_remove (appdata->g_timer_data.ptimer);
    appdata->g_timer_data.ptimer = 0;
  }
  event_reset_clip_signaled ();
  event_reset_incoming_signaled ();
  event_reset_keep_calling ();
  if (appdata->g_state.callstate != STATE_TALKING)
    
  {                             //     add_histroy_entry(g_state.historytype,g_state.contactinfo.name,g_state.contactinfo.number,g_state.contactinfo.picpath,g_state.starttime,0);
    add_histroy_entry (appdata, appdata->g_state.historytype,
                        appdata->g_peer_info.name,
                        appdata->g_peer_info.number,
                        appdata->g_peer_info.ID,
                        appdata->g_state.starttime,
                        appdata->g_state.startdate, 0);
  }
  DBG_LEAVE ();
}
void 
window_incoming_setup_timer (MOKO_DIALER_APP_DATA * appdata) 
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
    g_timeout_add (1000, (GSourceFunc) timer_incoming_time_out,
                    (gpointer) appdata);
}
void 
on_window_incoming_show (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata) 
{
  DBG_ENTER ();
  appdata->g_state.callstate = STATE_INCOMING;
  window_incoming_setup_timer (appdata);
  DBG_LEAVE ();
} gint  window_incoming_init (MOKO_DIALER_APP_DATA * p_dialer_data) 
{
  DBG_ENTER ();
  MokoFingerWindow * window;
  GtkWidget * vbox;
  GtkWidget * status;
  if (p_dialer_data->window_incoming == 0)
    
  {
    vbox = gtk_vbox_new (FALSE, 0);
    status = moko_dialer_status_new ();
    moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                         "incoming_0.png");
    moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                         "incoming_1.png");
    moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                         "incoming_2.png");
    moko_dialer_status_add_status_icon (MOKO_DIALER_STATUS (status),
                                         "incoming_3.png");
    moko_dialer_status_set_icon_by_index (MOKO_DIALER_STATUS (status), 0);
    gtk_box_pack_start (GTK_BOX (vbox), status, FALSE, FALSE, 0);
    GtkWidget * hbox2 = gtk_hbox_new (FALSE, 0);
    GtkWidget * button = gtk_button_new_with_label ("Answer");
    gtk_button_set_image (GTK_BUTTON (button),
                           file_new_image_from_relative_path ("answer.png"));
    g_signal_connect (G_OBJECT (button), "clicked",
                       G_CALLBACK (cb_answer_button_clicked), p_dialer_data);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE,
                          10);
    button = gtk_button_new_with_label ("Ignore");
    g_signal_connect (G_OBJECT (button), "clicked",
                       G_CALLBACK (cb_ignore_button_clicked), p_dialer_data);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE,
                         10);
    button = gtk_button_new_with_label ("Reject");
    gtk_button_set_image (GTK_BUTTON (button),
                           file_new_image_from_relative_path ("cancel.png"));
    g_signal_connect (G_OBJECT (button), "clicked",
                       G_CALLBACK (cb_reject_button_clicked), p_dialer_data);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE,
                         10);
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 50);
    
//currently     MokoDialogWindow is not finished, wating...
//   MokoDialogWindow* window = (MokoDialogWindow *)(moko_dialog_window_new());
//  moko_dialog_window_set_contents( window, GTK_WIDGET(vbox) );
      window = MOKO_FINGER_WINDOW (moko_finger_window_new ());
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    moko_finger_window_set_contents (window, GTK_WIDGET (vbox));
    
      //FIXME: dear thomas, I know that show & hide is not good, but when you removed the following 2 lines, the incoming window
      //is simply not displaying well. please have a try and then commit. If you cannot test the incoming window, you can simply modify the code to show it out debuggingly.
      //Pardon me to re-add the 2 lines. Tony Guan 14,3,2007
      gtk_widget_show_all (GTK_WIDGET (window));
    gtk_widget_hide (GTK_WIDGET (window));
    moko_dialer_status_set_title_label (MOKO_DIALER_STATUS (status),
                                          "Incoming call");
    moko_dialer_status_set_status_label (MOKO_DIALER_STATUS (status), "");
    p_dialer_data->window_incoming = GTK_WIDGET (window);
    p_dialer_data->status_incoming = MOKO_DIALER_STATUS (status);
    
//   DBG_MESSAGE("p_dialer_data->status_incoming=0X%x",p_dialer_data->status_incoming);
      g_signal_connect ((gpointer) window, "show",
                          G_CALLBACK (on_window_incoming_show),
                          p_dialer_data);
    g_signal_connect ((gpointer) window, "hide",
                       G_CALLBACK (on_window_incoming_hide), p_dialer_data);
  }
  DBG_LEAVE ();
  return 1;
}


