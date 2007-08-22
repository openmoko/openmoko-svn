/*
 *  moko-notify; a Notification object. This deals with notifying the user
 *  of an incoming call.
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

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "moko-notify.h"

#include <gst/gst.h>

G_DEFINE_TYPE (MokoNotify, moko_notify, G_TYPE_OBJECT)

#define MOKO_NOTIFY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_NOTIFY, MokoNotifyPrivate))

#define DEFAULT_RINGTONE "/default_ringtone.ogg"
#define SYS_BRIGHTNESS "/sys/class/backlight/gta01-bl"
#define SYS_VIBRATE "/sys/class/leds/gta01:vibrator"

struct _MokoNotifyPrivate
{
  gboolean    started;

  GstElement *bin;
};
/*
enum
{
  NOTHING,

  LAST_SIGNAL
};

static guint notify_signals[LAST_SIGNAL] = {0, };
*/


/*
 * Check the current screen brightness, raise it if necessary 
 */
static void
moko_notify_check_brightness (void)
{
  gint fd;
  gchar buf[50];
  gint brightness, len;

  fd = g_open (SYS_BRIGHTNESS"/brightness", O_WRONLY, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open brightness device");
    return;
  }

  
  if (read (fd, buf, sizeof (buf)) == -1)
  {
    close (fd);
    g_warning ("Unable to read from brightness device");
    return;
  }
  brightness = atoi (buf);
  g_print ("Brightness = %d\n", brightness);
  /*
  brightness = 0;
  if (brightness >= 5000)
  {
    close (fd);
    return;
  }
  */
  len = g_sprintf (buf, "%d", 5000);
  write (fd, buf, len);
  close (fd);
}

static void
on_bus_eos (GstBus *bus, GstMessage *message, MokoNotify *notify)
{
  MokoNotifyPrivate *priv;
  
  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  g_return_if_fail (GST_IS_ELEMENT (notify->priv->bin));
  priv = notify->priv;

  g_print (".");
  /* Rewind and play again */
  gst_element_set_state (priv->bin, GST_STATE_PAUSED);

  /* Seek to 0 */
  if (!gst_element_seek_simple (priv->bin, GST_FORMAT_TIME, 0, 0))
    g_error ("Seek error\n");
  return ;

  gst_element_set_state (priv->bin, GST_STATE_PLAYING);
 
  g_print ("Audio finished\n");
 }

static void
moko_notify_start_ringtone (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;
  GstElement *bin;
  gchar *pipeline;
  GError *err = NULL;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;
  
  /* Create the pipeline */
  pipeline = g_strdup_printf (
    "filesrc location=%s ! decodebin ! audioconvert !audioresample ! alsasink",
    PKGDATADIR DEFAULT_RINGTONE);
  g_print ("%s\n", PKGDATADIR DEFAULT_RINGTONE);
  
  /* Try and gstreamer to parse it */
  bin = gst_parse_launch (pipeline, &err);
  if (err)
  {
    g_error ("err->message");
    priv->bin = NULL;
    g_free (pipeline);
    return;
  }
  priv->bin = bin;

  g_signal_connect (gst_element_get_bus (bin), "message::eos",
                    G_CALLBACK (on_bus_eos), (gpointer)notify);

  /* Connect to eos signal to repeat the ringtone 
  gst_bus_add_watch (gst_element_get_bus (bin), 
                     (GstBusFunc)on_bus_message, (gpointer)notify);
  */
  /* Start playing */
  gst_element_set_state (bin, GST_STATE_PLAYING);

  g_free (pipeline);
}

static void
moko_notify_stop_ringtone (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;

  if  (GST_IS_ELEMENT (priv->bin))
  {
    gst_element_set_state (priv->bin, GST_STATE_NULL);
  }
}

static void
moko_notify_start_vibrate (void)
{
  gint fd;
  gchar buf[100];
  gint len;

  /* Set the trigger state */
  fd = g_open (SYS_VIBRATE"/trigger", O_WRONLY, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open vibration device");
    return;
  }
  len = g_sprintf (buf, "%s", "timer");
  write (fd, buf, len);
  close (fd);

  /* Set the 'on' delay */
  fd = g_open (SYS_VIBRATE"/delay_on", O_WRONLY, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open timer 'delay_on'");
    return;
  }
  len = g_sprintf (buf, "%d", 500);
  write (fd, buf, len);
  close (fd);

  /* Set the 'off' delay, this also starts the vibration */
  fd = g_open (SYS_VIBRATE"/delay_off", O_WRONLY, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open timer 'delay_off'");
    return;
  }
  len = g_sprintf (buf, "%d", 1000);
  write (fd, buf, len);
  close (fd);
}

static void
moko_notify_stop_vibrate (void)
{
  gint fd;
  gchar buf[100];
  gint len;

  /* Set the trigger state to none*/
  fd = g_open (SYS_VIBRATE"/trigger", O_WRONLY, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open vibration device");
    return;
  }
  len = g_sprintf (buf, "%s", "none");
  write (fd, buf, len);
  close (fd);
}

/* We need to do a few things here:
 *  1. If the backlight is dimmed down, undim it
 *  2. Start playing the ringtone
 *  3. Start the vibration alert
 */
void
moko_notify_start (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;

  if (priv->started)
    return;
  priv->started = TRUE;

  moko_notify_check_brightness ();
  moko_notify_start_vibrate ();
  moko_notify_start_ringtone (notify);
}

/* Stop the ringtone and the vibration alert */
void
moko_notify_stop (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;

  if (!priv->started)
    return;
  priv->started = FALSE;
 
  moko_notify_stop_vibrate ();
  moko_notify_stop_ringtone (notify);
}

/* GObject functions */
static void
moko_notify_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_notify_parent_class)->dispose (object);
}

static void
moko_notify_finalize (GObject *notify)
{
  G_OBJECT_CLASS (moko_notify_parent_class)->finalize (notify);
}

static void
moko_notify_class_init (MokoNotifyClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_notify_finalize;
  obj_class->dispose = moko_notify_dispose;

  g_type_class_add_private (obj_class, sizeof (MokoNotifyPrivate)); 
}

static void
moko_notify_init (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;
  
  priv = notify->priv = MOKO_NOTIFY_GET_PRIVATE (notify);

  priv->started = FALSE;
}

MokoNotify*
moko_notify_new (void)
{
  MokoNotify *notify = NULL;
    
  notify = g_object_new (MOKO_TYPE_NOTIFY, NULL);

  return notify;
}
