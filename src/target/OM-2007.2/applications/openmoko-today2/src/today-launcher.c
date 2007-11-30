
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include <libtaku/taku-table.h>
#include <libtaku/taku-launcher-tile.h>
#include <moko-finger-scroll.h>
#include <moko-search-bar.h>
#include "today.h"

/* NOTE: Following few functions (as well as libtaku) taken from
 * matchbox-desktop-2 and (slightly) modified where necessary
 */
static void
item_added_cb (TakuMenu *menu, TakuMenuItem *item, TodayData *data)
{
	const gchar *name;
	GtkWidget *tile;

	tile = taku_launcher_tile_new_from_item (item);
	name = taku_menu_item_get_name (item);
	
	if (strcmp (name, "Dialer") == 0) {
		gtk_widget_set_sensitive (GTK_WIDGET (data->dial_button), TRUE);
		data->dialer_item = item;
	} else if (strcmp (name, "Contacts") == 0) {
		gtk_widget_set_sensitive (GTK_WIDGET (data->contacts_button), TRUE);
		data->contacts_item = item;
	} else if (strcmp (name, "Tasks") == 0) {
		data->tasks_item = item;
	} else if (strcmp (name, "Dates") == 0) {
		gtk_widget_set_sensitive (GTK_WIDGET (data->dates_button), TRUE);
		data->dates_item = item;
	} else if (strcmp (name, "World Clock") == 0) {
		data->clock_item = item;
	} else if (strcmp (name, "Messages") == 0) {
		gtk_widget_set_sensitive (GTK_WIDGET (
			data->messages_button), TRUE);
		data->messages_item = item;
	}

	if (GTK_IS_WIDGET (tile))
		gtk_container_add (GTK_CONTAINER (data->launcher_table), tile); 
}

static void
item_removed_cb (TakuMenu *menu, TakuMenuItem *item, TodayData *data)
{
	TakuLauncherTile *tile = NULL;
	GList *tiles, *t;

	if (data->dialer_item == item) {
		data->dialer_item = NULL;
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->dial_button), FALSE);
	} else if (data->contacts_item == item) {
		data->contacts_item = NULL;
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->contacts_button), FALSE);
	} else if (data->tasks_item == item) {
		data->tasks_item = NULL;
	} else if (data->dates_item == item) {
		data->dates_item = NULL;
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->dates_button), FALSE);
	} else if (data->clock_item == item) {
		data->clock_item = NULL;
	} else if (data->messages_item == item) {
		data->messages_item = NULL;
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->messages_button), FALSE);
	}

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

static void
today_launcher_update_search (TodayData *data, const gchar *search_string)
{
	GList *children, *c;

	children = gtk_container_get_children (
		GTK_CONTAINER (data->launcher_table));
	for (c = children; c; c = c->next) {
		gchar *name, *desc;
		GtkWidget *tile = GTK_WIDGET (c->data);
		
		if (!TAKU_IS_LAUNCHER_TILE (tile)) continue;
		
		taku_launcher_tile_remove_group (TAKU_LAUNCHER_TILE (tile),
			data->search_cat);
		name = g_utf8_strup (taku_icon_tile_get_primary (
			TAKU_ICON_TILE (tile)), -1);
		desc = g_utf8_strup (taku_icon_tile_get_secondary (
			TAKU_ICON_TILE (tile)), -1);
		if ((!search_string) || strstr (name, search_string) ||
		    strstr (desc, search_string)) {
			taku_launcher_tile_add_group (TAKU_LAUNCHER_TILE (tile),
				data->search_cat);
		}
		g_free (name);
		g_free (desc);
	}
}

static void
today_launcher_filter_changed_cb (MokoSearchBar *bar, GtkEditable *editable,
				  TodayData *data)
{
	gchar *search_string;
	
	search_string = g_utf8_strup (gtk_entry_get_text (
		GTK_ENTRY (editable)), -1);
	today_launcher_update_search (data, search_string);
	g_free (search_string);

	/* Force the table to update */
	taku_table_set_filter (TAKU_TABLE (data->launcher_table),
		data->search_cat);
}

