
#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libtaku/taku-table.h>
#include <libtaku/taku-launcher-tile.h>
#include "today.h"
#include "today-header-box.h"
#include "today-pim-summary.h"

static GtkToolItem *
today_toolbutton_new (const gchar *icon_name)
{
	GtkWidget *icon = gtk_image_new_from_icon_name (icon_name,
		GTK_ICON_SIZE_DIALOG);
	GtkToolItem *button = gtk_tool_button_new (icon, NULL);
	gtk_tool_item_set_expand (button, TRUE);
	return button;
}

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

/* NOTE: Following (as well as libtaku) taken from matchbox-desktop-2 */
/*
 * Load all .desktop files in @datadir/applications/, and add them to @table.
 */
static void
load_data_dir (const char *datadir, TakuTable *table)
{
  GError *error = NULL;
  GDir *dir;
  char *directory;
  const char *name;

  g_assert (datadir);

  directory = g_build_filename (datadir, "applications", NULL);

  /* Check if the directory exists */
  if (! g_file_test (directory, G_FILE_TEST_IS_DIR)) {
    g_free (directory);
    return;
  }

  dir = g_dir_open (directory, 0, &error);
  if (error) {
    g_warning ("Cannot read %s: %s", directory, error->message);
    g_error_free (error);
    g_free (directory);
    return;
  }

  while ((name = g_dir_read_name (dir)) != NULL) {
    char *filename;
    GtkWidget *tile;
  
    if (! g_str_has_suffix (name, ".desktop"))
      continue;

    filename = g_build_filename (directory, name, NULL);

    /* TODO: load launcher data, probe that, and then create a tile */

    tile = taku_launcher_tile_for_desktop_file (filename);
    if (!tile)
      goto done;

    /*set_groups (TAKU_LAUNCHER_TILE (tile));*/

    gtk_container_add (GTK_CONTAINER (table), tile);
    gtk_widget_show (tile);

  done:
    g_free (filename);
  }

  g_free (directory);
  g_dir_close (dir);
}

static GtkWidget *
today_create_home_page (TodayData *data)
{
	GtkWidget *main_vbox, *vbox, *align, *viewport;
	
	/* Add home page */
	main_vbox = gtk_vbox_new (FALSE, 0);
		
	/* Toolbar */
	data->home_toolbar = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (main_vbox), data->home_toolbar, FALSE, TRUE, 0);

	data->dates_button = today_toolbutton_new ("dates");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->dates_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	data->messages_button = today_toolbutton_new ("openmoko-messages");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->messages_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	data->contacts_button = today_toolbutton_new ("contacts");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->contacts_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	data->dial_button = today_toolbutton_new ("openmoko-dialer");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->dial_button, 0);
	gtk_widget_show_all (data->home_toolbar);

	viewport = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (main_vbox), viewport, TRUE, TRUE, 0);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	gtk_widget_show (viewport);
	align = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (align), 6, 6, 6, 6);
	gtk_container_add (GTK_CONTAINER (viewport), align);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (align), vbox);
	gtk_widget_show_all (align);
	/*data->message_box = today_header_box_new_with_markup (
		"<b>Provider goes here</b>");
	gtk_box_pack_start (GTK_BOX (vbox), data->message_box, FALSE, TRUE, 0);*/

	data->summary_box = today_pim_summary_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), data->summary_box, FALSE, TRUE, 0);
	gtk_widget_show (data->summary_box);
	
	return main_vbox;
}

static void
today_tasks_search_toggle_cb (GtkWidget *button, TodayData *data)
{
	g_object_set (G_OBJECT (data->search_entry), "visible",
		!GTK_WIDGET_VISIBLE (data->search_entry), NULL);
	g_object_set (G_OBJECT (data->filter_combo), "visible",
		!GTK_WIDGET_VISIBLE (data->filter_combo), NULL);
}

static GtkWidget *
today_create_tasks_page (TodayData *data)
{
	GtkWidget *main_vbox, *hbox, *toggle, *viewport, *task_list;
	const char * const *dirs;

	main_vbox = gtk_vbox_new (FALSE, 0);
	
	/* search/filter bar */
	hbox = gtk_hbox_new (FALSE, 0);

	toggle = gtk_toggle_button_new ();
	gtk_widget_set_name (toggle, "mokosearchbutton");
	g_signal_connect (G_OBJECT (toggle), "toggled",
		G_CALLBACK (today_tasks_search_toggle_cb), data);
	gtk_button_set_image (GTK_BUTTON (toggle),
		gtk_image_new_from_stock (GTK_STOCK_FIND,
			GTK_ICON_SIZE_SMALL_TOOLBAR));
	
	gtk_box_pack_start (GTK_BOX (hbox), toggle, FALSE, FALSE, 0);
	gtk_widget_show_all (toggle);

	data->search_entry = gtk_entry_new ();
	gtk_widget_set_name (data->search_entry, "mokosearchentry");
	g_object_set (G_OBJECT (data->search_entry), "no-show-all", TRUE, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), data->search_entry, TRUE, TRUE, 0);

	data->filter_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (data->filter_combo), "All");
	gtk_combo_box_set_active (GTK_COMBO_BOX (data->filter_combo), 0);
	gtk_box_pack_start (GTK_BOX (hbox), data->filter_combo, TRUE, TRUE, 0);
	gtk_widget_show (data->filter_combo);
	
	gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	task_list = taku_table_new ();

	/* Populate the task list */
	/* TODO: Do this incrementally during idle time to increase
	 * start-up speed.
	 */
	for (dirs = g_get_system_data_dirs (); *dirs; dirs++) {
		load_data_dir (*dirs, TAKU_TABLE (task_list));
	}
	load_data_dir (g_get_user_data_dir (), TAKU_TABLE (task_list));

	gtk_container_add (GTK_CONTAINER (viewport), task_list);
	gtk_box_pack_start (GTK_BOX (main_vbox), viewport, TRUE, TRUE, 0);
	gtk_widget_show_all (viewport);
	
	return main_vbox;
}

int
main (int argc, char **argv)
{
	TodayData data;
	GOptionContext *context;
	GtkWidget *widget;
	GtkWidget *placeholder;
	
	static GOptionEntry entries[] = {
		{ NULL }
	};

	/* Initialise */
	bindtextdomain (GETTEXT_PACKAGE, TODAY_LOCALE_DIR);;
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	context = g_option_context_new (_(" - Today's information summary"));
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, NULL);

	/* Create widgets */
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (data.window), _("Today"));
	
	/* Notebook */
	data.notebook = gtk_notebook_new ();
	g_object_set (G_OBJECT (data.notebook), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data.notebook), GTK_POS_BOTTOM);
	gtk_container_add (GTK_CONTAINER (data.window), data.notebook);
	gtk_widget_show (data.notebook);

	/* Add home page */
	widget = today_create_home_page (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_HOME, 6);
	gtk_widget_show (widget);

	/* Add new tasks page */
	widget = today_create_tasks_page (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_ADD, 6);
	gtk_widget_show (widget);

	/* Add running tasks page */
	placeholder = gtk_label_new ("Running tasks");
	today_notebook_add_page_with_icon (data.notebook, placeholder,
		GTK_STOCK_EXECUTE, 6);
	gtk_widget_show (placeholder);
	
	/* Connect up signals */
	g_signal_connect (G_OBJECT (data.window), "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);

#if 1
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2", 
		"gtk-icon-theme-name", "openmoko-standard",
		NULL);
#endif

	
	/* Show and start */
	gtk_widget_show (data.window);
	gtk_main ();

	return 0;
}
