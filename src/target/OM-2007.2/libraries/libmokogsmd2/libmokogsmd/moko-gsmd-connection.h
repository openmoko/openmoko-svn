/*  moko-gsmd-connection.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 */

#ifndef _MOKO_GSMD_CONNECTION_H_
#define _MOKO_GSMD_CONNECTION_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_GSMD_CONNECTION moko_gsmd_connection_get_type()
#define MOKO_GSMD_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_GSMD_CONNECTION, MokoGsmdConnection))
#define MOKO_GSMD_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_GSMD_CONNECTION, MokoGsmdConnectionClass))
#define MOKO_IS_GSMD_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_GSMD_CONNECTION))
#define MOKO_IS_GSMD_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_GSMD_CONNECTION))
#define MOKO_GSMD_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_GSMD_CONNECTION, MokoGsmdConnectionClass))

typedef struct _MokoGsmdConnection MokoGsmdConnection;
typedef struct _MokoGsmdConnectionClass MokoGsmdConnectionClass;

struct _MokoGsmdConnection {
    GObject parent;
};

struct _MokoGsmdConnectionClass {
    GObjectClass parent_class;

    /* Call signals */
    void (*incoming_call) (MokoGsmdConnection* self, int type);
    void (*call_status_progress) (MokoGsmdConnection* self, int type);
    void (*pin_requested) (MokoGsmdConnection* self, int type);
    void (*incoming_clip) (MokoGsmdConnection* self, const gchar* number);

    /* SMS signals */

    /* GPRS signals */

    /* Network signals */
    void (*network_registration) (MokoGsmdConnection* self, int type, int lac, int cell);
    void (*signal_strength_changed) (MokoGsmdConnection* self, int strength);
    void (*network_current_operator) (MokoGsmdConnection* self, const gchar* name );
    void (*cipher_status_changed) (MokoGsmdConnection* self, int status );

    /* Misc */
    void (*gsmd_connection_status) (MokoGsmdConnection* self, gboolean status);
    void (*gsmd_antenna_status) (MokoGsmdConnection* self,gboolean status);
    void (*cme_cms_error) (MokoGsmdConnection *self, int code);

    /* Future padding */
    void (*_moko_gsmdconn_1) (void);
    void (*_moko_gsmdconn_2) (void);
    void (*_moko_gsmdconn_3) (void);
    void (*_moko_gsmdconn_4) (void);

};

typedef enum 
{
    MOKO_GSMD_CONNECTION_NETREG_NONE = 0,
    MOKO_GSMD_CONNECTION_NETREG_HOME = 1,
    MOKO_GSMD_CONNECTION_NETREG_SEARCHING = 2,
    MOKO_GSMD_CONNECTION_NETREG_DENIED = 3,
    MOKO_GSMD_CONNECTION_NETREG_UNKNOWN = 4,
    MOKO_GSMD_CONNECTION_NETREG_ROAMING = 5

} MokoGsmdConnectionNetregType;


typedef enum 
{
  MOKO_GSMD_PROG_SETUP = 0,
  MOKO_GSMD_PROG_DISCONNECT = 1,
  MOKO_GSMD_PROG_ALERT = 2,
  MOKO_GSMD_PROG_CALL_PROCEED = 3,
  MOKO_GSMD_PROG_SYNC = 4,
  MOKO_GSMD_PROG_PROGRESS = 5,
  MOKO_GSMD_PROG_CONNECTED = 6,
  MOKO_GSMD_PROG_RELEASE = 7,
  MOKO_GSMD_PROG_REJECT = 8,
  MOKO_GSMD_PROG_UNKNOWN = 9

} MokoGsmdConnectionProgress;

typedef enum
{
  MOKO_GSMD_ERROR_CONNECT, /* could not connect to gsmd */
  MOKO_GSMD_ERROR_POWER /* attenna power did not set correctly */
} MokoGsmdConnectionError;

GType moko_gsmd_connection_get_type ();

MokoGsmdConnection* moko_gsmd_connection_new ();

/* power */
void moko_gsmd_connection_set_antenna_power (MokoGsmdConnection *self, 
                                             gboolean on, GError **error);
/* network */
void moko_gsmd_connection_network_register (MokoGsmdConnection *self);
int moko_gsmd_connection_get_network_status (MokoGsmdConnection *self);
void moko_gsmd_connection_trigger_current_operator_event(MokoGsmdConnection* self);
void moko_gsmd_connection_trigger_signal_strength_event(MokoGsmdConnection* self);

G_END_DECLS

#endif /* _MOKO_GSMD_CONNECTION_H_ */

