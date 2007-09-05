
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libkoto/koto-task.h>
#include <libkoto/koto-task-view.h>
#include <libtaku/launcher-util.h>
#include "today-utils.h"
#include "today-pim-summary.h"
#include "today-events-store.h"
#include "today-tasks-store.h"
#include "today.h"

/**
 * today_update_date ()
 *
 * Update the specified GtkTreeViewColumn with the current date
 */
static gboolean
today_pim_summary_update_date (GtkTreeViewColumn *column)
{
	time_t t;
	struct tm *tmp;
	gchar date_str[64];

	time (&t);
	if (!(tmp = localtime (&t))) return TRUE;

	strftime (date_str, sizeof (date_str), "%A, %d. %B %Y", tmp);
	gtk_tree_view_column_set_title (column, date_str);
	
	return TRUE;
}

static gboolean
today_pim_summary_visible_cb (GtkTreeModel *model, GtkTreeIter *iter,
			      gpointer user_data)
{
	KotoTask *task;
	gboolean done;
	
	gtk_tree_model_get (model, iter, COLUMN_TASK, &task,
		COLUMN_DONE, &done, -1);
	
	if (!done) return TRUE;
	
	if (icaltime_compare (icalcomponent_get_dtstamp (task->comp),
	    icaltime_today ()) >= 0) return TRUE;
	
	return FALSE;
}

static void
today_pim_summary_show_notice (TodayData *data)
{
	/* Add notice */
	data->notice_visible = TRUE;
	gtk_list_store_insert_with_values (
		GTK_LIST_STORE (data->events_model),
		&data->notice, 0,
		TODAY_EVENTS_STORE_COL_COMP, NULL,
		TODAY_EVENTS_STORE_COL_UID, NULL,
		TODAY_EVENTS_STORE_COL_SUMMARY,
		_("No pending events or tasks"),
		-1);
}

static void
today_pim_summary_row_inserted_cb (GtkTreeModel *tree_model,
				   GtkTreePath *path, GtkTreeIter *iter,
				   gpointer user_data)
{
	TodayData *data = (TodayData *)user_data;
	data->rows ++;
	if ((data->notice_visible) && (data->rows == 2)) {
		/* Remove notice */
		data->notice_visible = FALSE;
		gtk_list_store_remove (GTK_LIST_STORE (data->events_model),
			&data->notice);
	}
}

static void
today_pim_summary_row_deleted_cb (GtkTreeModel *tree_model,
				  GtkTreePath *path, gpointer user_data)
{
	TodayData *data = (TodayData *)user_data;
	data->rows --;
	if ((data->rows < 1) && (!data->notice_visible)) {
		today_pim_summary_show_notice (data);
	}
}

static void
today_pim_summary_cell_data_cb (GtkTreeViewColumn *tree_column,
				GtkCellRenderer *cell, GtkTreeModel *tree_model,
				GtkTreeIter *iter, gpointer user_data)
{
	gboolean done;
	gchar *summary;
	
	gtk_tree_model_get (tree_model, iter, COLUMN_DONE, &done,
		COLUMN_SUMMARY, &summary, -1);

	g_object_set (G_OBJECT (cell), "text", summary,
		"strikethrough", done, NULL);
}

static void
today_pim_summary_header_clicked_cb (GtkTreeViewColumn *column, TodayData *data)
{
	g_debug ("TODO: App to set time/date");
}

static void
today_pim_summary_events_selection_changed_cb (GtkTreeSelection *selection,
					       TodayData *data)
{
	if (gtk_tree_selection_count_selected_rows (selection)) {
		gtk_tree_selection_unselect_all (selection);
		launcher_start (data->window, today_get_launcher (
			"openmoko-dates", TRUE, TRUE));
	}
}

static void
today_pim_summary_tasks_selection_changed_cb (GtkTreeSelection *selection,
					       TodayData *data)
{
	if (gtk_tree_selection_count_selected_rows (selection)) {
		gtk_tree_selection_unselect_all (selection);
		launcher_start (data->window, today_get_launcher (
			"openmoko-tasks", TRUE, TRUE));
	}
}

GtkWidget *
today_pim_summary_box_new (TodayData *data)
{
	GtkTreeModel *tasks_model;
	GtkWidget *vbox, *events_tree, *tasks_tree;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	data->rows = 0;
	data->events_model = GTK_TREE_MODEL (today_events_store_new ());
	tasks_model = GTK_TREE_MODEL (today_tasks_store_new ());
	data->tasks_model = gtk_tree_model_filter_new (tasks_model, NULL);
	events_tree = gtk_tree_view_new_with_model (data->events_model);
	tasks_tree = gtk_tree_view_new_with_model (data->tasks_model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (events_tree), FALSE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tasks_tree), FALSE);
	gtk_widget_show (events_tree);
	gtk_widget_show (tasks_tree);
	
	gtk_tree_model_filter_set_visible_func (
		GTK_TREE_MODEL_FILTER (data->tasks_model),
		today_pim_summary_visible_cb, NULL, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		"ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Tasks",
		renderer, "text", COLUMN_SUMMARY, NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tasks_tree),
		column, 0);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tasks_tree),
		FALSE);
	gtk_tree_view_column_set_cell_data_func (column, renderer,
		today_pim_summary_cell_data_cb,
		GTK_TREE_VIEW (tasks_tree), NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		"ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Events",
		renderer, "text", TODAY_EVENTS_STORE_COL_SUMMARY, NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (events_tree), column, 0);	
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (events_tree), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (events_tree), TRUE);

	today_pim_summary_update_date (column);
	g_timeout_add (60 * 1000, (GSourceFunc)
		today_pim_summary_update_date, column);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), events_tree, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), tasks_tree, FALSE, TRUE, 0);
	
	g_signal_connect (G_OBJECT (data->events_model), "row-inserted",
		G_CALLBACK (today_pim_summary_row_inserted_cb), data);
	g_signal_connect (G_OBJECT (tasks_model), "row-inserted",
		G_CALLBACK (today_pim_summary_row_inserted_cb), data);
	g_signal_connect (G_OBJECT (data->events_model), "row-deleted",
		G_CALLBACK (today_pim_summary_row_deleted_cb), data);
	g_signal_connect (G_OBJECT (tasks_model), "row-deleted",
		G_CALLBACK (today_pim_summary_row_deleted_cb), data);
	
	g_signal_connect (G_OBJECT (column), "clicked",
		G_CALLBACK (today_pim_summary_header_clicked_cb), data);
	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (
		GTK_TREE_VIEW (events_tree))), "changed", G_CALLBACK (
			today_pim_summary_events_selection_changed_cb), data);
	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (
		GTK_TREE_VIEW (tasks_tree))), "changed", G_CALLBACK (
			today_pim_summary_tasks_selection_changed_cb), data);

	today_pim_summary_show_notice (data);

	return vbox;
}
