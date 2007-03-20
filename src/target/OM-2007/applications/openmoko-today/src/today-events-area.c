/* vi:set sw=2: */

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
/*#include <libecal/e-cal-time-util.h>*/
/*#include <libical/e-cal-component.h>*/
#include <libical/icalcomponent.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtklabel.h>
#include "today-utils.h"
#include "today-events-area.h"


struct _TodayEventsAreaPrivate {
  GList *events ;
  GtkWidget *left ;
  GtkWidget *left_event_box ;
  GtkWidget *paging_info ;
  GtkWidget *right ;

  GList *cur_event ;
  GList *page_start ;
  int page_start_index ;
  int cur_event_index ;
  int max_visible_events ;
  int nb_events ;
};

enum TodayEventsAreaSignals
{
  EVENTS_ADDED_SIGNAL,
  EVENT_SELECTED_SIGNAL,
  PAGE_SWITCHED_SIGNAL,
  LAST_SIGNAL
};

enum TodayEventsAreaProps
{
  EVENTS_PROP=1,
  NB_EVENTS_PROP,
  NB_PAGES_PROP,
  CUR_EVENT_PROP,
  CUR_EVENT_INDEX_PROP,
  MAX_VISIBLE_EVENTS_PROP
};

static guint signals[LAST_SIGNAL] ;

static void     today_events_area_finalize  (GObject *a_obj);
static void     today_events_area_init      (TodayEventsArea *a_this);
static void     clear_right_hand_side       (TodayEventsArea *a_this);
static void     init_right_hand_side        (TodayEventsArea *a_this);
static void     clear_left_hand_side        (TodayEventsArea *a_this);
static void     init_left_hand_side         (TodayEventsArea *a_this);
static void     reinit_area                 (TodayEventsArea *a_this) ;
static int      get_nb_events_real          (TodayEventsArea *a_this) ;
static gboolean update_paging_info          (TodayEventsArea *a_this) ;
static GList*   select_event                (TodayEventsArea *a_this,
                                             GList *a_event) ;
static void     render_event                (TodayEventsArea *a_this,
                                             GList *a_event) ;
static void     render_events_page          (TodayEventsArea *a_this,
                                             GList *a_from) ;
static void     render_events_page_auto     (TodayEventsArea *a_this) ;
static void     event_selected_signal       (TodayEventsArea *a_this,
                                             guint a_index) ;
static void     events_added_signal       (TodayEventsArea *a_this,
                                           GList *a_index) ;

static void     get_property (GObject *a_this, guint a_prop_id,
                              GValue *a_val, GParamSpec *a_pspec) ;
static void     set_property (GObject *a_this, guint a_prop_id,
                              const GValue *a_value, GParamSpec *a_pspec) ;

G_DEFINE_TYPE (TodayEventsArea, today_events_area, GTK_TYPE_TABLE)

