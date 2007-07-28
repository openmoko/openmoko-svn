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
#include <libedataserver/e-data-server-util.h>
#include "ical-util.h"
#include "koto-cell-renderer-pixbuf.h"
#include "koto-platform.h"
#include "koto-task-view.h"

G_DEFINE_TYPE (KotoTaskView, koto_task_view, GTK_TYPE_TREE_VIEW);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_TASK_VIEW, KotoTaskViewPrivate))

typedef struct {
  KotoTaskStore *store;
  KotoGroupFilterModel *filter;
  
  GdkColor colour_done, colour_low, colour_normal, colour_high;
} KotoTaskViewPrivate;

enum {
  PROP_0,
  PROP_BASE_MODEL,
  PROP_FILTER,
};

/*
 * Private methods.
 */

/*
 * Toggle callback for the Done column.
 */
static void
on_done_toggled (GtkCellRendererToggle *cell_renderer, gchar *path,  KotoTaskView *view)
{
  KotoTaskViewPrivate *priv = GET_PRIVATE (view);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean done;
  
  /* Get the model used in the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));
  /* Convert the path to an iterator on the view model */
  if (gtk_tree_model_get_iter_from_string (model, &iter, path)) {
    /* If there is a filter, convert the iterator to a child iterator */
    if (priv->filter) {
      GtkTreeIter real_iter;
      gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER (priv->filter), &real_iter, &iter);
      iter = real_iter;
    }
    /* At this point we know the iterator is valid on the base model */
    gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, COLUMN_DONE, &done, -1);
    koto_task_store_set_done (priv->store, &iter, !done);
  }
}

/*
 * Cell data function for the summary column.
 */
static void
summary_func (GtkTreeViewColumn *tree_column,
              GtkCellRenderer   *cell,
              GtkTreeModel      *model,
              GtkTreeIter       *iter,
              gpointer           user_data)
{
  GtkWidget *treeview = user_data;
  KotoTaskViewPrivate *priv;
  gboolean done;
  char *summary;
  int weight, font_weight;
  GDate *due;
  GdkColor *colour;

  g_assert (treeview);
  
  priv = GET_PRIVATE (treeview);
  
  gtk_tree_model_get (model, iter,
                      COLUMN_WEIGHT, &weight,
                      COLUMN_DONE, &done,
                      COLUMN_SUMMARY, &summary,
                      COLUMN_DUE, &due,
                      -1);

  if (due) {
    char *temp, *date;
    
    date = ical_util_get_human_date (due);
    
    temp = g_strdup_printf (_("%s (due %s)"), summary, date);

    g_free (summary);
    g_free (date);
    g_date_free (due);

    summary = temp;
  }

  if (done) {
    colour = &priv->colour_done;
  } else if (weight > PRIORITY_MEDIUM) {
    colour = &priv->colour_low;
  } else if ( weight < PRIORITY_MEDIUM) {
    colour = &priv->colour_high;
  } else {
    colour = &priv->colour_normal;
  }

  if (done) {
    font_weight = PANGO_WEIGHT_NORMAL;
  } else {
    font_weight = weight < PRIORITY_MEDIUM
      ? PANGO_WEIGHT_BOLD
      : PANGO_WEIGHT_NORMAL;
  }
  
  g_object_set (cell,
                /* The text to display */
                "text", summary,

                /* Strike out done tasks */
                "strikethrough", done,

                /* The colour from the style */
                "foreground-gdk",
                colour,
                
                /* If important, bolden */
                "weight",
                font_weight,

                NULL);
  
  g_free (summary);
}

static void
icon_func (GtkTreeViewColumn *column, GtkCellRenderer *cell,
           GtkTreeModel *model, GtkTreeIter *iter,
           gpointer user_data)
{
  char *url = NULL;

  gtk_tree_model_get (model, iter, COLUMN_URL, &url, -1);

  g_object_set (cell, "visible", url ? TRUE : FALSE, NULL);

  g_free (url);
}

/*
 * Custom function for interactive searches.
 */
static gboolean
search_equal_func (GtkTreeModel *model, gint column,
                   const gchar *key, GtkTreeIter *iter, gpointer search_data)
{
  char *summary = NULL;
  gboolean found;
  
  gtk_tree_model_get (model, iter, COLUMN_SUMMARY, &summary, -1);

  if (!summary)
    return FALSE;

  found = e_util_utf8_strstrcasedecomp (summary, key) != NULL;
  
  g_free (summary);
  
  /* GtkTreeView is insane */
  return ! found;
}

/*
 * Callback when the URL icon is clicked, to start a web browser.
 */
static void
on_url_activated (KotoCellRendererPixbuf *cell, const char *path, GtkTreeView *view)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  char *url = NULL;
  
  model = gtk_tree_view_get_model (view);

  if (!gtk_tree_model_get_iter_from_string (model, &iter, path))
    return;

  gtk_tree_model_get (model, &iter, COLUMN_URL, &url, -1);
  
  if (url) {
    koto_platform_open_url (url);
    g_free (url);
  }
}

/*
 * Object methods.
 */

