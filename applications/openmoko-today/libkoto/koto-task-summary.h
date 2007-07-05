/*
 * Copyright (C) 2007 Ross Burton <ross@openedhand.com>
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

#ifndef _KOTO_TASK_SUMMARY
#define _KOTO_TASK_SUMMARY

#include <gtk/gtkscrolledwindow.h>

G_BEGIN_DECLS

#define KOTO_TYPE_TASK_SUMMARY koto_task_summary_get_type()

#define KOTO_TASK_SUMMARY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_TASK_SUMMARY, KotoTaskSummary))

#define KOTO_TASK_SUMMARY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_TASK_SUMMARY, KotoTaskSummaryClass))

#define KOTO_IS_TASK_SUMMARY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_TASK_SUMMARY))

#define KOTO_IS_TASK_SUMMARY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_TASK_SUMMARY))

#define KOTO_TASK_SUMMARY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_TASK_SUMMARY, KotoTaskSummaryClass))

typedef struct {
  GtkScrolledWindow parent;
} KotoTaskSummary;

typedef struct {
  GtkScrolledWindowClass parent_class;
} KotoTaskSummaryClass;

GType koto_task_summary_get_type (void);

GtkWidget* koto_task_summary_new (void);

G_END_DECLS

#endif /* _KOTO_TASK_SUMMARY */
