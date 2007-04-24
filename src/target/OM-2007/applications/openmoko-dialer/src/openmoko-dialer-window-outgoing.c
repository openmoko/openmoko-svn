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
#include "openmoko-dialer-window-outgoing.h"
#include "openmoko-dialer-window-history.h"
#include "openmoko-dialer-window-talking.h"
#include "dialergsm.h"

/* function declerations */
void window_outgoing_setup_timer (MOKO_DIALER_APP_DATA * appdata);

void
cb_speaker_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  //moko_dialer_status_update_icon(appdata->status_outgoing);


  appdata->g_state.callstate = STATE_TALKING;

  gtk_widget_hide (appdata->window_outgoing);


  //transfer the contact info
  window_talking_prepare (appdata);

  //start dialling.

  DBG_TRACE ();
//  gtk_widget_show_all (appdata->window_talking);
  gtk_widget_show (appdata->window_talking);


  DBG_LEAVE ();
}

void
cb_redial_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  gtk_widget_hide (appdata->buttonRedial);
  gtk_widget_show (appdata->buttonSpeaker);
  moko_dialer_status_set_title_label (appdata->status_outgoing,
                                      "Outgoing call");
  moko_dialer_status_set_status_label (appdata->status_outgoing,
                                       "Calling ... (00:00:00)");
  window_outgoing_setup_timer (appdata);

  appdata->g_state.callstate = STATE_CALLING;
  appdata->g_state.historytype = OUTGOING;
  gsm_dial (appdata->g_peer_info.number);

  DBG_LEAVE ();
}

void
cb_cancel_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  gsm_hangup ();
  appdata->g_state.callstate = STATE_FAILED;
  DBG_TRACE ();
  gtk_widget_hide (appdata->window_outgoing);
  DBG_LEAVE ();
}

void
window_outgoing_prepare (MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  if (appdata->window_outgoing == 0)
    window_outgoing_init (appdata);


  moko_dialer_status_set_person_number (appdata->status_outgoing,
                                        appdata->g_peer_info.number);
  if (appdata->g_peer_info.hasname)
  {
    moko_dialer_status_set_person_image (appdata->status_outgoing,
                                         appdata->g_peer_info.ID);
    moko_dialer_status_set_person_name (appdata->status_outgoing,
                                        appdata->g_peer_info.name);
  }
  else
  {
    moko_dialer_status_set_person_image (appdata->status_outgoing, "");
    moko_dialer_status_set_person_name (appdata->status_outgoing, "");

  }
//  strcpy (appdata->g_state.lastnumber, appdata->g_peer_info.number);
  g_stpcpy (appdata->g_state.lastnumber, appdata->g_peer_info.number);
  DBG_LEAVE ();

}

void
window_outgoing_fails (MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  moko_dialer_status_set_error (appdata->status_outgoing);
  moko_dialer_status_set_title_label (appdata->status_outgoing,
                                      "Call Failure");
  gtk_widget_hide (appdata->buttonSpeaker);
  gtk_widget_show (appdata->buttonRedial);
  DBG_LEAVE ();
}

gint
timer_outgoing_time_out (MOKO_DIALER_APP_DATA * appdata)
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
    window_outgoing_fails (appdata);
    return 0;                   //0 stops the timer.
  }
  else
    return 1;
}



void
on_window_outgoing_hide (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata)
{
  if (appdata->g_timer_data.ptimer != 0)
  {
    g_source_remove (appdata->g_timer_data.ptimer);
    appdata->g_timer_data.ptimer = 0;
  }
  if (appdata->g_state.callstate != STATE_TALKING)
  {                             //     add_histroy_entry(g_state.historytype,g_state.contactinfo.name,g_state.contactinfo.number,g_state.contactinfo.picpath,g_state.starttime,0);

    add_histroy_entry (appdata, appdata->g_state.historytype,
                       appdata->g_peer_info.name,
                       appdata->g_peer_info.number,
                       appdata->g_peer_info.picpath,
                       appdata->g_state.starttime,
                       appdata->g_state.startdate, 0);

  }


}

void
window_outgoing_setup_timer (MOKO_DIALER_APP_DATA * appdata)
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

void
on_window_outgoing_show (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata)
{

  //DBG_ENTER ();
  window_outgoing_setup_timer (appdata);
  //DBG_TRACE ();
  appdata->g_state.callstate = STATE_CALLING;
  //DBG_TRACE ();
  appdata->g_state.historytype = OUTGOING;
  //DBG_TRACE ();
  int retv = gsm_dial (appdata->g_peer_info.number);
  DBG_MESSAGE ("GSM_DIAL returns %d", retv);
  //DBG_LEAVE ();
}


gint
window_outgoing_init (MOKO_DIALER_APP_DATA * p_dialer_data)
{

  DBG_ENTER ();
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
    moko_dialer_status_set_error_icon (MOKO_DIALER_STATUS (status),
                                       "failure.png");
    moko_dialer_status_set_icon_by_index (MOKO_DIALER_STATUS (status), 0);

    gtk_box_pack_start (GTK_BOX (vbox), status, FALSE, FALSE, 0);


    GtkWidget *hbox2 = gtk_hbox_new (FALSE, 0);
    GtkWidget *button = gtk_button_new_with_label ("Speaker");
    gtk_button_set_image (GTK_BUTTON (button),
                          file_new_image_from_relative_path ("speaker.png"));
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_speaker_button_clicked), p_dialer_data);
    p_dialer_data->buttonSpeaker = button;
//gtk_widget_set_size_request(button,100,32);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE, 40);

    button = gtk_button_new_with_label ("Redial");
    gtk_button_set_image (GTK_BUTTON (button),
                          file_new_image_from_relative_path ("redial.png"));
    p_dialer_data->buttonRedial = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_redial_button_clicked), p_dialer_data);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE, 40);
    g_object_set (G_OBJECT (button), "no-show-all", TRUE, NULL);


    button = gtk_button_new_with_label ("Cancel");
    gtk_button_set_image (GTK_BUTTON (button),
                          file_new_image_from_relative_path ("cancel.png"));
    p_dialer_data->buttonCancel = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_cancel_button_clicked), p_dialer_data);
//gtk_widget_set_size_request(button,100,32);
    gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (button), TRUE, TRUE, 40);

    gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 50);


//currently     MokoDialogWindow is not finished, wating...
//   MokoDialogWindow* window = (MokoDialogWindow *)(moko_dialog_window_new());
//  moko_dialog_window_set_contents( window, GTK_WIDGET(vbox) );

    window = MOKO_FINGER_WINDOW (moko_finger_window_new ());
    moko_finger_window_set_contents (window, GTK_WIDGET (vbox));

    moko_dialer_status_set_title_label (MOKO_DIALER_STATUS (status),
                                        "Outgoing call");
    moko_dialer_status_set_status_label (MOKO_DIALER_STATUS (status),
                                         "Calling ... (00:00:00)");

    p_dialer_data->window_outgoing = GTK_WIDGET (window);
    p_dialer_data->status_outgoing = MOKO_DIALER_STATUS (status);

//   DBG_MESSAGE("p_dialer_data->status_outgoing=0X%x",p_dialer_data->status_outgoing);


    g_signal_connect ((gpointer) window, "show",
                      G_CALLBACK (on_window_outgoing_show), p_dialer_data);
    g_signal_connect ((gpointer) window, "hide",
                      G_CALLBACK (on_window_outgoing_hide), p_dialer_data);

  }



// gtk_widget_show(p_dialer_data->window_outgoing);

  DBG_LEAVE ();
  return 1;
}
