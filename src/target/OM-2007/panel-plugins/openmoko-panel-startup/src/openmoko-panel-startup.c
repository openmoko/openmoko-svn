/*
 * startup-monitor - A tray app that provides feedback
 *                             on application startup.
 *
 * Copyright 2004, Openedhand Ltd. By Matthew Allum <mallum@o-hand.com>
 * Copyright 2007 OpenMoko Inc. By Stefan Schmidt <stefan@openmoko.org>
 *
 * Based very roughly on GPE's startup monitor.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <libmokoui/moko-panel-applet.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

/* FIXME */
#define USE_LIBSN

#ifdef USE_LIBSN
  #define SN_API_NOT_YET_FROZEN 1
  #include <libsn/sn.h>
#endif

#include <string.h>

#define TIMEOUT		   20
#define HOURGLASS_PIXMAPS 8

typedef struct {
	GtkImage *image;
	GdkPixbuf *hglass[HOURGLASS_PIXMAPS];
	const char *last_icon;
	guint timeout_id;
	GdkWindow *root_window;
	GtkWidget* eventbox;
	SnDisplay *sn_display;
} StartupApplet;

typedef struct LaunchList LaunchList;

struct LaunchList {
	char *id;
	time_t when;
	LaunchList *next;
};

static GdkFilterReturn filter_func(GdkXEvent *gdk_xevent,
				GdkEvent *event, StartupApplet *applet);

static gboolean applet_main(StartupApplet *applet);

/* Applet destroyed */
static void startup_applet_free(StartupApplet *applet)
{
	gdk_window_remove_filter (applet->root_window,
					(GdkFilterFunc) filter_func, applet);
	g_source_remove(applet->timeout_id);
	g_slice_free(StartupApplet, applet);
}

/*
 * Lazy boy globals :/
 */
static LaunchList *launch_list = NULL;
static gboolean hourglass_shown = FALSE;
static int hourglass_cur_frame_n = 0;


static void show_hourglass(StartupApplet *applet)
{
	g_message("Entered %s", G_STRFUNC);
	gtk_widget_show_all(GTK_WIDGET (applet->eventbox));
	hourglass_shown = TRUE;
}

static void hide_hourglass(StartupApplet *applet)
{
	g_message("Entered %s", G_STRFUNC);
	gtk_widget_hide_all(GTK_WIDGET (applet->eventbox));
	hourglass_shown = FALSE;
}

static void monitor_event_func(SnMonitorEvent *event, void *user_data)
{
	SnMonitorContext *context;
	SnStartupSequence *sequence;
	const char *id;
	time_t t;
	StartupApplet *applet = (StartupApplet *) user_data;

	g_message("Entered %s", G_STRFUNC);
	context = sn_monitor_event_get_context(event);
	sequence = sn_monitor_event_get_startup_sequence(event);
	id = sn_startup_sequence_get_id(sequence);

	switch (sn_monitor_event_get_type(event)) {
	case SN_MONITOR_EVENT_INITIATED:
		{
			g_message("Entered SN_MONITOR_EVENT_INITIATED");

			/* Set up a timeout that will be called every 0.5 seconds */
			applet->timeout_id = g_timeout_add(500,
					   (GSourceFunc) applet_main, applet);

			LaunchList *item = launch_list;

			/* Reset counter */
			hourglass_cur_frame_n = 0;

			/* Add a new launch at the end of LaunchList */
			if (item == NULL) {
				launch_list = item = malloc(sizeof(LaunchList));
			} else {
				while (item->next != NULL)
					item = item->next;
				item->next = malloc(sizeof(LaunchList));
				item = item->next;
			}

			item->next = NULL;
			item->id = g_strdup(id);
			t = time(NULL);
			item->when = t + TIMEOUT;

			if (!hourglass_shown)
				show_hourglass(applet);
		}
		break;

	case SN_MONITOR_EVENT_COMPLETED:
	case SN_MONITOR_EVENT_CANCELED:
		{
			g_message("Entered SN_MONITOR_EVENT_CANCELED/COMPLETED");
			LaunchList *item = launch_list, *last_item = NULL;

			/* Find actual list item and free it*/
			while (item != NULL) {
				if (!strcmp(item->id, id)) {
					if (last_item == NULL)
						launch_list = item->next;
					else
						last_item->next = item->next;

					free(item->id);
					free(item);

					break;
				}
				last_item = item;
				item = item->next;
			}

			if (launch_list == NULL && hourglass_shown)
				hide_hourglass(applet);

			g_source_remove(applet->timeout_id);
		}
		break;
	default:
		break;		/* Nothing */
	}
}

