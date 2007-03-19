/**
 *  @file dbus-conn.h
 *  @brief dbus connection and message send for openmoko mainmenu
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *  
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */
#ifndef MOKO_DBUS_MESSAGE_SEND_H
#define MOKO_DBUS_MESSAGE_SEND_H

#ifndef DBUS_API_SUBJECT_TO_CHANGE
#define DBUS_API_SUBJECT_TO_CHANGE
#endif /*DBUS_API_SUBJECT_TO_CHANGE*/
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <stdio.h>

#define DBUS_API_SUBJECT_TO_CHANGE

gboolean moko_dbus_connect_init (void);

gboolean moko_dbus_send_message (const char *str);

#endif /*MOKO_DBUS_MESSAGE_SEND_H*/
