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

#include <time.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "koto-date-combo.h"

G_DEFINE_TYPE (KotoDateCombo, koto_date_combo, GTK_TYPE_TOGGLE_BUTTON);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_DATE_COMBO, KotoDateComboPrivate))

typedef struct {
  GtkWidget *label;
  GtkWidget *popup;
  GtkWidget *calendar;

  GDate *date;
  
  guint selected_id;

  gboolean month_changed;

  /* The day that is marked in the calendar (as today), or 0 if no day is
     marked */
  int marked_day;
} KotoDateComboPrivate;

enum {
  PROP_0,
  PROP_DATE,
};

static void
update_ui (KotoDateCombo *combo)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (combo);

  g_assert (priv);

  if (g_date_valid (priv->date)) {
    char buffer[32];
    g_date_strftime (buffer, sizeof (buffer), "%x", priv->date);
    gtk_label_set_text (GTK_LABEL (priv->label), buffer);
  } else {
    gtk_label_set_text (GTK_LABEL (priv->label), _("no date set"));
  }
}

/* STOLEN, do I need this? */
static gboolean
popup_grab_on_window (GdkWindow *window,
		      guint32    activate_time,
		      gboolean   grab_keyboard)
{
  if ((gdk_pointer_grab (window, TRUE,
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK,
                         NULL, NULL, activate_time) == 0)) {
    if (!grab_keyboard || gdk_keyboard_grab (window, TRUE, activate_time) == 0) {
      return TRUE;
    } else {
      gdk_display_pointer_ungrab (gdk_drawable_get_display (window), activate_time);
      return FALSE;
    }
  }
  
  return FALSE;
}

/*
 * Mark the current day on the calendar, or remove any markings if the calendar
 * doesn't show the current month.
 */
static void
mark_today (KotoDateCombo *combo)
{
  KotoDateComboPrivate *priv;
  GDate today;
  guint month;

  g_assert (KOTO_IS_DATE_COMBO (combo));
  priv = GET_PRIVATE (combo);

  g_date_set_time_t (&today, time (NULL));
  
  if (priv->marked_day > 1) {
    gtk_calendar_unmark_day (GTK_CALENDAR (priv->calendar), priv->marked_day);
  }

  gtk_calendar_get_date (GTK_CALENDAR (priv->calendar), NULL, &month, NULL);
  month++; /* Stupid 0-based months */
  
  if (month == g_date_get_month (&today)) {
    priv->marked_day = g_date_get_day (&today);
    gtk_calendar_mark_day (GTK_CALENDAR (priv->calendar), priv->marked_day);
  } else {
    priv->marked_day = 0;
  }
}

/*
 * Callback when the toggle button is clicked, to show or hide the popup.
 */
static void
koto_date_combo_toggled (GtkToggleButton *button)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (button);
  GtkWidget *widget = GTK_WIDGET (button);

  if (gtk_toggle_button_get_active (button)) {
    int x = 0, y = 0;
    gdk_window_get_origin (widget->window, &x, &y);
    x += widget->allocation.x;
    y += widget->allocation.y + widget->allocation.height;

    gtk_window_move (GTK_WINDOW (priv->popup), x, y);
    gtk_widget_show (priv->popup);

    g_signal_handler_block (priv->calendar, priv->selected_id);
    if (g_date_valid (priv->date)) {
      /* GtkCalendar has 0-based months */
      gtk_calendar_select_month (GTK_CALENDAR (priv->calendar), priv->date->month - 1, priv->date->year);
      gtk_calendar_select_day (GTK_CALENDAR (priv->calendar), priv->date->day);
    } else {
      mark_today (KOTO_DATE_COMBO (button));
      gtk_calendar_select_day (GTK_CALENDAR (priv->calendar), 0);
    }
    g_signal_handler_unblock (priv->calendar, priv->selected_id);

    gtk_widget_grab_focus (priv->calendar);
    /* TODO: do I need this? */
    popup_grab_on_window (priv->popup->window, GDK_CURRENT_TIME, TRUE);
    gtk_grab_add (priv->popup);
  } else {
    gtk_grab_remove (priv->popup);
    gtk_widget_hide (priv->popup);
  }
}

static gboolean
on_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  GtkWidget *ewidget;
  KotoDateCombo *combo = KOTO_DATE_COMBO (user_data);

  ewidget = gtk_get_event_widget ((GdkEvent *)event);
  if (ewidget == GTK_WIDGET (combo)) {
    /* Clicked outside the popup window, so pop down */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo), FALSE);
  }

  return FALSE;
}

/*
 * The Today button was clicked.
 */
static void
on_today_clicked (GtkButton *button, KotoDateCombo *combo)
{
  GDate date;

  g_date_clear (&date, 1);
  g_date_set_time_t (&date, time (NULL));

  koto_date_combo_set_date (combo, &date);
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo), FALSE);
}

/*
 * The None button was clicked.
 */
static void
on_none_clicked (GtkButton *button, KotoDateCombo *combo)
{
  GDate date;

  g_date_clear (&date, 1);

  koto_date_combo_set_date (combo, &date);
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo), FALSE);
}

/*
 * Callback from GtkCalendar when a date is selected.
 */
static void
on_day_selected (GtkCalendar *calendar, KotoDateCombo *combo)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (combo);
  guint year, month, day;

  g_assert (priv);

  if (priv->month_changed) {
    priv->month_changed = FALSE;
    return;
  }

  gtk_calendar_get_date (calendar, &year, &month, &day);
  
  g_date_set_dmy (priv->date, day, month + 1, year);

  g_object_notify (G_OBJECT (combo), "date");

  update_ui (combo);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo), FALSE);
}

