#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libtaku/taku-table.h>
#include <libtaku/taku-launcher-tile.h>
#include <moko-stock.h>
#include <moko-finger-scroll.h>
#include "launcher.h"
#include "today-task-manager.h"

static void
today_notebook_add_page_with_icon (GtkWidget *notebook, GtkWidget *child,
				   const gchar *icon_name, int padding)
{
	GtkWidget *icon = gtk_image_new_from_icon_name (icon_name,
		GTK_ICON_SIZE_LARGE_TOOLBAR);
	GtkWidget *align = gtk_alignment_new (0.5, 0.5, 1, 1);

	gtk_alignment_set_padding (GTK_ALIGNMENT (align), padding,
		padding, padding, padding);
	gtk_container_add (GTK_CONTAINER (align), icon);
	gtk_widget_show_all (align);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), child, align);
	gtk_container_child_set (GTK_CONTAINER (notebook), child,
		"tab-expand", TRUE, NULL);
}

static void
item_added_cb (TakuMenu *menu, TakuMenuItem *item, LauncherData *data)
{
	GtkWidget *tile;

	tile = taku_launcher_tile_new_from_item (item);
	if (GTK_IS_WIDGET (tile))
		gtk_container_add (GTK_CONTAINER (data->launcher_table), tile); 
}

static void
item_removed_cb (TakuMenu *menu, TakuMenuItem *item, LauncherData *data)
{
	TakuLauncherTile *tile = NULL;
	GList *tiles, *t;

	tiles = gtk_container_get_children (
		GTK_CONTAINER (data->launcher_table));
	for (t = tiles; t; t = t->next) {
		if (!TAKU_IS_LAUNCHER_TILE (t->data)) continue;

		if (item == taku_launcher_tile_get_item (t->data)) {
			tile = t->data;
			break;
		}
	}

	if (GTK_IS_WIDGET (tile)) gtk_container_remove (GTK_CONTAINER (
		data->launcher_table), GTK_WIDGET (tile));
}

#ifndef STANDALONE
static GtkWidget *window;

static void
workarea_changed (int x, int y, int w, int h)
{
	gtk_window_resize (GTK_WINDOW (window), w, h);
	gtk_window_move (GTK_WINDOW (window), x, y);
}
#endif

int
main (int argc, char **argv)
{
	static TakuMenu *menu;

	GtkWidget *widget, *scroll, *viewport;
	GOptionContext *context;
	LauncherData data;
	GList *items;
#ifndef STANDALONE
	gint x, y, w, h;
#endif
	
	static GOptionEntry entries[] = {
		{ NULL }
	};
	
	/* Initialise */
	bindtextdomain (GETTEXT_PACKAGE, LAUNCHER_LOCALE_DIR);;
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	context = g_option_context_new (_(" - Simple application launcher"));
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, NULL);

	/* init openmoko stock items */
	moko_stock_register ();

	/* Create widgets */
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (data.window), _("Launcher"));
	
	/* Notebook */
	data.notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data.notebook), GTK_POS_BOTTOM);
	gtk_container_add (GTK_CONTAINER (data.window), data.notebook);

	/* Add launcher page */
	menu = taku_menu_get_default ();
	g_signal_connect (menu, "item-added",
		G_CALLBACK (item_added_cb), &data);
	g_signal_connect (menu, "item-removed",
		G_CALLBACK (item_removed_cb), &data);  

	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
	gtk_icon_size_register ("taku-icon", 64, 64);
	data.launcher_table = taku_table_new ();
	gtk_container_add (GTK_CONTAINER (viewport), data.launcher_table);
	
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
	
	today_notebook_add_page_with_icon (data.notebook, scroll,
		GTK_STOCK_ADD, 6);

	for (items = taku_menu_get_items (menu); items; items = items->next) {
		if (!items->data) continue;
		g_signal_emit_by_name (menu, "item-added", items->data);
	}

	/* Add running tasks page */
	widget = today_task_manager_page_create (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_EXECUTE, 6);
	
	/* Connect up signals */
	g_signal_connect (G_OBJECT (data.window), "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);

#ifndef STANDALONE
	x = 0; y = 0; w = 480; h = 640;
	gtk_window_set_type_hint (GTK_WINDOW (data.window),
		GDK_WINDOW_TYPE_HINT_DESKTOP);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (data.window), TRUE);
	window = data.window;
	x_monitor_workarea (gtk_widget_get_screen (window), workarea_changed);
#else
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);
#endif
	
	/* Show and start */
	gtk_widget_show_all (data.window);

	gtk_main ();

	return 0;
}

