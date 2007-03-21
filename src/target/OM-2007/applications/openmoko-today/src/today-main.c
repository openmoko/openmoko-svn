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

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <libmokoui/moko-window.h>
#include <libmokoui/moko-pixmap-button.h>
#include "today-events-area.h"
#include "today-utils.h"

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
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (eventbox), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (eventbox), 6);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (eventbox), hbox);

  icon = gtk_image_new ();
  gtk_image_set_from_stock (GTK_IMAGE (icon), stock_id, GTK_ICON_SIZE_MENU);
  gtk_misc_set_alignment (GTK_MISC (icon), 0, 0);
  gtk_widget_show (icon) ;
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

  // FIXME: get this from the style... somehow
  gtk_widget_set_size_request (icon, 51, -1);


  label = gtk_label_new (message);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  return eventbox;
}

/* launcher buttons */

/**
 * callback for luncher buttons
 */
void
today_launcher_clicked_cb (GtkWidget *button, gchar *command)
{
  GError *error = NULL;

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
 * @exec: command to execute when the button is clicked
 *
 * Utility function to create new launcher buttons
 *
 * Return value: The parent widget of the new widgets
 */
static GtkWidget *
today_launcher_button_new (gchar * exec)
{
  GtkWidget *button = moko_pixmap_button_new ();
  GdkPixbuf *pb;
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();

  /* libmokoui api really needs fixing... */

  if (gtk_icon_theme_has_icon (icon_theme, exec))
  {
    pb = gtk_icon_theme_load_icon (icon_theme, exec, 48, GTK_ICON_LOOKUP_NO_SVG, NULL);
  }
  else
  {
    pb = gtk_icon_theme_load_icon (icon_theme, GTK_STOCK_MISSING_IMAGE, 48, GTK_ICON_LOOKUP_NO_SVG, NULL);
  }

  moko_pixmap_button_set_finger_toolbox_btn_center_image_pixbuf (
      MOKO_PIXMAP_BUTTON (button), pb);
  g_object_unref (pb);
  gtk_widget_set_name (button, "mokofingertoolbox-toolbutton");

  g_signal_connect (G_OBJECT (button),
                    "clicked",
                    G_CALLBACK (today_launcher_clicked_cb),
                    exec);
  return button;
}

/**
 * today_setup_events_area:
 *
 * Return value: The widget to use as the events area
 *
 */
GtkWidget *
today_setup_events_area (const gchar *stock_id)
{
  GtkWidget        *events_area;
  GList            *events;

  events = today_get_today_events () ;
  events_area = today_events_area_new_with_events (events) ;

  return events_area;
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
  infoline = today_setup_events_area (GTK_STOCK_NO);
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* shurtcut buttons */
  button_box = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box),
                             GTK_BUTTONBOX_SPREAD);
  gtk_box_pack_start (GTK_BOX (vbox), button_box, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new ("openmoko-dialer"));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new ("contacts"));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new ("openmoko-messages"));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new ("openmoko-gps"));
  gtk_container_add (GTK_CONTAINER (button_box),
                     today_launcher_button_new ("dates"));

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
