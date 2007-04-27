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

typedef struct {
    GObject parent;
} MokoGsmdConnection;

typedef struct {
    GObjectClass parent_class;
} MokoGsmdConnectionClass;

GType moko_gsmd_connection_get_type();
MokoGsmdConnection* moko_gsmd_connection_new();
void moko_gsmd_connection_network_register(MokoGsmdConnection* self); //TODO add type, i.e. MOKO_GSMD_CONNECTION_NETREG_AUTO
void moko_gsmd_connection_set_antenna_power(MokoGsmdConnection* self, gboolean on);

enum {
    MOKO_GSMD_CONNECTION_NETREG_NONE = 0,
    MOKO_GSMD_CONNECTION_NETREG_HOME = 1,
    MOKO_GSMD_CONNECTION_NETREG_SEARCHING = 2,
    MOKO_GSMD_CONNECTION_NETREG_DENIED = 3,
    MOKO_GSMD_CONNECTION_NETREG_ROAMING = 5,
} MokoGsmdConnectionNetregType;

/* signals */
void moko_gsmd_connection_incoming_call(MokoGsmdConnection* self, int type);
void moko_gsmd_connection_call_status_progress(MokoGsmdConnection* self, int type);
//sms
//gprs
void moko_gsmd_connection_incoming_clip(MokoGsmdConnection* self, const char* number);
void moko_gsmd_connection_network_registration(MokoGsmdConnection* self, int type, int lac, int cell);
void moko_gsmd_connection_signal_strength_changed(MokoGsmdConnection* self, int strength);
//voice
void moko_gsmd_connection_voice_accept(MokoGsmdConnection* self);
void moko_gsmd_connection_voice_hangup(MokoGsmdConnection* self);
void moko_gsmd_connection_voice_dial(MokoGsmdConnection* self, const gchar* number);

G_END_DECLS

#endif // _MOKO_GSMD_CONNECTION_H_

