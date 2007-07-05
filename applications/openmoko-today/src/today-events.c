
#include <gtk/gtk.h>
#include "today-events.h"
#include "today-header-box.h"
#include "today-events-list-store.h"

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
	GtkTreeModel *model;
	GtkWidget *tree;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	model = GTK_TREE_MODEL (today_events_list_store_new ());
	tree = gtk_tree_view_new_with_model (model);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),
		"ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Date",
		renderer, "text", TODAY_EVENTS_LIST_STORE_COL_STRING, NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (tree), column, 0);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), TRUE);
	
	today_events_update_date (column);
	g_timeout_add (60 * 60 * 1000, (GSourceFunc)
		today_events_update_date, column);

	return tree;
}
