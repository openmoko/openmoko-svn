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
#include "xutil.h"

#define LOG_ERROR \
g_warning ("Got error '%s', code '%d'", \
           error->message, error->code);

#define FREE_ERROR g_error_free (error) ; error = NULL ;

/*** configuration options ***/
/* default to false, although this might want to be reversed in the future */
static gboolean enable_desktop = FALSE;

static GOptionEntry option_entries[] =
{
  { "enable-desktop", 'd', 0, G_OPTION_ARG_NONE,  &enable_desktop, "Set as desktop window", NULL},
  { NULL }
};


/**
 * today_update_date ()
 *
 * Update the specified GtkLabel with the current date
 */
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
  strftime (date_str, sizeof (date_str), "<big>%a %d/%b/%Y</big>", tmp);
  gtk_label_set_markup (label, date_str);

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
today_infoline_new (gchar * exec, gchar * message)
{
  GtkWidget *eventbox, *hbox, *icon, *label;
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
  GdkPixbuf *pb;

  eventbox = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (eventbox), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (eventbox), 6);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (eventbox), hbox);

  if (gtk_icon_theme_has_icon (icon_theme, exec))
  {
    pb = gtk_icon_theme_load_icon (icon_theme, exec, 32, GTK_ICON_LOOKUP_NO_SVG, NULL);
  }
  else
  {
    pb = gtk_icon_theme_load_icon (icon_theme, GTK_STOCK_MISSING_IMAGE, 32, GTK_ICON_LOOKUP_NO_SVG, NULL);
  }
  icon = gtk_image_new_from_pixbuf (pb);
  g_object_unref (pb);
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
  /*GList            *events;

  events = today_get_today_events () ;
  events_area = today_events_area_new_with_events (events) ;
  */
  events_area = today_events_area_new () ;
  today_events_area_set_events_auto (TODAY_EVENTS_AREA (events_area)) ;

  return events_area;
}

static void
create_ui ()
{
  GtkWidget *window, *vbox;
  GtkWidget *date;
  GtkWidget *message;

  GtkWidget *alignment;
  GtkWidget *infoline;
  GtkWidget *button_box;

  /* main window */
  window = moko_window_new ();
  gtk_widget_set_name (window, "today-application-window");
  gtk_window_set_title (GTK_WINDOW (window), "Today");

  if (enable_desktop)
  {
    gint x, y, w, h;
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DESKTOP);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    if (x_get_workarea (&x, &y, &w, &h))
    {
      gtk_window_set_default_size (GTK_WINDOW (window), w, h);
      gtk_window_move (GTK_WINDOW (window), x, y);
    }
  }

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* date */
  alignment = gtk_alignment_new (1, 0, 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 12, 0, 0, 12);
  date = gtk_label_new (NULL);
  gtk_container_add (GTK_CONTAINER (alignment), date);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
  today_update_date (GTK_LABEL (date));
  g_timeout_add (60 * 60 * 1000, (GSourceFunc) today_update_date, date);

  /* main message */
  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  message = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (message),
                        "<span size=\"x-large\">Welcome to OpenMoko</span>");
  gtk_container_add (GTK_CONTAINER (alignment), message);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, TRUE, FALSE, 0);


  /* unread messages */
  infoline = today_infoline_new ("openmoko-messages", "Unread Messages");
  gtk_box_pack_start (GTK_BOX (vbox), infoline, FALSE, FALSE, 0);

  /* missed calls */
  infoline = today_infoline_new ("openmoko-dialer", "Missed Calls");
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


  gtk_widget_show_all (window);

}

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GOptionContext *context;

  gtk_init (&argc, &argv);

  /* parse command line options */
  context = g_option_context_new ("- OpenMoko Today Application");
  g_option_context_add_main_entries (context, option_entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  /* create the UI and run */
  create_ui ();
  gtk_main ();

  return 0;
}
