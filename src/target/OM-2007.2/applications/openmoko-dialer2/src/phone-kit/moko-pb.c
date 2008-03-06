/*
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Chia-I Wu <olv@openmoko.com>
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

#include "moko-listener.h"
#include "moko-pb.h"
#include "moko-marshal.h"

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/phonebook.h>

typedef struct _MokoPbPrivate MokoPbPrivate;

enum {
  ENTRY,
  LAST_SIGNAL
};

enum {
  PROP_STATUS = 1,
  PROP_NETWORK,
  PROP_TIMEOUT,
};

struct _MokoPbPrivate
{
  MokoNetwork *network;
  gint timeout;

  MokoPbStatus status;
  gint pb_first, pb_last;
  guint pb_timeout_id;
};

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_PB, MokoPbPrivate))

static guint pb_signals[LAST_SIGNAL] = { 0 };

static void moko_pb_iface_init (MokoListenerInterface *iface, gpointer iface_data);

G_DEFINE_TYPE_EXTENDED (MokoPb, moko_pb, G_TYPE_OBJECT, 0,
    G_IMPLEMENT_INTERFACE (MOKO_TYPE_LISTENER, moko_pb_iface_init));

static void
moko_pb_dispose (GObject *obj)
{
  MokoPbPrivate *priv = GET_PRIVATE (obj);

  if (priv->network) {
    moko_network_remove_listener (priv->network, MOKO_LISTENER (obj));
    g_object_unref (priv->network);

    priv->network = NULL;
  }

  G_OBJECT_CLASS (moko_pb_parent_class)->dispose (obj);
}

static void
moko_pb_finalize (GObject *obj)
{
  G_OBJECT_CLASS (moko_pb_parent_class)->finalize (obj);
}

static void
moko_pb_get_property (GObject *object, guint property_id,
                      GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_STATUS:
      g_value_set_int (value, moko_pb_get_status (MOKO_PB (object)));
      break;
    case PROP_TIMEOUT:
      g_value_set_int (value, moko_pb_get_timeout (MOKO_PB (object)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_pb_set_property (GObject *object, guint property_id,
                      const GValue *value, GParamSpec *pspec)
{
  MokoPbPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_NETWORK:
      if (priv->network) {
        moko_network_remove_listener (priv->network, MOKO_LISTENER (object));
        g_object_unref (priv->network);
      }
      priv->network = g_value_dup_object (value);
      moko_network_add_listener (priv->network, MOKO_LISTENER (object));
      break;
    case PROP_TIMEOUT:
      priv->timeout = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_pb_class_init (MokoPbClass *klass)
{
  GObjectClass* obj_class = G_OBJECT_CLASS (klass);

  obj_class->get_property = moko_pb_get_property;
  obj_class->set_property = moko_pb_set_property;
  obj_class->dispose = moko_pb_dispose;
  obj_class->finalize = moko_pb_finalize;

  g_object_class_install_property (obj_class,
                                   PROP_STATUS,
                                   g_param_spec_int (
                                   "status",
                                   "PB status",
                                   "The current PB status.",
                                   MOKO_PB_STATUS_READY,
                                   MOKO_PB_STATUS_BUSY,
                                   MOKO_PB_STATUS_READY,
                                   G_PARAM_READABLE));

  g_object_class_install_property (obj_class,
                                   PROP_NETWORK,
                                   g_param_spec_object (
                                   "network",
                                   "MokoNetwork *",
                                   "The parent MokoNetwork object.",
                                   MOKO_TYPE_NETWORK,
                                   G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (obj_class,
                                   PROP_TIMEOUT,
                                   g_param_spec_uint (
                                   "timeout",
                                   "Timeout",
                                   "Timeout for AT+CPBR.",
				   0, G_MAXUINT,
				   20,
                                   G_PARAM_WRITABLE |
				   G_PARAM_READABLE));

  pb_signals[ENTRY] =
    g_signal_new ("entry",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (MokoPbClass, entry),
		  NULL, NULL,
		  _moko_marshal_VOID__INT_STRING_STRING,
		  G_TYPE_NONE, 3,
		  G_TYPE_INT,
		  G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
		  G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

  g_type_class_add_private (klass, sizeof(MokoPbPrivate));
}

static void
moko_pb_init (MokoPb *pb)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  priv->pb_first = 0;
  priv->pb_last = 0;
  priv->pb_timeout_id = 0;
}

MokoPb *
moko_pb_get_default (MokoNetwork *network)
{
  static MokoPb *pb;
  MokoPbPrivate *priv;

  if (pb)
    return pb;

  pb = g_object_new (MOKO_TYPE_PB,
      "network", network,
      "timeout", 20,
      NULL);

  priv = GET_PRIVATE (pb);

  /* XXX */
  priv->pb_first = 1;
  priv->pb_last = 255;

  return pb;
}

