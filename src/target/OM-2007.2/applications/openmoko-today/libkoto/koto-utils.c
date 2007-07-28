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

#include <config.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "koto-task-store.h"
#include "koto-utils.h"

typedef struct {
  GtkWindow *window;
  char *title;
} WindowData;

/*
 * Foreach function called from update_title() that simply counts the number of
 * uncompleted tasks.  @userdata is an int* to the count.
 */
static gboolean
count_pending (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  int *count = data;
  gboolean done;
  KotoTask *task;

  gtk_tree_model_get (model, iter, COLUMN_DONE, &done,
                      COLUMN_ICAL, &task,
                      -1);

  /*
   * We need to check for a NULL task because this is called in the row-deleted
   * callback, which is called before the row is actually removed from the
   * model.  When this happens there is a dummy row with no data, which appears
   * to be incomplete.
   */
  if (!task)
    return FALSE;

  koto_task_unref (task);
  
  if (!done)
    (*count)++;

  /*
   * The store is always sorted, so we can stop counting when we find the first
   * completed task.
   */
  return done;
}

/*
 * Update the window title, generally as the number of tasks has changed.
 */
static void
update_title (WindowData *data, GtkTreeModel *model)
{
  int count = 0;
  char *title;

  g_assert (data);
  g_assert (model);

  gtk_tree_model_foreach (model, count_pending, &count);
  title = g_strdup_printf (data->title, count);
  gtk_window_set_title (data->window, title);
  g_free (title);
}

/*
 * Callback when rows are inserted into the task store, to update the window
 * title.
 */
static void
on_row_inserted (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, WindowData *data)
{
  update_title (data, model);
}

/*
 * Callback when rows are removed from the task store, to update the window
 * title.
 */
static void
on_row_deleted (GtkTreeModel *model, GtkTreePath *path, WindowData *data)
{
  update_title (data, model);
}

/*
 * Weak notify for WindowData, to free it when the window is destroyed.
 */
static void
on_weak_notify (gpointer user_data, GObject *dead)
{
  WindowData *data = user_data;
  g_free (data->title);
  g_slice_free (WindowData, data);
}

/**
 * koto_sync_window_title:
 * @window: a #GtkWindow title to set
 * @model: a #GtkTreeModel to count tasks from
 * @title: an untranslated format string to use as the title
 *
 * Synchronise the title of @window with then number of incomplete tasks in
 * @model.  @model must be a #KotoTaskStore, or a filter model based on
 * #KotoTaskStore.  @title should be untranslated and contain a single %d
 * format, which is the number of incomplete tasks.
 */
void
koto_sync_window_title (GtkWindow *window, GtkTreeModel *model, const char *title)
{
  WindowData *data;
  
  g_return_if_fail (GTK_WINDOW (window));
  g_return_if_fail (GTK_TREE_MODEL (model));
  g_return_if_fail (title);
  
  data = g_slice_new (WindowData);
  data->window = window;
  data->title = g_strdup (title);
  
  g_object_weak_ref (G_OBJECT (model), on_weak_notify, data);

  g_object_connect (model,
                    "signal::row-inserted", G_CALLBACK (on_row_inserted), data,
                    "signal::row-changed", G_CALLBACK (on_row_inserted), data,
                    "signal::row-deleted", G_CALLBACK (on_row_deleted), data,
                    NULL);

  update_title (data, model);
}