static void
today_events_area_class_init (TodayEventsAreaClass *a_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (a_class);
  object_class->finalize = today_events_area_finalize;
  object_class->get_property = get_property ;
  object_class->set_property = set_property ;

  g_type_class_add_private (object_class, sizeof (TodayEventsAreaPrivate));

  a_class->event_selected = event_selected_signal ;
  a_class->events_added = events_added_signal ;

  g_object_class_install_property
                    (object_class,
                     EVENTS_PROP,
                     g_param_spec_pointer ("events",
                                           "events",
                                           "a GList of calendar events"
                                            ", instances of ECalComponent",
                                           G_PARAM_READWRITE));
  g_object_class_install_property
                          (object_class,
                           NB_EVENTS_PROP,
                           g_param_spec_uint ("nb-events",
                                              "nb-events",
                                              "Number of events set",
                                              0, G_MAXUINT, 0,
                                              G_PARAM_READABLE)) ;
  g_object_class_install_property
                          (object_class,
                           NB_PAGES_PROP,
                           g_param_spec_uint ("nb-event-pages",
                                              "nb-event-pages",
                                              "Number of event pages",
                                              0, G_MAXUINT, 0,
                                              G_PARAM_READABLE)) ;
  g_object_class_install_property
                          (object_class,
                           CUR_EVENT_PROP,
                           g_param_spec_pointer ("cur-event",
                                                 "cur-event",
                                                 "Currently selected event",
                                                 G_PARAM_READABLE)) ;
  g_object_class_install_property
                    (object_class,
                     CUR_EVENT_INDEX_PROP,
                     g_param_spec_uint ("cur-event-index",
                                        "cur-event-index",
                                        "The index of the currently "
                                         "selected event",
                                         0, G_MAXUINT, 0,
                                        G_PARAM_READABLE)) ;
  g_object_class_install_property
                          (object_class,
                           MAX_VISIBLE_EVENTS_PROP,
                           g_param_spec_uint ("max-visible-events",
                                              "max-visible-events",
                                              "The max number of events in "
                                              "a page",
                                              0, G_MAXUINT, 0,
                                              G_PARAM_READWRITE)) ;

  signals[EVENTS_ADDED_SIGNAL] =
    g_signal_new ("event-added",
                  TODAY_TYPE_EVENTS_AREA,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (TodayEventsAreaClass, events_added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER) ;

  signals[EVENT_SELECTED_SIGNAL] =
    g_signal_new ("event-selected",
                   TODAY_TYPE_EVENTS_AREA,
                   G_SIGNAL_RUN_FIRST,
                   G_STRUCT_OFFSET (TodayEventsAreaClass, event_selected),
                   NULL, NULL,
                   g_cclosure_marshal_VOID__UINT,
                   G_TYPE_NONE,
                   1, G_TYPE_UINT) ;

  signals[PAGE_SWITCHED_SIGNAL] =
    g_signal_new ("page-switched",
                  TODAY_TYPE_EVENTS_AREA,
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (TodayEventsAreaClass, page_switched),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE,
                  1, G_TYPE_UINT) ;
}

static void
today_events_area_init (TodayEventsArea *a_this)
{
  g_return_if_fail (GTK_IS_TABLE (a_this)) ;

  a_this->priv = G_TYPE_INSTANCE_GET_PRIVATE (a_this,
                                              TODAY_TYPE_EVENTS_AREA,
                                              TodayEventsAreaPrivate);

  gtk_table_resize (GTK_TABLE (a_this), 1, 2) ;
  reinit_area (a_this) ;
}

static void
today_events_area_finalize (GObject *a_obj)
{
  TodayEventsArea *self = TODAY_EVENTS_AREA (a_obj) ;
  g_return_if_fail (self && self->priv) ;

  if (self->priv->events)
  {
    e_cal_free_object_list (self->priv->events) ;
    self->priv->events = NULL ;
  }
  self->priv->cur_event = NULL ;
  self->priv->cur_event_index = 0 ;
  self->priv->page_start = NULL ;
  self->priv->max_visible_events = 0 ;

  self->priv = NULL ;

  /*chain up*/
  (*G_OBJECT_CLASS (today_events_area_parent_class)->finalize) (a_obj);
}
/********************
 * <signal callbacks>
 ********************/
gboolean
on_button_pressed_in_left_cb (GtkWidget *a_event_box,
                              GdkEventButton *a_button,
                              TodayEventsArea *a_area)
{
    g_return_val_if_fail (a_area && TODAY_IS_EVENTS_AREA (a_area), FALSE) ;
    if (a_event_box) {/*keep compiler happy*/}

    if (a_button->type == GDK_BUTTON_PRESS)
      today_events_area_switch_to_next_page (a_area) ;

    return FALSE ;
}

gboolean
on_button_pressed_in_infoline_cb (GtkWidget *a_event_box,
                                  GdkEventButton *a_button,
                                  TodayEventsArea *a_area)
{
  int event_index = 0 ;

  g_return_val_if_fail (a_area && TODAY_IS_EVENTS_AREA (a_area), FALSE) ;

  if (a_button->type != GDK_BUTTON_PRESS)
    return FALSE ;

  event_index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (a_event_box),
                                                    "event-index")) ;
  g_return_val_if_fail (event_index >= 0, FALSE) ;
  g_signal_emit (G_OBJECT (a_area),
                 signals[EVENT_SELECTED_SIGNAL], 0, event_index) ;

  return FALSE;
}

