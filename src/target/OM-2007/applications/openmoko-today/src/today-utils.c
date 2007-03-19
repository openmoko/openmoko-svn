/* vi: set sw=2: */
/*
 *  Today - At a glance view of date, time, calender events, todo items and
 *  other images.
 *
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

/**
 * e_cal_component_list_free:
 * @list: the list ECalComooment to free
 *
 * Free a list of ECalComponent
 */
#include "today-utils.h"

#define LOG_ERROR \
g_warning ("Got error '%s', code '%d'", \
           error->message, error->code);

#define FREE_ERROR g_error_free (error) ; error = NULL ;

void
e_cal_component_list_free (GList * list)
{
  GList *cur = NULL;

  for (cur = list; cur; cur = cur->next)
  {
    /*if an element of the list is not of type ECalComponent, leak it */
    if (cur->data && E_IS_CAL_COMPONENT (cur->data))
    {
      g_object_unref (G_OBJECT (cur->data));
      cur->data = NULL;
    }
    else
    {
      g_warning ("cur->data is not of type ECalComponent !");
    }
  }
  g_list_free (list);
}

/*
 * if the timetype is today, then only display it's hour part,
 * without the seconds
 * If it's not today, then only display it's date part, without the year
 */
gchar*
icaltime_to_pretty_string (const icaltimetype *timetype)
{
#define TMP_STR_LEN 10
    icaltimetype today ;
    gboolean     hour_only              = FALSE ;
    gboolean     date_only              = FALSE ;
    gchar        *result                = NULL  ;
    gchar        tmp_str[TMP_STR_LEN+1]         ;
    struct tm    native_tm                      ;

    g_return_val_if_fail (timetype, NULL) ;

    today = icaltime_today () ;
    if (!icaltime_compare_date_only (*timetype, today))
    {
        hour_only = TRUE ;
    }
    else
    {
        date_only = TRUE ;
    }
    if (hour_only)
    {
        result = g_strdup_printf ("%d:%d", timetype->hour, timetype->minute) ;
    }
    else if (date_only)
    {
        native_tm = icaltimetype_to_tm ((icaltimetype*)timetype) ;
        memset (tmp_str, 0, TMP_STR_LEN+1) ;
        strftime (tmp_str, TMP_STR_LEN, "%d/%b", &native_tm) ;
        result = g_strdup (tmp_str) ;
    }
    return result ;
}


static GList*
icalcomps_to_ecalcomps (GList *a_icalcomps)
{
  GList *cur=NULL, *ecalcomps=NULL  ;

  g_return_val_if_fail (a_icalcomps, NULL) ;
  /*
   * build a list of ECalComponent, out of the list of icalcomponents
   * when an icalcomponent is set to an ECalComponent, the later
   * becomes responsible of freeing the former's memory
   */
  for (cur = a_icalcomps; cur; cur = cur->next)
  {
    ECalComponent *c = NULL;
    if (!cur->data)
      continue;

    c = e_cal_component_new ();
    if (!e_cal_component_set_icalcomponent (c, cur->data))
    {
      g_warning ("setting icalcomp into ecalcomp failed") ;
      icalcomponent_free (cur->data);
      cur->data = NULL;
      continue;
    }
    ecalcomps = g_list_prepend (ecalcomps, c);
    cur->data = NULL;
  }
  return ecalcomps ;

}


/**
 * today_get_today_events:
 *
 * Return value:  a list of ECalComponents, of type VEVENT
 * or VTODO
 * must be freed with e_cal_component_list_free()
 */
GList *
today_get_today_events ()
{
  GList *result=NULL, *ical_comps=NULL,
        *ecal_comps=NULL, *ecal_comps2=NULL;
  ECal *ecal = NULL;
  GError *error = NULL;
  gchar *query = NULL;

  ecal = e_cal_new_system_calendar ();
  g_return_val_if_fail (ecal, NULL);

  if (!e_cal_open (ecal, FALSE, &error))
  {
    g_warning ("failed to open the calendar");
  }
  if (error)
  {
    LOG_ERROR;
    goto out;
  }

  /*
  query = g_strdup_printf ("(occur-in-time-range? "
                               "(time-day-begin (time-now)) "
                               "(time-day-end   (time-now)) "
                           ")");
   */
  query = g_strdup_printf ("#t");
  e_cal_get_object_list (ecal, query, &ical_comps, &error);
  g_free (query) ;
  query = NULL ;
  if (error)
  {
    LOG_ERROR;
    FREE_ERROR;
  }

  /*
   * build a list of ECalComponent, out of the list of icalcomponents
   * when an icalcomponent is set to an ECalComponent, the later
   * becomes responsible of freeing the former's memory
   */
  if (ical_comps)
  {
    ecal_comps = icalcomps_to_ecalcomps (ical_comps) ;
    g_list_free (ical_comps) ;
    ical_comps = NULL ;
  }

  ecal = e_cal_new_system_tasks ();
  g_return_val_if_fail (ecal, NULL);
  if (!e_cal_open (ecal, FALSE, &error))
  {
    g_warning ("failed to open the calendar");
  }
  if (error)
  {
    LOG_ERROR;
    goto out;
  }

  query = g_strdup_printf ("#t");
  e_cal_get_object_list (ecal, query, &ical_comps, &error);
  g_free (query) ;
  query = NULL ;
  if (error)
  {
    LOG_ERROR;
    FREE_ERROR ;
  }
  if (ical_comps)
  {
    ecal_comps2 = icalcomps_to_ecalcomps (ical_comps) ;
    g_list_free (ical_comps) ;
    ical_comps = NULL ;
  }
  ecal_comps = g_list_concat (ecal_comps, ecal_comps2) ;

  result = ecal_comps;
  ecal_comps = ecal_comps2 = NULL;

out:
  if (ical_comps)
  {
    e_cal_free_object_list (ical_comps);
  }

  if (ecal_comps)
  {
    e_cal_component_list_free (ecal_comps);
  }
  if (ecal_comps2)
  {
    e_cal_component_list_free (ecal_comps2);
  }

  /*
   the calender must stay alive during the app's lifetime
  if (ecal)
  {
    g_object_unref (G_OBJECT (ecal));
  }
  */

  if (error)
  {
    g_error_free (error);
  }

  if (query)
  {
    g_free (query);
  }

  return result;
}

