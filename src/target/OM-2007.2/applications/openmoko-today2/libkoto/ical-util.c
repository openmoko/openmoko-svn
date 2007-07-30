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

#define ICAL_GET_FIELD(LOWER_NAME, UPPER_NAME, TYPE, DEFAULT) \
  TYPE                                                 \
  ical_util_get_##LOWER_NAME (icalcomponent *comp)             \
  {                                                           \
    icalproperty *prop;                                       \
    TYPE value;                                               \
    g_assert (comp);                                                    \
    prop = icalcomponent_get_first_property (comp, ICAL_##UPPER_NAME##_PROPERTY); \
    if (prop) {                                                         \
      value = icalproperty_get_##LOWER_NAME (prop);                     \
      icalproperty_free (prop);                                         \
      return value;                                                     \
    } else {                                                            \
      return DEFAULT;                                                   \
    }                                                                   \
  }

#include "ical-util.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

/*
 * Return a human-readable for the date @due, relative to the current date.
 */
char *
ical_util_get_human_date (GDate *due)
{
  GDate today;
  int days;
  char buffer[256];

  g_assert (g_date_valid (due));

  g_date_clear (&today, 1);
  g_date_set_time_t (&today, time (NULL));

  days = g_date_days_between (&today, due);
  if (days == 0)
    return g_strdup (_("today"));
  else if (days == 1)
    return g_strdup (_("tomorrow"));
  else if (days == -1)
    return g_strdup (_("yesterday"));
  else if (days > 1 && days < 7) {
    /* Return name of the day if it is in the next 6 days */
    g_date_add_days (&today, days);
    g_date_strftime (buffer, sizeof (buffer), "%A", &today);
    return g_strdup (buffer);
  } else {
    /* Fallback to returning the preferred date representation */
    g_date_strftime (buffer, sizeof (buffer), "%x", due);
    return g_strdup (buffer);
  }
}

#if WITH_TESTS

#include <locale.h>
#include <string.h>

static const char *weekdays[] = {
  "BAD WEEKDAY", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday", "Sunday"
};

int main (int argc, char **argv)
{
  GDate date;
  char *s;

  /* Set the locale to C so we can do string comparisons easily */
  setlocale(LC_ALL, "C");
  
  g_date_clear (&date, 1);
  
  g_date_set_dmy (&date, 1, G_DATE_MAY, 2001);
  s = ical_util_get_human_date (&date);
  g_assert (strcmp (s, "05/01/01") == 0);

  g_date_set_time_t (&date, time (NULL));
  s = ical_util_get_human_date (&date);
  g_assert (strcmp (s, "today") == 0);

  g_date_add_days (&date, 1);
  s = ical_util_get_human_date (&date);
  g_assert (strcmp (s, "tomorrow") == 0);

  g_date_subtract_days (&date, 2);
  s = ical_util_get_human_date (&date);
  g_assert (strcmp (s, "yesterday") == 0);

  g_date_add_days (&date, 3);
  s = ical_util_get_human_date (&date);
  g_assert (strcmp (s, weekdays[g_date_get_weekday (&date)]) == 0);

  return 0;
}

#endif
