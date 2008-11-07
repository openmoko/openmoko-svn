
#include <string.h>
#include <moko-stock.h>
#include <moko-finger-scroll.h>
#include <libtaku/taku-table.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <glib/gi18n.h>
#include "today-task-manager.h"
#include "today-utils.h"

#define DEFAULT_WINDOW_ICON_NAME "gnome-fs-executable"

/* NOTE: Lots of this code taken from windowselector applet in
 * 	 matchbox-panel-2.
 */

enum {
	_NET_CLIENT_LIST,
        UTF8_STRING,
        _NET_WM_VISIBLE_NAME,
        _NET_WM_NAME,
        _NET_WM_ICON,
	_NET_WM_WINDOW_TYPE,
	_NET_CLOSE_WINDOW,
	_NET_ACTIVE_WINDOW,
        N_ATOMS
};

static gboolean hidden = TRUE;
static Atom atoms[N_ATOMS];
static GtkIconSize icon_size;

static GdkFilterReturn
filter_func (GdkXEvent *xevent, GdkEvent *event, TodayData *data);

/* Retrieves the UTF-8 property @atom from @window */
static char *
get_utf8_property (TodayData *data, Window window, Atom atom)
{
        GdkDisplay *display;
        Atom type;
        int format, result;
        gulong nitems, bytes_after;
        guchar *val;
        char *ret;

        display = gtk_widget_get_display (GTK_WIDGET (data->tasks_table));

        type = None;
        val = NULL;
        
        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     window,
                                     atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     atoms[UTF8_STRING],
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &val);  
	gdk_flush ();
        if (gdk_error_trap_pop () || result != Success)
                return NULL;
  
        if (type != atoms[UTF8_STRING] || format != 8 || nitems == 0) {
                if (val)
                        XFree (val);

                return NULL;
        }

        if (!g_utf8_validate ((char *) val, nitems, NULL)) {
                g_warning ("Invalid UTF-8 in window title");

                XFree (val);

                return NULL;
        }
        
        ret = g_strndup ((char *) val, nitems);
  
        XFree (val);
  
        return ret;
}

/* Retrieves the text property @atom from @window */
static char *
get_text_property (TodayData *data, Window window, Atom atom)
{
        GdkDisplay *display;
        XTextProperty text;
        char *ret, **list;
        int result, count;

        display = gtk_widget_get_display (GTK_WIDGET (data->tasks_table));

        gdk_error_trap_push ();
        result = XGetTextProperty (GDK_DISPLAY_XDISPLAY (display),
                                   window,
                                   &text,
                                   atom);
	gdk_flush ();
        if (gdk_error_trap_pop () || result == 0)
                return NULL;

        count = gdk_text_property_to_utf8_list
                        (gdk_x11_xatom_to_atom (text.encoding),
                         text.format,
                         text.value,
                         text.nitems,
                         &list);
        if (count > 0) {
                int i;

                ret = list[0];

                for (i = 1; i < count; i++)
                        g_free (list[i]);
                g_free (list);
        } else
                ret = NULL;

        if (text.value)
                XFree (text.value);
  
        return ret;
}

/* Retrieves the name for @window */
static char *
window_get_name (TodayData *data, Window window)
{
        char *name;
  
        name = get_utf8_property (data,
                                  window,
                                  atoms[_NET_WM_VISIBLE_NAME]);
        if (name == NULL) {
                name = get_utf8_property (data,
                                          window,
                                          atoms[_NET_WM_NAME]);
        } if (name == NULL) {
                name = get_text_property (data,
                                          window,
                                          XA_WM_NAME);
        } if (name == NULL) {
                name = g_strdup (_("(untitled)"));
        }

        return name;
}

