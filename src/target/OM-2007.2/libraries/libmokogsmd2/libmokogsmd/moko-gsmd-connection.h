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
 *  Current Version: $Rev$ ($Date: 2006/12/21 18:03:04 $) [$Author: mickey $]
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

    /* Voice signals */
    void (*incoming_call) (MokoGsmdConnection *self, int type);
    void (*call_status_progress) (MokoGsmdConnection *self, int type);
    void (*pin_requested) (MokoGsmdConnection *self, int type);

    /* SMS signals */

    /* GPRS signals */
    
    /* Misc signals */
    void (*incoming_clip) (MokoGsmdConnection *self, const gchar *number);
    void (*network_registration) (MokoGsmdConnection *self,
                                  int type,
                                  int lac,
                                  int cell);
    void (*trigger_signal_strength_event) (MokoGsmdConnection *self);
    void (*signal_strength_changed) (MokoGsmdConnection *self, int strength);

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
/* pin */
void moko_gsmd_connection_send_pin (MokoGsmdConnection *self, const gchar *pin);

/* network */
void moko_gsmd_connection_network_register (MokoGsmdConnection *self);
int moko_gsmd_connection_get_network_status (MokoGsmdConnection *self);

/* TODO add type, i.e. MOKO_GSMD_CONNECTION_NETREG_AUTO */
/* voice calls */
void moko_gsmd_connection_voice_accept (MokoGsmdConnection *self);
void moko_gsmd_connection_voice_hangup (MokoGsmdConnection *self);
void moko_gsmd_connection_voice_dial (MokoGsmdConnection *self, 
                                      const gchar *number);
void moko_gsmd_connection_voice_dtmf (MokoGsmdConnection *self, 
                                      const gchar number);
G_END_DECLS

#endif /* _MOKO_GSMD_CONNECTION_H_ */

