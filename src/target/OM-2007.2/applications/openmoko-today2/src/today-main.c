
#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <moko-finger-scroll.h>
#include <moko-stock.h>
#include <libtaku/launcher-util.h>
#include <libtaku/xutil.h>
#include <unistd.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libjana-gtk/jana-gtk.h>
#include "today.h"
#include "today-utils.h"
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
	if (data->dialer_item) launcher_start (data->window, data->dialer_item,
		(gchar *[]){ "openmoko-dialer", NULL }, TRUE, TRUE);
}

static void
today_contacts_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	if (data->contacts_item) launcher_start (data->window, data->contacts_item,
		(gchar *[]){ "openmoko-contacts", NULL }, TRUE, TRUE);
}

static void
today_messages_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	if (data->messages_item) launcher_start (data->window,
		data->messages_item,
		(gchar *[]){ "openmoko-messages", NULL }, TRUE, TRUE);
}

static void
today_dates_button_clicked_cb (GtkToolButton *button, TodayData *data)
{
	if (data->dates_item) launcher_start (data->window, data->dates_item,
		(gchar *[]){ "openmoko-dates", NULL }, TRUE, TRUE);
}

static gboolean
bg_expose_cb (GtkWidget *widget, GdkEventExpose *event, TodayData *data)
{
	if (data->wallpaper)
		gdk_draw_drawable (widget->window, widget->style->black_gc,
			data->wallpaper, 0, 0, 0, 0, -1, -1);
	
	return FALSE;
}

static gboolean
bg_child_expose_cb (GtkWidget *widget, GdkEventExpose *event, TodayData *data)
{
	cairo_t *cr;
/*	GtkWidget *parent;
	gint x = 0, y = 0;
	
	if (!data->wallpaper) return FALSE;
	
	parent = widget;
	do {
		if (!GTK_WIDGET_NO_WINDOW (parent)) {
			x += parent->allocation.x;
			y += parent->allocation.y;
		}
		parent = gtk_widget_get_parent (parent);
	} while (parent && (parent != data->bg_ebox));
	if (!parent) return FALSE;
	
	gdk_draw_drawable (widget->window, widget->style->black_gc,
		data->wallpaper, x, y,
		0, 0, -1, -1);*/
	
	/* Draw a semi-transparent rounded rectangle */
	cr = gdk_cairo_create (widget->window);
	cairo_translate (cr, widget->allocation.x, widget->allocation.y);
	cairo_new_path (cr);
	cairo_move_to (cr, 12, 6);
	cairo_rel_line_to (cr, widget->allocation.width - 24, 0);
	cairo_rel_curve_to (cr, 6, 0, 6, 6, 6, 6);
	cairo_rel_line_to (cr, 0, widget->allocation.height - 24);
	cairo_rel_curve_to (cr, 0, 6, -6, 6, -6, 6);
	cairo_rel_line_to (cr, -(widget->allocation.width - 24), 0);
	cairo_rel_curve_to (cr, -6, 0, -6, -6, -6, -6);
	cairo_rel_line_to (cr, 0, -(widget->allocation.height - 24));
	cairo_rel_curve_to (cr, 0, -6, 6, -6, 6, -6);
	cairo_close_path (cr);
	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_fill (cr);
	cairo_destroy (cr);
	
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
	gchar *date_str;
	
	time = jana_ecal_utils_time_now (data->location);
	jana_gtk_clock_set_time (JANA_GTK_CLOCK (data->clock), time);
	date_str = jana_utils_strftime (time, "%A, %d. %B %Y");
	gtk_label_set_text (GTK_LABEL (data->date_label), date_str);
	g_free (date_str);
	
	/* Update query every half-hour */
	if (data->dates_view && ((jana_time_get_minutes (time) % 30) == 0)) {
		JanaTime *end = jana_time_duplicate (time);
		jana_time_set_day (end, jana_time_get_day (end) + 1);
		jana_time_set_isdate (end, TRUE);
		jana_store_view_set_range (data->dates_view, time, end);
		g_object_unref (end);
	}
	
#if GLIB_CHECK_VERSION(2,14,0)
	g_timeout_add_seconds (60 - jana_time_get_seconds (time),
		(GSourceFunc)set_time_idle, data);
#else
	g_timeout_add ((60 - jana_time_get_seconds (time)) * 1000,
		(GSourceFunc)set_time_idle, data);
#endif

	g_object_unref (time);
	
	return FALSE;
}