/* Retrieves the icon for @window */
static GdkPixbuf *
window_get_icon (TodayData *tdata, Window window)
{
        GdkPixbuf *pixbuf;
        GdkDisplay *display;
        Atom type;
        int format, result;
        int ideal_width, ideal_height, ideal_size;
        int best_width, best_height, best_size;
        int i, npixels, ip;
        gulong nitems, bytes_after, *data, *datap, *best_data;
        GtkSettings *settings;
        guchar *pixdata;

        /* First, we read the contents of the _NET_WM_ICON property */
        display = gtk_widget_get_display (GTK_WIDGET (tdata->tasks_table));

        type = 0;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     window,
                                     atoms[_NET_WM_ICON],
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_CARDINAL,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &data);
	gdk_flush ();
        if (gdk_error_trap_pop () || result != Success)
                return NULL;

        if (type != XA_CARDINAL || nitems < 3) {
                XFree (data);

                return NULL;
        }

        /* Got it. Now what size icon are we looking for? */
        settings = gtk_widget_get_settings (GTK_WIDGET (tdata->tasks_table));
        gtk_icon_size_lookup_for_settings (settings,
                                           icon_size,
                                           &ideal_width,
                                           &ideal_height);

        ideal_size = (ideal_width + ideal_height) / 2;

        /* Try to find the closest match */
        best_data = NULL;
        best_width = best_height = best_size = 0;

        datap = data;
        while (nitems > 0) {
                int cur_width, cur_height, cur_size;
                gboolean replace;

                if (nitems < 3)
                        break;

                cur_width = datap[0];
                cur_height = datap[1];
                cur_size = (cur_width + cur_height) / 2;

                if (nitems < (2 + cur_width * cur_height))
                        break;

                if (!best_data) {
                        replace = TRUE;
                } else {
                        /* Always prefer bigger to smaller */
                        if (best_size < ideal_size &&
                            cur_size > best_size)
                                replace = TRUE;
                        /* Prefer smaller bigger */
                        else if (best_size > ideal_size &&
                                 cur_size >= ideal_size && 
                                 cur_size < best_size)
                                replace = TRUE;
                        else
                                replace = FALSE;
                }

                if (replace) {
                        best_data = datap + 2;
                        best_width = cur_width;
                        best_height = cur_height;
                        best_size = cur_size;
                }

                datap += (2 + cur_width * cur_height);
                nitems -= (2 + cur_width * cur_height);
        }

        if (!best_data) {
                XFree (data);

                return NULL;
        }

        /* Got it. Load it into a pixbuf. */
        npixels = best_width * best_height;
        pixdata = g_new (guchar, npixels * 4);
        
        for (i = 0, ip = 0; i < npixels; i++) {
                /* red */
                pixdata[ip] = (best_data[i] >> 16) & 0xff;
                ip++;

                /* green */
                pixdata[ip] = (best_data[i] >> 8) & 0xff;
                ip++;

                /* blue */
                pixdata[ip] = best_data[i] & 0xff;
                ip++;

                /* alpha */
                pixdata[ip] = best_data[i] >> 24;
                ip++;
        }

        pixbuf = gdk_pixbuf_new_from_data (pixdata,
                                           GDK_COLORSPACE_RGB,
                                           TRUE,
                                           8,
                                           best_width,
                                           best_height,
                                           best_width * 4,
                                           (GdkPixbufDestroyNotify) g_free,
                                           NULL);

        /* Scale if necessary */
        if (best_width != ideal_width &&
            best_height != ideal_height) {
                GdkPixbuf *scaled;

                scaled = gdk_pixbuf_scale_simple (pixbuf,
                                                  ideal_width,
                                                  ideal_height,
                                                  GDK_INTERP_BILINEAR);
                g_object_unref (pixbuf);

                pixbuf = scaled;
        }

        /* Cleanup */
        XFree (data);
  
        /* Return */
        return pixbuf;
}

static void
today_task_manager_free_tasks (TodayData *data)
{
	GList *c, *children;

	/* Free window list */
	children = gtk_container_get_children (
		GTK_CONTAINER (data->tasks_table));
	
	for (c = children; c; c = c->next) {
		GtkWidget *child = GTK_WIDGET (c->data);
		if (TAKU_IS_TILE (child))
			gtk_container_remove (GTK_CONTAINER (
				data->tasks_table), child);
	}
}

