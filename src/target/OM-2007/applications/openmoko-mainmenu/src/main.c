/**
 *  @file main.c
 *  @brief The Main Menu in the Openmoko
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
 *
 */

#include "callbacks.h"


#include "main.h"

#include <libmokoui/moko-window.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define LOCK_FILE "/tmp/moko-mainmenu.lock"

static MokoMainmenuApp *mma;

static void
handle_sigusr1 (int value)
{
  if (!mma)
       return;
  //gtk_window_present (GTK_WINDOW(mma->window));
  g_debug ("Show finger menu");
  moko_window_set_status_message (mma->fm->window, "Openmoko main menu");

  signal (SIGUSR1, handle_sigusr1);
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

static void
moko_mainmenu_usage (const char *bin_name)
{
  fprintf (stderr, "%s usage: %s [Options...]\n"
    "Where options are:\n"
	"--help, -h             print help\n"
	"--finger-menum, -fm    Show finger menu when application started\n",
	bin_name, bin_name);
  exit (1);
}

static void
moko_mainmenu_init(MokoMainmenuApp *mma, int argc, char** argv)
{
  gint i;
  gint ret = 0 ;
  gboolean show_fm = FALSE;

  for (i=1; i< argc; i++)
  {
    if (!strcmp("--finger-menu", argv[i]) || !strcmp("-fm", argv[i]))
    {
	  show_fm = TRUE;
	  continue;
	}
	if (++i>=argc)
	  moko_mainmenu_usage (argv[0]);
  }

  /* Buid Root item, don't display */
  mma->top_item = mokodesktop_item_new_with_params ("Home",
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );

  /* Build Lists (parse .directory and .desktop files) */
  ret = mokodesktop_init(mma->top_item, ITEM_TYPE_CNT);

  gtk_init( &argc, &argv );

  /*MokoFingerMenu object*/
  mma->fm = moko_finger_menu_new ();
  moko_finger_menu_build (mma->fm, mma->top_item);
  if(show_fm)
	moko_finger_menu_show (mma->fm);

  /*MokoStylusMenu object*/
  mma->sm = moko_stylus_menu_new ();
  moko_stylus_menu_build (mma->sm, mma->top_item);
}
int
main( int argc, char** argv )
{
    pid_t lockapp;
    lockapp = testlock (LOCK_FILE);
	int ret;

    if (lockapp > 0)
     {
        kill (lockapp, SIGUSR1);
        return 0;
     }

    setlock (LOCK_FILE);

    mma = g_malloc0 (sizeof (MokoMainmenuApp));
    if (!mma)
    {
        g_error ("openmoko-mainmenu application initialize FAILED.");
		exit (0);
    }
    memset (mma, 0, sizeof (MokoMainmenuApp));

    if (!moko_dbus_connect_init ())
    {
        g_error ("Failed to initial dbus connection.");
		exit (0);
    }

 // moko_mainmenu_init (mma, argc, argv);

  /* Buid Root item, don't display */
  mma->top_item = mokodesktop_item_new_with_params ("Home",
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );

  /* Build Lists (parse .directory and .desktop files) */
  ret = mokodesktop_init(mma->top_item, ITEM_TYPE_CNT);

  gtk_init( &argc, &argv );

  /*MokoFingerMenu object*/
  mma->fm = moko_finger_menu_new ();
  moko_finger_menu_build (mma->fm, mma->top_item);
  moko_finger_menu_show (mma->fm);

  /*MokoStylusMenu object*/
 // mma->sm = moko_stylus_menu_new ();
 // moko_stylus_menu_build (mma->sm, mma->top_item);
  signal (SIGUSR1, handle_sigusr1);

  gtk_main();

  if (mma)
  {
	g_free (mma);
  }

  return 0;
}