static void
clock_clicked_cb (JanaGtkClock *clock, GdkEventButton *event, TodayData *data)
{
	/*if (data->clock_item) launcher_start (data->window, data->clock_item,
		(gchar *[]){ "openmoko-worldclock", NULL }, TRUE, TRUE);*/
}

static GtkWidget *
today_create_home_page (TodayData *data)
{
	GtkWidget *main_vbox, *align;
	
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
	gtk_widget_set_sensitive (GTK_WIDGET (data->dates_button), FALSE);

	data->messages_button = today_toolbutton_new ("openmoko-messages");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->messages_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (G_OBJECT (data->messages_button), "clicked",
		G_CALLBACK (today_messages_button_clicked_cb), data);
	gtk_widget_set_sensitive (GTK_WIDGET (data->messages_button), FALSE);

	data->contacts_button = today_toolbutton_new ("contacts");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->contacts_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (G_OBJECT (data->contacts_button), "clicked",
		G_CALLBACK (today_contacts_button_clicked_cb), data);
	gtk_widget_set_sensitive (GTK_WIDGET (data->contacts_button), FALSE);

	data->dial_button = today_toolbutton_new ("openmoko-dialer");
	gtk_toolbar_insert (GTK_TOOLBAR (data->home_toolbar),
		data->dial_button, 0);
	gtk_widget_show_all (data->home_toolbar);
	g_signal_connect (G_OBJECT (data->dial_button), "clicked",
		G_CALLBACK (today_dial_button_clicked_cb), data);
	gtk_widget_set_sensitive (GTK_WIDGET (data->dial_button), FALSE);

	/* Create event box with background */
	data->bg_ebox = gtk_event_box_new ();
	gtk_widget_set_app_paintable (data->bg_ebox, TRUE);
	g_signal_connect (data->bg_ebox, "expose-event",
		G_CALLBACK (bg_expose_cb), data);
	g_signal_connect (data->bg_ebox, "size-allocate",
		G_CALLBACK (bg_size_allocate_cb), data);
	gtk_box_pack_start (GTK_BOX (main_vbox), data->bg_ebox, TRUE, TRUE, 0);
	gtk_widget_show (data->bg_ebox);
	
	/* Get location and create clock widget */
	data->location = jana_ecal_utils_guess_location ();
	data->clock = jana_gtk_clock_new ();
	jana_gtk_clock_set_draw_shadow (JANA_GTK_CLOCK (data->clock), TRUE);
	gtk_widget_show (data->clock);
	g_signal_connect (data->clock, "clicked",
		G_CALLBACK (clock_clicked_cb), data);
	g_object_ref_sink (data->clock);

	/* Pack widgets */
	align = gtk_alignment_new (0.5, 0, 1, 0);
	data->message_box = today_pim_journal_box_new (data);
	gtk_container_add (GTK_CONTAINER (align), data->message_box);
	gtk_widget_set_app_paintable (align, TRUE);
	g_signal_connect (align, "expose-event",
		G_CALLBACK (bg_child_expose_cb), data);
	gtk_widget_show (data->message_box);
	gtk_widget_show (align);
	
	data->home_vbox = gtk_vbox_new (FALSE, 6);
	gtk_alignment_set_padding (GTK_ALIGNMENT (align), 6, 6, 6, 6);
	gtk_box_pack_end (GTK_BOX (data->home_vbox), align, FALSE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data->bg_ebox), data->home_vbox);
	gtk_widget_show (data->home_vbox);
	
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

static void
digital_clock_notify (GConfClient *client, guint cnxn_id,
		      GConfEntry *entry, TodayData *data)
{
	GConfValue *value;

	value = gconf_entry_get_value (entry);
	if (value && (!gconf_value_get_bool (value))) {
		gtk_aspect_frame_set (GTK_ASPECT_FRAME (data->date_aspect),
			1.0, 0.5, 1.0, FALSE);
		jana_gtk_clock_set_digital (JANA_GTK_CLOCK (data->clock),
			FALSE);
	} else {
		gtk_aspect_frame_set (GTK_ASPECT_FRAME (data->date_aspect),
			1.0, 0.5, 2.0, FALSE);
		jana_gtk_clock_set_digital (JANA_GTK_CLOCK (data->clock),
			TRUE);
	}
}