static void
today_task_manager_populate_tasks (TodayData *data)
{
        GdkDisplay *display;
	GdkScreen *screen;
	GdkWindow *current;
        Atom type;
        int format, result, i;
        gulong nitems, bytes_after;
        Window *windows;

	/* Empty list */
	today_task_manager_free_tasks (data);
	
	/* Return if our main window has gone (e.g. on quit) */
	if ((!data->window) || (!data->window->window)) return;
	
        /* Retrieve list of app windows from root window */
        display = gtk_widget_get_display (data->tasks_table);
	screen = gtk_widget_get_screen (data->tasks_table);

        type = None;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     GDK_WINDOW_XWINDOW (data->root_window),
                                     atoms[_NET_CLIENT_LIST],
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_WINDOW,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &windows);
	gdk_flush ();
        if (gdk_error_trap_pop () || result != Success)
                return;

        if (type != XA_WINDOW) {
                XFree (windows);

                return;
        }

        /* Load into menu */
	current = gdk_screen_get_active_window (screen);
        for (i = 0; i < nitems; i++) {
                char *name;
		GtkWidget *task_tile;
		GdkPixbuf *icon;
		GdkWindow *window;

		if (current && GDK_WINDOW_XID (current) == windows[i]) continue;
		if (GDK_WINDOW_XID (data->window->window) == windows[i])
			continue;
		
		if (!(window = gdk_window_foreign_new_for_display (
			display, windows[i]))) continue;
		
		gdk_error_trap_push ();
		if (gdk_window_get_type_hint (window) !=
		    GDK_WINDOW_TYPE_HINT_NORMAL) {
			gdk_flush ();
			gdk_error_trap_pop ();
			continue;
		}
		gdk_flush ();
		if (gdk_error_trap_pop ()) continue;
		
                name = window_get_name (data, windows[i]);
                task_tile = taku_icon_tile_new ();
		taku_icon_tile_set_primary (TAKU_ICON_TILE (task_tile), name);
                g_free (name);
		taku_icon_tile_set_secondary (TAKU_ICON_TILE (task_tile), "");
		
		icon = window_get_icon (data, windows[i]);
		if (icon) {
			taku_icon_tile_set_pixbuf (
				TAKU_ICON_TILE (task_tile), icon);
			g_object_unref (icon);
		} else {
			taku_icon_tile_set_icon_name (
				TAKU_ICON_TILE (task_tile),
				DEFAULT_WINDOW_ICON_NAME);
		}
		
                g_object_set_data (G_OBJECT (task_tile),
                                   "window",
                                   GUINT_TO_POINTER (windows[i]));

                /*g_signal_connect (task_tile,
                                  "activate",
                                  G_CALLBACK (window_menu_item_activate_cb),
                                  applet);*/

		gtk_container_add (GTK_CONTAINER (data->tasks_table),
			task_tile);
                gtk_widget_show (task_tile);
        }
	if (current) g_object_unref (current);

	/* If no windows were found, insert an insensitive "No tasks" item */
	if (nitems == 0) {
		GtkWidget *task_tile;
		
		task_tile = taku_icon_tile_new ();
		taku_icon_tile_set_primary (TAKU_ICON_TILE (task_tile),
			_("No active tasks"));
		
		gtk_widget_set_sensitive (task_tile, FALSE);

		gtk_container_add (GTK_CONTAINER (data->tasks_table),
			task_tile);
                gtk_widget_show (task_tile);
		
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->killall_button), FALSE);
        } else {
		gtk_widget_set_sensitive (
			GTK_WIDGET (data->killall_button), TRUE);
        }

	/* Cleanup */
	XFree (windows);
}

