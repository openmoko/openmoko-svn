/*  moko-message-dialog.c
 *
 *  Authored (in part) by Rob Bradford <rob@openedhand.com>
 *
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
 *  Also contains code directly derived from GTK+ (gtk/gtkdialog.c) with the
 *  following Copyright notice:
 *
 *  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 *  file for a list of people on the GTK+ Team.  See the ChangeLog
 *  files for a list of changes.  These files are distributed with
 *  GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 *
 *  Derivation Copyright (C) 2007 OpenMoko Inc.
 *  Derivation Authored by Rob Bradford <rob@openedhand.com?
 */

#include "moko-message-dialog.h"
#include <gtk/gtk.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

G_DEFINE_TYPE (MokoMessageDialog, moko_message_dialog, GTK_TYPE_DIALOG)

#define MESSAGE_DIALOG_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_MESSAGE_DIALOG, MokoMessageDialogPrivate))

typedef struct _MokoMessageDialogPrivate MokoMessageDialogPrivate;

struct _MokoMessageDialogPrivate
{
  gchar *message;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *image_alignment;
  gboolean window_shaped;
  GdkPixbuf *background_pixbuf;
  GdkPixbuf *root_pixbuf;
  GdkPixbuf *pixbuf;
};

static void
moko_message_dialog_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_message_dialog_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_message_dialog_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_message_dialog_parent_class)->dispose)
    G_OBJECT_CLASS (moko_message_dialog_parent_class)->dispose (object);
}

static void
moko_message_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_message_dialog_parent_class)->finalize (object);
}

/* 
 * We need to override the map event handling to grab the root window before
 * our widget gets drawn onto it.
 */
static void
moko_message_dialog_map (GObject *object)
{
  MokoMessageDialogPrivate *priv = MESSAGE_DIALOG_PRIVATE (object);
  MokoMessageDialog *dialog = MOKO_MESSAGE_DIALOG (object);

  gint w, h;
  GdkWindow *root_window;
  GdkPixbuf *root_pixbuf;

  GdkPixbuf *old_pixbuf;

  /* Grab the root window and its dimensions */
  root_window = gtk_widget_get_root_window (GTK_WIDGET(dialog));
  gdk_drawable_get_size (root_window, &w, &h);

  /* Generate a pixbuf from this root window */
  root_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, w, h);
  gdk_pixbuf_get_from_drawable (root_pixbuf, root_window, NULL, 0, 0, 0, 0, w, h);

  old_pixbuf = priv->root_pixbuf;
  priv->root_pixbuf = root_pixbuf;
  g_object_unref (old_pixbuf);

  g_object_unref (priv->pixbuf);
  priv->pixbuf = NULL;

  if (GTK_WIDGET_CLASS (moko_message_dialog_parent_class)->map)
    GTK_WIDGET_CLASS (moko_message_dialog_parent_class)->map (object);
}

static void
moko_message_dialog_class_init (MokoMessageDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoMessageDialogPrivate));

  object_class->get_property = moko_message_dialog_get_property;
  object_class->set_property = moko_message_dialog_set_property;
  object_class->dispose = moko_message_dialog_dispose;
  object_class->finalize = moko_message_dialog_finalize;

  widget_class->map = moko_message_dialog_map;
}

/*
static void
moko_message_dialog_shape_window (GtkWidget *widget)
{
  GtkRcStyle *rc_style;
  GdkPixbuf *pixbuf;
  GError *error = NULL;
  GdkBitmap *mask;

  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE (widget);

  rc_style = widget->style->rc_style;

  pixbuf = gdk_pixbuf_new_from_file (rc_style->bg_pixmap_name[GTK_STATE_NORMAL], &error);

  if (pixbuf == NULL)
    {
      g_warning ("Error loading background pixbuf: %s", error->message);
      g_clear_error (&error);
      return;
    }

  mask = gdk_pixmap_new (NULL, gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf), 1);
  gdk_pixbuf_render_threshold_alpha (pixbuf, mask, 0, 0, 0, 0, -1, -1, 100);

  g_object_unref (pixbuf);
  
  gdk_window_shape_combine_mask (widget->window, mask, 0, 0);

  g_object_unref (mask);

  priv->window_shaped = TRUE;
}
*/

/* 
 * Need to grab the pixbuf for background in the realize callback since we
 * need to have the style setup
 */
static void
moko_message_dialog_realize_cb (GtkWidget *widget, gpointer user_data)
{
  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE (widget);

  GtkRcStyle *rc_style;
  GError *error = NULL;

  if (!priv->background_pixbuf)
  {
    rc_style = widget->style->rc_style;

    priv->background_pixbuf = gdk_pixbuf_new_from_file (rc_style->bg_pixmap_name[GTK_STATE_NORMAL], &error);

    if (priv->background_pixbuf == NULL)
    {
      g_warning ("Error loading background pixbuf: %s", error->message);
      g_clear_error (&error);
      return;
    }
  }
}

