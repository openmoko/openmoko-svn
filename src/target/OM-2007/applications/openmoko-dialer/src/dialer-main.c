/*  openmoko-dialer.c
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
#include <libmokogsmd/moko-gsmd-connection.h>

#include <gtk/gtk.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "contacts.h"
#include "error.h"
#include "errno.h"
#include "dialer-main.h"
#include "dialer-window-dialer.h"
#include "dialer-window-talking.h"
#include "dialer-window-outgoing.h"
#include "dialer-window-incoming.h"
#include "dialer-window-pin.h"
#include "dialer-window-history.h"

#include "dialer-callbacks-connection.h"

MokoDialerData *p_dialer_data = 0;
MokoDialerData *
moko_get_app_data ()
{
  return p_dialer_data;
}

static void
handle_sigusr1 (int value)
{
  DBG_ENTER ();
  MokoDialerData *p_data = moko_get_app_data ();
  if (!p_data)
    return;
  GtkWidget *mainwindow = p_data->window_present;
  if (mainwindow == 0)
    mainwindow = p_data->window_dialer;

  if (mainwindow == NULL)
  {
    return;
  }
  gtk_widget_show_all (mainwindow);
  gtk_window_present (GTK_WINDOW (mainwindow));
  DBG_TRACE ();
  signal (SIGUSR1, handle_sigusr1);
  DBG_LEAVE ();
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

  fd = open (fname, O_WRONLY | O_CREAT, S_IWUSR);
  if (fd < 0)
  {
    perror ("Set lock open file");
    return;
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

static gboolean show_gui;

static GOptionEntry entries[] = {
  {"show-gui", 's', 0, G_OPTION_ARG_NONE, &show_gui,
   "Show the GUI at startup (default off)", "N"},
  {NULL}
};


int
main (int argc, char **argv)
{
  pid_t lockapp;


  if (argc != 1)
  {
    /* Add init code. */
    GError *error = NULL;
    GOptionContext *context = g_option_context_new ("");

    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_parse (context, &argc, &argv, &error);

    g_option_context_free (context);
  }

  //FIXME: the following lines to enable unique instance will be changed.
  lockapp = testlock ("/tmp/dialer.lock");
  if (lockapp > 0)
  {
    kill (lockapp, SIGUSR1);

    /* make sure startup notifaction is terminated */
    gdk_init(&argc, &argv);
    gdk_notify_startup_complete ();

    return 0;
  }
  setlock ("/tmp/dialer.lock");

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);
  moko_stock_register ();

  p_dialer_data = g_new0 (MokoDialerData, 1);

  //init application data
  contact_init_contact_data (&(p_dialer_data->g_contactlist));


  /* application object */
  g_set_application_name ("OpenMoko Dialer");

  /* Set up gsmd connection object */
  MokoGsmdConnection* conn = p_dialer_data->connection = moko_gsmd_connection_new ();

  /* power on GSM */
  moko_gsmd_connection_set_antenna_power (conn, TRUE);
  /* handle network registration 4 seconds after powering GSM */
  g_timeout_add( 4 * 1000, (GSourceFunc) initial_timeout_cb, conn );

  g_signal_connect (G_OBJECT (conn), "network-registration", (GCallback) network_registration_cb, p_dialer_data);
  g_signal_connect (G_OBJECT (conn), "incoming-call", (GCallback) incoming_call_cb, p_dialer_data);
  g_signal_connect (G_OBJECT (conn), "incoming-clip", (GCallback) incoming_clip_cb, p_dialer_data);
  g_signal_connect (G_OBJECT (conn), "pin-requested", (GCallback) incoming_pin_request_cb, p_dialer_data);

  /* Set up journal handling */
  p_dialer_data->journal = moko_journal_open_default ();

  signal (SIGUSR1, handle_sigusr1);

  //init the dialer window
  window_dialer_init (p_dialer_data);
  window_incoming_init (p_dialer_data);
  window_pin_init (p_dialer_data);
  window_outgoing_init (p_dialer_data);
  window_history_init (p_dialer_data);

  if (show_gui)
  {
    handle_sigusr1 (SIGUSR1);
  }

  gtk_main ();


  //release everything
  contact_release_contact_list (&(p_dialer_data->g_contactlist));

  /* closes the journal and frees allocated memory */
  moko_journal_close (p_dialer_data->journal);

  return 0;
}