static void
today_launcher_combo_changed_cb (MokoSearchBar *bar, GtkComboBox *widget,
				 TodayData *data)
{
	taku_table_set_filter (TAKU_TABLE (data->launcher_table),
		(TakuLauncherCategory *)g_list_nth_data (data->categories,
			gtk_combo_box_get_active (widget)));
}

static void
today_launcher_search_toggle_cb (MokoSearchBar *bar, gboolean search_visible,
				 TodayData *data)
{
	if (search_visible) {
		/* Set the category to the created search category */
		today_launcher_filter_changed_cb (bar, GTK_EDITABLE (
			moko_search_bar_get_entry (bar)), data);
	} else {
		/* Set the category back to the one pointed to by the
		 * drop-down.
		 */
		 today_launcher_combo_changed_cb (bar,
		 	moko_search_bar_get_combo_box (bar), data);
	}
}

/* If there's zero padding around tiles and redraw is forced for every scroll,
 * the following function also works to paint the wallpaper under the tiles.
 */
/*static gboolean
tile_expose_cb (GtkWidget *widget, GdkEventExpose *event, TodayData *data)
{
	GtkAdjustment *vadjustment;
	
	if (!data->wallpaper) return FALSE;

	vadjustment = gtk_viewport_get_vadjustment (
		(GtkViewport *)data->launcher_viewport);
	gdk_draw_drawable (widget->window, widget->style->black_gc,
		data->wallpaper, widget->allocation.x,
		widget->allocation.y - vadjustment->value,
		widget->allocation.x, widget->allocation.y,
		widget->allocation.width, widget->allocation.height);
	
	return FALSE;
}*/

#ifdef SLOW_BLING
static void
redraw_widget_now (GtkWidget *widget)
{
	GdkRectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->allocation.width;
	rect.height = widget->allocation.height;
	gdk_window_invalidate_rect (widget->window, &rect, TRUE);
	gdk_window_process_updates (widget->window, TRUE);
}

static void
viewport_set_scroll_adjustments_cb (GtkViewport *viewport,
				    GtkAdjustment *hadjust,
				    GtkAdjustment *vadjust,
				    TodayData *data)
{
	static gpointer instance = NULL;
	static gulong id = 0;
	
	if (instance) g_signal_handler_disconnect (instance, id);
	if (vadjust) {
		instance = vadjust;
		id = g_signal_connect_swapped (vadjust, "value-changed",
			G_CALLBACK (redraw_widget_now), viewport);
	} else {
		instance = NULL;
	}
}

static gboolean
table_expose_cb (GtkWidget *widget, GdkEventExpose *event, TodayData *data)
{
	GtkAdjustment *vadjustment;
	
	if (!data->wallpaper) return FALSE;

	vadjustment = gtk_viewport_get_vadjustment (
		(GtkViewport *)data->launcher_viewport);
	gdk_draw_drawable (widget->window, widget->style->black_gc,
		data->wallpaper, 0, 0, 0, vadjustment->value, -1, -1);
	
	return FALSE;
}
#endif

static gboolean
add_item_idle (TakuMenuItem *item)
{
	g_signal_emit_by_name (taku_menu_get_default (), "item-added", item);
	return FALSE;
}

static gboolean
load_items (TodayData *data)
{
	GList *items;
	TakuMenu *menu = taku_menu_get_default ();
	
	for (items = taku_menu_get_items (menu); items; items = items->next) {
		if (!items->data) continue;
		g_idle_add ((GSourceFunc)add_item_idle, items->data);
	}
	return FALSE;
}