static void
small_clock_notify (GConfClient *client, guint cnxn_id,
		    GConfEntry *entry, TodayData *data)
{
	GConfValue *value;
	GtkWidget *parent;

	if ((parent = gtk_widget_get_parent (data->clock)))
		gtk_container_remove (GTK_CONTAINER (parent), data->clock);
	
	value = gconf_entry_get_value (entry);
	if (value && (gconf_value_get_bool (value))) {
		gtk_container_add (GTK_CONTAINER (
			data->date_aspect), data->clock);
	} else {
		gtk_box_pack_start (GTK_BOX (data->home_vbox),
			data->clock, TRUE, TRUE, 0);
	}
}

static void
location_notify (GConfClient *client, guint cnxn_id,
		 GConfEntry *entry, TodayData *data)
{
	GConfValue *value;

	value = gconf_entry_get_value (entry);
	if (value) {
		const gchar *location = gconf_value_get_string (value);
		if (location) {
			g_free (data->location);
			data->location = g_strdup (location);
		}
	}
}

static gboolean active = TRUE;

static void
set_window_title (TodayData *data)
{
	gtk_window_set_title (GTK_WINDOW (data->window),
		data->network_name ? data->network_name :
		"Registering...");
}

static void
is_active_notify (GObject *window, GParamSpec *arg1, TodayData *data)
{
	g_object_get (window, "is-active", &active, NULL);
	if (!active)
		gtk_window_set_title (GTK_WINDOW (data->window), "Home");
	else
		set_window_title (data);
}

static void
provider_changed_cb (DBusGProxy *proxy, const gchar *name, TodayData *data)
{
	g_free (data->network_name);
	data->network_name = g_strdup (name);
	
	if (active) set_window_title (data);
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
	TodayData data;
	DBusGConnection *connection;
	GOptionContext *context;
	GtkWidget *widget;
#ifndef STANDALONE
	gint x, y, w, h;
#endif
	GError *error = NULL;
	
	static GOptionEntry entries[] = {
		{ NULL }
	};
	
	data.wallpaper = NULL;
	data.network_name = NULL;

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

	/* Get phone-kit Network dbus proxy */
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to get dbus connection: %s", error->message);
		g_error_free (error);
		data.network_proxy = NULL;
	} else {
		data.network_proxy = dbus_g_proxy_new_for_name (connection,
			"org.openmoko.PhoneKit",
			"/org/openmoko/PhoneKit/Network",
			"org.openmoko.PhoneKit.Network");
		dbus_g_proxy_add_signal (data.network_proxy,
			"ProviderChanged", G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal (data.network_proxy,
			"ProviderChanged", G_CALLBACK (provider_changed_cb),
			&data, NULL);
	}

	/* Create widgets */
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (data.window), _("Home"));
	
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
	/* This block here to ease testing, please don't remove! */
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2",
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
	window = data.window;
	x_monitor_workarea (gtk_widget_get_screen (window), workarea_changed);
#else
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);
#endif
	
	/* Show and start */
	g_signal_connect (data.window, "notify::is-active",
		G_CALLBACK (is_active_notify), &data);
	gtk_widget_show (data.window);

	/* Listen to settings */
	gconf_client_add_dir (gconf_client_get_default (),
		JANA_ECAL_LOCATION_KEY_DIR, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add (gconf_client_get_default (),
		JANA_ECAL_LOCATION_KEY,
		(GConfClientNotifyFunc)location_notify,
		&data, NULL, NULL);
	
	gconf_client_add_dir (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER,
		(GConfClientNotifyFunc)wallpaper_notify,
		&data, NULL, NULL);
	gconf_client_notify_add (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_DIGITAL,
		(GConfClientNotifyFunc)digital_clock_notify,
		&data, NULL, NULL);
	gconf_client_notify_add (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_SMALLCLOCK,
		(GConfClientNotifyFunc)small_clock_notify,
		&data, NULL, NULL);
	
	/* Fire off signals */
	gconf_client_notify (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_WALLPAPER);
	gconf_client_notify (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_DIGITAL);
	gconf_client_notify (gconf_client_get_default (),
		GCONF_POKY_INTERFACE_PREFIX GCONF_POKY_SMALLCLOCK);

	gtk_main ();

	return 0;
}
