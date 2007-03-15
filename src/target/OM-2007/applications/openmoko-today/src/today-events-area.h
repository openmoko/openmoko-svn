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

#ifndef __TODAY_EVENTS_AREA_H__
#define __TODAY_EVENTS_AREA_H__

#include <libecal/e-cal-component.h>
#include <gtk/gtktable.h>

G_BEGIN_DECLS

#define TODAY_TYPE_EVENTS_AREA (today_events_area_get_type ())
#define TODAY_EVENTS_AREA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TODAY_TYPE_EVENTS_AREA, TodayEventsArea))
#define TODAY_EVENTS_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TODAY_TYPE_EVENTS_AREA, TodayEventsAreaClass))
#define TODAY_IS_EVENTS_AREA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TODAY_TYPE_EVENTS_AREA))
#define TODAY_IS_EVENTS_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TODAY_TYPE_EVENTS_AREA))
#define TODAY_EVENTS_AREA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), TODAY_TYPE_EVENTS_AREATodayEventsAreaClass))

typedef struct _TodayEventsAreaPrivate TodayEventsAreaPrivate;
typedef struct _TodayEventsAreaClass TodayEventsAreaClass;
typedef struct _TodayEventsArea TodayEventsArea;

struct _TodayEventsArea {
  GtkTable table;
  TodayEventsAreaPrivate *priv;
};

struct _TodayEventsAreaClass {
  GtkTableClass parent_class;

  /*
   * <noticeable events>
   */
  void (*events_added) (TodayEventsArea *self, GList *events) ;
  void (*event_selected) (TodayEventsArea *self, guint event_index) ;
  void (*page_switched) (TodayEventsArea *self, guint page_index) ;
  /*
   * </noticeable events>
   */
};

GType          today_events_area_get_type (void);
GtkWidget*     today_events_area_new ();
void           today_events_area_set_events (TodayEventsArea *self,
                                             GList *a_events);
GList*         today_events_area_get_events (TodayEventsArea *self);
int            today_events_area_get_nb_events (TodayEventsArea *self);
int            today_events_area_get_nb_pages (TodayEventsArea *self);
ECalComponent* today_events_area_get_cur_event (TodayEventsArea *self);
int            today_events_area_get_cur_event_index (TodayEventsArea *self);
ECalComponent* today_events_area_get_event_from_index (TodayEventsArea *self,
                                                       int index) ;
ECalComponent* today_events_area_select_next_event (TodayEventsArea *self) ;
void           today_events_area_set_max_visible_events (TodayEventsArea *self,
                                                         int max) ;
int            today_events_area_get_max_visible_events (TodayEventsArea *self);
void           today_events_area_switch_to_next_page (TodayEventsArea *self) ;
void           today_events_area_switch_to_prev_page (TodayEventsArea *self) ;

G_END_DECLS

#endif /*__TODAY_EVENTS_AREA_H__*/

