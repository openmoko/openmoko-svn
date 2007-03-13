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

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libecal/e-cal.h>
#include <libecal/e-cal-time-util.h>
#include <gtk/gtk.h>
#include <libmokoui/moko-window.h>

#define LOG_ERROR \
g_warning ("Got error '%s', code '%d'", \
           error->message, error->code);

#define FREE_ERROR g_error_free (error) ; error = NULL ;

static void
today_update_date (GtkLabel * label)
{
  time_t t;
  struct tm *tmp;
  gchar date_str[64];

  t = time (NULL);
  tmp = localtime (&t);

  if (tmp == NULL)
  {
    // error = could not get localtime
    return;
  }

  /* TODO: use something nicer from the locale here */
  strftime (date_str, sizeof (date_str), "%a %d/%b/%Y", tmp);
  gtk_label_set_text (label, date_str);

}

static void
today_update_time (GtkLabel * label)
{
  time_t t;
  struct tm *tmp;
  gchar time_str[64];

  t = time (NULL);
  tmp = localtime (&t);

  if (tmp == NULL)
  {
    // error = could not get localtime
    return;
  }

  /* TODO: make 12/24 hr optional */
  strftime (time_str, sizeof (time_str), "<big>%I:%M</big> %p", tmp);
  gtk_label_set_markup (label, time_str);
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

/**
 * returns a list of ECalComponents, of type VEVENT
 * it must freed it with e_cal_component_list_free()
 */
static GList *
get_today_events ()
{
  GList *result = NULL;
  GList *ical_comps = NULL;
  GList *ecal_comps = NULL;
  GList *cur = NULL;
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

  query = g_strdup_printf ("(occur-in-time-range? "
                               "(time-day-begin (time-now)) "
                               "(time-day-end   (time-now)) "
                           ")");
  e_cal_get_object_list (ecal, query, &ical_comps, &error);
  if (error)
  {
    LOG_ERROR;
    goto out;
  }

  /*
   * build a list of ECalComponent, out of the list of icalcomponents
   * when an icalcomponent is set to an ECalComponent, the later
   * becomes responsible of freeing the former's memory
   */
  for (cur = ical_comps; cur; cur = cur->next)
  {
    ECalComponent *c = NULL;
    if (!cur->data)
      continue;

    c = e_cal_component_new ();
    if (!e_cal_component_set_icalcomponent (c, cur->data))
    {
      icalcomponent_free (cur->data);
      cur->data = NULL;
      continue;
    }

    ecal_comps = g_list_prepend (ecal_comps, c);
    cur->data = NULL;
  }
  result = ecal_comps;
  ecal_comps = NULL;

out:
  if (ical_comps)
  {
    e_cal_free_object_list (ical_comps);
  }

  if (ecal_comps)
  {
    e_cal_component_list_free (ecal_comps);
  }
  ecal_comps = NULL;

  g_object_unref (G_OBJECT (ecal));

  if (error)
  {
    g_error_free (error);
  }

  g_free (query);

  return result;
}

/* information lines */

/**
 * today_infoline_new:
 * @stock_id: name of the stock icon to use
 * @message: string containing the message
 *
 * Utility function to create new info lines
 *
 * Return value: The parent widget of the new widgets
 */

static GtkWidget *
today_infoline_new (gchar * stock_id, gchar * message)
{
  GtkWidget *eventbox, *hbox, *icon, *label;

  eventbox = gtk_event_box_new ();
  gtk_container_set_border_width (GTK_CONTAINER (eventbox), 6);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (eventbox), hbox);

  icon = gtk_image_new ();
  gtk_image_set_from_stock (GTK_IMAGE (icon), stock_id, GTK_ICON_SIZE_MENU);
  gtk_misc_set_alignment (GTK_MISC (icon), 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

  label = gtk_label_new (message);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  return eventbox;
}


/* launcher buttons */

/**
 * callback for luncher buttons
 */
static void
today_launcher_clicked_cb (GtkWidget *button, gchar *command)
{
  GError *error;

  g_spawn_command_line_async (command, &error);

  if (error)
  {
    LOG_ERROR;
    g_error_free (error);
  }

  /* TODO: should we hide or quit after launching an application? */
}

/**
 * today_launcher_button_new:
 * @icon: stock name to use as the icon in the button
 * @exec: command to execute when the button is clicked
 *
 * Utility function to create new launcher buttons
 *
 * Return value: The parent widget of the new widgets
 */
static GtkWidget *
today_launcher_button_new (gchar * icon, gchar * exec)
{
  GtkWidget *button = gtk_button_new ();
  if (exec) {/*unused parameter, to be used in the future*/}

  gtk_container_add (GTK_CONTAINER (button),
                     gtk_image_new_from_stock (icon, GTK_ICON_SIZE_BUTTON));

  gtk_widget_set_name (button, "today-launcher-button");

  g_signal_connect (G_OBJECT (button),
                    "clicked",
                    G_CALLBACK (today_launcher_clicked_cb),
                    exec);

  return button;
}

GtkWidget *
get_today_events_infoline ()
{
  GtkWidget        *infoline  = NULL ;
  GList            *events    = NULL ;
  GList            *cur       = NULL ;
  GString          *lines     = NULL ;

  events = get_today_events () ;
  lines = g_string_new (NULL) ;

  for (cur = events ; cur ; cur = cur->next)
  {
    ECalComponentText      text ;
    ECalComponentDateTime  start_date ;
    gchar                  *tmp_str = NULL ;

    if (!E_IS_CAL_COMPONENT (cur->data)) {
      g_warning ("cur->data is not of type ECalComponent!") ;
      continue ;
    }
    if (e_cal_component_get_vtype (cur->data) != E_CAL_COMPONENT_EVENT)
    {
      g_warning ("Event type is not 'EVENT', but rather %d",
                 e_cal_component_get_vtype (cur->data));
      continue;
    }
    /*get the event summary*/
    e_cal_component_get_summary (cur->data, &text) ;
    /*get the event starting date*/
    e_cal_component_get_dtstart (cur->data, &start_date) ;
    /*pretty print the starting date*/
    tmp_str = icaltime_to_pretty_string (start_date.value) ;
    if (tmp_str)
    {
        g_string_append_printf (lines, "%s  %s\n", text.value, tmp_str) ;
        g_free (tmp_str) ;
    }
    else
    {
        g_string_append_printf (lines, "%s  %s\n", text.value, "No Date") ;
    }
    e_cal_component_free_datetime (&start_date) ;
  }
  if (lines->len)
  {
    infoline = today_infoline_new (GTK_STOCK_NO,
                                   lines->str) ;
  }
  else
  {
    infoline = today_infoline_new (GTK_STOCK_NO, "No events for today") ;
  }

  if (lines->len)
    infoline = today_infoline_new (GTK_STOCK_NO, lines->str);
  else
    infoline = today_infoline_new (GTK_STOCK_NO, "No events for today");

  if (events)
  {
    e_cal_component_list_free (events);
  }

  if (lines)
  {
    g_string_free (lines, TRUE);
    lines = NULL;
  }
  return infoline;
}

static void
create_ui ()
{
  GtkWidget *window, *vbox;
  GtkWidget *date, *time_label;
  GtkWidget *message;

  GtkWidget *alignment;
  GtkWidget *infoline;
  GtkWidget *button_box;

  /* main window */
  window = moko_window_new ();
  gtk_widget_set_name (window, "today-application-window");

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* date */
  alignment = gtk_alignment_new (1, 0, 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 12, 0, 0, 12);
  date = gtk_label_new ("MON 15/JAN/2007");
  gtk_container_add (GTK_CONTAINER (alignment), date);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
  today_update_date (GTK_LABEL (date));
  g_timeout_add (60 * 60 * 1000, (GSourceFunc) today_update_date, date);

  /* time */
  alignment = gtk_alignment_new (1, 0, 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 0, 12);
  time_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (time_label), "<big>10:30am</big>");
  gtk_container_add (GTK_CONTAINER (alignment), time_label);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
  today_update_time (GTK_LABEL (time_label));
  g_timeout_add (60 * 1000, (GSourceFunc) today_update_time, time_label);

  /* main message */
  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  message = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (message),
                        "<span size=\"x-large\">Welcome to OpenMoko</span>");
  gtk_container_add (GTK_CONTAINER (alignment), message);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, TRUE, FALSE, 0);


  /* unread messages */
  infoline = today_infoline_new (GTK_STOCK_YES, "Unread Messages (5)");
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* missed calls */
  infoline = today_infoline_new (GTK_STOCK_NO, "Missed Calls (1)");
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* upcoming events */
  infoline = get_today_events_infoline ();
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* shurtcut buttons */
  button_box = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box),
                             GTK_BUTTONBOX_SPREAD);
  gtk_box_pack_start (GTK_BOX (vbox), button_box, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new (GTK_STOCK_EXECUTE, "contacts"));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new (GTK_STOCK_EXECUTE, ""));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new (GTK_STOCK_EXECUTE, ""));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new (GTK_STOCK_EXECUTE, ""));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new (GTK_STOCK_EXECUTE, ""));

  /* signals */
  g_signal_connect (G_OBJECT (window), "delete-event",
                    (GCallback) gtk_main_quit, NULL);


  /* temporary */
  GtkSettings *settings = gtk_settings_get_default ();
  g_object_set (G_OBJECT (settings), "gtk-theme-name", "openmoko-standard",
                NULL);

  gtk_widget_show_all (window);

}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  create_ui ();
  gtk_main ();
  return 0;
}