/********************
 * </signal callbacks>
 ********************/

/**********************
 * <private api>
 **********************/

static void
event_selected_signal (TodayEventsArea *a_this,
                       guint a_index)
{
  GList *elem ;

  g_return_if_fail (a_this && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  elem = g_list_nth (a_this->priv->events, a_index) ;
  select_event (a_this, elem) ;
}

static void
events_added_signal (TodayEventsArea *a_this,
                     GList *a_events)
{
  if (a_events) {/*keep compiler happy*/}


  if (today_events_area_get_nb_pages (a_this) > 1)
  {
     gtk_widget_set_name (a_this->priv->left_event_box,
                          "today-events-area-postit-multi");
  }
  else
  {
     gtk_widget_set_name (a_this->priv->left_event_box,
                          "today-events-area-postit-single");
  }
  /*
   * reload the styles to render the left hand side correctly
   * so that it matches the new widget name
   */
  gtk_widget_reset_rc_styles (GTK_WIDGET (a_this)) ;
}

static void
get_property (GObject *a_this, guint a_prop_id,
              GValue *a_val, GParamSpec *a_pspec)
{
  TodayEventsArea *area ;
  g_return_if_fail (a_this && TODAY_IS_EVENTS_AREA (a_this)) ;
  g_return_if_fail (a_val && a_pspec) ;

  area = TODAY_EVENTS_AREA (area) ;

  switch (a_prop_id)
  {
    case EVENTS_PROP:
      g_value_set_pointer (a_val, today_events_area_get_events (area)) ;
      break ;
    case NB_EVENTS_PROP:
      g_value_set_uint (a_val, today_events_area_get_nb_events (area)) ;
      break ;
    case NB_PAGES_PROP:
      g_value_set_uint (a_val, today_events_area_get_nb_pages (area)) ;
      break ;
    case CUR_EVENT_PROP:
      g_value_set_pointer (a_val, today_events_area_get_cur_event (area)) ;
      break ;
    case CUR_EVENT_INDEX_PROP:
      g_value_set_uint (a_val, today_events_area_get_cur_event_index (area)) ;
      break ;
    case MAX_VISIBLE_EVENTS_PROP:
      g_value_set_uint (a_val, today_events_area_get_max_visible_events (area));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (a_this, a_prop_id, a_pspec) ;
      break ;
  }
}

static void
set_property (GObject *a_this, guint a_prop_id,
              const GValue *a_val, GParamSpec *a_pspec)
{
  TodayEventsArea * area ;
  g_return_if_fail (a_this && TODAY_IS_EVENTS_AREA (a_this)) ;
  g_return_if_fail (a_val && a_pspec) ;

  area = TODAY_EVENTS_AREA (a_this) ;

  switch (a_prop_id)
  {
    case EVENTS_PROP:
      today_events_area_set_events (area, g_value_get_pointer (a_val)) ;
      break ;
    case MAX_VISIBLE_EVENTS_PROP:
      today_events_area_set_max_visible_events (area, g_value_get_uint (a_val));
      break ;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (a_this, a_prop_id, a_pspec) ;
      break ;
  }
}

static void
clear_right_hand_side (TodayEventsArea *a_this)
{
  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  if (a_this->priv->right)
  {
    gtk_widget_destroy (a_this->priv->right) ;
    a_this->priv->right = NULL ;
  }
}

static void
init_right_hand_side (TodayEventsArea *a_this)
{
  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  if (a_this->priv->right)
    clear_right_hand_side (a_this) ;

  a_this->priv->right = gtk_vbox_new (TRUE, 0) ;
  gtk_widget_show (a_this->priv->right) ;
  gtk_table_attach_defaults (GTK_TABLE (a_this),
                             a_this->priv->right,
                             1, 2, 0, 1) ;
}

static void
clear_left_hand_side (TodayEventsArea *a_this)
{
  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  if (a_this->priv->left)
  {
    gtk_widget_destroy (a_this->priv->left) ;
    a_this->priv->left = NULL ;
    a_this->priv->left_event_box = NULL ;
    a_this->priv->paging_info = NULL ;
  }
}

static void
init_left_hand_side (TodayEventsArea *a_this)
{
  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  if (a_this->priv->left)
    clear_left_hand_side (a_this) ;

  a_this->priv->left_event_box = gtk_event_box_new ();

  gtk_widget_set_name (a_this->priv->left_event_box,
                       "today-events-area-postit-single");

  // FIXME: get this size from the style... somehow
  gtk_widget_set_size_request (a_this->priv->left_event_box, 51, 131);

  gtk_widget_add_events (a_this->priv->left_event_box,
                         GDK_BUTTON_PRESS_MASK) ;
  g_signal_connect (G_OBJECT (a_this->priv->left_event_box),
                    "button-press-event",
                    G_CALLBACK (on_button_pressed_in_left_cb),
                    a_this) ;
  a_this->priv->paging_info = gtk_label_new ("0/0") ;
  gtk_container_add (GTK_CONTAINER (a_this->priv->left_event_box),
                     a_this->priv->paging_info) ;

  a_this->priv->left = gtk_vbox_new (TRUE, 0) ;
  gtk_box_pack_start (GTK_BOX (a_this->priv->left),
                      a_this->priv->left_event_box,
                      FALSE, FALSE, 0) ;

  gtk_table_attach (GTK_TABLE (a_this),
                    a_this->priv->left,
                    0, 1, 0, 1,
                    GTK_FILL, GTK_FILL, 0, 0) ;
  gtk_widget_show_all (a_this->priv->left) ;
}

static void
reinit_area (TodayEventsArea *a_this)
{
  g_return_if_fail (a_this && TODAY_IS_EVENTS_AREA (a_this)) ;

  init_left_hand_side (a_this) ;
  init_right_hand_side (a_this) ;
}

static int
get_nb_events_real (TodayEventsArea *a_this)
{
  GList *cur = NULL ;
  int result = 0 ;

  g_return_val_if_fail (a_this
                        && TODAY_IS_EVENTS_AREA (a_this)
                        && a_this->priv,
                        -1) ;

  for (cur = a_this->priv->events ; cur ; cur = cur->next)
    ++result ;
  return result ;
}

static gboolean
update_paging_info (TodayEventsArea *a_this)
{
  gchar *str ;

  g_return_val_if_fail (a_this
                        && TODAY_IS_EVENTS_AREA (a_this)
                        && a_this->priv,
                        FALSE) ;

  g_return_val_if_fail (a_this->priv->paging_info, FALSE) ;
  g_return_val_if_fail (a_this->priv->cur_event, FALSE) ;
  g_return_val_if_fail (a_this->priv->events, FALSE) ;

  str = g_strdup_printf ("%d/%d",
                         a_this->priv->cur_event_index + 1,
                         a_this->priv->nb_events) ;

  gtk_label_set_text (GTK_LABEL (a_this->priv->paging_info), str) ;
  g_free (str) ;

  return TRUE ;
}

static GList*
select_event (TodayEventsArea *a_this,
              GList *a_event)
{
  int event_index = 0 ;

  g_return_val_if_fail (a_this
                        && TODAY_IS_EVENTS_AREA (a_this)
                        && a_this->priv,
                        FALSE) ;

  if (!a_this->priv->events || !a_event)
  {
    return NULL ;
  }
  event_index = g_list_position (a_this->priv->events,
                                 a_event) ;
  g_return_val_if_fail (event_index >= 0, NULL) ;

  a_this->priv->cur_event = a_event ;
  a_this->priv->cur_event_index = event_index ;

  /*
   * if the index of the current event is out of the range of the
   * currently visible page, move the range of the page and re-render the
   * page so that the current event becomes visible
   */
  if ((a_this->priv->page_start_index + a_this->priv->max_visible_events
      <= a_this->priv->cur_event_index)
      ||
      (a_this->priv->cur_event_index < a_this->priv->page_start_index))
  {
    render_events_page_auto (a_this) ;
  }

  /*update the left hand side page info label*/
  update_paging_info (a_this) ;

  return a_event;

}

static void
render_event (TodayEventsArea *a_this,
              GList *a_event)
{
  GtkWidget *infoline, *label, *event_box, *icon ;
  icaltimetype date ;
  icalcomponent *event=NULL ;
  int event_index=0 ;
  gchar *tmp_str=NULL, *tmp_str2=NULL, *summary=NULL ;
  gboolean has_alarm=FALSE, is_todo=FALSE, is_event=FALSE ;

  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv
                    && a_this->priv->right
                    && GTK_IS_BOX (a_this->priv->right)) ;

  g_return_if_fail (a_event && a_event->data) ;

  event = a_event->data ;
  g_return_if_fail (icalcomponent_isa_component (event)) ;

  event_index = g_list_position (a_this->priv->events, a_event) ;
  g_return_if_fail (event_index >= 0) ;

  /*does the comp has an alarm ?*/
  has_alarm = icalcomponent_has_alarm (event);

  /*is the comp a todo item ? */
  is_todo = (icalcomponent_isa (event) == ICAL_VTODO_COMPONENT);

  /*is the comp a calendar event ?*/
  is_event = (icalcomponent_isa (event) == ICAL_VEVENT_COMPONENT);

  /*a comp must be either a calendar event or a todo item*/
  g_return_if_fail (is_event != is_todo) ;

  /*get the event summary*/
  summary = (gchar*) icalcomponent_get_summary (event) ;

  /*get the event starting date*/
  if (is_event)
  {
    date = icalcomponent_get_dtstart (event) ;
    tmp_str = icaltime_to_pretty_string (&date) ;
  }
  else if (is_todo)
  {
    date = icalcomponent_get_due (event) ;
    tmp_str = icaltime_to_pretty_string (&date) ;
  }


  /*build event infoline*/
  if (tmp_str)
  {
    tmp_str2 = g_strdup_printf ("%s %s", summary, tmp_str) ;
    g_free (tmp_str) ;
  }
  else
  {
    tmp_str2 = g_strdup_printf ("%s", summary) ;
  }
  label = gtk_label_new (tmp_str2) ;
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0) ;
  gtk_widget_show (label) ;
  g_free (tmp_str2) ;
  infoline = gtk_hbox_new (TRUE, 0) ;
  gtk_box_set_homogeneous (GTK_BOX (infoline), FALSE) ;
  gtk_widget_show (infoline) ;
  gtk_box_pack_start (GTK_BOX (infoline), label, FALSE, FALSE, 0) ;
  icon = gtk_image_new () ;

  if (has_alarm)
  {
    gtk_image_set_from_stock (GTK_IMAGE (icon),
                              "openmoko-today-bell",
                              GTK_ICON_SIZE_MENU);
  }
  else if (is_event)
  {
    gtk_image_set_from_stock (GTK_IMAGE (icon),
                              "openmoko-today-event",
                              GTK_ICON_SIZE_MENU);
  }
  else if (is_todo)
  {
    gtk_image_set_from_stock (GTK_IMAGE (icon),
                              "openmoko-today-todo",
                              GTK_ICON_SIZE_MENU);
  }

  gtk_misc_set_alignment (GTK_MISC (icon), 0, 0);
  gtk_widget_show_all (icon) ;
  gtk_box_pack_start (GTK_BOX (infoline), icon, FALSE, FALSE, 6) ;
  event_box = gtk_event_box_new () ;
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box), FALSE) ;
  gtk_widget_show (event_box) ;
  g_object_set_data (G_OBJECT (event_box),
                     "event-index",
                     GINT_TO_POINTER (event_index)) ;
  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (on_button_pressed_in_infoline_cb),
                    a_this) ;
  gtk_container_add (GTK_CONTAINER (event_box), infoline) ;
  gtk_box_pack_start_defaults (GTK_BOX (a_this->priv->right), event_box) ;
}

