
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

/* inotify support derived/copied from code in matchbox-desktop-2 */
#if WITH_INOTIFY
#include "inotify/inotify-path.h"
#include "inotify/local_inotify.h"

static gboolean with_inotify;
G_LOCK_DEFINE(inotify_lock);
#endif

/* NOTE: Following 6 functions (as well as libtaku) taken from
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

typedef struct {
	gchar *name;
	gchar *directory;
	TakuTable *table;
	GList *categories;
	TakuLauncherCategory *fallback_category;
} LoadDesktopData;

static gboolean
load_desktop_cb (LoadDesktopData *data)
{
	gchar *filename;
	GtkWidget *tile;

	if (!g_str_has_suffix (data->name, ".desktop")) goto not_desktop;

	filename = g_build_filename (data->directory, data->name, NULL);

	/* TODO: load launcher data, probe that, and then create a tile */

	tile = taku_launcher_tile_for_desktop_file (filename);
	if (!tile) goto done;

	set_groups (TAKU_LAUNCHER_TILE (tile), data->categories,
		data->fallback_category);

	gtk_container_add (GTK_CONTAINER (data->table), tile);
	gtk_widget_show (tile);

done:
	g_free (filename);
not_desktop:
	g_free (data->name);
	g_free (data->directory);
	g_free (data);
	
	return FALSE;
}

#if WITH_INOTIFY
/*
 * Monitor @directory with inotify, if available.
 */
static void
monitor (const char *directory)
{
  inotify_sub *sub;

  if (!with_inotify)
    return;
  
  sub = _ih_sub_new (directory, NULL, NULL);
  _ip_start_watching (sub);
}

/*
 * Used to delete tiles when they are removed from disk.  @a is the tile, @b is
 * the desktop filename to look for.
 */
static void
find_and_destroy (GtkWidget *widget, gpointer data)
{
  TakuLauncherTile *tile;
  const char *removed, *filename;

  tile = TAKU_LAUNCHER_TILE (widget);
  if (!tile)
    return;
  
  removed = data;
  
  filename = taku_launcher_tile_get_filename (tile);
  if (strcmp (removed, filename) == 0)
    gtk_widget_destroy (widget);
}

static TodayData *data_static;
static void
inotify_event (ik_event_t *event, inotify_sub *sub)
{
  char *path;

  if (event->mask & IN_MOVED_TO || event->mask & IN_CREATE) {
    if (g_str_has_suffix (event->name, ".desktop")) {
      LoadDesktopData *data = g_new0 (LoadDesktopData, 1);
      data->name = g_strdup (event->name);
      data->directory = g_strdup (sub->dirname);
      data->table = (TakuTable *)data_static->launcher_table;
      data->categories = data_static->categories;
      load_desktop_cb (data);
    }
  } else if (event->mask & IN_MOVED_FROM || event->mask & IN_DELETE) {
    path = g_build_filename (sub->dirname, event->name, NULL);
    gtk_container_foreach (GTK_CONTAINER (data_static->launcher_table),
                           find_and_destroy, path);
    g_free (path);
  }
}
#endif

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

#if WITH_INOTIFY
  monitor (directory);
#endif

  dir = g_dir_open (directory, 0, &error);
  if (error) {
    g_warning ("Cannot read %s: %s", directory, error->message);
    g_error_free (error);
    g_free (directory);
    return;
  }

  while ((name = g_dir_read_name (dir)) != NULL) {
	LoadDesktopData *data = g_new (LoadDesktopData, 1);
	data->name = g_strdup (name);
	data->directory = g_strdup (directory);
	data->table = table;
	data->categories = categories;
	data->fallback_category = fallback_category;
	g_idle_add ((GSourceFunc)load_desktop_cb, data);
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

GtkWidget *
today_launcher_page_create (TodayData *data)
{
	GtkWidget *main_vbox, *hbox, *toggle, *scroll;
	const char * const *dirs;
	gchar *vfolder_dir;

#if WITH_INOTIFY
	data_static = data;
	with_inotify = _ip_startup (inotify_event);
#endif

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

	g_signal_connect (G_OBJECT (data->search_bar), "text_changed",
		G_CALLBACK (today_launcher_filter_changed_cb), data);
	g_signal_connect (G_OBJECT (data->search_bar), "combo_changed",
		G_CALLBACK (today_launcher_combo_changed_cb), data);
	g_signal_connect (G_OBJECT (data->search_bar), "toggled",
		G_CALLBACK (today_launcher_search_toggle_cb), data);
	
	/* Populate the app list */
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

	gtk_container_add (GTK_CONTAINER (data->launcher_viewport),
		data->launcher_table);
	
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->launcher_viewport);
	
	gtk_box_pack_start (GTK_BOX (main_vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (scroll);
	
	return main_vbox;
}
