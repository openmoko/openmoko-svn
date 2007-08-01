
#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include <libtaku/taku-table.h>
#include <libtaku/taku-launcher-tile.h>
#include <moko-finger-scroll.h>
#include "today.h"

/* NOTE: Following 4 functions (as well as libtaku) taken from
 * matchbox-desktop-2 and (slightly) modified where necessary
 */
/*
 * Load all .directory files from @vfolderdir, and add them as tables.
 */
GList *
load_vfolder_dir (const char *vfolderdir,
		  TakuLauncherCategory **fallback_category)
{
  GList *categories = NULL;
  GError *error = NULL;
  FILE *fp;
  char name[NAME_MAX], *filename;
  TakuLauncherCategory *category;

  filename = g_build_filename (vfolderdir, "Root.order", NULL);
  fp = fopen (filename, "r");
  if (fp == NULL) {
    g_warning ("Cannot read %s", filename);
    g_free (filename);
    return NULL;
  }
  g_free (filename);

  while (fgets (name, NAME_MAX - 9, fp) != NULL) {
    char **matches = NULL, *local_name = NULL;
    char **l;
    GKeyFile *key_file;

    if (name[0] == '#' || isspace (name[0]))
      continue;

    strcpy (name + strlen (name) - 1, ".directory");
  
    filename = g_build_filename (vfolderdir, name, NULL);

    key_file = g_key_file_new ();
    g_key_file_load_from_file (key_file, filename, G_KEY_FILE_NONE, &error);
    if (error) {
      g_warning ("Cannot read %s: %s", filename, error->message);
      g_error_free (error);
      error = NULL;

      goto done;
    }

    matches = g_key_file_get_string_list (key_file, "Desktop Entry", "Match", NULL, NULL);
    if (matches == NULL)
      goto done;

    local_name = g_key_file_get_locale_string (key_file, "Desktop Entry",
                                               "Name", NULL, NULL);
    if (local_name == NULL) {
      g_warning ("Directory file %s does not contain a \"Name\" field",
                 filename);
      g_strfreev (matches);
      goto done;
    }

    category = taku_launcher_category_new ();
    category->matches = matches;
    category->name  = local_name;

    categories = g_list_append (categories, category);

    /* Find a fallback category */
    if (fallback_category && (!*fallback_category))
      for (l = matches; *l; l++)
        if (strcmp (*l, "meta-fallback") == 0)
          *fallback_category = category;
    
  done:
    g_key_file_free (key_file);
    g_free (filename);
  }

  fclose (fp);
	
  return categories;
}

static void
match_category (TakuLauncherCategory *category,
		TakuLauncherTile *tile, gboolean *placed)
{
  const char **groups;
  char **match;
  
  for (match = category->matches; *match; match++) {
    /* Add all tiles to the all group */
    if (strcmp (*match, "meta-all") == 0) {
      taku_launcher_tile_add_group (tile, category);
      return;
    }
    
    for (groups = taku_launcher_tile_get_categories (tile);
         *groups; groups++) {
      if (strcmp (*match, *groups) == 0) {
        taku_launcher_tile_add_group (tile, category);
        *placed = TRUE;
        return;
      }
    }
  }
}

static void
set_groups (TakuLauncherTile *tile, GList *categories,
	    TakuLauncherCategory *fallback_category)
{
  gboolean placed = FALSE;
  GList *l;
  
  for (l = categories; l ; l = l->next) {
    match_category (l->data, tile, &placed);
  }

  if (!placed && fallback_category)
    taku_launcher_tile_add_group (tile, fallback_category);
}

/*
 * Load all .desktop files in @datadir/applications/, and add them to @table.
 */
static void
load_data_dir (const char *datadir, TakuTable *table, GList *categories,
	       TakuLauncherCategory *fallback_category)
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

    set_groups (TAKU_LAUNCHER_TILE (tile), categories, fallback_category);

    gtk_container_add (GTK_CONTAINER (table), tile);
    gtk_widget_show (tile);

  done:
    g_free (filename);
  }

  g_free (directory);
  g_dir_close (dir);
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
		if (strstr (name, search_string) ||
		    strstr (desc, search_string)) {
			taku_launcher_tile_add_group (TAKU_LAUNCHER_TILE (tile),
				data->search_cat);
		}
		g_free (name);
		g_free (desc);
	}
}

