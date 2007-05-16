/*  openmoko-dialer-window-outgoing.h
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
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
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */

#ifndef _OPENMOKO_DIALER_WINDOW_INCOMING_H
#define _OPENMOKO_DIALER_WINDOW_INCOMING_H

#include "dialer-main.h"

void window_incoming_init (MokoDialerData * p_dialer_data);
void window_incoming_prepare (MokoDialerData * appdata);
void window_incoming_show (MokoDialerData *data);
void window_incoming_update_message (MokoDialerData *data, const gchar *clip);


#endif
