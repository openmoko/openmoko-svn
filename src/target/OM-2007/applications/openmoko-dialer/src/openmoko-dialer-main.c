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
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "contacts.h"
#include  "history.h"
#include "error.h"
#include "dialergsm.h"
#include "openmoko-dialer-main.h"
#include "openmoko-dialer-window-dialer.h"
#include "openmoko-dialer-window-talking.h"
#include "openmoko-dialer-window-outgoing.h"
#include "openmoko-dialer-window-incoming.h"
#include "openmoko-dialer-window-pin.h"
#include "openmoko-dialer-window-history.h"

MOKO_DIALER_APP_DATA *p_dialer_data = 0;
MOKO_DIALER_APP_DATA *
moko_get_app_data ()
{
return p_dialer_data;
}
void gsm_pin_require(struct lgsm_handle *lh)
{
MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();

if(appdata)
{
appdata->lh=lh;
gtk_widget_show(appdata->window_pin);
}
else
{
DBG_ERROR("gui failed to initialize.try another time.");
}
	

}
void gsm_incoming_call(gchar * number)
{

MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();

if(appdata)
{
//first, we should remove the "" from the number.
char temp[20];
int start=0;
int end=strlen(number);
while(number[start]=='\"'&&start<end)start++;
if(end>1)while(number[end-1]=='\"'&&start<end)end--;

DBG_MESSAGE("START=%d,END=%d",start,end);
g_stpcpy(temp,number+start);
temp[end-1]=0;
DBG_MESSAGE("%s",temp);	



//got the number;
g_stpcpy(appdata->g_peer_info.number,temp);

//retrieve the contact information if any.
contact_get_peer_info_from_number(appdata->g_contactlist.contacts , &(appdata->g_peer_info));
// contact_get_peer_info_from_number


//transfer the contact info
window_incoming_prepare(appdata);

gtk_widget_show(appdata->window_incoming);
}
else
{
DBG_ERROR("gui failed to initialize.try another time.");
}


}

void gsm_peer_accept()
{
MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();
DBG_ENTER();
//moko_dialer_status_update_icon(appdata->status_outgoing);


appdata->g_state.callstate=STATE_TALKING;

gtk_widget_hide(appdata->window_outgoing);


//transfer the contact info
window_talking_prepare(appdata);

//start talking.

gtk_widget_show(appdata->window_talking);


DBG_LEAVE();
}

void gsm_peer_refuse()
{
MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();
window_outgoing_fails(appdata);
}

void gsm_peer_abort()
{

MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();
if(appdata->window_incoming)
	gtk_widget_hide(appdata->window_incoming);


}

void gsm_peer_disconnect()
{

     MOKO_DIALER_APP_DATA* appdata=moko_get_app_data();
     gsm_hangup();
     gtk_widget_hide(appdata->window_talking);

}



static void 
handle_sigusr1 (int value)
{
DBG_ENTER();
 MOKO_DIALER_APP_DATA*  p_data=moko_get_app_data();
if(!p_data)return;
GtkWidget* mainwindow=p_data->window_present;
 if(mainwindow==0)
 	mainwindow=p_data->window_dialer;
 
  if (mainwindow == NULL)
    {
      return;
    }
  gtk_widget_show_all (mainwindow);
  gtk_window_present (GTK_WINDOW (mainwindow));
  DBG_TRACE();
  signal (SIGUSR1, handle_sigusr1);
  DBG_LEAVE();
}

static pid_t 
testlock (char *fname)
{
  int fd;
  struct flock fl;

  fd = open (fname, O_WRONLY, S_IWUSR);
  if (fd < 0)
    {
      if (errno == ENOENT)
        {
          return 0;
        }
      else
        {
          perror ("Test lock open file");
          return -1;
        }
    }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;

  if (fcntl (fd, F_GETLK, &fl) < 0)
    {
      close (fd);
      return -1;
    }
  close (fd);

  if (fl.l_type == F_UNLCK)
    return 0;

  return fl.l_pid;
}

static void 
setlock (char *fname)
{
  int fd;
  struct flock fl;

  fd = open (fname, O_WRONLY|O_CREAT, S_IWUSR);
  if (fd < 0)
    {
      perror ("Set lock open file");
      return ;
    }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;

  if (fcntl (fd, F_SETLK, &fl) < 0)
    {
      perror ("Lock file");
      close (fd);
    }
}

int main( int argc, char** argv )
{
  pid_t           lockapp;

    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

if (argc != 1)
    {
      /* Add init code. */
    }
  lockapp = testlock ("/tmp/dialer.lock");
  if (lockapp > 0)
    {
      kill (lockapp, SIGUSR1);
      return 0;
    }
  setlock ("/tmp/dialer.lock");


    p_dialer_data=calloc(1,sizeof(MOKO_DIALER_APP_DATA));

    //init application data
   contact_init_contact_data(&(p_dialer_data->g_contactlist));
   history_init_history_data(&(p_dialer_data->g_historylist));
   

    /* application object */
//    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Dialer" );


//   gtk_main();



 GMainLoop* mainloop=0;
 mainloop = g_main_loop_new(NULL, FALSE );
 p_dialer_data->mainloop=mainloop;

signal (SIGUSR1, handle_sigusr1);
//init the dialer window
  window_dialer_init(p_dialer_data); 
  window_incoming_init(p_dialer_data); 
  window_pin_init(p_dialer_data); 
  window_outgoing_init(p_dialer_data); 
  window_history_init(p_dialer_data); 


DBG_WARN("\nusage: \"openmoko-dialer\" will not show any GUI initialy until you reactivate the app using another \"openmoko-dialer\" command");



//from now on we will not use multithreads.
  gsm_lgsm_start(mainloop);
  //gtk_widget_show(p_dialer_data->window_pin);
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
  g_main_loop_run(mainloop);


//release everything    
  contact_release_contact_list(&(p_dialer_data->g_contactlist)); 

  history_release_history_list(&(p_dialer_data->g_historylist));
    return 0;
}

