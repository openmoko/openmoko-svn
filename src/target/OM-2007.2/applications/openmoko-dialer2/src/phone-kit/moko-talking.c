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

#include "moko-sound.h"
#include "moko-talking.h"
#include "moko-dialer-panel.h"
#include "moko-dialer-textview.h"
#include "moko-alsa-volume-control.h"
#include "moko-alsa-volume-scale.h"

#include "moko-headset.h"

G_DEFINE_TYPE (MokoTalking, moko_talking, GTK_TYPE_WIDGET)

#define MOKO_TALKING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_TALKING, MokoTalkingPrivate))

#define N_PICS 5

enum
{
  CALL_DIRECTION_INCOMING,
  CALL_DIRECTION_OUTGOING
};

struct _MokoTalkingPrivate
{
  GtkWidget *window;
  GtkWidget *notebook;

  GtkWidget *incoming_bar;
  GtkWidget *main_bar;

  GtkWidget *title;
  GtkWidget *duration;
  GtkWidget *icon;

  GtkWidget *person;
  GtkWidget *status;
  GtkWidget *volume;
  
  GtkToolItem *speaker_toggle_btn;
  
  GtkWidget *dtmf_display;
  GtkWidget *dtmf_pad;

  GdkPixbuf *talking[N_PICS];
  GdkPixbuf *incoming[4];
  GdkPixbuf *outgoing[4];

  GTimer *dtimer;
  guint timeout;
  
  gint call_direction;
  
  MokoAlsaVolumeControl *headphone;
};

enum
{
  ACCEPT_CALL = 0,
  REJECT_CALL,
  CANCEL_CALL,
  SILENCE,
  SPEAKER_TOGGLE,
  DTMF_KEY_PRESS,

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
                             PKGDATADIR"/unknown.png");

  g_free (markup);
}

static void
moko_talking_reset_ui (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  priv = MOKO_TALKING_GET_PRIVATE (talking);

  gtk_toggle_tool_button_set_active (
                      GTK_TOGGLE_TOOL_BUTTON (priv->speaker_toggle_btn), FALSE);
  moko_dialer_textview_empty (MOKO_DIALER_TEXTVIEW (priv->dtmf_display));
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
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
  
  moko_talking_reset_ui (talking);

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  gtk_widget_hide (priv->main_bar);
  gtk_widget_show_all (priv->incoming_bar);

  gtk_window_set_title (GTK_WINDOW (priv->window), "Incoming Call");
  gtk_label_set_text (GTK_LABEL (priv->title), "Incoming Call");
  gtk_label_set_text (GTK_LABEL (priv->duration), "");
  gtk_image_set_from_file (GTK_IMAGE (priv->icon), 
                           PKGDATADIR"/incoming_3.png");

  gtk_label_set_text (GTK_LABEL (priv->status), number);
  gtk_image_set_from_file (GTK_IMAGE (priv->person),
                           PKGDATADIR"/unknown.png");
  if (priv->timeout)
    g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)incoming_timeout,
                                 (gpointer)talking);
  priv->call_direction = CALL_DIRECTION_INCOMING;
  gtk_window_present (GTK_WINDOW (priv->window));
  gtk_window_deiconify (GTK_WINDOW (priv->window));
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

  moko_talking_reset_ui (talking);
    
  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;
  
  gtk_widget_hide (priv->incoming_bar);
  gtk_widget_show_all (priv->main_bar);

  if ( HEADSET_STATUS_IN == moko_headset_status_get() ) 
    moko_sound_profile_set(SOUND_PROFILE_GSM_HEADSET);
  else
    moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);

  if (entry)
    markup = g_strdup_printf ("<b>%s</b>\n%s", entry->contact->name, number);
  else
    markup = g_strdup (number);

  gtk_window_set_title (GTK_WINDOW (priv->window), "Dialing");
  gtk_label_set_text (GTK_LABEL (priv->title), "Outgoing Call");
  gtk_label_set_text (GTK_LABEL (priv->duration), "");

  gtk_label_set_markup (GTK_LABEL (priv->status), markup);
  
  if (entry && GDK_IS_PIXBUF (entry->contact->photo))
    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->person), entry->contact->photo);
  else
    gtk_image_set_from_file (GTK_IMAGE (priv->person),
                             PKGDATADIR"/unknown.png");
  if (priv->timeout)
    g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)outgoing_timeout,
                                 (gpointer)talking);

  g_free (markup);
  priv->call_direction = CALL_DIRECTION_OUTGOING;
  
  gtk_window_present (GTK_WINDOW (priv->window));
  gtk_window_deiconify (GTK_WINDOW (priv->window));
}

