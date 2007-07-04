/*
 *  moko-talking; a GObject wrapper for the talking which exports method and
 *  signals over dbus
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

#ifndef _HAVE_MOKO_TALKING_H
#define _HAVE_MOKO_TALKING_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_TALKING (moko_talking_get_type ())

#define MOKO_TALKING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_TALKING, MokoTalking))

#define MOKO_TALKING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_TALKING, MokoTalkingClass))

#define MOKO_IS_TALKING(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_TALKING))

#define MOKO_IS_TALKING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_TALKING))

#define MOKO_TALKING_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_TALKING, MokoTalkingClass))

typedef struct _MokoTalking MokoTalking;
typedef struct _MokoTalkingClass MokoTalkingClass;
typedef struct _MokoTalkingPrivate MokoTalkingPrivate;

struct _MokoTalking
{
  GtkHBox         parent;

  /*< private >*/
  MokoTalkingPrivate   *priv;
};

struct _MokoTalkingClass 
{
  /*< private >*/
  GtkHBoxClass    parent_class;
}; 

GType moko_talking_get_type (void) G_GNUC_CONST;

GtkWidget*        
moko_talking_new (void);

G_END_DECLS

#endif /* _HAVE_MOKO_TALKING_H */
