/*
 *  moko-talking; a GObject wrapper for the talking/incoming/outgoing page
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

#include "moko-contacts.h"

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
  GtkVBox         parent;

  /*< private >*/
  MokoTalkingPrivate   *priv;
};

struct _MokoTalkingClass 
{
  /*< private >*/
  GtkVBoxClass    parent_class;

  /* Signals */
  void (*accept_call) (MokoTalking *talking);
  void (*reject_call) (MokoTalking *talking);
  void (*cancel_call) (MokoTalking *talking);
  void (*silence) (MokoTalking *talking);
  void (*speaker_toggle) (MokoTalking *talking, gboolean speaker_phone);
  void (*dtmf_key_press) (MokoTalking *talking, const char key);
}; 

GType moko_talking_get_type (void) G_GNUC_CONST;

GtkWidget* moko_talking_new ();

void
moko_talking_incoming_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry);

void
moko_talking_outgoing_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry);

void
moko_talking_accepted_call (MokoTalking      *talking, 
                            const gchar      *number,
                            MokoContactEntry *entry);

void
moko_talking_set_clip (MokoTalking      *talking, 
                       const gchar      *number,
                       MokoContactEntry *entry);

void 
moko_talking_hide_window (MokoTalking *talking);

G_END_DECLS

#endif /* _HAVE_MOKO_TALKING_H */
