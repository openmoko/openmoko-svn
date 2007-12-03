/*
 *  moko-dialer; a GObject wrapper for the dialer which exports method and
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

#ifndef _HAVE_MOKO_DIALER_H
#define _HAVE_MOKO_DIALER_H

#include <glib.h>
#include <glib-object.h>
#include "moko-network.h"

G_BEGIN_DECLS

#define MOKO_TYPE_DIALER (moko_dialer_get_type ())

#define MOKO_DIALER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_DIALER, MokoDialer))

#define MOKO_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_DIALER, MokoDialerClass))

#define MOKO_IS_DIALER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_DIALER))

#define MOKO_IS_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_DIALER))

#define MOKO_DIALER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_DIALER, MokoDialerClass))

#define PHONE_KIT_DIALER_ERROR g_quark_from_static_string("phone-kit-dialer")

typedef struct _MokoDialer MokoDialer;
typedef struct _MokoDialerClass MokoDialerClass;
typedef struct _MokoDialerPrivate MokoDialerPrivate;

typedef enum {
  PK_DIALER_ERROR_BUSY,
  PK_DIALER_ERROR_INVALID_NUMBER,
} PhoneKitDialerError;

typedef enum {
  PK_DIALER_NORMAL,
  PK_DIALER_INCOMING,
  PK_DIALER_DIALING,
  PK_DIALER_TALKING,
} PhoneKitDialerStatus;

struct _MokoDialer
{
  GObject         parent;

  /*< private >*/
  MokoDialerPrivate   *priv;
};

struct _MokoDialerClass 
{
  /*< private >*/
  GObjectClass    parent_class;
  
  /* signals */
  void (*status_changed) (MokoDialer *dialer, PhoneKitDialerStatus status);

    /* Initiating a connection */
  void (*incoming_call) (MokoDialer *dialer, const gchar *number);
  void (*outgoing_call) (MokoDialer *dialer, const gchar *number);
  
    /* Connected, either user accepted the call, or the outgoing call was
     * successful
     */
  void (*talking)       (MokoDialer *dialer);

    /* Finished a call */
  void (*hung_up)       (MokoDialer *dialer);
  void (*rejected)      (MokoDialer *dialer);  
    
  /* future padding */
  void (*_moko_dialer_1) (void);
  void (*_moko_dialer_2) (void);
  void (*_moko_dialer_3) (void);
  void (*_moko_dialer_4) (void);
}; 

GType moko_dialer_get_type (void) G_GNUC_CONST;

MokoDialer*        
moko_dialer_get_default (MokoNetwork *network);

gboolean
moko_dialer_show_dialer (MokoDialer *dialer, GError **error);

gboolean
moko_dialer_show_missed_calls (MokoDialer *dialer, GError **error);

PhoneKitDialerStatus
moko_dialer_get_status (MokoDialer *dialer);

/* Dialer interface */

gboolean
moko_dialer_dial (MokoDialer *dialer, const gchar *number, GError **error);

void
moko_dialer_outgoing_call (MokoDialer *dialer, const gchar *number);

void
moko_dialer_talking (MokoDialer *dialer);

void
moko_dialer_hung_up (MokoDialer *dialer);

void
moko_dialer_rejected (MokoDialer *dialer);

G_END_DECLS

#endif /* _HAVE_MOKO_DIALER_H */
