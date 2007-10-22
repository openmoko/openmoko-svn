
#include <gtk/gtk.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libjana-gtk/jana-gtk.h>
#include <libmokoui2/moko-finger-scroll.h>

static gchar *location;

typedef struct {
	GtkWidget *window;
	GtkWidget *map;
	GtkWidget *load_window;
	GtkWidget *load_bar;
	
	guint render_idle;
	
	gchar *location;
	gdouble zoom_level;
} WorldClockData;

static inline GtkToolItem *
worldclock_utils_toolbutton_new (const gchar *icon_name)
{
	GtkToolItem *button = gtk_tool_button_new_from_stock (icon_name);
	gtk_tool_item_set_expand (button, TRUE);
	return button;
}

static void
zoom_map (WorldClockData *data)
{
	if (data->zoom_level <= 0.95) {
		data->zoom_level = 1;
		gtk_widget_set_size_request (data->map, -1, -1);
	} else {
		gint width, height;
		gtk_window_get_size (GTK_WINDOW (data->window),
			&width, &height);
		gtk_widget_set_size_request (data->map,
			width * data->zoom_level,
			height * data->zoom_level);
	}
}

static void
zoom_in_clicked_cb (GtkToolButton *button, WorldClockData *data)
{
	data->zoom_level *= 1.2;
	zoom_map (data);
}

static void
zoom_out_clicked_cb (GtkToolButton *button, WorldClockData *data)
{
	data->zoom_level /= 1.2;
	zoom_map (data);
}

static gboolean
set_time (GtkWidget *map)
{
	JanaTime *time;

	time = jana_ecal_utils_time_now (location);
	jana_gtk_world_map_set_time (JANA_GTK_WORLD_MAP (map), time);
	g_object_unref (time);

	return TRUE;
}

static gboolean
render_idle (WorldClockData *data)
{
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (data->load_bar));
	gtk_widget_queue_draw (data->map);
	return TRUE;
}

static void
render_start_cb (JanaGtkWorldMap *map, WorldClockData *data)
{
	data->render_idle = g_timeout_add (
		1000/5, (GSourceFunc)render_idle, data);
	gtk_widget_show (data->load_window);
}

static void
render_stop_cb (JanaGtkWorldMap *map, WorldClockData *data)
{
	g_source_remove (data->render_idle);
	gtk_widget_hide (data->load_window);
}

int
main (int argc, char **argv)
{
	WorldClockData data;
	GtkToolItem *button;
	GtkWidget *vbox, *scroll, *toolbar, *ebox, *label, *load_vbox, *frame;
	guint id;
	
	gtk_init (&argc, &argv);
	
	data.zoom_level = 1;
	
	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	
	/* Create toolbar */
	toolbar = gtk_toolbar_new ();

	/* Zoom in button */
	button = worldclock_utils_toolbutton_new (GTK_STOCK_ZOOM_IN);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (button, "clicked",
		G_CALLBACK (zoom_in_clicked_cb), &data);

	/* Zoom out button */
	button = worldclock_utils_toolbutton_new (GTK_STOCK_ZOOM_OUT);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), button, 0);
	g_signal_connect (button, "clicked",
		G_CALLBACK (zoom_out_clicked_cb), &data);
	
	gtk_widget_show_all (toolbar);

	/* Create scrolling map */
	data.map = jana_gtk_world_map_new ();
	scroll = moko_finger_scroll_new ();
	moko_finger_scroll_add_with_viewport (MOKO_FINGER_SCROLL (scroll),
		data.map);
	g_object_set (G_OBJECT (scroll), "mode", MOKO_FINGER_SCROLL_MODE_PUSH,
		NULL);
	gtk_widget_show_all (scroll);
	g_signal_connect (data.map, "render_start",
		G_CALLBACK (render_start_cb), &data);
	g_signal_connect (data.map, "render_stop",
		G_CALLBACK (render_stop_cb), &data);
	
	/* Create rendering indicator */
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	ebox = gtk_event_box_new ();
	load_vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (load_vbox), 12);
	label = gtk_label_new ("Loading...");
	data.load_bar = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (load_vbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (load_vbox), data.load_bar, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (ebox), load_vbox);
	gtk_container_add (GTK_CONTAINER (frame), ebox);
	gtk_widget_show_all (frame);
	data.load_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_transient_for (GTK_WINDOW (data.load_window),
		GTK_WINDOW (data.window));
	gtk_window_set_position (GTK_WINDOW (data.load_window),
		GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_container_add (GTK_CONTAINER (data.load_window), frame);
	
	/* Pack */
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data.window), vbox);
	gtk_widget_show (vbox);
	
#if 0
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2", /* Moko */
		"gtk-icon-theme-name", "openmoko-standard",
		"gtk-xft-dpi", 285 * 1024,
		"gtk-font-name", "Sans 6",
		NULL);
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);
#endif

	location = jana_ecal_utils_guess_location ();
	id = g_timeout_add (1000 * 60 * 10, (GSourceFunc)set_time, data.map);
	set_time (data.map);
	
	g_signal_connect (data.window, "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (data.window);

	gtk_main ();
	
	g_source_remove (id);
	g_free (location);

	return 0;
}

