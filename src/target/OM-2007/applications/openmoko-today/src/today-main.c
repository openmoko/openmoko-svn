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

static void
today_update_date (GtkLabel *label)
{
  time_t t;
  struct tm *tmp;
  gchar date_str[64];

  t = time (NULL);
  tmp = localtime (&t);

  if (tmp == NULL) {
    // error = could not get localtime
    return;
  }

  /* TODO: use something nicer from the locale here */
  strftime (date_str, sizeof (date_str), "%a %d/%b/%Y", tmp);
  gtk_label_set_text (label, date_str);

}

static void
today_update_time (GtkLabel *label)
{
  time_t t;
  struct tm *tmp;
  gchar time_str[64];

  t = time (NULL);
  tmp = localtime (&t);

  if (tmp == NULL) {
    // error = could not get localtime
    return;
  }

  /* TODO: make 12/24 hr optional */
  strftime (time_str, sizeof (time_str), "<big>%l:%M</big> %p", tmp);
  gtk_label_set_markup (label, time_str);
}

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

static GtkWidget *
today_launcher_button_new (gchar *icon, gchar *exec)
{
  GtkWidget *button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button),
      gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON));
  gtk_widget_set_name (button, "today-launcher-button");

  return button;
}

static void
create_ui ()
{
  GtkWidget *window, *vbox;
  GtkWidget *date, *time;
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
  g_timeout_add (60*60*1000, (GSourceFunc)today_update_date, date);

  /* time */
  alignment = gtk_alignment_new (1, 0, 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 0, 12);
  time = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (time), "<big>10:30am</big>");
  gtk_container_add (GTK_CONTAINER (alignment), time);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
  today_update_time (GTK_LABEL (time));
  g_timeout_add (60*1000, (GSourceFunc)today_update_time, time);

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
  infoline = today_infoline_new (GTK_STOCK_NO,
                                 "Carrie's Birthday 16/Jan\n"
                                 "Taxi to Airport 13:00\n"
                                 "Meeting with client 17:00\n"
                                 "Call Sean 19:30\n"
                                 "Dinner with John 20:00\n");
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* shurtcut buttons */
  button_box = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box),
                             GTK_BUTTONBOX_SPREAD);
  gtk_box_pack_start (GTK_BOX (vbox), button_box, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (button_box),
      today_launcher_button_new (GTK_STOCK_EXECUTE, ""));
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