static void
koto_task_view_style_set (GtkWidget *widget, GtkStyle *previous)
{
  KotoTaskViewPrivate *priv;
  
  g_assert (KOTO_IS_TASK_VIEW (widget));

  priv = GET_PRIVATE (widget);

#if HAVE_DECL_GTK_STYLE_LOOKUP_COLOR
  if (!gtk_style_lookup_color (widget->style, "priority-low", &priv->colour_low)) {
    priv->colour_low = widget->style->text[GTK_STATE_INSENSITIVE];
  }
  if (!gtk_style_lookup_color (widget->style, "priority-normal", &priv->colour_normal)) {
    priv->colour_normal = widget->style->text[GTK_STATE_NORMAL];
  }
  if (!gtk_style_lookup_color (widget->style, "priority-high", &priv->colour_high)) {
    priv->colour_high = widget->style->text[GTK_STATE_NORMAL];
  }
  if (!gtk_style_lookup_color (widget->style, "priority-done", &priv->colour_done)) {
    priv->colour_done = widget->style->text[GTK_STATE_INSENSITIVE];
  }
#else
  priv->colour_low = widget->style->text[GTK_STATE_INSENSITIVE];
  priv->colour_normal = widget->style->text[GTK_STATE_NORMAL];
  priv->colour_high = widget->style->text[GTK_STATE_NORMAL];
  priv->colour_done = widget->style->text[GTK_STATE_INSENSITIVE];
#endif

  GTK_WIDGET_CLASS (koto_task_view_parent_class)->style_set (widget, previous);
}


static void
koto_task_view_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoTaskViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_FILTER:
    g_value_set_object (value, priv->filter);
    break;
  case PROP_BASE_MODEL:
    g_value_set_object (value, priv->store);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_view_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  KotoTaskViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_FILTER:
    if (priv->filter) {
      g_object_unref (priv->filter);
    }
    priv->filter = KOTO_GROUP_MODEL_FILTER (g_value_dup_object (value));
    break;
  case PROP_BASE_MODEL:
    if (priv->store) {
      g_object_unref (priv->store);
    }
    priv->store = KOTO_TASK_STORE (g_value_dup_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_view_dispose (GObject *object)
{
  KotoTaskViewPrivate *priv = GET_PRIVATE (object);

  if (priv->filter) {
    g_object_unref (priv->filter);
    priv->filter = NULL;
  }

  if (priv->store) {
    g_object_unref (priv->store);
    priv->store = NULL;
  }

  if (G_OBJECT_CLASS (koto_task_view_parent_class)->dispose)
    G_OBJECT_CLASS (koto_task_view_parent_class)->dispose (object);
}

static void
koto_task_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (koto_task_view_parent_class)->finalize (object);
}

static void
koto_task_view_class_init (KotoTaskViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoTaskViewPrivate));

  object_class->get_property = koto_task_view_get_property;
  object_class->set_property = koto_task_view_set_property;
  object_class->dispose = koto_task_view_dispose;
  object_class->finalize = koto_task_view_finalize;

  widget_class->style_set = koto_task_view_style_set;

  g_object_class_install_property (object_class, PROP_BASE_MODEL,
                                   g_param_spec_object ("base-model", "Base model", NULL,
                                                        KOTO_TYPE_TASK_STORE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class, PROP_FILTER,
                                   g_param_spec_object ("filter", "Filter", NULL,
                                                        KOTO_TYPE_GROUP_MODEL_FILTER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void
koto_task_view_init (KotoTaskView *self)
{
  GtkTreeView *treeview;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  treeview = GTK_TREE_VIEW (self);

  gtk_tree_view_set_rules_hint (treeview, TRUE);
  gtk_tree_view_set_headers_visible (treeview, FALSE);
  gtk_tree_view_set_enable_search (treeview, TRUE);
  gtk_tree_view_set_search_column (treeview, COLUMN_SUMMARY);
  gtk_tree_view_set_search_equal_func (treeview, search_equal_func, NULL, NULL);
  
  /* Done column */
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled", G_CALLBACK (on_done_toggled), self);
  column = gtk_tree_view_column_new_with_attributes (_("Done"), renderer,
                                                     "active", COLUMN_DONE,
                                                     NULL);
  gtk_tree_view_append_column (treeview, column);

  /* Summary column */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Summary"),
                                                     renderer, 
                                                     NULL);
  g_object_set (renderer,
                "ellipsize", PANGO_ELLIPSIZE_END,
                NULL);
  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           summary_func, treeview, NULL);

  /* Icon column */
  renderer = koto_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "icon-name", "stock_internet", NULL);
  g_signal_connect (renderer, "activated", G_CALLBACK (on_url_activated), self);
  gtk_tree_view_column_pack_end (column, renderer, FALSE);
  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           icon_func, treeview, NULL);
  
  gtk_tree_view_append_column (treeview, column);
}


/*
 * Public methods.
 */

GtkWidget *
koto_task_view_new (KotoTaskStore *store, KotoGroupFilterModel *filter)
{
  return g_object_new (KOTO_TYPE_TASK_VIEW,
                       "model", filter ? (GtkTreeModel*)filter : (GtkTreeModel*)store,
                       "base-model", store,
                       "filter", filter,
                       NULL);
}

/**
 * koto_task_view_get_selected_task:
 * @view: A #KotoTaskView.
 *
 * Return the currently selected #KotoTask, or #NULL if there is now row
 * selected.  When finished with the #KotoTask, unref it with koto_task_unref().
 *
 * Returns: a #KotoTask with its reference count incremented, or #NULL.
 */
KotoTask *
koto_task_view_get_selected_task (KotoTaskView *view)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  KotoTask *task = NULL;
  
  g_return_val_if_fail (KOTO_IS_TASK_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
    return NULL;
  }
  
  gtk_tree_model_get (model, &iter, COLUMN_ICAL, &task, -1);

  return task;
}

gboolean
koto_task_view_get_selected_iter (KotoTaskView *view, GtkTreeIter *iter)
{
  GtkTreeSelection *selection;
  
  g_return_val_if_fail (KOTO_IS_TASK_VIEW (view), FALSE);
  g_return_val_if_fail (iter, FALSE);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  
  return gtk_tree_selection_get_selected (selection, NULL, iter);
}
  