static void
render_events_page (TodayEventsArea *a_this, GList *a_from)
{
  GList *cur = NULL ;
  int nb_rendered = 0 ;
  int prev_page_start_index = 0 ;

  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv
                    && a_this->priv->right
                    && GTK_IS_BOX (a_this->priv->right)) ;

  if (!a_this->priv->events)
    return ;

  prev_page_start_index = a_this->priv->page_start_index ;

  if (a_from)
  {
    a_this->priv->page_start = a_from ;
  }
  else
  {
    a_this->priv->page_start = a_this->priv->events ;
  }
  g_return_if_fail (a_this->priv->page_start) ;

  a_this->priv->page_start_index =
    g_list_position (a_this->priv->events, a_this->priv->page_start) ;

  init_right_hand_side (a_this) ;
  for (cur = a_this->priv->page_start ;
       cur && (nb_rendered < a_this->priv->max_visible_events);
       cur = cur->next, ++nb_rendered)
  {
    render_event (a_this, cur) ;
  }

  /*if we rendered a new page, tell the world about it*/
  if (a_this->priv->page_start_index != prev_page_start_index)
  {
    int page_num = (a_this->priv->page_start_index+1) %
                      a_this->priv->max_visible_events ;
    g_signal_emit (G_OBJECT (a_this), signals[PAGE_SWITCHED_SIGNAL], 0,
                   page_num) ;
  }
}

