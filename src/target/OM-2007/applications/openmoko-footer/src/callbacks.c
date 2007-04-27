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
#include <X11/Xlib.h>
#include <dbus/dbus.h>
#include <gdk/gdk.h>
#include <glib/gthread.h>
#include <pthread.h>
#include <sys/time.h>

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
footer_leftbutton_clicked(GtkWidget *widget, gpointer my_data)
{
    switch (fork())
    {
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
footer_rightbutton_clicked(GtkWidget *widget, gpointer my_data)
{
    XEvent ev;
    int done = 0;
    struct timeval then, now;
    Time click_time=800;
    Display *dpy;

    dpy = GDK_DISPLAY ();

    gettimeofday(&then, NULL);
 //check the buttoen event type: tap "done = 1 "; tap with hold "done = 2";
 //Fixme : when double clicked, there is three outputs, two "tab" and one" tab hold". 
    while (!done )
    {
        if (XCheckMaskEvent(dpy,ButtonReleaseMask, &ev))
        if (ev.type == ButtonRelease)
        {
          done=1;
        }
        gettimeofday(&now, NULL);
        if ( (now.tv_usec-then.tv_usec) > (click_time*1000) )
        {
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
    if (done == 1)
    {
      g_debug ("tap");
      mbcommand(dpy, MB_CMD_NEXT, NULL);
      return;
    }
  //function for "tap with hold" action, pop a popupmenu to change time format.
    else if (done == 2)
    {
      g_debug ("tap hold");
      mbcommand(dpy, MB_CMD_PREV, NULL);
    }
  /* Fixme : click event
  else if (done >1){
  	g_debug ("clicked");
  	mbcommand(dpy, MB_CMD_DESKTOP, NULL);
  }
  */
}

GdkFilterReturn
target_window_event_filter_cb (GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
  MokoFooterApp *app = (MokoFooterApp *)user_data;

  XEvent *xev;
  gchar *message;

  xev = (XEvent *) xevent;

  if (xev->type == PropertyNotify)
  {
    if (xev->xproperty.atom == 
        gdk_x11_get_xatom_by_name ("_MOKO_STATUS_MESSAGE"))
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems;
      unsigned long bytes;
      guchar *data;
      Display *xdisplay;
      Window w;

      xdisplay = gdk_x11_get_default_xdisplay ();

      w = GDK_WINDOW_XID (app->target_window);

      XGetWindowProperty (xdisplay,
          w,
          gdk_x11_get_xatom_by_name ("_MOKO_STATUS_MESSAGE"),
          0, 512, FALSE,
          AnyPropertyType,
          &actual_type,
          &actual_format,
          &nitems,
          &bytes,
          &data);

        if (nitems > 0)
        {
          message = g_strndup (data, nitems);
          footer_set_status_message (FOOTER (app->footer), message);
          g_free (message);
        }

        XFree (data);
    }

    if (xev->xproperty.atom == 
        gdk_x11_get_xatom_by_name ("_MOKO_STATUS_PROGRESS"))
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems;
      unsigned long bytes;
      guchar *data;
      Display *xdisplay;
      Window w;

      xdisplay = gdk_x11_get_default_xdisplay ();

      w = GDK_WINDOW_XID (app->target_window);

      XGetWindowProperty (xdisplay,
          w,
          gdk_x11_get_xatom_by_name ("_MOKO_STATUS_PROGRESS"),
          0, sizeof (gdouble), FALSE,
          AnyPropertyType,
          &actual_type,
          &actual_format,
          &nitems,
          &bytes,
          &data);

        if (nitems > 0)
        {
          gdouble progress;

          progress = *((gdouble *)data);
          footer_set_status_progress (FOOTER (app->footer), progress);
        }

        XFree (data);
    }
  }

  return GDK_FILTER_CONTINUE;
}

GdkFilterReturn 
root_window_event_filter_cb (GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
  MokoFooterApp *app = (MokoFooterApp *)user_data;
  gchar *message;
  XEvent *xev;

  xev = (XEvent *) xevent;

  if (xev->type == PropertyNotify)
  {
    if (xev->xproperty.atom == 
        gdk_x11_get_xatom_by_name ("_NET_ACTIVE_WINDOW"))
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems;
      unsigned long bytes;
      guchar *data;
      Display *xdisplay;
      Window xroot_window;
      Window w;

      xdisplay = gdk_x11_get_default_xdisplay ();
      xroot_window = gdk_x11_get_default_root_xwindow ();

      XGetWindowProperty (xdisplay, 
          xroot_window, 
          gdk_x11_get_xatom_by_name ("_NET_ACTIVE_WINDOW"),
          0, 3, FALSE, 
          AnyPropertyType,
          &actual_type,
          &actual_format,
          &nitems,
          &bytes,
          &data);

      w = (Window)((gint32 *)data)[0];

      XFree (data);

      if (w != 0)
      {
        if (app->target_window == NULL || w != GDK_WINDOW_XID (app->target_window))
        {
          if (app->target_window != NULL)
          {
            gdk_window_remove_filter (app->target_window, 
                (GdkFilterFunc)target_window_event_filter_cb, app);

            gdk_window_set_events (app->target_window, 
            gdk_window_get_events (app->target_window) & ~GDK_PROPERTY_CHANGE_MASK);
          }

          app->target_window = gdk_window_foreign_new (w);
          gdk_window_set_events (app->target_window, 
              gdk_window_get_events (app->target_window) | GDK_PROPERTY_CHANGE_MASK);
          gdk_window_add_filter (app->target_window, 
              (GdkFilterFunc)target_window_event_filter_cb, app);
        }

        XGetWindowProperty (xdisplay,
          w,
          gdk_x11_get_xatom_by_name ("_MOKO_STATUS_MESSAGE"),
          0, 512, FALSE,
          AnyPropertyType,
          &actual_type,
          &actual_format,
          &nitems,
          &bytes,
          &data);

        if (nitems > 0)
        {
          message = g_strndup (data, nitems);
          footer_set_status_message (FOOTER (app->footer), message);
          g_free (message);
        }

        XFree (data);

        XGetWindowProperty (xdisplay,
          w,
          gdk_x11_get_xatom_by_name ("_MOKO_STATUS_PROGRESS"),
          0, sizeof (gdouble), FALSE,
          AnyPropertyType,
          &actual_type,
          &actual_format,
          &nitems,
          &bytes,
          &data);

        if (nitems > 0)
        {
          gdouble progress;

          progress = *((gdouble *)data);
          footer_set_status_progress (FOOTER (app->footer), progress);
        }

        XFree (data);
      }
    }
  }

  return GDK_FILTER_CONTINUE;
}

