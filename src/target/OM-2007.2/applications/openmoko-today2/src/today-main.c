
#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <moko-finger-scroll.h>
#include <libtaku/launcher-util.h>
#include <libtaku/xutil.h>
#include <unistd.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libjana-gtk/jana-gtk.h>
#include "today.h"
#include "today-utils.h"
#include "today-pim-summary.h"
#include "today-pim-journal.h"
#include "today-launcher.h"
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
today_dial_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	launcher_start (data->window, today_get_launcher ((const gchar *[])
		{"openmoko-dialer", NULL }, TRUE, TRUE));
}

static void
today_contacts_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	launcher_start (data->window, today_get_launcher ((const gchar *[])
		{ "openmoko-contacts", NULL }, TRUE, TRUE));
}

static void
today_messages_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	launcher_start (data->window, today_get_launcher ((const gchar *[])
		{ "openmoko-messages", NULL }, TRUE, TRUE));
}

static void
today_dates_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	launcher_start (data->window, today_get_launcher ((const gchar *[])
		{ "openmoko-dates", NULL }, TRUE, TRUE));
}

static gboolean
bg_expose_cb (GtkWidget *widget, GdkEventExpose *event, TodayData *data)
{
	if (data->wallpaper)
		gdk_draw_drawable (widget->window, widget->style->black_gc,
			data->wallpaper, 0, 0, 0, 0, -1, -1);
	
	return FALSE;
}

static void
bg_size_allocate_cb (GtkWidget *widget, GtkAllocation *allocation,
		     TodayData *data)
{
	static gint width = 0, height = 0;
	
	/* Re-scale wallpaper */
	if ((width != allocation->width) || (height != allocation->height)) {
		width = allocation->width;
		height = allocation->height;
		gconf_client_notify (gconf_client_get_default (),
			GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER);
	}
}

static gboolean
set_time_idle (TodayData *data)
{
	JanaTime *time;
	
	time = jana_ecal_utils_time_now (data->location);
	jana_gtk_clock_set_time (JANA_GTK_CLOCK (data->clock), time);
	
#if GLIB_CHECK_VERSION(2,14,0)
	g_timeout_add_seconds (60 - jana_time_get_seconds (time),
		(GSourceFunc)set_time_idle, data);
#else
	g_timeout_add ((60 - jana_time_get_seconds (time)) * 1000,
		(GSourceFunc)set_time_idle, data);
#endif

	g_object_unref (time);
	
	return TRUE;
}

static GtkWidget *
today_create_home_page (TodayData *data)
{
	GtkWidget *main_vbox, *vbox, *align, *viewport, *scroll;
	
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
	g_signal_connect (G_OBJECT (data->dates_button), "clicked",
		G_CALLBACK (today_dates_button_clicked_cb), data);

	data->messages_button = today_toolbutton_new ("openmoko-messages");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->messages_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (G_OBJECT (data->messages_button), "clicked",
		G_CALLBACK (today_messages_button_clicked_cb), data);

	data->contacts_button = today_toolbutton_new ("contacts");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->contacts_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (G_OBJECT (data->contacts_button), "clicked",
		G_CALLBACK (today_contacts_button_clicked_cb), data);

	data->dial_button = today_toolbutton_new ("openmoko-dialer");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->dial_button, 0);
	gtk_widget_show_all (data->home_toolbar);
	g_signal_connect (G_OBJECT (data->dial_button), "clicked",
		G_CALLBACK (today_dial_button_clicked_cb), data);

	/* Create event box with background */
	data->bg_ebox = gtk_event_box_new ();
	gtk_widget_set_app_paintable (data->bg_ebox, TRUE);
	g_signal_connect (data->bg_ebox, "expose-event",
		G_CALLBACK (bg_expose_cb), data);
	g_signal_connect (data->bg_ebox, "size-allocate",
		G_CALLBACK (bg_size_allocate_cb), data);
	
	/* Get location and create clock widget */
	data->location = jana_ecal_utils_guess_location ();
	data->clock = jana_gtk_clock_new ();
	jana_gtk_clock_set_draw_shadow (JANA_GTK_CLOCK (data->clock), TRUE);

	/* Create viewport for clock/journal/PIM summary widgets */
	viewport = gtk_viewport_new (NULL, NULL);
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
	gtk_box_pack_start (GTK_BOX (main_vbox), scroll, TRUE, TRUE, 0);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	gtk_widget_show_all (scroll);

	/* Pack widgets */
	vbox = gtk_vbox_new (FALSE, 6);

	gtk_box_pack_start (GTK_BOX (vbox), data->clock, TRUE, TRUE, 0);
	gtk_widget_show_all (data->clock);
	
	data->message_box = today_pim_journal_box_new (data);
	gtk_box_pack_start (GTK_BOX (vbox), data->message_box, FALSE, TRUE, 0);
	gtk_widget_show (data->message_box);
	
	data->summary_box = today_pim_summary_box_new (data);
	gtk_box_pack_start (GTK_BOX (vbox), data->summary_box, FALSE, TRUE, 6);
	gtk_widget_show (data->summary_box);
	
	align = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (align), 6, 6, 6, 6);
	gtk_container_add (GTK_CONTAINER (viewport), data->bg_ebox);
	gtk_container_add (GTK_CONTAINER (data->bg_ebox), align);
	gtk_container_add (GTK_CONTAINER (align), vbox);
	gtk_widget_show_all (data->bg_ebox);
	
	/* Set the time on the clock */
	set_time_idle (data);
	
	return main_vbox;
}

