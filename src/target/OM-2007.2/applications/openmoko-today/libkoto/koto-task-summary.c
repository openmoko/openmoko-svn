/*
 * Copyright (C) 2007 Ross Burton <ross@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>
#include <glib/gi18n.h>
#include <gtk/gtktextview.h>
#include "koto-task.h"
#include "ical-util.h"
#include "koto-task-summary.h"

G_DEFINE_TYPE (KotoTaskSummary, koto_task_summary, GTK_TYPE_SCROLLED_WINDOW);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_TASK_SUMMARY, KotoTaskSummaryPrivate))

enum {
  PROP_0,
  PROP_TASK,
};

typedef struct {
  KotoTask *task;

  GtkTextBuffer *buffer;
  
  GtkTextTag *summary_tag;
  GtkTextTag *due_tag;
  GtkTextTag *desc_tag;
  GtkTextTag *url_tag;
} KotoTaskSummaryPrivate;


static void
update_buffer (KotoTaskSummary *summary)
{
  KotoTaskSummaryPrivate *priv;

  g_assert (KOTO_IS_TASK_SUMMARY (summary));

  priv = GET_PRIVATE (summary);

  gtk_text_buffer_set_text (priv->buffer, "", -1);

  if (priv->task) {
    icalcomponent *comp = priv->task->comp;
    GtkTextIter end;
    const char *value;

    gtk_text_buffer_get_end_iter (priv->buffer, &end);

    /* Summary */
    value = ical_util_get_summary (comp);
    if (value) {
      gtk_text_buffer_insert_with_tags (priv->buffer, &end, value, -1, priv->summary_tag, NULL);
      gtk_text_buffer_insert (priv->buffer, &end, "\n", -1);
    }

    /* TODO: category */

    /* Due date */
    {
      icaltimetype time;
      GDate date;
      char *due, *s;
      time = icalcomponent_get_due (comp);
      if (!icaltime_is_null_time (time)) {
        g_date_clear (&date, 1);
        /* TODO: don't use this, but use _with_zone */
        g_date_set_time_t (&date, icaltime_as_timet (time));
        due = ical_util_get_human_date (&date);
        s = g_strdup_printf ("Due %s", due);
        gtk_text_buffer_insert_with_tags (priv->buffer, &end, s, -1, priv->due_tag, NULL);
        gtk_text_buffer_insert (priv->buffer, &end, "\n", -1);
        g_free (s);
        g_free (due);
      }
    }

    /* Description */
    value = icalcomponent_get_description (comp);
    if (value) {
      gtk_text_buffer_insert_with_tags (priv->buffer, &end, value, -1, priv->desc_tag, NULL);
      gtk_text_buffer_insert (priv->buffer, &end, "\n", -1);
    }

    /* URL */
    value = ical_util_get_url (comp);
    if (value)
      gtk_text_buffer_insert_with_tags (priv->buffer, &end, value, -1, priv->url_tag, NULL);
  }
}

/*
 * Event callback on the URL tag, which activates the URL that was clicked.
 */
static gboolean
on_url_event (GtkTextTag *texttag, GtkTextView *textview, GdkEvent *event, GtkTextIter *iter, gpointer user_data)
{
  KotoTaskSummaryPrivate *priv;
  GtkTextIter start, end;
  char *url;
  
  g_assert (KOTO_IS_TASK_SUMMARY (user_data));
  priv = GET_PRIVATE (user_data);

  /* TODO: handle in/out and change the cursor */

  if (event->type == GDK_BUTTON_RELEASE) {    
    start = end = *iter;
    
    if (gtk_text_iter_forward_to_tag_toggle (&end, priv->url_tag) &&
        gtk_text_iter_backward_to_tag_toggle (&start, priv->url_tag)) {
      url = gtk_text_buffer_get_slice (gtk_text_iter_get_buffer (iter), &start, &end, FALSE);
      
      /* Probably best if there is some sort of defined interface that shells have
         to implement, including open_url. */
      g_debug (G_STRLOC ": TODO: open URL %s", url);
      g_free (url);
    }
  }

  return FALSE;
}

static void
koto_task_summary_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoTaskSummaryPrivate *priv = GET_PRIVATE (object);
  
  switch (property_id) {
  case PROP_TASK:
    g_value_set_boxed (value, priv->task);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_summary_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  KotoTaskSummaryPrivate *priv = GET_PRIVATE (object);
  
  switch (property_id) {
  case PROP_TASK:
    priv->task = g_value_dup_boxed (value);
    update_buffer (KOTO_TASK_SUMMARY (object));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_summary_dispose (GObject *object)
{
  KotoTaskSummaryPrivate *priv = GET_PRIVATE (object);
  
  if (priv->task) {
    koto_task_unref (priv->task);
    priv->task = NULL;
  }

  if (G_OBJECT_CLASS (koto_task_summary_parent_class)->dispose)
    G_OBJECT_CLASS (koto_task_summary_parent_class)->dispose (object);
}

static void
koto_task_summary_finalize (GObject *object)
{
  G_OBJECT_CLASS (koto_task_summary_parent_class)->finalize (object);
}

static void
koto_task_summary_class_init (KotoTaskSummaryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoTaskSummaryPrivate));

  object_class->get_property = koto_task_summary_get_property;
  object_class->set_property = koto_task_summary_set_property;
  object_class->dispose = koto_task_summary_dispose;
  object_class->finalize = koto_task_summary_finalize;

  g_object_class_install_property (object_class, PROP_TASK,
                                   g_param_spec_boxed ("task", "task", NULL,
                                                       KOTO_TYPE_TASK,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
}

static void
koto_task_summary_init (KotoTaskSummary *self)
{
  KotoTaskSummaryPrivate *priv = GET_PRIVATE (self);
  GtkWidget *textview;

  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (self), NULL);
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (self), NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  textview = g_object_new (GTK_TYPE_TEXT_VIEW,
                                 "wrap-mode", GTK_WRAP_WORD,
                                 "editable", FALSE,
                                 "cursor-visible", FALSE,
                                 "left-margin", 8,
                                 "right-margin", 8,
                                 NULL);
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (self), textview);

  priv->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

  priv->summary_tag = gtk_text_buffer_create_tag (priv->buffer, "summary",
                                                  "scale", PANGO_SCALE_LARGE,
                                                  "weight", PANGO_WEIGHT_BOLD,
                                                  "pixels-above-lines", 6,
                                                  "pixels-below-lines", 6,
                                                  NULL);

  priv->due_tag = gtk_text_buffer_create_tag (priv->buffer, "due",
                                              "weight", PANGO_WEIGHT_BOLD,
                                              "pixels-below-lines", 6,
                                              NULL);

  priv->desc_tag = gtk_text_buffer_create_tag (priv->buffer, "description",
                                               "left-margin", 16,
                                               "right-margin", 16,
                                               NULL);

  priv->url_tag = gtk_text_buffer_create_tag (priv->buffer, "url",
                                              /* TODO: set from theme */
                                              "foreground", "blue",
                                              "underline", TRUE,
                                              "pixels-above-lines", 6,
                                              NULL);

  g_signal_connect (priv->url_tag, "event", G_CALLBACK (on_url_event), self);
}

GtkWidget *
koto_task_summary_new (void)
{
  return g_object_new (KOTO_TYPE_TASK_SUMMARY, NULL);
}
