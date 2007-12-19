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

#include <pulse/pulseaudio.h>

G_DEFINE_TYPE (MokoNotify, moko_notify, G_TYPE_OBJECT)

#define MOKO_NOTIFY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_NOTIFY, MokoNotifyPrivate))

#define DEFAULT_RINGTONE "/default_ringtone.ogg"
#define SYS_BRIGHTNESS "/sys/class/backlight/gta01-bl"
#define SYS_VIBRATE "/sys/class/leds/gta01:vibrator"

struct _MokoNotifyPrivate
{
  int    started;

  /* Sound stuff */
  pa_context *pac;
  pa_operation *operation;
};
/*
enum
{
  NOTHING,

  LAST_SIGNAL
};

static guint notify_signals[LAST_SIGNAL] = {0, };
*/
static void moko_notify_start_ringtone (MokoNotify *notify);

/*
 * Check the current screen brightness, raise it if necessary
 */
static void
moko_notify_check_brightness (void)
{
  gint fd;
  gchar buf[50];
  GIOChannel *dev;
  gsize bytes = 0;
  GError *err = NULL;

  fd = g_open (SYS_BRIGHTNESS"/brightness", O_RDWR, 0);
  if (fd == -1)
  {
    g_warning ("Unable to open brightness device");
    return;
  }

  /* The reading is for a 'smooth' brightness from current to max */
  dev = g_io_channel_unix_new (fd);
  if (g_io_channel_read_chars (dev, buf, 50, &bytes, &err)
        == G_IO_STATUS_NORMAL)
  {
    buf[bytes] = '\0';
    /*g_debug ("Current brightness = %s", buf); */
  }
  else
  {
    g_warning (err->message);
    g_error_free (err);
  }
  err = NULL;
  if (g_io_channel_write_chars (dev, "5000", -1, &bytes, &err)
        != G_IO_STATUS_NORMAL)
  {
    g_warning (err->message);
    g_error_free (err);
  }
  g_io_channel_shutdown (dev, TRUE, NULL);
  close (fd);
}

static gboolean
play (MokoNotify *notify)
{
  moko_notify_start_ringtone (notify);
  g_debug ("1500 timeout");
  return FALSE;
}

static gboolean
play_timeout (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_val_if_fail (MOKO_IS_NOTIFY (notify), FALSE);
  priv = notify->priv;

  if (!priv->pac)
    return FALSE;

  if (!priv->operation)
  {
    g_debug ("No operation");
    return FALSE;
  }
  if (!priv->started)
  {
    pa_operation_cancel (priv->operation);
    g_debug ("Cancelling early");
    return FALSE;
  }
  if (pa_operation_get_state (priv->operation) == PA_OPERATION_DONE)
  {
    g_timeout_add (1500, (GSourceFunc)play, (gpointer)notify);
    g_debug ("Playing done");
    return FALSE;
  }
  g_debug ("Not finshed yet");
  return TRUE;
}

static void
moko_notify_start_ringtone (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;

  if (!priv->pac || !priv->started)
    return;

  priv->operation = pa_context_play_sample (priv->pac,
                                            "ringtone",
                                            NULL,
                                            PA_VOLUME_NORM,
                                            NULL,
                                            NULL);
  g_timeout_add (500, (GSourceFunc)play_timeout, (gpointer)notify);
  g_debug ("Playing");
}

static void
moko_notify_stop_ringtone (MokoNotify *notify)
{
  MokoNotifyPrivate *priv;

  g_return_if_fail (MOKO_IS_NOTIFY (notify));
  priv = notify->priv;

  if (priv->operation)
  {
    pa_operation_cancel (priv->operation);
    g_debug ("Cancelling");
  }

  priv->operation = NULL;
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

  priv->started ++;
  if (priv->started != 1)
    return;

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
  priv->started --;

  if (!priv->started) {
    moko_notify_stop_vibrate ();
    moko_notify_stop_ringtone (notify);
  }
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
  pa_threaded_mainloop *mainloop = NULL;
  pa_mainloop_api *mapi = NULL;

  priv = notify->priv = MOKO_NOTIFY_GET_PRIVATE (notify);

  priv->started = 0;
  priv->pac = NULL;

  /* Start up pulse audio */
  mainloop = pa_threaded_mainloop_new ();
  if (!mainloop)
  {
    g_warning ("Unable to create PulseAudio mainloop.");
    return;
  }
  mapi = pa_threaded_mainloop_get_api (mainloop);
  priv->pac = pa_context_new (mapi, "Openmoko Dialer");
  if (!priv->pac)
  {
    g_warning ("Could create the PulseAudio context");
    return;
  }
  pa_context_connect (priv->pac, NULL, 0, NULL);
  pa_threaded_mainloop_start (mainloop);
}

MokoNotify*
moko_notify_new (void)
{
  MokoNotify *notify = NULL;

  notify = g_object_new (MOKO_TYPE_NOTIFY, NULL);

  return notify;
}

MokoNotify*
moko_notify_get_default (void)
{
  static MokoNotify *notify = NULL;
  
  if (!notify) notify = moko_notify_new ();
  
  return notify;
}