static void
wallpaper_notify (GConfClient *client, guint cnxn_id,
		  GConfEntry *entry, TodayData *data)
{
	gint width, height, pwidth, pheight;
	GdkPixbuf *pixbuf, *pixbuf_scaled;
	GConfValue *value;
	const gchar *path = NULL;
	gfloat scale;

	if (!GTK_WIDGET_REALIZED (data->bg_ebox))
		gtk_widget_realize (data->bg_ebox);

	/* Return if the background is tiny, we'll get called again when it 
	 * resizes anyway.
	 */
	width = data->bg_ebox->allocation.width;
	height = data->bg_ebox->allocation.height;
	if ((width <= 0) || (height <= 0)) return;
	
	value = gconf_entry_get_value (entry);
	if (value) path = gconf_value_get_string (value);
	if (!path || (!(pixbuf = gdk_pixbuf_new_from_file (path, NULL)))) {
		if (data->wallpaper) {
			g_object_unref (data->wallpaper);
			data->wallpaper = NULL;
			gtk_widget_queue_draw (data->bg_ebox);
		}
		return;
	}
	
	/* Create background pixmap */
	if (data->wallpaper) g_object_unref (data->wallpaper);
	data->wallpaper = gdk_pixmap_new (data->bg_ebox->window,
		width, height, -1);
	
	/* Scale and draw pixbuf */
	pwidth = gdk_pixbuf_get_width (pixbuf);
	pheight = gdk_pixbuf_get_height (pixbuf);
	if (((gfloat)pwidth / (gfloat)pheight) >
	    ((gfloat)width / (gfloat)height))
		scale = (gfloat)height/(gfloat)pheight;
	else
		scale = (gfloat)width/(gfloat)pwidth;
	pwidth *= scale;
	pheight *= scale;
	pixbuf_scaled = gdk_pixbuf_scale_simple (pixbuf, pwidth, pheight,
		GDK_INTERP_BILINEAR);
	if (pixbuf_scaled) {
		gdk_draw_pixbuf (data->wallpaper, NULL, pixbuf_scaled,
			0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_MAX, 0, 0);
		g_object_unref (pixbuf_scaled);
	}
	g_object_unref (pixbuf);
	
	/* Redraw */
	gtk_widget_queue_draw (data->bg_ebox);
}

int
main (int argc, char **argv)
{
	TodayData data;
	GOptionContext *context;
	GtkWidget *widget;
#ifndef STANDALONE
	gint x, y, w, h;
#endif
	
	static GOptionEntry entries[] = {
		{ NULL }
	};
	
	data.wallpaper = NULL;

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
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data.notebook), GTK_POS_BOTTOM);
	gtk_container_add (GTK_CONTAINER (data.window), data.notebook);
	gtk_widget_show (data.notebook);

	/* Add home page */
	widget = today_create_home_page (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_HOME, 6);
	gtk_widget_show (widget);

	/* Add new launcher page */
	widget = today_launcher_page_create (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_ADD, 6);
	gtk_widget_show (widget);

	/* Add running tasks page */
	widget = today_task_manager_page_create (&data);
	today_notebook_add_page_with_icon (data.notebook, widget,
		GTK_STOCK_EXECUTE, 6);
	gtk_widget_show (widget);
	
	/* Connect up signals */
	g_signal_connect (G_OBJECT (data.window), "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);
	
#if 0
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2", /* Moko */
		"gtk-icon-theme-name", "openmoko-standard",
		"gtk-xft-dpi", 285 * 1024,
		"gtk-font-name", "Sans 6",
		NULL);
#endif

#ifndef STANDALONE
	x = 0; y = 0; w = 480; h = 640;
	gtk_window_set_type_hint (GTK_WINDOW (data.window),
		GDK_WINDOW_TYPE_HINT_DESKTOP);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (data.window), TRUE);
	if (x_get_workarea (&x, &y, &w, &h)) {
		gtk_window_set_default_size (GTK_WINDOW (data.window), w, h);
		gtk_window_move (GTK_WINDOW (data.window), x, y);
	}
#else
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);
#endif
	
	/* Show and start */
	gtk_widget_show (data.window);

	/* Listen to wallpaper setting */
	gconf_client_add_dir (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER,
		(GConfClientNotifyFunc)wallpaper_notify,
		&data, NULL, NULL);
	gconf_client_notify (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER);

	gtk_main ();

	return 0;
}
