/*
 *  moko-talking; a GObject wrapper for the talking/incoming/outgoing page.
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

#include <moko-journal.h>

#include "moko-sound.h"
#include "moko-talking.h"

G_DEFINE_TYPE (MokoTalking, moko_talking, GTK_TYPE_VBOX)

#define MOKO_TALKING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_TALKING, MokoTalkingPrivate))

#define N_PICS 5

struct _MokoTalkingPrivate
{
  MokoJournal        *journal;

  GtkWidget          *incoming_bar;
  GtkWidget          *main_bar;

  GtkWidget          *title;
  GtkWidget          *icon;

  GtkWidget          *person;
  GtkWidget          *status;

  GdkPixbuf          *talking[N_PICS];
  GdkPixbuf          *incoming[4];
  GdkPixbuf          *outgoing[4];

  guint               timeout;
};

enum
{
  ACCEPT_CALL = 0,
  REJECT_CALL,
  CANCEL_CALL,
  SILENCE,
  SPEAKER_TOGGLE,

  LAST_SIGNAL
};

static guint talking_signals[LAST_SIGNAL] = {0, };


void
moko_talking_set_clip (MokoTalking      *talking, 
                       const gchar      *number,
                       MokoContactEntry *entry)
{
  MokoTalkingPrivate *priv;
  gchar *markup;
  
  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  if (number == NULL)
    number = "Unknown number";

  if (entry)
    markup = g_strdup_printf ("<b>%s</b>\n%s", entry->contact->name, number);
  else
    markup = g_strdup (number);

  gtk_label_set_markup (GTK_LABEL (priv->status), markup);
  
  if (entry && GDK_IS_PIXBUF (entry->contact->photo))
    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->person), entry->contact->photo);
  else
    gtk_image_set_from_file (GTK_IMAGE (priv->person),
                             PKGDATADIR"/unkown.png");

  g_free (markup);
}

static gboolean
incoming_timeout (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  static gint i = 0;

  g_return_val_if_fail (MOKO_IS_TALKING (talking), FALSE);
  priv = talking->priv;

  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->icon), 
                             priv->incoming[i]);
  
  i++;
  if (i == 4)
    i = 0;
  
  return TRUE;
}

void
moko_talking_incoming_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry)
{
  MokoTalkingPrivate *priv;

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  gtk_widget_hide (priv->main_bar);
  gtk_widget_show (priv->incoming_bar);

  gtk_label_set_text (GTK_LABEL (priv->title), "Incoming Call");
  gtk_image_set_from_file (GTK_IMAGE (priv->icon), 
                           PKGDATADIR"/incoming_3.png");

  gtk_label_set_text (GTK_LABEL (priv->status), number);
  gtk_image_set_from_file (GTK_IMAGE (priv->person),
                           PKGDATADIR"/unkown.png");
  g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)incoming_timeout,
                                 (gpointer)talking);
}

static gboolean
outgoing_timeout (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  static gint i = 0;

  g_return_val_if_fail (MOKO_IS_TALKING (talking), FALSE);
  priv = talking->priv;

  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->icon), 
                             priv->outgoing[i]);
  
  i++;
  if (i == 4)
    i = 0;
  
  return TRUE;
}

void
moko_talking_outgoing_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry)
{
  MokoTalkingPrivate *priv;
  gchar *markup = NULL;

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  gtk_widget_hide (priv->incoming_bar);
  gtk_widget_show (priv->main_bar);

  moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);

  if (entry)
    markup = g_strdup_printf ("<b>%s</b>\n%s", entry->contact->name, number);
  else
    markup = g_strdup (number);

  gtk_label_set_text (GTK_LABEL (priv->title), "Outgoing Call");

  gtk_label_set_markup (GTK_LABEL (priv->status), markup);
  
  if (entry && GDK_IS_PIXBUF (entry->contact->photo))
    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->person), entry->contact->photo);
  else
    gtk_image_set_from_file (GTK_IMAGE (priv->person),
                             PKGDATADIR"/unkown.png");

  g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)outgoing_timeout,
                                 (gpointer)talking);

  g_free (markup);
}

static gboolean
talking_timeout (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  static gint i = 0;

  g_return_val_if_fail (MOKO_IS_TALKING (talking), FALSE);
  priv = talking->priv;

  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->icon), 
                             priv->talking[i]);
  
  i++;
  if (i == 5)
    i = 0;
  
  return TRUE;
}
  
void
moko_talking_accepted_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry)
{
  MokoTalkingPrivate *priv;
  gchar *markup = NULL;

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  gtk_widget_hide (priv->incoming_bar);
  gtk_widget_show (priv->main_bar);

  moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);

  if (entry)
    markup = g_strdup_printf ("<b>%s</b>\n%s", entry->contact->name, number);
  else
    markup = g_strdup (number);

  gtk_label_set_text (GTK_LABEL (priv->title), "Talking");
  gtk_image_set_from_file (GTK_IMAGE (priv->icon), 
                           PKGDATADIR"/talking_3.png");

  /* We don't change the status or person widgets, as incoming call has already
   * set them for us.
   */
  g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)talking_timeout,
                                 (gpointer)talking);

  g_free (markup);
}


/* Toolbar callbacks */
static void
on_answer_clicked (GtkToolButton *button, MokoTalking *talking)
{
  g_signal_emit (G_OBJECT (talking), talking_signals[ACCEPT_CALL], 0);
}

static void
on_reject_clicked (GtkToolButton *button, MokoTalking *talking)
{
  g_source_remove (talking->priv->timeout);
  g_signal_emit (G_OBJECT (talking), talking_signals[REJECT_CALL], 0);
}