static void
render_events_page_auto (TodayEventsArea *a_this)
{
  int page_end = 0 ;
  int page_start = 0 ;
  GList *from = NULL ;

  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv) ;

  /*
   * compute a page start index so that the current event index
   * is inside [page_start page_end],
   * with page_end - page_start == max_visible_events
   */
  if (a_this->priv->cur_event_index > a_this->priv->page_start_index)
  {
    for (page_end = a_this->priv->max_visible_events ;
         page_end <= a_this->priv->cur_event_index;
         page_end += a_this->priv->max_visible_events)
    {
    }
    page_start = page_end - a_this->priv->max_visible_events ;
  }
  else
  {
    for (page_start = 0 ;
         page_start +  a_this->priv->max_visible_events <=
         a_this->priv->cur_event_index;
         page_start += a_this->priv->max_visible_events)
    {
    }
  }

  /*now we have a decent page range. We can just render the page*/
  from = g_list_nth (a_this->priv->events, page_start) ;
  g_return_if_fail (from) ;
  render_events_page (a_this, from) ;
}


/**********************
 * </private api>
 **********************/

/**********************
 * <public api>
 **********************/

GtkWidget*
today_events_area_new ()
{
  GObject *result;
  result = g_object_new (TODAY_TYPE_EVENTS_AREA,
                         "max-visible-events", 4,
                         NULL) ;
  return GTK_WIDGET (result);
}

