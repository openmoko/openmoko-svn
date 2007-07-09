
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libkoto/koto-task.h>
#include <libkoto/koto-task-view.h>
#include "today-pim-summary.h"
#include "today-events-store.h"
#include "today-tasks-store.h"

typedef struct {
	GtkWidget *notice;
	int rows;
} TodayPimSummaryData;

/**
 * today_update_date ()
 *
 * Update the specified GtkTreeViewColumn with the current date
 */
static void
today_pim_summary_update_date (GtkTreeViewColumn *column)
{
	time_t t;
	struct tm *tmp;
	gchar date_str[64];

	time (&t);
	if (!(tmp = localtime (&t))) return;

	strftime (date_str, sizeof (date_str),
		"%I:%M%p %x", tmp);
	gtk_tree_view_column_set_title (column, date_str);
}

static gboolean
today_pim_summary_visible_cb (GtkTreeModel *model, GtkTreeIter *iter,
			      gpointer user_data)
{
	KotoTask *task;
	gboolean done;
	
	gtk_tree_model_get (model, iter, COLUMN_ICAL, &task,
		COLUMN_DONE, &done, -1);
	
	if (!done) return TRUE;
	
	if (icaltime_compare (icalcomponent_get_dtstamp (task->comp),
	    icaltime_today ()) >= 0) return TRUE;
	
	return FALSE;
}

static void
today_pim_summary_row_inserted_cb (GtkTreeModel *tree_model,
				   GtkTreePath *path, GtkTreeIter *iter,
				   gpointer user_data)
{
	TodayPimSummaryData *data = (TodayPimSummaryData *)user_data;
	data->rows ++;
	if (data->rows == 1) {
		gtk_widget_hide (data->notice);
	}
}

static void
today_pim_summary_row_deleted_cb (GtkTreeModel *tree_model,
				  GtkTreePath *path, gpointer user_data)
{
	TodayPimSummaryData *data = (TodayPimSummaryData *)user_data;
	data->rows --;
	if (data->rows < 1) {
		gtk_widget_show (data->notice);
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

GtkWidget *
today_pim_summary_box_new ()
{
	GtkTreeModel *events_model, *tasks_model, *tasks_sort_model;
	GtkWidget *vbox, *events_tree, *tasks_tree, *label;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	TodayPimSummaryData *data;

	data = g_new0 (TodayPimSummaryData, 1);
	events_model = GTK_TREE_MODEL (today_events_store_new ());
	tasks_model = GTK_TREE_MODEL (today_tasks_store_new ());
	tasks_sort_model = gtk_tree_model_filter_new (tasks_model, NULL);
	events_tree = gtk_tree_view_new_with_model (events_model);
	tasks_tree = gtk_tree_view_new_with_model (tasks_sort_model);
	gtk_widget_show (events_tree);
	gtk_widget_show (tasks_tree);
	
	gtk_tree_model_filter_set_visible_func (
		GTK_TREE_MODEL_FILTER (tasks_sort_model),
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

	today_pim_summary_update_date (column);
	g_timeout_add (60 * 60 * 1000, (GSourceFunc)
		today_pim_summary_update_date, column);
	
	label = gtk_label_new (_("No pending events or tasks"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 4, 4);
	gtk_widget_show (label);
	data->notice = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (data->notice), label);
	gtk_widget_show (data->notice);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), events_tree, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), tasks_tree, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), data->notice, FALSE, TRUE, 0);
	
	gtk_widget_modify_bg (data->notice, GTK_STATE_NORMAL,
		&events_tree->style->base[GTK_STATE_NORMAL]);

	g_signal_connect (G_OBJECT (events_model), "row-inserted",
		G_CALLBACK (today_pim_summary_row_inserted_cb), data);
	g_signal_connect (G_OBJECT (tasks_model), "row-inserted",
		G_CALLBACK (today_pim_summary_row_inserted_cb), data);
	g_signal_connect (G_OBJECT (events_model), "row-deleted",
		G_CALLBACK (today_pim_summary_row_deleted_cb), data);
	g_signal_connect (G_OBJECT (tasks_model), "row-deleted",
		G_CALLBACK (today_pim_summary_row_deleted_cb), data);
	g_signal_connect_swapped (G_OBJECT (vbox), "destroy",
		G_CALLBACK (g_free), data);

	return vbox;
}
