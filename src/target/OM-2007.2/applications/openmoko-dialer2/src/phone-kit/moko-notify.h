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

#ifndef _HAVE_MOKO_NOTIFY_H
#define _HAVE_MOKO_NOTIFY_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_NOTIFY (moko_notify_get_type ())

#define MOKO_NOTIFY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_NOTIFY, MokoNotify))

#define MOKO_NOTIFY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_NOTIFY, MokoNotifyClass))

#define MOKO_IS_NOTIFY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_NOTIFY))

#define MOKO_IS_NOTIFY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_NOTIFY))

#define MOKO_NOTIFY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_NOTIFY, MokoNotifyClass))

typedef struct _MokoNotify MokoNotify;
typedef struct _MokoNotifyClass MokoNotifyClass;
typedef struct _MokoNotifyPrivate MokoNotifyPrivate;

struct _MokoNotify
{
  GObject              parent;

  /*< private >*/
  MokoNotifyPrivate   *priv;
};

struct _MokoNotifyClass 
{
  /*< private >*/
  GObjectClass    parent_class;
}; 

GType moko_notify_get_type (void) G_GNUC_CONST;

MokoNotify*        
moko_notify_new (void);

MokoNotify*
moko_notify_get_default (void);

void
moko_notify_start (MokoNotify *notify);

void
moko_notify_stop (MokoNotify *notify);

G_END_DECLS

#endif /* _HAVE_MOKO_NOTIFY_H */