GtkWidget*
today_events_area_new_with_events (GList *a_events)
{
  GObject *result ;

  result = g_object_new (TODAY_TYPE_EVENTS_AREA,
                         "max-visible-events", 4,
                         "events", a_events,
                         NULL) ;
  return GTK_WIDGET (result) ;
}

/**
 *today_events_area_set_events:
 *@a_this: current instance of TodayEventsArea
 *@a_events: the events to set. Once set, the events belong to
 *the current instance of EventsArea and will be freed by it
 */
void
today_events_area_set_events (TodayEventsArea *a_this,
                              GList *a_events)
{
  g_return_if_fail (a_this && TODAY_IS_EVENTS_AREA (a_this)) ;
  g_return_if_fail (a_this->priv) ;

  if (a_this->priv->events)
    e_cal_free_object_list (a_this->priv->events) ;

  a_this->priv->events = a_events ;
  a_this->priv->nb_events = get_nb_events_real (a_this) ;
  a_this->priv->cur_event = NULL ;
  a_this->priv->page_start = NULL ;
  a_this->priv->cur_event_index = 0 ;

  reinit_area (a_this) ;

  if (!a_this->priv->events)
    return ;

  today_events_area_select_next_event (a_this) ;

  /*
   * walk the events and render them
   * on the right hand side
   * of the event area
   */
  render_events_page (a_this, a_events) ;
  update_paging_info (a_this) ;
  g_signal_emit (G_OBJECT (a_this), signals[EVENTS_ADDED_SIGNAL], 0,
                 a_events) ;
}

GList*
today_events_area_get_events (TodayEventsArea *a_this)
{
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        NULL);
  return a_this->priv->events ;
}

