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
#include <time.h>

#include "moko-gsm-conn.h"

typedef struct {
	int gsm_quality;
	GtkImage *image;
	guint timeout_id;
} GsmApplet;

static void
gsm_applet_free (GsmApplet *applet)
{
    g_source_remove (applet->timeout_id);

    g_slice_free (GsmApplet, applet);
}

static gboolean
timeout_cb (GsmApplet *applet)
{
  int new_gsm_q = moko_panel_gsm_signal_quality ();
  
  if (applet->gsm_quality == new_gsm_q)
	/*Keep going, image need not change*/
    return TRUE;
  else
    applet->gsm_quality = new_gsm_q;

  g_debug ("gsm signal = %d", applet->gsm_quality);

	switch (applet->gsm_quality)
	{
		case GSM_SIGNAL_ERROR :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_00.png");
		    break;

		case GSM_SIGNAL_LEVEL_1 :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_01.png");
			break;

		case GSM_SIGNAL_LEVEL_2 :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_02.png");
			break;

		case GSM_SIGNAL_LEVEL_3 :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_03.png");
			break;

		case GSM_SIGNAL_LEVEL_4 :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_04.png");
			break;

		case GSM_SIGNAL_LEVEL_5 :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_05.png");
			break;

		default :
			gtk_image_set_from_file (applet->image, PKGDATADIR"/SignalStrength_00.png");
			break;
	}
    /* Keep going */
    return TRUE;
}

G_MODULE_EXPORT GtkWidget* 
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    GsmApplet *applet;
    applet = g_slice_new (GsmApplet);

    applet->gsm_quality = GSM_SIGNAL_ERROR;

    applet->image = GTK_IMAGE(gtk_image_new_from_file (PKGDATADIR"/SignalStrength_00.png"));
    gtk_widget_set_name( applet->image, "OpenMoko gsm applet" );
    g_object_weak_ref( G_OBJECT(applet->image), (GWeakNotify) gsm_applet_free, applet );

    applet->timeout_id = g_timeout_add(10000, (GSourceFunc) timeout_cb, applet);

    moko_panel_applet_set_widget( GTK_CONTAINER(mokoapplet), applet->image );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );

    gsm_watcher_install ();

    return GTK_WIDGET(mokoapplet);
};
