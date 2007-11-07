/*
 *  The application manager in the Openmoko
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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
 *  Author: OpenedHand Ltd. <info@openedhand.com>
 *
 */
 
#include <gtk/gtk.h>
 
#include "appmanager-data.h"
#include "ipkgapi.h"
#include "am-progress-dialog.h"

typedef struct
{
  gchar *package_name;
  GtkWidget *progress_dialog;
  gboolean remove; /* true if we want to remove the package */
} ThreadData;

static gpointer
install_thread_func (ThreadData *data)
{
  int ret;
  gchar *real_name;
  gchar *success_string, *failure_string;

  if (!data->remove)
  {
    ret = ipkg_install_cmd (data->package_name, "root", &real_name);
    success_string = "Install succeeded\n";
    failure_string = "Install failed\n";
  }
  else
  {
    ret = ipkg_remove_cmd (data->package_name);
    success_string = "Removal succeeded\n";
    failure_string = "Removal failed\n";
  }

  if (ret == 0)
  {
    gdk_threads_enter ();
    am_progress_dialog_set_progress (data->progress_dialog, 1);
    am_progress_dialog_append_details_text (data->progress_dialog, success_string);
    am_progress_dialog_set_label_text (data->progress_dialog, success_string);
    gdk_threads_leave ();
  }
  else
  {
    gchar *err;

    err = get_error_msg ();

    gdk_threads_enter ();
    am_progress_dialog_set_progress (data->progress_dialog, 1);
    am_progress_dialog_append_details_text (data->progress_dialog, failure_string);
    am_progress_dialog_set_label_text (data->progress_dialog, failure_string);
    am_progress_dialog_append_details_text (data->progress_dialog, err);
    gdk_threads_leave ();
  
  }

  g_free (data);

  return NULL;
}

static gpointer
update_package_list_thread (AmProgressDialog *dlg)
{
  int ret;
  
  ret = ipkg_update_cmd ();
  
  if (ret == 0)
  {
    gdk_threads_enter ();
    am_progress_dialog_set_progress (AM_PROGRESS_DIALOG (dlg), 1);
    am_progress_dialog_append_details_text (AM_PROGRESS_DIALOG (dlg),
                                            "Updated\n");
    am_progress_dialog_set_label_text (AM_PROGRESS_DIALOG (dlg),
                                       "Package list updated");
    gdk_threads_leave ();
  }
  else
  {
    gchar *err;

    err = get_error_msg ();

    gdk_threads_enter ();
    am_progress_dialog_set_progress (AM_PROGRESS_DIALOG (dlg), 1);
    am_progress_dialog_set_label_text (AM_PROGRESS_DIALOG (dlg),
                                       "Error updating package list");
    am_progress_dialog_append_details_text (AM_PROGRESS_DIALOG (dlg),
                                            "Error updating package list\n");
    am_progress_dialog_append_details_text (AM_PROGRESS_DIALOG (dlg), err);
    gdk_threads_leave ();
  
  }
  
  return NULL;
}

void
install_package (ApplicationManagerData *data, gchar *name)
{
  GtkWidget *dlg;
  gchar *s;
  ThreadData *td;

  td = g_new0 (ThreadData, 1);

  s = g_strdup_printf ("Installing \"%s\"", name);
  dlg = am_progress_dialog_new_full ("Install", s, -1);
  g_free (s);
  
  td->package_name = name;
  td->progress_dialog = dlg;
  td->remove = FALSE;
  
  g_thread_create ((GThreadFunc) install_thread_func, td, FALSE, NULL);

  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);  
}

void
remove_package (ApplicationManagerData *data, gchar *name)
{
  GtkWidget *dlg;
  gchar *s;
  ThreadData *td;
  
  td = g_new0 (ThreadData, 1);

  s = g_strdup_printf ("Removing \"%s\"", name);
  dlg = am_progress_dialog_new_full ("Install", s, -1);
  g_free (s);
  
  td->package_name = name;
  td->progress_dialog = dlg;
  td->remove = TRUE;
  
  g_thread_create ((GThreadFunc) install_thread_func, td, FALSE, NULL);

  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);  
}

void
update_package_list (ApplicationManagerData *data)
{
  GtkWidget *dlg;

  dlg = am_progress_dialog_new_full ("Update", "Updating package list", -1);
  
  g_thread_create ((GThreadFunc) update_package_list_thread, dlg, FALSE, NULL);

  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);
}
