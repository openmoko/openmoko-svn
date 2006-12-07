/**
 * @file callbacks.c
 * @brief callbacks of openmoko-taskmanager based on callbacks.c.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
 #include <glib.h>

#include "callbacks.h"
#include "footer.h"

/* footer */
/**
*@brief footer leftbutton clicked callback function
*@param widget	GtkWidget reference
*@param my_data	user data
*@return none
*/
void 
footer_leftbutton_clicked(GtkWidget *widget, gpointer my_data) {	
    switch (fork()) {
    	  case 0:
    	      setpgid(0, 0); /* Stop us killing child */
    	      execvp("openmoko-taskmanager", NULL);
    	      exit(1);
    	  case -1:
    	  	g_debug ("Failed to fork()");
    	  	break;
    	  }
    }



/**
*@brief footer rightbutton clicked callback function
*@param widget	GtkWidget reference
*@param my_data	user data
*@return none
*/
void 
footer_rightbutton_clicked(GtkWidget *widget, gpointer my_data) {
    XEvent ev;
    int done = 0;
    Bool finish = FALSE;
    struct timeval then, now;
    Time click_time=800;
    Display *dpy;

    dpy = GDK_DISPLAY ();

    gettimeofday(&then, NULL);
    
 //check the buttoen event type: tap "done = 1 "; tap with hold "done = 2";
 //Fixme : when double clicked, there is three outputs, two "tab" and one" tab hold". 
  while (!done ) {
    if (XCheckMaskEvent(dpy,ButtonReleaseMask, &ev))
      if (ev.type == ButtonRelease) {
      	 done=1;
      }
    gettimeofday(&now, NULL);
    if ( (now.tv_usec-then.tv_usec) > (click_time*1000) ) {
    	done=2;
    }
  }
  /*check buttoen event type: tap "done = 1 "; tap with hold "done = 0"; activate done >1 ??.
  do{
  	gettimeofday(&now, NULL);
  	if (XCheckMaskEvent(dpy,ButtonReleaseMask, &ev))
  	  if (ev.type == ButtonRelease){
      	    done ++;
  	  }
  }while ((now.tv_usec-then.tv_usec) < (click_time*1000) 
  		&& (now.tv_sec == then.tv_sec));
  */
  
  //function for "tap" action, execute "openmoko-clocks application".
  if (done == 1){  
  	g_debug ("tab");
    mbcommand(dpy, MB_CMD_NEXT, NULL);
    return;
  }
  //function for "tap with hold" action, pop a popupmenu to change time format.
  else if (done == 2){
  	g_debug ("tab hold");
     mbcommand(dpy, MB_CMD_PREV, NULL);
  }
  /* Fixme : click event
  else if (done >1){
  	g_debug ("clicked");
  	mbcommand(dpy, MB_CMD_DESKTOP, NULL);
  }
  */
}


/* dbus */
/**
*@brief dbus filter function, which use to receive and filter dbus message.
*@param connection	DBusConnection
*@param message		DBusMessage
*@param user_data	
*@return none
*/
DBusHandlerResult signal_filter(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    g_debug( "signal_filter called" );
    g_debug( "type of message was %d", dbus_message_get_type(message));
    g_debug( "path of message was %s", dbus_message_get_path(message));
    g_debug( "interface of message was %s", dbus_message_get_interface(message));

    /* Application object is the user data */
    OMTaskManager* app = user_data;

    /* A signal from the bus saying we are about to be disconnected */
    if (dbus_message_is_signal
        (message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        /* Tell the main loop to quit */
        g_main_loop_quit(app->loop);
        /* We have handled this message, don't pass it on */
        return DBUS_HANDLER_RESULT_HANDLED;
        }
        /* A message on our interface */
        else if (dbus_message_is_signal(message, "org.openmoko.dbus.TaskManager", "push_statusbar_message")) {
            DBusError error;
            char *s;
            dbus_error_init (&error);
            if (dbus_message_get_args
                (message, &error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID)) {
                g_debug("Setting status bar text to '%s", s);
                footer_set_status( app->footer, s );
                //FIXME: SIGSEGV, when uncommented. It now leaks! :M:
                //dbus_free(s);
                } else {
                    g_print("Ping received, but error getting message: %s", error.message);
                    dbus_error_free (&error);
                }
                return DBUS_HANDLER_RESULT_HANDLED;
        }
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