static gboolean
talking_timeout (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  gdouble elapsed;
  gint hour, min, sec;
  gchar *markup = NULL;
  static gint i = 0;

  g_return_val_if_fail (MOKO_IS_TALKING (talking), FALSE);
  priv = talking->priv;

  if (priv->dtimer)
  {
    elapsed = g_timer_elapsed(priv->dtimer, NULL);

    hour = (gint) (elapsed / 3600);
    min = (gint) ((elapsed - 3600 * hour) / 60);
    sec = (gint) (elapsed - 3600 * hour - 60 * min);

    markup = g_strdup_printf ("%02d:%02d:%02d", hour, min, sec);
    gtk_label_set_markup (GTK_LABEL (priv->duration), markup);
  }

  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->icon), 
                             priv->talking[i]);
  
  i++;
  if (i == 5)
    i = 0;
  
  g_free(markup);
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
  gtk_widget_show_all (priv->main_bar);

  if ( HEADSET_STATUS_IN == moko_headset_status_get() ) 
    moko_sound_profile_set(SOUND_PROFILE_GSM_HEADSET);
  else
    moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);

  if (entry)
    markup = g_strdup_printf ("<b>%s</b>\n%s", entry->contact->name, number);
  else
    markup = g_strdup (number);

  gtk_window_set_title (GTK_WINDOW (priv->window), "Talking");
  gtk_label_set_text (GTK_LABEL (priv->title), "Talking");
  gtk_label_set_text (GTK_LABEL (priv->duration), "00:00:00");
  gtk_image_set_from_file (GTK_IMAGE (priv->icon), 
                           PKGDATADIR"/talking_3.png");

  /* start call duration timer */
  if (priv->dtimer)
    g_timer_destroy(priv->dtimer);
  priv->dtimer = g_timer_new();

  /* We don't change the status or person widgets, as incoming call has already
   * set them for us.
   */
  if (priv->timeout)
    g_source_remove (priv->timeout);
  priv->timeout = g_timeout_add (1000, 
                                 (GSourceFunc)talking_timeout,
                                 (gpointer)talking);

  g_free (markup);
}

void
moko_talking_hide_window (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  if ( HEADSET_STATUS_IN == moko_headset_status_get() ) 
    moko_sound_profile_set(SOUND_PROFILE_HEADSET);
  else 
    moko_sound_profile_set(SOUND_PROFILE_STEREO_OUT);

  if (priv->dtimer)
    g_timer_destroy(priv->dtimer);
  priv->dtimer = NULL;

  if (priv->timeout)
    g_source_remove (priv->timeout);

  gtk_widget_hide (priv->window);
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
  MokoTalkingPrivate *priv;
  g_warning("on_reject_clicked");
  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  if (priv->timeout)
    g_source_remove (priv->timeout);

  gtk_widget_hide (priv->window);
  g_signal_emit (G_OBJECT (talking), talking_signals[REJECT_CALL], 0);
}

static void
on_silence_clicked (GtkToolButton *button, MokoTalking *talking)
{
  g_signal_emit (G_OBJECT (talking), talking_signals[SILENCE], 0);
}

static void
on_cancel_clicked (GtkToolButton *button, MokoTalking *talking)
{
  MokoTalkingPrivate *priv;

  g_return_if_fail (MOKO_IS_TALKING (talking));
  priv = talking->priv;

  g_warning("on_cancel_clicked");
  /* stop call duration timer */
  if (priv->dtimer)
    g_timer_destroy(priv->dtimer);
  priv->dtimer = NULL;
  
  if (priv->timeout)
    g_source_remove (priv->timeout);

  if ( HEADSET_STATUS_IN == moko_headset_status_get() ) 
    moko_sound_profile_set(SOUND_PROFILE_HEADSET);
  else
    moko_sound_profile_set(SOUND_PROFILE_STEREO_OUT);
  gtk_widget_hide (priv->window);
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

   talking_signals[DTMF_KEY_PRESS] =
    g_signal_new ("dtmf_key_press", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoTalkingClass,  dtmf_key_press),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__CHAR,
                  G_TYPE_NONE, 1, G_TYPE_CHAR);

  g_type_class_add_private (obj_class, sizeof (MokoTalkingPrivate)); 
}

static gboolean
window_delete_event_cb (GtkWidget *widget, GdkEvent  *event, MokoTalking *talking)
{
  MokoTalkingPrivate *priv = MOKO_TALKING_GET_PRIVATE (talking);
  g_warning("window_delete_event_cb");
  if (priv->call_direction == CALL_DIRECTION_INCOMING)
	on_reject_clicked (NULL, talking);
  else
	on_cancel_clicked (NULL, talking);
  
  return TRUE;
}