static void
on_month_changed (GtkCalendar *calendar, KotoDateCombo *combo)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (combo);

  g_assert (priv);

  priv->month_changed = TRUE;

  mark_today (combo);
}

/*
 * GObject methods.
 */

static void
koto_date_combo_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_DATE:
    g_value_set_boxed (value, priv->date);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_date_combo_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_DATE:
    if (priv->date) {
      g_free (priv->date);
    }
    priv->date = g_value_get_boxed (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_date_combo_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (koto_date_combo_parent_class)->dispose)
    G_OBJECT_CLASS (koto_date_combo_parent_class)->dispose (object);
}

static void
koto_date_combo_finalize (GObject *object)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (object);

  g_date_free (priv->date);
  
  G_OBJECT_CLASS (koto_date_combo_parent_class)->finalize (object);
}

static void
koto_date_combo_map (GtkWidget *widget)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (widget);
  GtkWidget *toplevel;

  GTK_WIDGET_CLASS (koto_date_combo_parent_class)->map (widget);

  /* This is used to limit the scope of the grab */
  
  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (widget));
  if (GTK_WIDGET_TOPLEVEL (toplevel)) {
    gtk_window_set_transient_for (GTK_WINDOW (priv->popup),
                                  GTK_WINDOW (toplevel));
  }
}

static void
koto_date_combo_class_init (KotoDateComboClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkToggleButtonClass *toggle_class = GTK_TOGGLE_BUTTON_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoDateComboPrivate));

  object_class->get_property = koto_date_combo_get_property;
  object_class->set_property = koto_date_combo_set_property;
  object_class->dispose = koto_date_combo_dispose;
  object_class->finalize = koto_date_combo_finalize;

  widget_class->map = koto_date_combo_map;

  toggle_class->toggled = koto_date_combo_toggled;

  g_object_class_install_property (object_class, PROP_DATE,
                                   g_param_spec_boxed ("date", "date", "date",
                                                       G_TYPE_DATE,
                                                       G_PARAM_READABLE | G_PARAM_WRITABLE |
                                                       G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

                                                       
}

static void
koto_date_combo_init (KotoDateCombo *self)
{
  KotoDateComboPrivate *priv = GET_PRIVATE (self);
  GtkWidget *frame, *hbox, *vbox, *button;

  priv->date = g_date_new ();

  priv->month_changed = FALSE;

  /* Create the contents of the button */
  hbox = gtk_hbox_new (FALSE, 0);

  priv->label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (priv->label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox),
                      gtk_vseparator_new (),
                      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox),
                      gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE),
                      FALSE, FALSE, 0);
  
  gtk_widget_show_all (hbox);
  gtk_container_add (GTK_CONTAINER (self), hbox);

  priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
  g_signal_connect (priv->popup, "button-release-event", G_CALLBACK (on_button_release), self);
  gtk_window_set_resizable (GTK_WINDOW (priv->popup), FALSE);
#if HAVE_DECL_GDK_WINDOW_TYPE_HINT_COMBO
  gtk_window_set_type_hint (GTK_WINDOW (priv->popup), GDK_WINDOW_TYPE_HINT_COMBO);
#endif

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (priv->popup), frame);
  gtk_widget_show (frame);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  hbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_EDGE);
  button = gtk_button_new_with_label (_("Today"));
  g_signal_connect (button, "clicked", G_CALLBACK (on_today_clicked), self);
  gtk_container_add (GTK_CONTAINER (hbox), button);
  button = gtk_button_new_with_label (_("None"));
  g_signal_connect (button, "clicked", G_CALLBACK (on_none_clicked), self);
  gtk_container_add (GTK_CONTAINER (hbox), button);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  priv->calendar = gtk_calendar_new ();
  gtk_calendar_set_display_options (GTK_CALENDAR (priv->calendar),
                                    GTK_CALENDAR_SHOW_HEADING | GTK_CALENDAR_SHOW_DAY_NAMES | GTK_CALENDAR_SHOW_WEEK_NUMBERS);
  priv->selected_id = g_signal_connect (priv->calendar, "day-selected", G_CALLBACK (on_day_selected), self);
  g_signal_connect (priv->calendar, "month-changed", G_CALLBACK (on_month_changed), self);
  gtk_box_pack_start (GTK_BOX (vbox), priv->calendar, TRUE, TRUE, 0);
  gtk_widget_show_all (vbox);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
}


/*
 * Public methods.
 */

GtkWidget *
koto_date_combo_new (void)
{
  return g_object_new (KOTO_TYPE_DATE_COMBO, NULL);
}

void
koto_date_combo_set_date (KotoDateCombo *combo, GDate *date)
{
  KotoDateComboPrivate *priv;

  g_return_if_fail (KOTO_IS_DATE_COMBO (combo));
  g_return_if_fail (date);

  priv = GET_PRIVATE (combo);

  if (g_date_valid (date)) {
    g_date_set_julian (priv->date, g_date_get_julian (date));
  } else {
    g_date_clear (priv->date, 1);
  }
  
  g_object_notify (G_OBJECT (combo), "date");

  update_ui (combo);
}

/* Note that this doesn't copy the date yet so don't mess around with it */
GDate *
koto_date_combo_get_date (KotoDateCombo *combo)
{
  KotoDateComboPrivate *priv;
  
  g_return_val_if_fail (KOTO_IS_DATE_COMBO (combo), NULL);
  
  priv = GET_PRIVATE (combo);
  
  return priv->date;
}
