/*  openmoko-callbacks-connection.h
 *
 *  Authored By Tony Guan <tonyguan@fic-sh.com.cn>
 *  Thomas Wood <thomas@o-hand.com>
 *  Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 */

#ifndef _DIALER_CALLBACKS_CONNECTION_H
#define _DIALER_CALLBACKS_CONNECTION_H

#include <libmokogsmd/moko-gsmd-connection.h>
#include <dialer-main.h>

void network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell);
void incoming_call_cb (MokoGsmdConnection *self, int type, MokoDialerData *data);
void incoming_clip_cb (MokoGsmdConnection *self, const char *number, MokoDialerData *data);
void incoming_pin_request_cb (MokoGsmdConnection *self, int type, MokoDialerData *data);

gboolean initial_timeout_cb (MokoGsmdConnection *conn);

#endif
