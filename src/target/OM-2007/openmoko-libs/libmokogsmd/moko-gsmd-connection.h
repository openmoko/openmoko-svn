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

/* signals */
void moko_gsmd_connection_incoming_call(MokoGsmdConnection* self, int type);
void moko_gsmd_connection_signal_strength_changed(MokoGsmdConnection* self, int strength);

G_END_DECLS

#endif // _MOKO_GSMD_CONNECTION_H_

