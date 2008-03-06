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

#include <glib.h>
#include <glib-object.h>
#include "moko-network.h"

#ifndef _MOKO_PB_H_
#define _MOKO_PB_H_

G_BEGIN_DECLS

#define MOKO_TYPE_PB		(moko_pb_get_type ())
#define MOKO_PB(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_PB, MokoPb))
#define MOKO_PB_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_PB, MokoPbClass))
#define MOKO_IS_PB(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_PB))
#define MOKO_IS_PB_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_PB))
#define MOKO_PB_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_PB, MokoPbClass))

typedef struct _MokoPb MokoPb;
typedef struct _MokoPbClass MokoPbClass;

struct _MokoPb
{
  GObject parent_object;
};

struct _MokoPbClass
{
  GObjectClass parent_class;

  void (*entry) (MokoPb *pb, int index, const gchar *number, const gchar *text);
};

typedef enum _MokoPbStatus MokoPbStatus;

enum _MokoPbStatus {
  MOKO_PB_STATUS_READY,
  MOKO_PB_STATUS_BUSY,
};

GType moko_pb_get_type (void) G_GNUC_CONST;

MokoPb *
moko_pb_get_default (MokoNetwork *network);

MokoPbStatus
moko_pb_get_status (MokoPb *pb);

guint
moko_pb_get_timeout (MokoPb *pb);

void
moko_pb_get_range (MokoPb *pb, int *first, int *last);

void
moko_pb_get_entries (MokoPb *pb, const gchar *storage);

G_END_DECLS

#endif /* _MOKO_PB_H_ */