GtkWidget *
today_launcher_page_create (TodayData *data)
{
	static TakuMenu *menu;
	GtkWidget *main_vbox, *hbox, *toggle, *scroll;

	main_vbox = gtk_vbox_new (FALSE, 0);
	
	/* search/filter bar */
	hbox = gtk_hbox_new (FALSE, 0);

	toggle = gtk_toggle_button_new ();
	gtk_widget_set_name (toggle, "mokosearchbutton");
	g_signal_connect (G_OBJECT (toggle), "toggled",
		G_CALLBACK (today_launcher_search_toggle_cb), data);
	gtk_button_set_image (GTK_BUTTON (toggle),
		gtk_image_new_from_stock (GTK_STOCK_FIND,
			GTK_ICON_SIZE_SMALL_TOOLBAR));
	
	gtk_box_pack_start (GTK_BOX (hbox), toggle, FALSE, FALSE, 0);
	gtk_widget_show_all (toggle);

	data->filter_combo = gtk_combo_box_new_text ();
	data->search_bar = moko_search_bar_new_with_combo (
		GTK_COMBO_BOX (data->filter_combo));
	gtk_box_pack_start (GTK_BOX (main_vbox), data->search_bar,
		FALSE, TRUE, 0);
	gtk_widget_show (data->search_bar);
	
	data->launcher_viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (data->launcher_viewport),
				      GTK_SHADOW_NONE);
	gtk_icon_size_register ("taku-icon", 64, 64);
	data->launcher_table = taku_table_new ();

#ifdef SLOW_BLING
	/* Draw the wallpaper in the background of the table */
	g_signal_connect (data->launcher_viewport, "set-scroll-adjustments",
		G_CALLBACK (viewport_set_scroll_adjustments_cb), data);
	g_signal_connect (data->launcher_table, "expose-event",
		G_CALLBACK (table_expose_cb), data);
#endif

	/* Create search category */
	data->search_cat = g_new0 (TakuLauncherCategory, 1);
	data->search_cat->name = g_strdup ("Search");
	data->search_cat->matches = g_new0 (gchar *, 2);
	data->search_cat->matches[0] = g_strdup ("meta-search");
	
	/* Load application data */
	menu = taku_menu_get_default ();
	g_signal_connect (menu, "item-added",
		G_CALLBACK (item_added_cb), data);
	g_signal_connect (menu, "item-removed",
		G_CALLBACK (item_removed_cb), data);  

	data->categories = taku_menu_get_categories (menu);
	if (data->categories) {
		GList *c;
		for (c = data->categories; c; c = c->next) {
			TakuLauncherCategory *category =
				(TakuLauncherCategory *)c->data;
			gtk_combo_box_append_text (GTK_COMBO_BOX (
				data->filter_combo), category->name);
		}
		taku_table_set_filter (TAKU_TABLE (data->launcher_table),
			data->categories->data);
	} else {
		gtk_combo_box_append_text (
			GTK_COMBO_BOX (data->filter_combo), _("All"));
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (data->filter_combo), 0);

	g_signal_connect (G_OBJECT (data->search_bar), "text_changed",
		G_CALLBACK (today_launcher_filter_changed_cb), data);
	g_signal_connect (G_OBJECT (data->search_bar), "combo_changed",
		G_CALLBACK (today_launcher_combo_changed_cb), data);
	g_signal_connect (G_OBJECT (data->search_bar), "toggled",
		G_CALLBACK (today_launcher_search_toggle_cb), data);
	
	/* Populate the app list */
	data->dialer_item = NULL;
	data->contacts_item = NULL;
	data->tasks_item = NULL;
	data->dates_item = NULL;
	data->clock_item = NULL;
	g_idle_add ((GSourceFunc)load_items, data);
	
	/* Make sure initial search shows all items */
	today_launcher_update_search (data, "");

	gtk_container_add (GTK_CONTAINER (data->launcher_viewport),
		data->launcher_table);
	
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->launcher_viewport);
	
	gtk_box_pack_start (GTK_BOX (main_vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (scroll);
	
	return main_vbox;
}