static gboolean applet_main(StartupApplet *applet)
{
	LaunchList *item = launch_list;
	LaunchList *last_item = NULL;
	time_t t;

	g_message("Entered %s", G_STRFUNC);

	if (!hourglass_shown)
		return TRUE;

	t = time(NULL);

	/* handle launchee timeouts */
	while (item) {
		if ((item->when - t) <= 0) {
			if (last_item == NULL)
				launch_list = item->next;
			else
				last_item->next = item->next;

			free(item->id);
			free(item);

			break;
		}

		last_item = item;
		item = item->next;
	}

	if (launch_list == NULL && hourglass_shown) {
		hide_hourglass(applet);
		return TRUE;
	}

	hourglass_cur_frame_n++;
	if (hourglass_cur_frame_n == 8)
		hourglass_cur_frame_n = 0;

	g_message("hourglass_cur_frame_n =%i", hourglass_cur_frame_n);

    gtk_image_set_from_pixbuf( applet->image, applet->hglass[hourglass_cur_frame_n] );

	return TRUE;
}

static GdkFilterReturn filter_func(GdkXEvent *gdk_xevent, GdkEvent *event, StartupApplet *applet) {
	XEvent *xevent;
	xevent = (XEvent *) gdk_xevent;
	gboolean ret;

	ret = sn_display_process_event(applet->sn_display, xevent);

	g_message("%s: sn_display_process return value: %i", G_STRFUNC, ret);

	return GDK_FILTER_CONTINUE;
}

G_MODULE_EXPORT GtkWidget *mb_panel_applet_create(const char *id,
						  GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = MOKO_PANEL_APPLET(moko_panel_applet_new());

	StartupApplet *applet;
	Display *xdisplay;
	SnMonitorContext *context;

	/* Create applet data structure */
	applet = g_slice_new(StartupApplet);

	//applet->last_icon = NULL;

	/* Create image */
    applet->image = GTK_IMAGE(gtk_image_new());

	gtk_widget_set_name( GTK_WIDGET(applet->image), "MatchboxPanelStartupMonitor" );
    g_object_weak_ref( G_OBJECT(applet->image), (GWeakNotify) startup_applet_free, applet );

	/* preload pixbufs */
	guint i = 0;
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-0.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-1.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-2.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-3.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-4.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-5.png", NULL);
	applet->hglass[i++] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-6.png", NULL);
	applet->hglass[i] = gdk_pixbuf_new_from_file(PKGDATADIR "/hourglass-7.png", NULL);

	xdisplay = GDK_DISPLAY_XDISPLAY
				(gtk_widget_get_display(GTK_WIDGET (applet->image)));

	applet->sn_display = sn_display_new (xdisplay, NULL, NULL);

	context = sn_monitor_context_new (applet->sn_display, DefaultScreen(xdisplay),
					monitor_event_func, (void *)applet, NULL);

	/* We have to select for property events on at least one
	 * root window (but not all as INITIATE messages go to
	 * all root windows)
	 */
	XSelectInput (xdisplay, DefaultRootWindow(xdisplay), PropertyChangeMask);

	/* Get root window */
	//applet->root_window = gdk_screen_get_root_window
	//					(gtk_widget_get_screen( GTK_WIDGET (applet->image)));

	applet->root_window = gdk_window_lookup_for_display(gdk_x11_lookup_xdisplay(xdisplay), 0);

	gdk_window_add_filter (applet->root_window, (GdkFilterFunc) filter_func, applet);

	/* Show! */
	applet->eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(applet->eventbox), GTK_WIDGET(applet->image));
	gtk_event_box_set_visible_window(applet->eventbox, FALSE);
	moko_panel_applet_set_widget( MOKO_PANEL_APPLET(mokoapplet), GTK_WIDGET(applet->image) );
	gtk_widget_show_all(GTK_WIDGET(mokoapplet));

	return GTK_WIDGET(mokoapplet);
}
