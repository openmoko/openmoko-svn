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
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <libecal/e-cal-component.h>
#include <libecal/e-cal.h>
#include <libecal/e-cal-time-util.h>

#define LOG_ERROR \
g_warning ("Got error '%s', code '%d'", \
           error->message, error->code);

#define FREE_ERROR g_error_free (error) ; error = NULL ;
/*static const char *s_query = "" ;*/

void
display_usage (const char *prog_name)
{
    g_return_if_fail (prog_name) ;
    printf ("usage: %s <query sexpression>\n", prog_name) ;
    printf ("or: %s\n", prog_name) ;
}

void
display_events (GList *a_events/*list of icalcomponents*/)
{
    GList          *cur         = NULL ;
    ECalComponent  *cal_comp    = NULL ;
    char           *event_str   = NULL ;

    if (!a_events) {
        g_message ("No events") ;
        return ;
    }
    cal_comp = e_cal_component_new () ;
    g_return_if_fail (cal_comp) ;

    for (cur = a_events ; cur ; cur = cur->next) {
        if (!cur->data) {continue;}
        e_cal_component_set_icalcomponent (cal_comp, cur->data) ;
        if (e_cal_component_get_vtype (cal_comp) != E_CAL_COMPONENT_EVENT) {
            g_warning ("component is not an event, rather of type %d",
                       e_cal_component_get_vtype (cal_comp));
            continue ;
        }
        event_str = e_cal_component_get_as_string (cal_comp) ;
        if (event_str) {
            g_message ("Got event '%s'", event_str) ;
            g_free (event_str) ;
        }
    }
}

int
main (int argc, char **argv)
{
    int            ret       = 0 ;
    ECal           *cal      = NULL ;
    char           *query    = NULL ;
    GList          *objects  = NULL ;
    GError         *error    = NULL ;

    g_type_init () ;

    if (argc != 1 && (!strcmp (argv[1], "--help") || !strcmp (argv[0], "-h"))) {
        display_usage (argv[0]) ;
        return -1 ;
    }

    cal = e_cal_new_system_calendar () ;
    g_return_val_if_fail (cal, -1) ;
    if (!e_cal_open (cal, TRUE, &error)) {
        g_warning ("failed to open the calendar") ;
        ret= -1 ;
    }
    if (error) {
        ret = -1 ;
        LOG_ERROR ;
        FREE_ERROR ;
    }
    if (ret) {goto out ;}

    query = g_strdup_printf ("(occur-in-time-range? "
                                 "(time-day-begin (time-now)) "
                                 "(time-day-end   (time-now))"
                             ")");

    printf ("Issuing query: '%s'\n", query) ;
    if (!e_cal_get_object_list (cal, query, &objects, &error)) {
        g_message ("Querying system calendar failed\n") ;
        ret = -1 ;
    }
    if (error) {
        ret = -1 ;
        LOG_ERROR ;
        FREE_ERROR ;
    }
    if (ret) {goto out ;}
    g_message ("Query succeded\n") ;
    if (objects) {
        display_events (objects) ;
    }

out:
    if (cal) {
        g_object_unref (G_OBJECT (cal)) ;
    }
    if (objects) {
        e_cal_free_object_list (objects) ;
    }
    if (query) {
        g_free (query) ;
    }
    return ret ;
}