int
today_events_area_get_nb_events (TodayEventsArea *a_this)
{
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        -1);
  return a_this->priv->nb_events ;
}

int
today_events_area_get_nb_pages (TodayEventsArea *a_this)
{
  int res = 0 ;
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        -1);

  if (!a_this->priv->max_visible_events)
    return 0 ;

  res = a_this->priv->nb_events / a_this->priv->max_visible_events ;
  if (a_this->priv->nb_events % a_this->priv->max_visible_events)
    ++res ;
  return res ;
}

ECalComponent*
today_events_area_get_cur_event (TodayEventsArea *a_this)
{
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        NULL);

  return a_this->priv->cur_event->data ;
}

int
today_events_area_get_cur_event_index (TodayEventsArea *a_this)
{
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        -1);
  return a_this->priv->cur_event_index ;
}

ECalComponent*
today_events_area_get_event_from_index (TodayEventsArea *a_this,
                                        int a_index)
{
  GList *elem ;
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        NULL);
  g_return_val_if_fail (a_this->priv->events, NULL) ;

  elem = g_list_nth (a_this->priv->events, a_index) ;
  if (elem)
  {
    return elem->data ;
  }
  return NULL ;
}

ECalComponent*
today_events_area_select_next_event (TodayEventsArea *a_this)
{
  GList *result = NULL ;

  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        NULL);

  if (!a_this->priv->events)
  {
    a_this->priv->cur_event = NULL ;
    a_this->priv->cur_event_index = -1 ;
    goto out ;
  }

  if (!a_this->priv->cur_event)
  {
    result = select_event (a_this, a_this->priv->events) ;
    goto out ;
  }

  if (a_this->priv->cur_event->next)
  {
    result = select_event (a_this, a_this->priv->cur_event->next) ;
  }
  else
  {
    /*reached end of events, cycle to the first event in list*/
    result = select_event (a_this, a_this->priv->events) ;
  }

out:
  if (!result)
    return NULL ;
  return result->data ;
}

void
today_events_area_set_max_visible_events (TodayEventsArea *a_this,
                                          int a_max)
{
  g_return_if_fail (a_this &&
                    TODAY_IS_EVENTS_AREA (a_this) &&
                    a_this->priv);

  a_this->priv->max_visible_events = a_max ;
}

int
today_events_area_get_max_visible_events (TodayEventsArea *a_this)
{
  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        -1);

  return a_this->priv->max_visible_events ;
}

void
today_events_area_switch_to_next_page (TodayEventsArea *a_this)
{
  GList *event_elem ;
  int event_index ;

  g_return_if_fail (a_this &&
                    TODAY_IS_EVENTS_AREA (a_this) &&
                    a_this->priv);
  g_return_if_fail (a_this->priv->events) ;

  if (a_this->priv->nb_events <= a_this->priv->max_visible_events)
    return ;

  event_index = a_this->priv->cur_event_index ;
  event_index += a_this->priv->max_visible_events ;

  event_elem = g_list_nth (a_this->priv->events, event_index) ;
  if (!event_elem)
    event_elem = a_this->priv->events ;

  select_event (a_this, event_elem) ;
}

void
today_events_area_switch_to_prev_page (TodayEventsArea *a_this)
{
  GList *event_elem ;
  int event_index ;

  g_return_if_fail (a_this &&
                    TODAY_IS_EVENTS_AREA (a_this) &&
                    a_this->priv);
  g_return_if_fail (a_this->priv->events) ;

  if (a_this->priv->nb_events <= a_this->priv->max_visible_events)
    return ;

  event_index = a_this->priv->cur_event_index ;
  if (event_index > a_this->priv->max_visible_events)
    event_index -= a_this->priv->max_visible_events ;
  else
    event_index = a_this->priv->nb_events - a_this->priv->max_visible_events-1;
  event_index = MAX (event_index, 0) ;

  event_elem = g_list_nth (a_this->priv->events, event_index) ;
  g_return_if_fail (event_elem) ;

  select_event (a_this, event_elem) ;
}

/**********************
 * </public api>
 **********************/