static void
set_focus_cb (GtkWindow *window, GtkWidget *widget, TodayData *data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data->kill_button),
		TAKU_IS_TILE (widget));
	gtk_widget_set_sensitive (GTK_WIDGET (data->switch_button),
		TAKU_IS_TILE (widget));
}

static void
page_shown (TodayData *data)
{
	today_task_manager_populate_tasks (data);
	g_signal_connect (data->window, "set-focus",
		G_CALLBACK (set_focus_cb), data);
}

static void
page_hidden (TodayData *data)
{
	today_task_manager_free_tasks (data);
	g_signal_handlers_disconnect_by_func (data->window, set_focus_cb, data);
}

static void
today_task_manager_notify_visible_cb (GObject *gobject,
				      GParamSpec *arg1,
				      TodayData *data)
{
	if ((!hidden) && (!GTK_WIDGET_VISIBLE (gobject))) {
		hidden = TRUE;
		page_hidden (data);
	}
}

static gboolean
today_task_manager_visibility_notify_event_cb (GtkWidget *widget,
					       GdkEventVisibility *event,
					       TodayData *data)
{
	if (((event->state == GDK_VISIBILITY_PARTIAL) ||
	     (event->state == GDK_VISIBILITY_UNOBSCURED)) && (hidden)) {
		hidden = FALSE;
		page_shown (data);
	} else if ((event->state == GDK_VISIBILITY_FULLY_OBSCURED) &&
		   (!hidden)) {
		hidden = TRUE;
		page_hidden (data);
	}
	
	return FALSE;
}

static void
today_task_manager_unmap_cb (GtkWidget *widget, TodayData *data)
{
	if (!hidden) {
		hidden = TRUE;
		page_hidden (data);
	}
}

static void
screen_changed_cb (GtkWidget *button, GdkScreen *old_screen, TodayData *data)
{
        GdkScreen *screen;
        GdkDisplay *display;
        GdkEventMask events;

        if (data->root_window) {
                gdk_window_remove_filter (data->root_window,
                                          (GdkFilterFunc) filter_func,
                                          data);
        }

        screen = gtk_widget_get_screen (data->tasks_table);
        display = gdk_screen_get_display (screen);

        /* Get atoms */
        atoms[_NET_CLIENT_LIST] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_CLIENT_LIST");
        atoms[UTF8_STRING] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "UTF8_STRING");
        atoms[_NET_WM_NAME] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_NAME");
        atoms[_NET_WM_VISIBLE_NAME] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_VISIBLE_NAME");
        atoms[_NET_WM_ICON] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_ICON");
	atoms[_NET_WM_WINDOW_TYPE] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_WINDOW_TYPE");
	atoms[_NET_CLOSE_WINDOW] =
		gdk_x11_get_xatom_by_name_for_display
			(display, "_NET_CLOSE_WINDOW");
	atoms[_NET_ACTIVE_WINDOW] =
		gdk_x11_get_xatom_by_name_for_display
			(display, "_NET_ACTIVE_WINDOW");
        
        /* Get root window */
        data->root_window = gdk_screen_get_root_window (screen);

        /* Watch _NET_CLIENT_LIST */
        events = gdk_window_get_events (data->root_window);
        if ((events & GDK_PROPERTY_CHANGE_MASK) == 0) {
                gdk_window_set_events (data->root_window,
                                       events & GDK_PROPERTY_CHANGE_MASK);
        }
        
        gdk_window_add_filter (data->root_window,
                               (GdkFilterFunc) filter_func,
                               data);
        
        /* Rebuild list if around */
	if (!hidden) today_task_manager_populate_tasks (data);
}

/* Something happened on the root window */
static GdkFilterReturn
filter_func (GdkXEvent *xevent, GdkEvent *event, TodayData *data)
{
        XEvent *xev;

        xev = (XEvent *) xevent;

        if (xev->type == PropertyNotify) {
                if (xev->xproperty.atom ==
                    atoms[_NET_CLIENT_LIST]) {
                        /* _NET_CLIENT_LIST changed.
                         * Rebuild menu if around. */
                        if (!hidden)
                                today_task_manager_populate_tasks (data);
                }
        }

        return GDK_FILTER_CONTINUE;
}