static void
today_launcher_filter_changed_cb (GtkEditable *editable, TodayData *data)
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
today_launcher_search_toggle_cb (GtkWidget *button, TodayData *data)
{
	g_object_set (G_OBJECT (data->search_entry), "visible",
		!GTK_WIDGET_VISIBLE (data->search_entry), NULL);
	g_object_set (G_OBJECT (data->filter_combo), "visible",
		!GTK_WIDGET_VISIBLE (data->filter_combo), NULL);

	if (GTK_WIDGET_VISIBLE (data->search_entry)) {
		/* Set the category to the created search category */
		taku_table_set_filter (TAKU_TABLE (data->launcher_table),
			data->search_cat);
	} else {
		/* Set the category back to the one pointed to by the
		 * drop-down.
		 */
		taku_table_set_filter (TAKU_TABLE (data->launcher_table),
			(TakuLauncherCategory *)g_list_nth_data (
				data->categories, gtk_combo_box_get_active (
					GTK_COMBO_BOX (data->filter_combo))));
		gtk_widget_grab_focus (data->search_entry);
	}
}

static void
today_launcher_combo_changed_cb (GtkComboBox *widget, TodayData *data)
{
	taku_table_set_filter (TAKU_TABLE (data->launcher_table),
		(TakuLauncherCategory *)g_list_nth_data (data->categories,
			gtk_combo_box_get_active (widget)));
}

GtkWidget *
today_launcher_page_create (TodayData *data)
{
	GtkWidget *main_vbox, *hbox, *toggle, *viewport, *scroll;
	const char * const *dirs;
	gchar *vfolder_dir;

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

	data->search_entry = gtk_entry_new ();
	g_signal_connect (G_OBJECT (data->search_entry), "changed",
		G_CALLBACK (today_launcher_filter_changed_cb), data);
	gtk_widget_set_name (data->search_entry, "mokosearchentry");
	g_object_set (G_OBJECT (data->search_entry), "no-show-all", TRUE, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), data->search_entry, TRUE, TRUE, 0);

	data->filter_combo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (hbox), data->filter_combo, TRUE, TRUE, 0);
	gtk_widget_show (data->filter_combo);
	
	gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	gtk_icon_size_register ("TakuIcon", 64, 64);
	data->launcher_table = taku_table_new ();

	/* Create search category */
	data->search_cat = g_new0 (TakuLauncherCategory, 1);
	data->search_cat->name = g_strdup ("Search");
	data->search_cat->matches = g_new0 (gchar *, 2);
	data->search_cat->matches[0] = g_strdup ("meta-search");
	
	/* Load app categories */
	vfolder_dir = g_build_filename (g_get_home_dir (),
		".matchbox", "vfolders", NULL);
	if (g_file_test (vfolder_dir, G_FILE_TEST_EXISTS))
		data->categories = load_vfolder_dir (vfolder_dir, NULL);
	else
		data->categories = load_vfolder_dir (PKGDATADIR "/vfolders",
			NULL);
	g_free (vfolder_dir);
	
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
	g_signal_connect (G_OBJECT (data->filter_combo), "changed",
		G_CALLBACK (today_launcher_combo_changed_cb), data);

	/* Populate the task list */
	/* TODO: Do this incrementally during idle time to increase
	 * start-up speed.
	 */
	for (dirs = g_get_system_data_dirs (); *dirs; dirs++) {
		load_data_dir (*dirs, TAKU_TABLE (data->launcher_table),
			data->categories, NULL);
	}
	load_data_dir (g_get_user_data_dir (),
		TAKU_TABLE (data->launcher_table), data->categories, NULL);
	
	/* Make sure initial search shows all items */
	today_launcher_update_search (data, "");

	gtk_container_add (GTK_CONTAINER (viewport), data->launcher_table);
	
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
	
	gtk_box_pack_start (GTK_BOX (main_vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (scroll);
	
	return main_vbox;
}
