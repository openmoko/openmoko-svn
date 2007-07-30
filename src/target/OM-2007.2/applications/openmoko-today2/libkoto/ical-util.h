/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _ICAL_UTIL_H
#define _ICAL_UTIL_H

#include <glib.h>
#include <libical/ical.h>

enum {
  PRIORITY_NONE = 0,
  PRIORITY_HIGH = 1,
  PRIORITY_MEDIUM = 5,
  PRIORITY_LOW = 9,
};

#ifndef ICAL_GET_FIELD
#define ICAL_GET_FIELD(LOWER_NAME, UPPER_NAME, TYPE, DEFAULT)   \
  TYPE ical_util_get_##LOWER_NAME (icalcomponent *comp);
#endif

ICAL_GET_FIELD(summary, SUMMARY, const char*, NULL);
ICAL_GET_FIELD(priority, PRIORITY, int, PRIORITY_NONE);
ICAL_GET_FIELD(url, URL, const char*, NULL);
ICAL_GET_FIELD(categories, CATEGORIES, const char*, NULL);

/* TODO: split this out */
char * ical_util_get_human_date (GDate *due);

#endif /* _ICAL_UTIL_H */
