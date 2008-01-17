/*
 *  moko-sms; a GObject wrapper for phone-kit that exports method and
 *  signals over dbus relating to SMS messaging
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

#ifndef _HAVE_MOKO_SMS_H
#define _HAVE_MOKO_SMS_H

#include <glib.h>
#include <glib-object.h>
#include "moko-listener.h"
#include "moko-network.h"

G_BEGIN_DECLS

#define MOKO_TYPE_SMS (moko_sms_get_type ())

#define MOKO_SMS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_SMS, MokoSms))

#define MOKO_SMS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_SMS, MokoSmsClass))

#define MOKO_IS_SMS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_SMS))

#define MOKO_IS_SMS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_SMS))

#define MOKO_SMS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_SMS, MokoSmsClass))

#define PHONE_KIT_SMS_ERROR g_quark_from_static_string("phone-kit-sms")

typedef struct _MokoSms MokoSms;
typedef struct _MokoSmsClass MokoSmsClass;
typedef struct _MokoSmsPrivate MokoSmsPrivate;

typedef enum {
  PK_SMS_ERROR_STORE_NOTOPEN,
  PK_SMS_ERROR_MSG_TOOLONG,
  PK_SMS_ERROR_INVALID_NUMBER,
} PhoneKitSmsError;

typedef enum {
  PK_SMS_NOTREADY,
  PK_SMS_READY,
} PhoneKitSmsStatus;

struct _MokoSms
{
  GObject         parent;

  /*< private >*/
  MokoSmsPrivate   *priv;
};

struct _MokoSmsClass 
{
  /*< private >*/
  GObjectClass    parent_class;
  
  /* signals */
  void (*status_changed) (MokoSms *sms, PhoneKitSmsStatus status);
  void (*memory_full)    (MokoSms *sms, gboolean sim, gboolean phone);
    
  /* future padding */
  void (*_moko_sms_1) (void);
  void (*_moko_sms_2) (void);
  void (*_moko_sms_3) (void);
  void (*_moko_sms_4) (void);
}; 

GType moko_sms_get_type (void) G_GNUC_CONST;

MokoSms*        
moko_sms_get_default (MokoNetwork *network);

PhoneKitSmsStatus
moko_sms_get_status (MokoSms *sms);

/* SMS interface */
gboolean
moko_sms_send (MokoSms *self, const gchar *number,
               const gchar *message, gboolean report, gchar **uid,
               GError **error);

gboolean
moko_sms_get_memory_status (MokoSms *self, gboolean *sim, gboolean *phone,
                            GError **error);

G_END_DECLS

#endif /* _HAVE_MOKO_SMS_H */