static void
on_pad_user_input (MokoDialerPanel *panel, const gchar digit,
                   MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  gchar buf[2];
  priv = MOKO_TALKING_GET_PRIVATE (talking);

  /* Create a string from the new digit */
  buf[0] = digit;
  buf[1] = '\0';
  
  moko_dialer_textview_insert (MOKO_DIALER_TEXTVIEW (priv->dtmf_display), buf);

  g_signal_emit (G_OBJECT (talking), talking_signals[DTMF_KEY_PRESS],
                0, digit);
}

static void
moko_talking_init (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  GtkWidget *notebook;
  GtkWidget *toolbar, *image, *vbox, *hbox, *label, *align, *frame, *main_vbox;
  GtkWidget *duration;
  GtkToolItem *item;
  gint i;

  priv = talking->priv = MOKO_TALKING_GET_PRIVATE (talking);

  /* initialize dtimer to NULL */
  priv->dtimer = NULL;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
  priv->notebook = notebook;

  /* status page */
  main_vbox = gtk_vbox_new (FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), main_vbox,
                            gtk_image_new_from_file (PKGDATADIR"/phone.png"));
  gtk_container_child_set (GTK_CONTAINER (notebook), main_vbox, "tab-expand",
                           TRUE, NULL);
  
  priv->incoming_bar = toolbar = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (main_vbox), toolbar, FALSE, FALSE, 0);

  item = gtk_tool_button_new (gtk_image_new_from_file (PKGDATADIR"/moko-call-answer.png"), NULL);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "clicked", G_CALLBACK (on_answer_clicked), talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

  item = gtk_tool_button_new (gtk_image_new_from_file (PKGDATADIR"/moko-call-ignore.png"), NULL);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "clicked", G_CALLBACK (on_silence_clicked), talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

  item = gtk_tool_button_new (gtk_image_new_from_file (PKGDATADIR"/moko-call-hangup.png"), NULL);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "clicked", G_CALLBACK (on_reject_clicked), talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  
  /* Volume controls */
  priv->headphone = moko_alsa_volume_control_new ();
  moko_alsa_volume_control_set_device_from_name (priv->headphone, "neo1973");
  moko_alsa_volume_control_set_element_from_name (priv->headphone, "Headphone");
  
  priv->volume = moko_alsa_volume_scale_new (GTK_ORIENTATION_HORIZONTAL);
  moko_alsa_volume_scale_set_control (MOKO_ALSA_VOLUME_SCALE (priv->volume),
                                      priv->headphone);
  
  /* Outgoing call and talking share the same toolbar */
  priv->main_bar = toolbar = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (main_vbox), toolbar, FALSE, FALSE, 0);

  item = gtk_toggle_tool_button_new ();
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (item),
      gtk_image_new_from_file (PKGDATADIR"/speaker.png"));

  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "toggled", G_CALLBACK (on_speaker_toggled), talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  priv->speaker_toggle_btn = item;

  item = gtk_tool_button_new (gtk_image_new_from_file (PKGDATADIR"/moko-call-hangup.png"), NULL);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "clicked", G_CALLBACK (on_cancel_clicked), talking);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

  /* The title label and image */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
  
  priv->title = label = gtk_label_new ("Incoming Call");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 8);

  priv->icon = image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (align), image);

  priv->duration = duration = gtk_label_new ("00:00:00");
  gtk_misc_set_alignment (GTK_MISC (duration), 0.5, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), duration, FALSE, FALSE, 0);

  /* The status area */
  align = gtk_alignment_new (0.5, 0.5, 1, 0 );
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add (GTK_CONTAINER (align), frame);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  priv->person = image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  priv->status = label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  
  /* The volume control */
  gtk_box_pack_start (GTK_BOX (vbox), priv->volume, FALSE, TRUE, 12);

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
  
  /* dtmf page */
  GtkWidget *pad, *display;
  main_vbox = gtk_vbox_new (FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), main_vbox,
                            gtk_image_new_from_file (PKGDATADIR"/dtmf.png"));
  gtk_container_child_set (GTK_CONTAINER (notebook), main_vbox, "tab-expand",
                           TRUE, NULL);
  
  display = moko_dialer_textview_new ();
  gtk_box_pack_start_defaults (GTK_BOX (main_vbox), display);
  priv->dtmf_display = display;
  
  pad = moko_dialer_panel_new ();
  gtk_box_pack_start_defaults (GTK_BOX (main_vbox), pad);
  g_signal_connect (pad, "user_input", G_CALLBACK (on_pad_user_input), talking);
  priv->dtmf_pad = pad;

  priv->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (priv->window, "delete-event", G_CALLBACK (window_delete_event_cb), talking);
  gtk_container_add (GTK_CONTAINER (priv->window), notebook);
  
  gtk_widget_show_all (notebook);

}

GtkWidget*
moko_talking_new ()
{
  MokoTalking *talking = NULL;
    
  talking = g_object_new (MOKO_TYPE_TALKING, NULL);

  return GTK_WIDGET (talking);
}
