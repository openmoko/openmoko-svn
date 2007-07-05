
#include <gtk/gtk.h>
#include "today-events.h"
#include "today-header-box.h"
#include "today-events-store.h"
#include "today-tasks-store.h"

/**
 * today_update_date ()
 *
 * Update the specified GtkTreeViewColumn with the current date
 */
static void
today_events_update_date (GtkTreeViewColumn *column)
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

GtkWidget *
today_events_box_new ()
{
	GtkTreeModel *events_model, *tasks_model;
	GtkWidget *vbox, *events_tree, *tasks_tree;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	events_model = GTK_TREE_MODEL (today_events_store_new ());
	tasks_model = GTK_TREE_MODEL (today_tasks_store_new ());
	events_tree = gtk_tree_view_new_with_model (events_model);
	tasks_tree = gtk_tree_view_new_with_model (tasks_model);
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		"ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Tasks",
		renderer, "text", COLUMN_SUMMARY, NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tasks_tree), column, 0);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tasks_tree), FALSE);

	/* Using the same text renderer. Is this ok? It seems to work... */
	column = gtk_tree_view_column_new_with_attributes ("Events",
		renderer, "text", TODAY_EVENTS_STORE_COL_SUMMARY, NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (events_tree), column, 0);	
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (events_tree), TRUE);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), events_tree, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), tasks_tree, FALSE, TRUE, 0);
	
	today_events_update_date (column);
	g_timeout_add (60 * 60 * 1000, (GSourceFunc)
		today_events_update_date, column);

	return vbox;
}