MokoPbStatus
moko_pb_get_status (MokoPb *pb)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  return priv->status;
}

guint
moko_pb_get_timeout (MokoPb *pb)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  return priv->timeout;
}

void
moko_pb_get_range (MokoPb *pb, int *first, int *last)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  if (first)
    *first = priv->pb_first;

  if (last)
    *last = priv->pb_last;
}

static void
moko_pb_get_entries_fini (MokoPb *pb)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  if (priv->pb_timeout_id) {
    g_source_remove (priv->pb_timeout_id);
    priv->pb_timeout_id = 0;
  }

  if (priv->status != MOKO_PB_STATUS_READY) {
    priv->status = MOKO_PB_STATUS_READY;
    g_object_notify (G_OBJECT (pb), "status");
  }
}

static gboolean
moko_pb_get_entries_timeout (gpointer data)
{
  g_warning ("Reading phonebook entries timeout");

  moko_pb_get_entries_fini (data);

  return FALSE;
}

void
moko_pb_get_entries (MokoPb *pb, const gchar *storage)
{
  MokoPbPrivate *priv = GET_PRIVATE (pb);
  struct lgsm_handle *handle;
  struct lgsm_phonebook_readrg prr;

  if (priv->status != MOKO_PB_STATUS_READY)
    return;

  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;

  priv->status = MOKO_PB_STATUS_BUSY;
  g_object_notify (G_OBJECT (pb), "status");

  moko_pb_get_range (pb, &prr.index1, &prr.index2);
  lgsm_pb_read_entries (handle, &prr);

  priv->pb_timeout_id = g_timeout_add_seconds (priv->timeout,
		  moko_pb_get_entries_timeout, pb);
}

static void
moko_pb_on_read_phonebook (MokoListener *listener,
                           struct lgsm_handle *handle,
                           struct gsmd_phonebooks *gps)
{
  MokoPb *pb = MOKO_PB (listener);
  MokoPbPrivate *priv = GET_PRIVATE (pb);

  if (priv->status != MOKO_PB_STATUS_BUSY) {
    g_warning ("strayed phonebook entry");

    return;
  }

  if (gps->pb.index < priv->pb_first ||
      gps->pb.index > priv->pb_last) {
    g_warning ("index %d is out of range", gps->pb.index);

    return;
  }

  if (!g_utf8_validate (gps->pb.numb, -1, NULL)) {
    g_warning ("entry %d has invalid number\n", gps->pb.index);

    return;
  }

  if (!g_utf8_validate (gps->pb.text, -1, NULL)) {
    g_warning ("entry %d has invalid text\n", gps->pb.index);

    return;
  }

  g_signal_emit (MOKO_PB (pb), pb_signals[ENTRY], 0,
      gps->pb.index, gps->pb.numb, gps->pb.text);

  /* XXX is_last is not reliable */
  /*
  if (gps->is_last)
    moko_pb_get_entries_fini (pb);
    */
}

static void
moko_pb_iface_init (MokoListenerInterface *iface, gpointer iface_data)
{
  iface->on_read_phonebook = moko_pb_on_read_phonebook;
}
