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

typedef struct
{
  gchar *package_name;
  int pulse_source;
  GtkWidget *label;
  GtkWidget *pbar;
  GtkWidget *details;
  GtkWidget *dlg;
} InstallData;

static void
add_text_to_textview (GtkTextView *tv, gchar *text)
{
  GtkTextBuffer *buf;
  GtkTextIter iter;
  
  buf = gtk_text_view_get_buffer (tv);
  
  gtk_text_buffer_get_end_iter (buf, &iter);
  gtk_text_buffer_insert (buf, &iter, text, -1);
}

static gboolean
progress_bar_pulse (GtkProgressBar *pbar)
{
  if (GTK_IS_PROGRESS_BAR (pbar))
  {
    gtk_progress_bar_pulse (pbar);
    return TRUE;
  }
  
  return FALSE;
}

static gpointer
install_thread_func (InstallData *data)
{
  int ret;
  gchar *real_name;
  
  ret = ipkg_install_cmd (data->package_name, "root", &real_name);
  g_source_remove (data->pulse_source);

  if (ret == 0)
  {
    gdk_threads_enter ();
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data->pbar), 1.0);
    gtk_label_set_text (GTK_LABEL (data->label), "Install succeeded!");
    add_text_to_textview (GTK_TEXT_VIEW (data->details), "Install succeeded\n");
    gtk_widget_set_sensitive (GTK_WIDGET (GTK_DIALOG(data->dlg)->action_area), TRUE);
    gdk_threads_leave ();
  }
  else
  {
    gchar *err, *message;

    err = get_error_msg ();
    message = g_strdup_printf ("Install failed: %s", err);

    gdk_threads_enter ();
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data->pbar), 1.0);
    gtk_label_set_text (GTK_LABEL (data->label), "Install failed");
    add_text_to_textview (GTK_TEXT_VIEW (data->details), message);
    gtk_widget_set_sensitive (GTK_WIDGET (GTK_DIALOG(data->dlg)->action_area), TRUE);
    gdk_threads_leave ();
  
    g_free (message);
  }

  g_free (data);

  return NULL;
}

void
install_package (ApplicationManagerData *data, gchar *name)
{
  gchar *s;
  GtkWidget *dlg, *vbox, *label, *progress, *details, *w, *sw;

  InstallData *install_data;
  
  dlg = gtk_dialog_new_with_buttons ("Install", GTK_WINDOW (data->mwindow),
                                     GTK_DIALOG_MODAL,
                                     GTK_STOCK_OK, GTK_RESPONSE_CANCEL,
                                     NULL);
  gtk_dialog_set_has_separator (GTK_DIALOG (dlg), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (GTK_DIALOG (dlg)->action_area), FALSE);
  
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dlg)->vbox), vbox);
  
  s = g_strdup_printf ("Installing \"%s\"", name);
  label = gtk_label_new (s);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  g_free (s);
  
  progress = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), progress, FALSE, FALSE, 0);
  
  w = gtk_expander_new ("Details");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), w);
  
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (w), sw);
  
  details = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER (sw), details);

  install_data = g_new0 (InstallData, 1);
  install_data->package_name = name;
  install_data->pulse_source = g_timeout_add (250, (GSourceFunc) progress_bar_pulse, progress);
  install_data->label = label;
  install_data->pbar = progress;
  install_data->details = details;
  install_data->dlg = dlg;

  g_thread_create ((GThreadFunc) install_thread_func, install_data, FALSE, NULL);

  gtk_widget_show_all (vbox);
  gtk_dialog_run (GTK_DIALOG (dlg));

  gtk_widget_destroy (dlg);
}

