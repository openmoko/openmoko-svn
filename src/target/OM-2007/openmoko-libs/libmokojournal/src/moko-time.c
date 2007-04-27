/* vi: set sw=2: */
/*
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <libical/icaltime.h>
#include <glib.h>
#include "moko-time.h"
#include "moko-time-priv.h"


MokoTime*
moko_time_new_today ()
{
    MokoTime *result ;
    result = g_new0 (MokoTime, 1) ;
    result->t = icaltime_today () ;
    return result ;
}

MokoTime*
moko_time_from_timet (const time_t a_t, gboolean a_is_date)
{
    MokoTime *result ;

    result = g_new0 (MokoTime, 1) ;
    result->t = icaltime_from_timet (a_t, a_is_date) ;
    return result ;
}

MokoTime*
moko_time_from_string (const gchar *a_iso_format_date)
{
    MokoTime *result ;

    g_return_val_if_fail (a_iso_format_date, NULL) ;

    result = g_new0 (MokoTime, 1) ;
    result->t = icaltime_from_string (a_iso_format_date) ;
    return result ;
}

MokoTime*
moko_time_new_from_icaltimetype (icaltimetype a_dt)
{
  MokoTime *result ;

  result = g_new0 (MokoTime, 1) ;
  result->t = a_dt ;
  return result ;
}

void
moko_time_free (MokoTime *a_time)
{
    g_return_if_fail (a_time) ;

    g_free (a_time) ;
}

const gchar*
moko_time_as_ical_string (MokoTime *a_t)
{
    g_return_val_if_fail (a_t, NULL) ;
    return icaltime_as_ical_string (a_t->t) ;
}

time_t
moko_time_as_timet (MokoTime *time)
{
  g_return_val_if_fail (time, 0) ;
  return icaltime_as_timet (time->t) ;
}
