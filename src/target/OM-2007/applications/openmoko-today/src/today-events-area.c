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
#include <libecal/e-cal-time-util.h>
#include <libecal/e-cal-component.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtklabel.h>
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
static void     e_cal_component_list_free   (GList * list) ;
static gchar*   icaltime_to_pretty_string   (const icaltimetype *timetype) ;
static int      get_list_elem_index         (GList *a_elem) ;

G_DEFINE_TYPE (TodayEventsArea, today_events_area, GTK_TYPE_TABLE)

static void
today_events_area_class_init (TodayEventsAreaClass *a_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (a_class);
  object_class->finalize = today_events_area_finalize;
  g_type_class_add_private (object_class, sizeof (TodayEventsAreaPrivate));
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
    e_cal_component_list_free (self->priv->events) ;
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
      today_events_area_select_next_event (a_area) ;

    return FALSE ;
}

gboolean
on_button_pressed_in_infoline_cb (GtkWidget *a_event_box,
                                  GdkEventButton *a_button,
                                  TodayEventsArea *a_area)
{
  int event_index = 0 ;
  GList *event_elem = NULL ;

  g_return_val_if_fail (a_area && TODAY_IS_EVENTS_AREA (a_area), FALSE) ;

  if (a_button->type != GDK_BUTTON_PRESS)
    return FALSE ;

  event_index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (a_event_box),
                                                    "event-index")) ;
  event_elem = g_list_nth (a_area->priv->events, event_index) ;
  select_event (a_area, event_elem) ;

  return FALSE;
}

/********************
 * </signal callbacks>
 ********************/

/**********************
 * <private api>
 **********************/

/**
 * e_cal_component_list_free:
 * @list: the list ECalComooment to free
 *
 * Free a list of ECalComponent
 */
static void
e_cal_component_list_free (GList * a_list)
{
  GList *cur = NULL;

  for (cur = a_list; cur; cur = cur->next)
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
  g_list_free (a_list);
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

  gtk_table_attach_defaults (GTK_TABLE (a_this),
                             a_this->priv->left,
                             0, 1, 0, 1) ;
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
  event_index = get_list_elem_index (a_event) ;
  g_return_val_if_fail (event_index >= 0, NULL) ;

  a_this->priv->cur_event = a_event ;
  a_this->priv->cur_event_index = event_index ;

  if ((a_this->priv->page_start_index + a_this->priv->max_visible_events
      <= a_this->priv->cur_event_index)
      ||
      (a_this->priv->cur_event_index < a_this->priv->page_start_index))
  {
    render_events_page_auto (a_this) ;
  }

  update_paging_info (a_this) ;

  return a_event;

}

/*
 * if the timetype is today, then only display it's hour part,
 * without the seconds
 * If it's not today, then only display it's date part, without the year
 */
static gchar*
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

static int
get_list_elem_index (GList *a_elem)
{
  int nb = -1 ;
  GList *cur = NULL ;

  for (cur = a_elem ; cur ; cur = cur->prev)
    ++nb ;

  return nb ;
}

static void
render_event (TodayEventsArea *a_this,
              GList *a_event)
{
  GtkWidget *infoline ;
  GtkWidget *label ;
  GtkWidget *event_box ;
  ECalComponentText text ;
  ECalComponentDateTime start_date ;
  ECalComponent *event ;
  int event_index ;
  gchar *tmp_str, *tmp_str2 ;

  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv
                    && a_this->priv->right
                    && GTK_IS_BOX (a_this->priv->right)) ;

  g_return_if_fail (a_event && a_event->data) ;

  event = a_event->data ;
  g_return_if_fail (E_IS_CAL_COMPONENT (event)) ;
  event_index = get_list_elem_index (a_event) ;
  g_return_if_fail (event_index >= 0) ;

  /*get the event summary*/
  e_cal_component_get_summary (event, &text) ;

  /*get the event starting date*/
  e_cal_component_get_dtstart (event, &start_date) ;
  tmp_str = icaltime_to_pretty_string (start_date.value) ;
  e_cal_component_free_datetime (&start_date) ;

  /*build event infoline*/
  tmp_str2 = g_strdup_printf ("%s %s", text.value, tmp_str) ;
  g_free (tmp_str) ;
  label = gtk_label_new (tmp_str2) ;
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0) ;
  gtk_widget_show (label) ;
  g_free (tmp_str2) ;
  infoline = gtk_hbox_new (TRUE, 0) ;
  gtk_widget_show (infoline) ;
  gtk_box_pack_start_defaults (GTK_BOX (infoline), label) ;
  event_box = gtk_event_box_new () ;
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

  g_return_if_fail (a_this
                    && TODAY_IS_EVENTS_AREA (a_this)
                    && a_this->priv
                    && a_this->priv->right
                    && GTK_IS_BOX (a_this->priv->right)) ;

  if (!a_this->priv->events)
    return ;

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
    get_list_elem_index (a_this->priv->page_start) ;

  init_right_hand_side (a_this) ;
  for (cur = a_this->priv->page_start ;
       cur && (nb_rendered < a_this->priv->max_visible_events);
       cur = cur->next, ++nb_rendered)
  {
    render_event (a_this, cur) ;
  }
}

static void
render_events_page_auto (TodayEventsArea *a_this)
{
  int page_end=0 ;
  int page_start=0 ;
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
  result = g_object_new (TODAY_TYPE_EVENTS_AREA, NULL) ;
  /*provide gobject param getter/setter for this*/
  today_events_area_set_max_visible_events (TODAY_EVENTS_AREA (result), 4) ;
  return GTK_WIDGET (result);
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
    e_cal_component_list_free (a_this->priv->events) ;

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

ECalComponent*
today_events_area_goto_next_page (TodayEventsArea *a_this)
{
  int start_index = 0 ;
  GList *page_start = NULL, *event = NULL;

  g_return_val_if_fail (a_this &&
                        TODAY_IS_EVENTS_AREA (a_this) &&
                        a_this->priv,
                        NULL);

  start_index = get_list_elem_index (a_this->priv->page_start) ;
  start_index += a_this->priv->max_visible_events ;
  page_start = g_list_nth (a_this->priv->events, start_index) ;
  if (!page_start)
    page_start = a_this->priv->events ;
  event = select_event (a_this, page_start) ;
  if (!event)
    return NULL ;
  return event->data ;
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

/**********************
 * </public api>
 **********************/

