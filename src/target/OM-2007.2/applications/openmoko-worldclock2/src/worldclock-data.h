/*
 *  openmoko-worldclock -- OpenMoko Clock Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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

#ifndef WORLDCLOCK_DATA_H
#define WORLDCLOCK_DATA_H

#include <glib.h>

typedef struct {
	gchar *name;
	gchar *tzname;
	gdouble lat;
	gdouble lon;
	gchar *country;
} WorldClockZoneData;

extern const WorldClockZoneData world_clock_tzdata[39];

#endif /* WORLDCLOCK_DATA_H */
