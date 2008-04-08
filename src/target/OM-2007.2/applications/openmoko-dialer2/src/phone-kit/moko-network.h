/*
 *  moko-network; a GObject wrapper for phone-kit that exports method and
 *  signals over dbus relating to GSM network status and properties
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

#ifndef _HAVE_MOKO_NETWORK_H
#define _HAVE_MOKO_NETWORK_H

#include <glib.h>
#include <glib-object.h>
#include <libgsmd/libgsmd.h>
#include "moko-listener.h"

G_BEGIN_DECLS

#define MOKO_TYPE_NETWORK (moko_network_get_type ())

#define MOKO_NETWORK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_NETWORK, MokoNetwork))

#define MOKO_NETWORK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_NETWORK, MokoNetworkClass))

#define MOKO_IS_NETWORK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_NETWORK))

#define MOKO_IS_NETWORK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_NETWORK))

#define MOKO_NETWORK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_NETWORK, MokoNetworkClass))

#define PHONE_KIT_NETWORK_ERROR g_quark_from_static_string("phone-kit-network")

typedef struct _MokoNetwork MokoNetwork;
typedef struct _MokoNetworkClass MokoNetworkClass;
typedef struct _MokoNetworkPrivate MokoNetworkPrivate;

typedef enum {
  PK_NETWORK_ERROR_GSMD,
  PK_NETWORK_ERROR_NOT_CONNECTED,
  PK_NETWORK_ERROR_NO_PROVIDER_NAME,
  PK_NETWORK_ERROR_NO_PROVIDER_NUM,
  PK_NETWORK_ERROR_NO_IMSI,
  PK_NETWORK_ERROR_NO_SUBSCRIBER_NUM,
} PhoneKitNetworkError;

typedef enum {
  PK_NETWORK_POWERDOWN,
  PK_NETWORK_UNREGISTERED,
  PK_NETWORK_DENIED,
  PK_NETWORK_REGISTERED_HOME,
  PK_NETWORK_REGISTERED_ROAMING,
} PhoneKitNetworkStatus;

struct _MokoNetwork
{
  GObject         parent;

  /*< private >*/
  MokoNetworkPrivate   *priv;
};

struct _MokoNetworkClass 
{
  /*< private >*/
  GObjectClass    parent_class;
  
  /* signals */
  void (*status_changed) (MokoNetwork *network, PhoneKitNetworkStatus status);
  void (*subscriber_number_changed) (MokoNetwork *network, const gchar *number);
  void (*provider_changed) (MokoNetwork *network, const gchar *name);
    
  /* future padding */
  void (*_moko_network_1) (void);
  void (*_moko_network_2) (void);
  void (*_moko_network_3) (void);
  void (*_moko_network_4) (void);
}; 

GType moko_network_get_type (void) G_GNUC_CONST;

MokoNetwork*        
moko_network_get_default (void);

PhoneKitNetworkStatus
moko_network_get_status (MokoNetwork *network);

gboolean
moko_network_get_lgsm_handle (MokoNetwork *network, struct lgsm_handle **handle,
                              GError **error);

void
moko_network_add_listener (MokoNetwork *network, MokoListener *listener);

void
moko_network_remove_listener (MokoNetwork *network, MokoListener *listener);

/* Network interface */
gboolean
moko_network_get_provider_name (MokoNetwork *self, gchar **name, GError **error);

gboolean
moko_network_get_subscriber_number (MokoNetwork *self, gchar **number,
                                    GError **error);

gboolean
moko_network_get_country_code (MokoNetwork *self, gchar **dial_code,
                               GError **error);

gboolean
moko_network_get_home_country_code (MokoNetwork *self, gchar **dial_code,
                                    GError **error);

gboolean
moko_network_get_imsi (MokoNetwork *self, gchar **imsi, GError **error);

G_END_DECLS

#endif /* _HAVE_MOKO_NETWORK_H */

