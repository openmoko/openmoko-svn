/**
 *  taskmanager.c
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "taskmanager.h"

#include <stdio.h> 
#include <X11/Xlib.h> 
#include <X11/Xatom.h> 
#include <glib.h> 
#include <gtk/gtk.h> 
#include <gdk/gdk.h> 
#include <gdk/gdkx.h> // GDK_WINDOW_XWINDOW
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "callbacks.h"
#include "dbus-conn.h"

#define OPAQUE  0x55555555
/* set the widget's transparency to opacity 
 * opacity is guint 0x00000000-0xffffffff 
 */ 
#define LOCK_FILE "/tmp/moko-taskmanager.lock"

static MokoTaskManager *tm;

static void 
handle_sigusr1 (int value)
{
  if (!tm)
       return;

  gtk_widget_show_all (GTK_WIDGET (tm->window));
  gtk_window_present (GTK_WIDGET (tm->window));
  gtk_widget_show (GTK_WIDGET (tm->wheel));
  gtk_widget_show (GTK_WIDGET (tm->toolbox));

  signal (SIGUSR1, handle_sigusr1);
  return;
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

int 
gtk_widget_set_transparency(GtkWidget *widget, guint opacity) 
{ 
   Display *display; 
   Window window; 
   Window parent_win; 
   Window root_win; 
   Window* child_windows; 
   int num_child_windows; 

   if(!GTK_IS_WIDGET(widget)){ 
                printf("gtk_widget_set_transparency: not a widget!\n"); 
                return -1; 
   } 

   if(widget->window == NULL){ 
                printf("gtk_widget_set_transparency: please init widget before set transparency!\n"); 
                return -1; 
   } 

   /* Set the Display and Screen */ 
   display = (Display*)gdk_x11_get_default_xdisplay(); 
   /* sync, so the window manager can know the new widget */ 
   XSync(display, False); 
   window = GDK_WINDOW_XWINDOW(widget->window); 

   /* Get the cureent window's top-level window */ 
   while(1){ 
        XQueryTree(display, window, 
                            &root_win, 
                            &parent_win, 
                            &child_windows, &num_child_windows); 
         XFree(child_windows); 
       /* found the top-level window */ 
          if(root_win == parent_win) break; 
          window = parent_win; 
   } 

   if(opacity == OPAQUE){ 
           XDeleteProperty(display, window, 
                                   XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False)); 
   }
   else{ 
           XChangeProperty(display, window, 
                                XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False), 
                                XA_CARDINAL, 32, PropModeReplace, 
                                (unsigned char *) &opacity, 1L); 
   } 

   XSync(display, False); 

   return 0; 
} 

int 
main (int argc, char** argv)
{
    Display *dpy;
    GtkWidget *image;
    pid_t lockapp;

    lockapp = testlock (LOCK_FILE);
    if (lockapp > 0)
     {
        kill (lockapp, SIGUSR1);
        return 0;
     }
    setlock (LOCK_FILE);

    tm = g_malloc (sizeof (MokoTaskManager));
    memset (tm, 0, sizeof (MokoTaskManager));
	
    gtk_init (&argc, &argv);
    dpy = GDK_DISPLAY ();

    tm->app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Openmoko-taskmanager" );
    
    /* finger based window */
    tm->window = MOKO_FINGER_WINDOW(moko_finger_window_new());
//    gtk_window_set_decorated (mma->window, FALSE);
    gtk_widget_show (GTK_WIDGET (tm->window));

    /* finger wheel object*/
    tm->wheel = moko_finger_window_get_wheel (tm->window);

    /* finger toolbox object*/
    tm->toolbox = moko_finger_window_get_toolbox (tm->window);

    tm->go_to =  moko_finger_tool_box_add_button_without_label (tm->toolbox);   
    image = gtk_image_new_from_file (PKGDATADIR"/active_task.png");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->go_to, image);
    tm->kill =   moko_finger_tool_box_add_button_without_label (tm->toolbox);
    image = gtk_image_new_from_file (PKGDATADIR"/close.png");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->kill, image);
    tm->kill_all = moko_finger_tool_box_add_button_without_label (tm->toolbox);
    image = gtk_image_new_from_file (PKGDATADIR"/close_all.png");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->kill_all, image);
    tm->quit =  moko_finger_tool_box_add_button_without_label (tm->toolbox);
    image = gtk_image_new_from_file (PKGDATADIR"/exit.png");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->quit, image);

    tm->l = moko_task_list_new();
    moko_update_store_list(dpy, tm->l->list_store);
    //moko_set_list_highlight(dpy, tm->l);
    gtk_widget_show (GTK_WIDGET (tm->l));
	
    moko_finger_window_set_contents (tm->window, GTK_WIDGET(tm->l));

    g_signal_connect (tm->go_to, "clicked", 
    			G_CALLBACK (moko_go_to_btn_cb), tm);
    g_signal_connect (tm->kill, "clicked", 
    			G_CALLBACK (moko_kill_btn_cb), tm);
    g_signal_connect (tm->kill_all, "clicked", 
    			G_CALLBACK (moko_kill_all_btn_cb), tm);
    g_signal_connect (tm->quit, "clicked", 
    			G_CALLBACK (moko_quit_btn_cb), tm);
    g_signal_connect (tm->wheel, "press_left_up",
    			G_CALLBACK 	(moko_wheel_left_up_press_cb), tm);
    g_signal_connect (tm->wheel, "press_right_down",
    			G_CALLBACK (moko_wheel_right_down_press_cb), tm);

    gdk_window_add_filter (NULL, moko_window_filter, tm->l);
    XSelectInput (dpy, DefaultRootWindow (dpy), PropertyChangeMask);

    signal (SIGUSR1, handle_sigusr1);

    //gtk_widget_set_transparency(tm->window, 50);
	
	moko_dbus_connect_init ();

    gtk_widget_show_all (GTK_WIDGET (tm->window));
    gtk_widget_show (GTK_WIDGET (tm->wheel));
    gtk_widget_show (GTK_WIDGET (tm->toolbox));

    gtk_main();

    if (tm)
        g_free (tm);
}
