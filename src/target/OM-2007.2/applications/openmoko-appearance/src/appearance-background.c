/*
 * OpenMoko Appearance - Change various appearance related settings
 *
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "appearance-background.h"
#include <gconf/gconf-client.h>

static gboolean
unselect_file_idle (GtkFileChooser *chooser)
{
  gtk_file_chooser_unselect_all (chooser);
  return FALSE;
}

/* Following three functions taken from openmoko-today2 */
static void
wallpaper_notify (GConfClient *client, guint cnxn_id,
  	  GConfEntry *entry, AppearanceData *data)
{
  gint width, height, pwidth, pheight;
  GdkPixbuf *pixbuf, *pixbuf_scaled;
  GConfValue *value;
  const gchar *path = NULL;
  gfloat scale;

  if (!GTK_WIDGET_REALIZED (data->bg_ebox))
  	gtk_widget_realize (data->bg_ebox);

  /* Return if the background is tiny, we'll get called again when it 
   * resizes anyway.
   */
  width = data->bg_ebox->allocation.width;
  height = data->bg_ebox->allocation.height;
  if ((width <= 0) || (height <= 0)) return;
  
  value = gconf_entry_get_value (entry);
  if (value) path = gconf_value_get_string (value);
  if (!path || (!(pixbuf = gdk_pixbuf_new_from_file (path, NULL)))) {
  	/* We need to do this in an idle, otherwise there's some weird
  	 * race condition where it won't work...
  	 */
  	g_idle_add_full (G_PRIORITY_HIGH_IDLE, (GSourceFunc)
  		unselect_file_idle, data->bg_chooser, NULL);
  	if (data->wallpaper) {
  		g_object_unref (data->wallpaper);
  		data->wallpaper = NULL;
  		gtk_widget_queue_draw (data->bg_ebox);
  	}
  	return;
  }
  
  /* Select the file in the background chooser, if it isn't already */
  gtk_file_chooser_set_filename (
  	GTK_FILE_CHOOSER (data->bg_chooser), path);

  /* Create background pixmap */
  if (data->wallpaper) g_object_unref (data->wallpaper);
  data->wallpaper = gdk_pixmap_new (data->bg_ebox->window,
  	width, height, -1);
  
  /* Scale and draw pixbuf */
  pwidth = gdk_pixbuf_get_width (pixbuf);
  pheight = gdk_pixbuf_get_height (pixbuf);
  if (((gfloat)pwidth / (gfloat)pheight) >
      ((gfloat)width / (gfloat)height))
  	scale = (gfloat)height/(gfloat)pheight;
  else
  	scale = (gfloat)width/(gfloat)pwidth;
  pwidth *= scale;
  pheight *= scale;
  pixbuf_scaled = gdk_pixbuf_scale_simple (pixbuf, pwidth, pheight,
  	GDK_INTERP_BILINEAR);
  if (pixbuf_scaled) {
  	gdk_draw_pixbuf (data->wallpaper, NULL, pixbuf_scaled,
  		0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_MAX, 0, 0);
  	g_object_unref (pixbuf_scaled);
  }
  g_object_unref (pixbuf);
  
  /* Redraw */
  gtk_widget_queue_draw (data->bg_ebox);
}

static gboolean
bg_expose_cb (GtkWidget *widget, GdkEventExpose *event, AppearanceData *data)
{
  if (data->wallpaper)
  	gdk_draw_drawable (widget->window, widget->style->black_gc,
  		data->wallpaper, 0, 0, 0, 0, -1, -1);
  
  return FALSE;
}

static void
bg_size_allocate_cb (GtkWidget *widget, GtkAllocation *allocation,
  	     AppearanceData *data)
{
  static gint width = 0, height = 0;
  
  /* Re-scale wallpaper */
  if ((width != allocation->width) || (height != allocation->height)) {
  	width = allocation->width;
  	height = allocation->height;
  	gconf_client_notify (gconf_client_get_default (),
  		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER);
  }
}

static void
bg_response_cb (GtkDialog *dialog, gint response, AppearanceData *data)
{
  gchar *file;
  
  switch (response) {
      case GTK_RESPONSE_NO :
  	gconf_client_unset (gconf_client_get_default (),
  		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER,
  		NULL);
  	break;
      case GTK_RESPONSE_ACCEPT :
  	file = gtk_file_chooser_get_filename (
  		GTK_FILE_CHOOSER (data->bg_chooser));
  	gconf_client_set_string (gconf_client_get_default (),
  		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER,
  		file, NULL);
  	g_free (file);
  	break;
      case GTK_RESPONSE_CANCEL :
      default :
  	break;
  }
}

GtkWidget *
background_page_new (AppearanceData *data)
{
  GtkWidget *button, *align;
  
  /* Create an event box so we can draw a background for the page */
  data->bg_ebox = gtk_event_box_new ();
  
  /* Tell GTK we want to paint on this widget */
  gtk_widget_set_app_paintable (data->bg_ebox, TRUE);
  
  /* Connect to the 'expose' event to know when we should draw */
  g_signal_connect (data->bg_ebox, "expose-event",
  	G_CALLBACK (bg_expose_cb), data);
  
  /* Connect to the 'size-allocate' event so we can resize the image to 
   * fit the background.
   */
  g_signal_connect (data->bg_ebox, "size-allocate",
  	G_CALLBACK (bg_size_allocate_cb), data);
  
  /* Create a file-chooser dialog */
  data->bg_chooser = gtk_file_chooser_dialog_new ("Choose an image",
  	GTK_WINDOW (data->window), GTK_FILE_CHOOSER_ACTION_OPEN,
  	"No image", GTK_RESPONSE_NO,
  	GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
  	GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  
  /* Connect to the 'response' signal for the file chooser */
  g_signal_connect (data->bg_chooser, "response",
  	G_CALLBACK (bg_response_cb), data);
  
  /* Create a file-chooser button */
  button = gtk_file_chooser_button_new_with_dialog (data->bg_chooser);
  
  /* Create an alignment so we can squish the button to the bottom of 
   * the page, with padding
   */
  align = gtk_alignment_new (0.5, 1, 1, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), 0, 12, 12, 12);
  
  /* Pack widgets into the page/each other and show them */
  gtk_container_add (GTK_CONTAINER (align), button);
  gtk_container_add (GTK_CONTAINER (data->bg_ebox), align);
  gtk_widget_show_all (data->bg_ebox);
  
  /* Connect to GConf signals */
  gconf_client_add_dir (gconf_client_get_default (),
  	GCONF_POKY_INTERFACE_PREFIX, GCONF_CLIENT_PRELOAD_NONE, NULL);
  gconf_client_notify_add (gconf_client_get_default (),
  	GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER,
  	(GConfClientNotifyFunc)wallpaper_notify,
  	data, NULL, NULL);

  return data->bg_ebox;
}