static void
today_task_manager_kill (TodayData *data, GdkWindow *window)
{
	/* NOTE: See
	 * http://standards.freedesktop.org/wm-spec/wm-spec-1.3.html#id2506711
	 */
	if (GDK_IS_WINDOW (window)) {
		XEvent ev;
		memset(&ev, 0, sizeof(ev));

		ev.xclient.type         = ClientMessage;
		ev.xclient.window       = GDK_WINDOW_XID (window);
		ev.xclient.message_type = atoms[_NET_CLOSE_WINDOW];
		ev.xclient.format       = 32;
		ev.xclient.data.l[0]    = CurrentTime;
		ev.xclient.data.l[1]    = GDK_WINDOW_XID (data->root_window);

		gdk_error_trap_push();
		XSendEvent (GDK_WINDOW_XDISPLAY (window),
			GDK_WINDOW_XID (data->root_window), False,
			SubstructureRedirectMask, &ev);

		XSync(GDK_DISPLAY(),FALSE);
		gdk_flush ();
		gdk_error_trap_pop ();
		
		/* The following code looks equivalent to me, but isn't.. */
		/*GdkEvent *event = gdk_event_new (GDK_CLIENT_EVENT);
		((GdkEventAny *)event)->window = g_object_ref (window);
		((GdkEventAny *)event)->send_event = TRUE;
		((GdkEventClient *)event)->message_type =
			gdk_x11_xatom_to_atom (atoms[_NET_CLOSE_WINDOW]);
		((GdkEventClient *)event)->data_format = 32;
		((GdkEventClient *)event)->data.l[0] = GDK_CURRENT_TIME;
		((GdkEventClient *)event)->data.l[1] = GDK_WINDOW_XID (
			data->root_window);
		
		gdk_event_send_client_message (event,
			GDK_WINDOW_XID (data->root_window));
		gdk_event_free (event);*/
	}
}

static void
today_task_manager_kill_clicked_cb (GtkToolButton *widget, TodayData *data)
{
	GdkWindow *window;
	GdkDisplay *display;
	Window xid;
	GtkWidget *tile = gtk_window_get_focus (GTK_WINDOW (data->window));
	
	if (!TAKU_IS_ICON_TILE (tile)) return;
	
        display = gtk_widget_get_display (data->tasks_table);
	xid = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tile), "window"));
	
	gdk_error_trap_push ();
	window = gdk_window_foreign_new_for_display (display, xid);
	gdk_flush ();
	gdk_error_trap_pop ();

	if (window) today_task_manager_kill (data, window);
}

static void
today_task_manager_raise_clicked_cb (GtkToolButton *widget, TodayData *data)
{
	XEvent ev;
	GdkWindow *window;
	GdkDisplay *display;
	Window xid;
	GtkWidget *tile = gtk_window_get_focus (GTK_WINDOW (data->window));
	
	if (!TAKU_IS_ICON_TILE (tile)) return;
	
        display = gtk_widget_get_display (data->tasks_table);
	xid = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tile), "window"));
	window = gdk_window_foreign_new_for_display (display, xid);
	
	/* NOTE: gdk_window_raise doesn't work? */
	/*gdk_window_raise (window);*/

	/* Send a _NET_ACTIVE_WINDOW message */
	/* See
	 * http://standards.freedesktop.org/wm-spec/wm-spec-1.3.html#id2506353
	 */
	memset(&ev, 0, sizeof(ev));

	ev.xclient.type         = ClientMessage;
	ev.xclient.window       = GDK_WINDOW_XID (window);
	ev.xclient.message_type = atoms[_NET_ACTIVE_WINDOW];
	ev.xclient.format       = 32;
	ev.xclient.data.l[0]	= 1;
	ev.xclient.data.l[1]    = CurrentTime;
	ev.xclient.data.l[2]    = GDK_WINDOW_XID (data->root_window);

	gdk_error_trap_push();
	XSendEvent (GDK_WINDOW_XDISPLAY (window),
		GDK_WINDOW_XID (data->root_window), False,
		SubstructureRedirectMask, &ev);

	XSync(GDK_DISPLAY(),FALSE);
	gdk_flush ();
	gdk_error_trap_pop();
}