static gboolean
moko_message_dialog_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  MokoMessageDialogPrivate *priv = MESSAGE_DIALOG_PRIVATE (widget);
  int x, y, w, h;
  
  gdk_drawable_get_size (widget->window, &w, &h);

  if (!priv->pixbuf)
  {
    gdk_window_get_origin (widget->window, &x, &y);

    /* 
     * Extract a sub pixbuf for the area of the root window which the window
     * now covers
     */
    priv->pixbuf = gdk_pixbuf_new_subpixbuf (priv->root_pixbuf, x, y, w, h);

    /* Composite the background pixbuf over said subpixbuf */
    gdk_pixbuf_composite (priv->background_pixbuf, priv->pixbuf, 
        0, 0, 
        w, h, 
        0, 0, 
        1, 1, 
        GDK_INTERP_BILINEAR, 
        255);  
  }

  /* Draw our pixbuf onto the window */
  gdk_draw_pixbuf (widget->window, NULL, priv->pixbuf, 
      0, 0, 
      0, 0, 
      w, h, 
      GDK_RGB_DITHER_NONE, 0, 0);

  return FALSE;
}

static void
moko_message_dialog_init (MokoMessageDialog *self)
{
  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE(self);
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *alignment;
  GtkWidget *image_alignment;

  priv->message = NULL;

  vbox = gtk_vbox_new (FALSE, 6);

  alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  image_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);

  gtk_container_add (GTK_CONTAINER (alignment), vbox);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG (self)->vbox), alignment, TRUE, TRUE, 6);

  gtk_button_box_set_layout (GTK_BUTTON_BOX(GTK_DIALOG (self)->action_area), GTK_BUTTONBOX_SPREAD);

  label = gtk_label_new (NULL);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_misc_set_padding (GTK_MISC (label), 12, 12);

  gtk_widget_set_size_request (GTK_WIDGET (label), 300, -1);

  gtk_box_pack_end (GTK_BOX (vbox), label, TRUE, FALSE, 6);

  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (self)->vbox));

  gtk_window_set_modal (GTK_WINDOW (self), TRUE);

  gtk_box_pack_start (GTK_BOX (vbox), image_alignment, TRUE, TRUE, 6);

  gtk_dialog_set_has_separator (GTK_DIALOG (self), FALSE);

  gtk_widget_set_size_request (GTK_WIDGET (self), 480, 560);
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (self), FALSE);


  /* Indicate that we want to paint this ourselves */
  gtk_widget_set_app_paintable (self, TRUE);

  g_signal_connect (self, "realize", G_CALLBACK (moko_message_dialog_realize_cb), NULL);
  g_signal_connect (self, "expose-event", G_CALLBACK (moko_message_dialog_expose_event_cb), NULL);

  priv->image_alignment = image_alignment;
  priv->label = label;

  priv->root_pixbuf = NULL;
  priv->background_pixbuf = NULL;
  priv->pixbuf = NULL;
}

void
moko_message_dialog_set_message (MokoMessageDialog *dialog, const gchar *message_format, ...)
{
  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE(dialog);
  va_list args;

  g_free (priv->message);

  va_start (args, message_format);
  priv->message = g_strdup_vprintf (message_format, args);
  va_end (args);

  gtk_label_set_text (GTK_LABEL (priv->label), priv->message);
}

void
moko_message_dialog_set_image_from_stock (MokoMessageDialog *dialog, const gchar *stock_id)
{
  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE(dialog);
  GtkWidget *old_image = priv->image;

  priv->image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_DIALOG);

  if (old_image != NULL)
    gtk_container_remove (GTK_CONTAINER (priv->image_alignment), old_image);

  gtk_container_add (GTK_CONTAINER (priv->image_alignment), priv->image);
  gtk_widget_show_all (GTK_WIDGET (priv->image_alignment));

  if (old_image !=NULL)
    g_object_unref (old_image);
}


void
moko_message_dialog_set_image (MokoMessageDialog *dialog, GtkWidget *image)
{
  MokoMessageDialogPrivate* priv = MESSAGE_DIALOG_PRIVATE(dialog);
  GtkWidget *old_image = priv->image;

  priv->image = image;

  if (old_image != NULL)
    gtk_container_remove (GTK_CONTAINER (priv->image_alignment), old_image);

  gtk_container_add (GTK_CONTAINER (priv->image_alignment), priv->image);
  gtk_widget_show_all (GTK_WIDGET (priv->image_alignment));

  if (old_image !=NULL)
    g_object_unref (old_image);
}



GtkWidget*
moko_message_dialog_new (void)
{
  return g_object_new (MOKO_TYPE_MESSAGE_DIALOG, NULL);
}
