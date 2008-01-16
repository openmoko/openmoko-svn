/*
 *  moko-tips; The autocomplete tips
 *  
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <gtk/gtk.h>
#include <moko-stock.h>

#include "moko-tips.h"

#include <gtk/gtk.h>
#include "moko-contacts.h"

G_DEFINE_TYPE (MokoTips, moko_tips, GTK_TYPE_HBOX)

#define MOKO_TIPS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_TIPS, MokoTipsPrivate))

#define N_TIPS 3

struct _MokoTipsPrivate
{
  GtkWidget     *image;
  GtkWidget     *tips[N_TIPS];
};

enum
{
  SELECTED = 1,

  LAST_SIGNAL
};

static guint tips_signals[LAST_SIGNAL] = {0, };

void
moko_tips_set_matches (MokoTips *tips, GList *list)
{
  MokoTipsPrivate *priv;
  gint i;

  g_return_if_fail (MOKO_IS_TIPS (tips));
  priv = tips->priv;

  for (i = 0; i < N_TIPS; i++)
  {
    MokoContactEntry *entry = NULL;
    GtkWidget *label = gtk_bin_get_child (GTK_BIN (priv->tips[i]));

    entry = (MokoContactEntry*)g_list_nth_data (list, i);

    if (entry && entry->contact)
    {
      gtk_label_set_text (GTK_LABEL (label), entry->contact->name);
      if (i == 0)
      {
        GdkPixbuf *scaled = NULL;

        if (!entry->contact->photo)
          moko_contacts_get_photo (moko_contacts_get_default (), 
                                   entry->contact);
        
        if (!entry->contact->photo)
        {
          gtk_image_clear (GTK_IMAGE (priv->image));
        } else {
          scaled = gdk_pixbuf_scale_simple (entry->contact->photo,
                                            36, 36,
                                            GDK_INTERP_BILINEAR);
          
          gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), scaled);
          g_object_unref (scaled);
        }
      }
      gtk_widget_show (label);
    }
    else
    {
      gtk_widget_hide (label);
      gtk_label_set_text (GTK_LABEL (label), "");
      if (i == 0)
      {
        gtk_image_clear (GTK_IMAGE (priv->image));
        gtk_widget_show (label);
      }

    }
    g_object_set_data (G_OBJECT (priv->tips[i]), "entry", entry);
  }
}

/* Callbacks */
static gboolean
on_tip_selected (GtkWidget *eb, GdkEventButton *event, MokoTips *tips)
{
  MokoTipsPrivate *priv;
  MokoContactEntry *entry;

  g_return_val_if_fail (MOKO_IS_TIPS (tips), FALSE);
  priv = tips->priv;
  
  entry = (MokoContactEntry*)g_object_get_data (G_OBJECT (eb), "entry");

  if (entry && entry->contact)
   g_signal_emit ((gpointer) tips, tips_signals[SELECTED], 0, entry);

  return FALSE;
}

/* GObject functions */
static void
moko_tips_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_tips_parent_class)->dispose (object);
}

static void
moko_tips_finalize (GObject *tips)
{
  G_OBJECT_CLASS (moko_tips_parent_class)->finalize (tips);
}


static void
moko_tips_class_init (MokoTipsClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_tips_finalize;
  obj_class->dispose = moko_tips_dispose;

  tips_signals[SELECTED] =
    g_signal_new ("selected", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTipsClass, selected),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);
  g_type_class_add_private (obj_class, sizeof (MokoTipsPrivate)); 
}

static void
moko_tips_init (MokoTips *tips)
{
  MokoTipsPrivate *priv;
  GtkWidget *hbox;
  gint i = 0;
  
  priv = tips->priv = MOKO_TIPS_GET_PRIVATE (tips);

  priv->image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (tips), priv->image, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (tips), hbox, TRUE, TRUE, 0);

  for (i = 0; i < N_TIPS; i++)
  {
    GtkWidget *eb, *label;
    priv->tips[i] = eb = gtk_event_box_new ();
    g_signal_connect (eb, "button-release-event",
                      G_CALLBACK (on_tip_selected), (gpointer)tips);
    gtk_box_pack_start (GTK_BOX (hbox), eb, TRUE, TRUE, 0);   

    label = gtk_label_new ("");
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    //gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_container_add (GTK_CONTAINER (eb), label);
  }
}

GtkWidget*
moko_tips_new (void)
{
  MokoTips *tips = NULL;
  
  tips = g_object_new (MOKO_TYPE_TIPS, 
                         "homogeneous", FALSE,
                         "spacing", 0,
                         NULL);

  return GTK_WIDGET (tips);
}