static void
today_task_manager_killall_clicked_cb (GtkToolButton *widget, TodayData *data)
{
	GList *c, *children;
	GdkDisplay *display;
	GtkWidget *dialog;
	gint response;
	
	/* Confirmation dialog first */
	dialog = gtk_message_dialog_new (GTK_WINDOW (data->window),
		GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
					 _("Are you sure you want to close all applications?"));
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	if (response != GTK_RESPONSE_YES) return;
	
        display = gtk_widget_get_display (data->tasks_table);

	/* Kill all windows */
	children = gtk_container_get_children (
		GTK_CONTAINER (data->tasks_table));
	
	for (c = children; c; c = c->next) {
		GdkWindow *window;
		Window xid;
		GtkWidget *child = GTK_WIDGET (c->data);

		if (!TAKU_IS_ICON_TILE (child)) continue;

		xid = GPOINTER_TO_UINT (g_object_get_data (
			G_OBJECT (child), "window"));
		window = gdk_window_foreign_new_for_display (display, xid);
		
		today_task_manager_kill (data, window);
	}
}

GtkWidget *
today_task_manager_page_create (TodayData *data)
{
	GtkWidget *vbox, *toolbar, *viewport, *scroll;
	
	icon_size = gtk_icon_size_from_name ("taku-icon");
	if (icon_size == GTK_ICON_SIZE_INVALID) {
		icon_size = GTK_ICON_SIZE_BUTTON;
	}
	
	vbox = gtk_vbox_new (FALSE, 0);
	
	/* Create toolbar */
	toolbar = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);

	/* Kill all apps button */
	data->kill_button = gtk_tool_button_new_from_stock (
		MOKO_STOCK_FOLDER_DELETE);
	gtk_tool_item_set_expand (data->kill_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->kill_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (data->kill_button, "clicked",
		G_CALLBACK (today_task_manager_killall_clicked_cb), data);

	/* Switch to app button */
	data->switch_button = gtk_tool_button_new_from_stock (GTK_STOCK_JUMP_TO);
	gtk_tool_item_set_expand (data->switch_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->switch_button, 0);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 0);
	g_signal_connect (data->switch_button, "clicked",
		G_CALLBACK (today_task_manager_raise_clicked_cb), data);

	/* Kill app button */
	data->killall_button = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
	gtk_tool_item_set_expand (data->killall_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->killall_button, 0);
	g_signal_connect (data->killall_button, "clicked",
		G_CALLBACK (today_task_manager_kill_clicked_cb), data);

	/* Viewport / tasks table */
	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_NONE);
	
	data->tasks_table = taku_table_new ();
	gtk_container_add (GTK_CONTAINER (viewport), data->tasks_table);
	gtk_widget_show (data->tasks_table);

	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show (viewport);
	gtk_widget_show (scroll);
	
	gtk_widget_show_all (toolbar);
	
	data->root_window = NULL;
	
	gtk_widget_add_events (viewport, GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (G_OBJECT (viewport), "visibility-notify-event",
		G_CALLBACK (today_task_manager_visibility_notify_event_cb),
		data);
	g_signal_connect (G_OBJECT (data->tasks_table), "notify::visible",
		G_CALLBACK (today_task_manager_notify_visible_cb), data);
        g_signal_connect (G_OBJECT (data->tasks_table), "screen-changed",
		G_CALLBACK (screen_changed_cb), data);
	g_signal_connect (G_OBJECT (vbox), "unmap",
		G_CALLBACK (today_task_manager_unmap_cb), data);
	
	return vbox;
}