static void
on_silence_clicked (GtkToolButton *button, MokoTalking *talking)
{
  g_source_remove (talking->priv->timeout);
  g_signal_emit (G_OBJECT (talking), talking_signals[SILENCE], 0);
}

static void
on_cancel_clicked (GtkToolButton *button, MokoTalking *talking)
{
  g_source_remove (talking->priv->timeout);
  moko_sound_profile_set(SOUND_PROFILE_STEREO_OUT);
  g_signal_emit (G_OBJECT (talking), talking_signals[CANCEL_CALL], 0);
}

static void
on_speaker_toggled (GtkToggleToolButton *toggle, MokoTalking *talking)
{
  g_signal_emit (G_OBJECT (talking), talking_signals[SPEAKER_TOGGLE],
                0, gtk_toggle_tool_button_get_active (toggle));
}

/* GObject functions */
static void
moko_talking_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_talking_parent_class)->dispose (object);
}

static void
moko_talking_finalize (GObject *talking)
{
  G_OBJECT_CLASS (moko_talking_parent_class)->finalize (talking);
}

static void
moko_talking_class_init (MokoTalkingClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_talking_finalize;
  obj_class->dispose = moko_talking_dispose;


   talking_signals[ACCEPT_CALL] =
    g_signal_new ("accept_call", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  accept_call),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

   talking_signals[REJECT_CALL] =
    g_signal_new ("reject_call", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  reject_call),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

   talking_signals[CANCEL_CALL] =
    g_signal_new ("cancel_call", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  cancel_call),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
   talking_signals[SILENCE] =
    g_signal_new ("silence", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  silence),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

   talking_signals[SPEAKER_TOGGLE] =
    g_signal_new ("speaker_toggle", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  speaker_toggle),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  g_type_class_add_private (obj_class, sizeof (MokoTalkingPrivate)); 
}

static void
moko_talking_init (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  GtkWidget *toolbar, *image, *vbox, *hbox, *label, *align, *frame;
  GtkToolItem *item;
  GdkPixbuf *icon;
  gint i;

  priv = talking->priv = MOKO_TALKING_GET_PRIVATE (talking);
  
  priv->incoming_bar = toolbar = gtk_toolbar_new ();
  gtk_widget_set_no_show_all (priv->incoming_bar, TRUE);
  gtk_box_pack_start (GTK_BOX (talking), toolbar, FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/answer.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "Answer");
  gtk_widget_show_all (GTK_WIDGET (item));
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_answer_clicked), (gpointer)talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);
  
  image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);
  item = gtk_tool_button_new (image, "Reject");
  gtk_widget_show_all (GTK_WIDGET (item)); 
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_silence_clicked), (gpointer)talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 2);


  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 3);
  
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/cancel.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "Reject");
  gtk_widget_show_all (GTK_WIDGET (item)); 
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_reject_clicked), (gpointer)talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 4);
    
  /* Outgoing call and talking share the same toolbar */
  priv->main_bar = toolbar = gtk_toolbar_new ();
  gtk_widget_set_no_show_all (priv->main_bar, TRUE);
  gtk_box_pack_start (GTK_BOX (talking), toolbar, FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/speaker.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_toggle_tool_button_new ();
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (item), image);
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), "Speaker Phone");
  gtk_widget_show_all (GTK_WIDGET (item)); 
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "toggled", 
                    G_CALLBACK (on_speaker_toggled), (gpointer)talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/cancel.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "Cancel");
  gtk_widget_show_all (GTK_WIDGET (item)); 
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_cancel_clicked), (gpointer)talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 2);  

  /* The title label and image */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (talking), vbox, FALSE, FALSE, 0);
  
  priv->title = label = gtk_label_new ("Incoming Call");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 8);

  priv->icon = image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (align), image);

  /* The status area */
  align = gtk_alignment_new (0.5, 0.5, 1, 0 );
  gtk_box_pack_start (GTK_BOX (talking), align, TRUE, TRUE, 0);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add (GTK_CONTAINER (align), frame);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  priv->person = image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  priv->status = label = gtk_label_new ("01923 820 124");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  /* Load the pixbufs */
  for (i = 0; i < N_PICS; i++)
  {
    if (i == 0)
      priv->talking[i] = gdk_pixbuf_new_from_file (PKGDATADIR"/talking.png",
                                                   NULL);
    else
    {
      gchar *name = g_strdup_printf ("%s/talking_%d.png", PKGDATADIR, i-1);
      priv->talking[i] = gdk_pixbuf_new_from_file (name, NULL);
      g_free (name);
    }
    if (G_IS_OBJECT (priv->talking[i]))
      g_object_ref (priv->talking[i]);
  }
  for (i = 0; i < N_PICS-1; i++)
  {
    gchar *name = g_strdup_printf ("%s/outgoing_%d.png", PKGDATADIR, i);
    priv->outgoing[i] = gdk_pixbuf_new_from_file (name, NULL);
    g_free (name);
    if (G_IS_OBJECT (priv->outgoing[i]))
      g_object_ref (priv->outgoing[i]);
  }
  for (i = 0; i < N_PICS-1; i++)
  {
    gchar *name = g_strdup_printf ("%s/incoming_%d.png", PKGDATADIR, i);
    priv->incoming[i] = gdk_pixbuf_new_from_file (name, NULL);
    g_free (name);
    if (G_IS_OBJECT (priv->incoming[i]))
      g_object_ref (priv->incoming[i]);

  }

}

GtkWidget*
moko_talking_new (MokoJournal *journal)
{
  MokoTalking *talking = NULL;
    
  talking = g_object_new (MOKO_TYPE_TALKING, NULL);

  talking->priv->journal = journal;

  return GTK_WIDGET (talking);
}
