/*  openmoko-panel-gsm.c
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */
#include <libmokoui/moko-panel-applet.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkbox.h>
#include <gtk/gtk.h>
#include <time.h>

#include "moko-gsm-conn.h"

typedef struct{
  GtkWidget *image;
  SignalStatus status;
} MokoSignal;

typedef struct {
  MokoSignal gsm;
  MokoSignal gprs;
  GtkWidget *hbox;
  guint timeout_id;
} GsmApplet;

static gchar *gsm_q_name[TOTAL_STATUS]={
  "SignalStrength_01.png",
  "SignalStrength_02.png",
  "SignalStrength_03.png",
  "SignalStrength_04.png",
  "SignalStrength_05.png",
  "SignalStrength_00.png"
};

static gchar *gprs_q_name[TOTAL_STATUS]={
  "SignalStrength25g_01.png",
  "SignalStrength25g_02.png",
  "SignalStrength25g_03.png",
  "SignalStrength25g_04.png",
  "SignalStrength25g_05.png",
  NULL
};

static void
gsm_applet_free (GsmApplet *applet)
{
  g_source_remove (applet->timeout_id);

  g_slice_free (GsmApplet, applet);
}

static gboolean
timeout_cb (GsmApplet *applet)
{
  char path[512];
  static SignalStatus status = 0;
  
  status = moko_panel_gsm_signal_quality ();
  if (status != applet->gsm.status )
  {
    applet->gsm.status = status;
    snprintf (path, 512, "%s/%s", PKGDATADIR, gsm_q_name[applet->gsm.status]);
    if (GTK_IS_IMAGE(applet->gsm.image))
      gtk_image_set_from_file (GTK_IMAGE(applet->gsm.image), path);
    else 
      g_error ("gsm image set failed");
  }

  status = moko_panel_gprs_signal_quality (); 
  g_debug ("status = %d", status);
  if (status == UN_CONN && GTK_WIDGET (applet->gprs.image))
  {
    gtk_widget_hide (GTK_WIDGET(applet->gprs.image));
  }
  else
  {
    applet->gprs.status = status;
    snprintf (path, 512, "%s/%s", PKGDATADIR, gprs_q_name[applet->gprs.status]);
    if (GTK_IS_IMAGE(applet->gprs.image))
      gtk_image_set_from_file (GTK_IMAGE(applet->gprs.image), path);
    gtk_widget_show (GTK_WIDGET (applet->gprs.image));
  }
  
  /* keep going*/
  return TRUE;
}

G_MODULE_EXPORT GtkWidget* 
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
  MokoPanelApplet* mokoapplet = moko_panel_applet_new();

  GsmApplet *applet;
  applet = g_slice_new (GsmApplet);
   
  g_object_weak_ref (G_OBJECT(mokoapplet), (GWeakNotify) gsm_applet_free, applet );
  applet->timeout_id = g_timeout_add(4000, (GSourceFunc) timeout_cb, applet);

  applet->gsm.image = gtk_image_new ();//make an empty GtkImage object
  applet->gsm.status = UN_INIT;
  applet->gprs.image = gtk_image_new ();//make an empty GtkImage object
  applet->gprs.status = UN_INIT;

  applet->hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show (applet->hbox);

  gtk_box_pack_start (GTK_BOX(applet->hbox), GTK_WIDGET(applet->gprs.image), FALSE, FALSE, 2);
  gtk_box_pack_end (GTK_BOX(applet->hbox), GTK_WIDGET(applet->gsm.image), FALSE, FALSE, 2);

  moko_panel_applet_set_widget (GTK_CONTAINER(mokoapplet), GTK_WIDGET(applet->hbox));
  gtk_widget_show_all (GTK_WIDGET(mokoapplet) );

  gsm_watcher_install ();

  return GTK_WIDGET(mokoapplet);
};
