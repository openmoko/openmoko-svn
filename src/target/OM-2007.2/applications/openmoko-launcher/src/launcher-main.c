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
	GOptionContext *context;
	LauncherData data;
	GtkWidget *widget;
#ifndef STANDALONE
	gint x, y, w, h;
#endif
	
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

	/* init openmoko stock items */
	moko_stock_register ();

	/* Create widgets */
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (data.window), _("Home"));
	
	/* Notebook */
	data.notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data.notebook), GTK_POS_BOTTOM);
	gtk_container_add (GTK_CONTAINER (data.window), data.notebook);
	gtk_widget_show (data.notebook);

	/* Add launcher page */
	/*widget = today_launcher_page_create (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_ADD, 6);
	gtk_widget_show (widget);*/

	/* Add running tasks page */
	widget = today_task_manager_page_create (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_EXECUTE, 6);
	gtk_widget_show (widget);
	
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
	gtk_widget_show (data.window);

	gtk_main ();

	return 0;
}

