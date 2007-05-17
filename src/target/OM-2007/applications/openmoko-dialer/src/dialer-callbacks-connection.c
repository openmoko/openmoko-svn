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

#include "dialer-callbacks-connection.h"
#include "dialer-window-incoming.h"


void
network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell)
{
  /* network registration */
}

void
incoming_call_cb (MokoGsmdConnection *self, int type, MokoDialerData *data)
{
  /* incoming call */
  window_incoming_show (data);
}

void
incoming_clip_cb (MokoGsmdConnection *self, const char *number, MokoDialerData *data)
{
  /* caller id */
  window_incoming_update_message (data, number);
}

void
incoming_pin_request_cb (MokoGsmdConnection *self, int type, MokoDialerData *data)
{
    g_debug( "INCOMING PIN REQUEST!\n!\n!\n!\n" );
}

gboolean initial_timeout_cb (MokoGsmdConnection *conn)
{
    g_debug( "INITIAL TIMEOUT" );
    //moko_gsmd_connection_network_register( conn );
    return FALSE;
}
