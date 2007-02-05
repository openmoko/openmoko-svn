/*   openmoko-dialer.c
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
#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvbox.h>
#include "contacts.h"
#include  "history.h"
#include "error.h"
#include "dialergsm.h"
#include "openmoko-dialer-main.h"
#include "openmoko-dialer-window-dialer.h"
#include "openmoko-dialer-window-outgoing.h"
#include "openmoko-dialer-window-incoming.h"
MOKO_DIALER_APP_DATA *p_dialer_data = 0;
MOKO_DIALER_APP_DATA *
moko_get_app_data ()
{
  return p_dialer_data;
}

void
gsm_incoming_call (gchar * number)
{

  MOKO_DIALER_APP_DATA *appdata = moko_get_app_data ();

  if (appdata)
  {
//first, we should remove the "" from the number.
    char temp[20];
    int start = 0;
    int end = strlen (number);
    while (number[start] == '\"' && start < end)
      start++;
    if (end > 1)
      while (number[end - 1] == '\"' && start < end)
        end--;

    DBG_MESSAGE ("START=%d,END=%d", start, end);
    strcpy (temp, number + start);
    temp[end - 1] = 0;
    DBG_MESSAGE ("%s", temp);



//got the number;
    strcpy (appdata->g_peer_info.number, temp);

//retrieve the contact information if any.
    contact_get_peer_info_from_number (appdata->g_contactlist.contacts,
                                       &(appdata->g_peer_info));
// contact_get_peer_info_from_number


//transfer the contact info
    window_incoming_prepare (appdata);

    gtk_widget_show (appdata->window_incoming);
  }
  else
  {
    DBG_ERROR ("gui failed to initialize.try another time.");
  }


}

void
gsm_peer_accept ()
{
  MOKO_DIALER_APP_DATA *appdata = moko_get_app_data ();
  DBG_ENTER ();
//moko_dialer_status_update_icon(appdata->status_outgoing);


  appdata->g_state.callstate = STATE_TALKING;

  gtk_widget_hide (appdata->window_outgoing);


//transfer the contact info
  window_talking_prepare (appdata);

//start talking.

  gtk_widget_show (appdata->window_talking);


  DBG_LEAVE ();
}

void
gsm_peer_refuse ()
{
  MOKO_DIALER_APP_DATA *appdata = moko_get_app_data ();
  window_outgoing_fails (appdata);
}

void
gsm_peer_abort ()
{

  MOKO_DIALER_APP_DATA *appdata = moko_get_app_data ();
  if (appdata->window_incoming)
    gtk_widget_hide (appdata->window_incoming);


}

void
gsm_peer_disconnect ()
{

  MOKO_DIALER_APP_DATA *appdata = moko_get_app_data ();
  gsm_hangup ();
  gtk_widget_hide (appdata->window_talking);

}

int
main (int argc, char **argv)
{
  p_dialer_data = calloc (1, sizeof (MOKO_DIALER_APP_DATA));
  /* Initialize GTK+ */
  gtk_init (&argc, &argv);


  //init application data
  contact_init_contact_data (&(p_dialer_data->g_contactlist));
  history_init_history_data (&(p_dialer_data->g_historylist));


  /* application object */
//    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
  g_set_application_name ("OpenMoko Dialer");


//   gtk_main();



  GMainLoop *mainloop = 0;
  mainloop = g_main_loop_new (NULL, FALSE);
  p_dialer_data->mainloop = mainloop;


//init the dialer window
  window_dialer_init (p_dialer_data);
//  window_incoming_init(p_dialer_data); 
//  window_outgoing_init(p_dialer_data); 
//  window_history_init(p_dialer_data); 



//from now on we will not use multithreads.
  gsm_lgsm_start (mainloop);
  //start a timer to monitor incoming calls
  //gtk_timeout_add(100,incoming_calls,0);

//instead, we add a g_source


  //gdk_threads_enter();
  //gtk_main ();
  //gdk_threads_leave();

//  GMainLoop* mainloop = g_main_loop_new(NULL, FALSE );

//  [ set up a GSource ]
//  [ add a GPollFD ]
//  g_source_attach( gsource, NULL );
  g_main_loop_run (mainloop);







//release everything    
  contact_release_contact_list (&(p_dialer_data->g_contactlist));

  history_release_history_list (&(p_dialer_data->g_historylist));
  return 0;
}
