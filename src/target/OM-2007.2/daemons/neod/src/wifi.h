/*
 *  Authored by Rob Bradford <rob@o-hand.com>
 *  Copyright (C) 2008 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 */

#ifndef __WIFI_H_
#define __WIFI_H_

#include <glib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/wireless.h>

gboolean wifi_radio_is_on (const gchar *iface);
gboolean wifi_radio_control (const gchar *iface, gboolean enable);
#endif /* __WIFI_H_ */

