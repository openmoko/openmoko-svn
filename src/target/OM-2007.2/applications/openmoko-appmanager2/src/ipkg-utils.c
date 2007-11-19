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

static int verrevcmp(const char *val, const char *ref);

static gpointer
install_thread_func (ThreadData *data)
{
  int ret;
  gchar *real_name;
  gchar *success_string, *failure_string, *s;

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
    s = success_string;
  }
  else
  {
    s = failure_string;
  }
  gchar *err;

  err = get_error_msg ();  
  
  gdk_threads_enter ();
  am_progress_dialog_set_progress (AM_PROGRESS_DIALOG (data->progress_dialog), 1);
  am_progress_dialog_append_details_text (AM_PROGRESS_DIALOG (data->progress_dialog), s);
  am_progress_dialog_set_label_text (AM_PROGRESS_DIALOG (data->progress_dialog), s);
  am_progress_dialog_append_details_text (AM_PROGRESS_DIALOG (data->progress_dialog), err);
  gdk_threads_leave ();

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

gboolean
check_for_upgrade (IPK_PACKAGE *package, PKG_LIST_HEAD *list)
{
  IPK_PACKAGE *p;
  
  p = list->pkg_list;
  
  while (p)
  {
    if (g_str_equal (package->name, p->name))
    {
      gint ret;
      
      ret = verrevcmp (p->version, package->version);
      
      if (ret > 0)
      {
        g_debug ("Found upgradeable package: %s (old: %s, new: %s)", p->name, package->version, p->version);
        return TRUE;
      }
    }
    p = p->next;
  }
  return FALSE;
}

GList *
get_upgrade_list ()
{
  /* find duplicate package names in the available packages list and compare versions */
  IPK_PACKAGE *p;
  PKG_LIST_HEAD list;
  GList *upgradelist = NULL;
  gint ret;
  
  ret = ipkg_list_available_cmd (&list);
  
  p = list.pkg_list;
  while (p)
  {
    if (check_for_upgrade (p, &list))
    {
      upgradelist = g_list_prepend (upgradelist, p);
    }
    p = p->next;
  }
  return upgradelist;
}

static gpointer
upgrade_packages_thread (AmProgressDialog *dlg)
{
  args_t args;

  memset (&args, 0, sizeof (args));
  args_init (&args);

  ipkg_packages_upgrade (&args);

  gdk_threads_enter ();
  am_progress_dialog_set_progress (dlg, 1);
  am_progress_dialog_set_label_text (dlg, "Upgrading finished");
  gdk_threads_leave ();
}

void
upgrade_packages ()
{
  GtkWidget *dlg;

  dlg = am_progress_dialog_new_full ("Upgrade", "Upgrading packages...", -1);
  
  g_thread_create ((GThreadFunc) upgrade_packages_thread, dlg, FALSE, NULL);

  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);
}

/*
 * @brief Version compare
 *
 * This function is copy from ipkg.(pkg.c)
 * The verrevcmp() function compares the two version string "val" and
 * "ref". It returns an integer less than, equal to, or greater than 
 * zero if "val" is found, respectively, to be less than, to match, or
 * be greater than "ref".
 */
static int 
verrevcmp(const char *val, const char *ref)
{
  int vc, rc;
  long vl, rl;
  const char *vp, *rp;
  const char *vsep, *rsep;

  if (!val) val= "";
  if (!ref) ref= "";
  for (;;) 
    {
      vp= val;  while (*vp && !isdigit(*vp)) vp++;
      rp= ref;  while (*rp && !isdigit(*rp)) rp++;
      for (;;) 
        {
          vc= (val == vp) ? 0 : *val++;
          rc= (ref == rp) ? 0 : *ref++;
          if (!rc && !vc) break;
          if (vc && !isalpha(vc)) vc += 256;
          if (rc && !isalpha(rc)) rc += 256;
          if (vc != rc) return vc - rc;
        }
      val= vp;
      ref= rp;
      vl=0;  if (isdigit(*vp)) vl= strtol(val,(char**)&val,10);
      rl=0;  if (isdigit(*rp)) rl= strtol(ref,(char**)&ref,10);
      if (vl != rl) return vl - rl;

      vc = *val;
      rc = *ref;
      vsep = strchr(".-", vc);
      rsep = strchr(".-", rc);
      if (vsep && !rsep) return -1;
      if (!vsep && rsep) return +1;

      if (!*val && !*ref) return 0;
      if (!*val) return -1;
      if (!*ref) return +1;
    }
}
