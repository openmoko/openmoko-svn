
#include <libmokoui/moko-stock.h>
#include <libmokoui/moko-finger-scroll.h>
#include <libtaku/taku-table.h>
#include "today-task-manager.h"
#include "today-utils.h"

GtkWidget *
today_task_manager_page_create (TodayData *data)
{
	GtkWidget *vbox, *toolbar, *table, *viewport, *scroll;
	GtkToolItem *button;
	
	vbox = gtk_vbox_new (FALSE, 0);
	
	/* Create toolbar */
	toolbar = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);

	/* Kill all apps button */
	button = today_toolbutton_new (MOKO_STOCK_FOLDER_DELETE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);

	/* Kill app button */
	button = today_toolbutton_new (GTK_STOCK_DELETE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);

	/* Switch to app button */
	button = today_toolbutton_new (GTK_STOCK_JUMP_TO);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);

	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	table = taku_table_new ();
	gtk_container_add (GTK_CONTAINER (viewport), table);
	gtk_widget_show (table);

	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show (viewport);
	gtk_widget_show (scroll);
	
	gtk_widget_show_all (toolbar);
	return vbox;
}
