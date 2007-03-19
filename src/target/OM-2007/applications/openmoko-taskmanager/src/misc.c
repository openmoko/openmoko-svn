/**
 *  misc.c
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "misc.h"

gboolean 
moko_X_ev_init (Display *dpy, GtkWidget *gtkwidget) 
{
    if (my_win == None)
    	  my_win = GDK_WINDOW_XWINDOW (gtkwidget);
    if (!moko_initialize_X_atoms(dpy))
    	  return FALSE;
    return TRUE;
}

gboolean 
moko_update_net_undocked_client_list(Display *dpy, Window** list, guint * nr) 
{
    Atom actual_type, type;
    Window *temp_list;
    int actual_format;
    unsigned long nitems, bytes_after = 0;
    unsigned char *prop = NULL;
    int counter = 0, i = 0, rc;

    moko_initialize_X_atoms(dpy);
    
    rc = XGetWindowProperty (dpy, DefaultRootWindow (dpy), atoms[_NET_CLIENT_LIST],
    						0, G_MAXLONG, False, XA_WINDOW, &actual_type, &actual_format,
						   &nitems, &bytes_after, &prop);
    if (rc != Success || prop == NULL)
    	  return FALSE;
    //moko_print_win_list(dpy, prop, nitems);
    temp_list = g_malloc0 (sizeof (Window) * nitems);
    if (temp_list == NULL) 
    {
    	XFree (prop);
    	return FALSE;
    }
    memcpy (temp_list, prop, sizeof (Window) * nitems);
    XFree (prop);
    
    /*need to make clear thar whether the "Client List" is ordered by the 
    atom "_NET_WM_WINDOW_TYPE", if it does, what we need to do will become 
    more simple that only find the boundary of "dock" and "Undock" window in the list.
    */
    for (i=0; i<nitems; i++) 
    {
    	type = moko_get_window_property (dpy, temp_list[i], atoms[_NET_WM_WINDOW_TYPE]);
		if (type == atoms[_NET_WM_WINDOW_TYPE_NORMAL])
		{
	    	temp_list[counter] = temp_list[i];
	    	counter ++;
		}
    }
    *nr = counter;
    *list = g_malloc0 (sizeof (Window) * counter);
    memcpy (*list, temp_list, sizeof (Window) * counter);
    XFree (temp_list);
    return TRUE;
}

gboolean 
moko_iconify_client(Display* dpy, Window* w) 
{
    if (dpy == NULL || w == NULL)
    	return FALSE;
    else
	{
    	GdkScreen * screen = gdk_screen_get_default ();
    	int i ;
    	i = gdk_screen_get_number (screen);
    	gdk_error_trap_push ();
    	g_debug ("test XIconifyWindow");
    	XIconifyWindow (dpy, w, 0);
    	XFlush (dpy);
    	if (gdk_error_trap_pop ())
    		return FALSE;
    	return TRUE;
    }
}

GdkPixbuf *
moko_get_window_icon (Display *dpy, Window w) 
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    int rc;
    GdkPixbuf *pixbuf = NULL;
    
    gdk_error_trap_push ();
    rc = XGetWindowProperty (dpy, w, atoms[_NET_WM_ICON],
    		0, G_MAXLONG, False, XA_CARDINAL, &actual_type, &actual_format,
    		&nitems, &bytes_after, &data);

    if (gdk_error_trap_pop () || rc != Success)
    	  return NULL;
    if (nitems) 
	{
    	  guint *prop = (guint *)data;
    	  guint w = prop[0], h = prop[1];
    	  guint i;
    	  guchar *pixels = g_malloc (w * h * 4);
    	  guchar *p = pixels;
    	  for (i = 0; i < w * h; i++) 
		  {
    		 gulong l = prop[2 + i];
    		 *(p++) = (l & 0x00ff0000) >> 16;
    		 *(p++) = (l & 0x0000ff00) >> 8;
    		 *(p++) = (l & 0x000000ff);
    		 *(p++) = (l & 0xff000000) >> 24;
    	 }
    	
    	 pixbuf = gdk_pixbuf_new_from_data (pixels,
					 GDK_COLORSPACE_RGB,
					 TRUE,
					 8,
					 w, h,
					 w * 4,
					 (GdkPixbufDestroyNotify)g_free,
					 NULL);
    }
    if (data)
    	XFree (data);
    return pixbuf;
}

gchar *
moko_get_window_name (Display *dpy, Window w) 
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    gchar *name = NULL;
    int rc;

    gdk_error_trap_push ();
    rc = XGetWindowProperty (dpy, w, atoms[_NET_WM_NAME],
    	0, G_MAXLONG, False, atoms[UTF8_STRING], &actual_type, &actual_format,
    	&nitems, &bytes_after, &prop);
    if (gdk_error_trap_pop () || rc != Success)
    	return NULL;
    if (nitems)
	{
    	name = g_strdup (prop);
    	XFree (prop);
    }
    else
	{
    	gdk_error_trap_push ();
    	rc = XGetWindowProperty (dpy, w, XA_WM_NAME,
    		0, G_MAXLONG, False, XA_STRING, &actual_type, &actual_format,
    		&nitems, &bytes_after, &prop);
    	if (gdk_error_trap_pop () || rc != Success)
    		return FALSE;
    	if (nitems) 
		{
    		name = g_locale_to_utf8 (prop, -1, NULL, NULL, NULL);
    		XFree (prop);
    	}
    }
    return name;
}


void 
moko_print_win_list (Display* dpy, Window* win_list, guint win_num) 
{
    int i;
    char* winname = NULL;

    if (win_num > 1) 
    	g_debug ("****there are %d windows in total****", win_num);
    
    for (i=0; i<win_num; i++) 
    {
    	winname = moko_get_window_name(dpy, win_list[i]);
    	g_debug ("%d. %s ", i, winname);
    }
}

Atom
moko_get_window_property (Display *dpy, Window w, Atom property) 
{
    Atom result = None;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    int rc;
    
    gdk_error_trap_push ();
    rc = XGetWindowProperty (dpy, w, property,
    			0, 1, False, XA_ATOM, &actual_type, &actual_format,
			  &nitems, &bytes_after, &prop);
    if (gdk_error_trap_pop () || rc != Success)
	{
    	//g_debug ("Have not obtain the property");
    	return None;
    }
    
	if (prop) 
	{
    	memcpy (&result, prop, sizeof (result));
    	XFree (prop);
    }
    return result;
}

guint
moko_tab_event_check (Display *dpy) 
{
    XEvent ev;
    guint done = 0;
    Bool	clicked = FALSE;
    struct timeval then, now;
    Time click_time = 800;
    Time final_time = 2*click_time;
    
    gettimeofday(&then, NULL);

    //check the click type: tap "done = 1 "; tap with hold "done = 2";
    while (!done) 
	{
    	if (XCheckMaskEvent(dpy, ButtonReleaseMask, &ev))
    		if (ev.type == ButtonRelease) 
    			done=1;
    		gettimeofday(&now, NULL);
    	if ((now.tv_usec-then.tv_usec)>(click_time*1000) || now.tv_sec > then.tv_sec)
    		done=2;
    }
    return done;
}


gboolean 
moko_get_current_active_client(Display *dpy, Window *window_return) 
{
    Atom actual_type, type;
    unsigned char* prop = NULL;
    int actual_format;
    unsigned long nitems, bytes_after = 0;

    if (XGetWindowProperty (dpy, DefaultRootWindow (dpy), atoms[_NET_ACTIVE_WINDOW],
    			0, 4, False, XA_WINDOW, &type, &actual_format, &nitems, &bytes_after, (unsigned char **)&prop)
    		== Success)
	{
    	*window_return = g_malloc0(sizeof (Window) * nitems);
    	memcpy (*window_return, prop, sizeof (Window) * nitems);
    	XFree (prop);
    	return TRUE;
    }
    else return FALSE;
}

void
mbcommand(Display *dpy, int cmd_id, Window win, char *data) 
{
    XEvent ev;
    Window root;
    Atom cmd_prop;

    root = DefaultRootWindow(dpy);

	switch (cmd_id)
	{
    /*use to grab desktop later*/
	    case CMD_SHOW_DESKTOP :
	        cmd_prop = atoms[_NET_SHOW_DESKTOP];
			break;
		case CMD_CLOSE_WINDOW :
	        //cmd_prop = atoms[_NET_CLOSE_WINDOW]; FIXME:children windows, (wheel and toolbox) could not be close in this way
			cmd_prop == NULL;
			moko_kill_window (dpy, win);
			break;
		case CMD_ACTIVATE_WINDOW :
		    cmd_prop = atoms[_NET_ACTIVE_WINDOW];
			break;
		default :
			cmd_prop = XInternAtom(dpy, "_MB_COMMAND", False);
	}
	
    memset(&ev, '\0', sizeof(ev));
    ev.xclient.type = ClientMessage;
    if (win == NULL)
    	ev.xclient.window = root; 	/* we send it _from_ root as we have no win  */
    else
    	ev.xclient.window = win;
    ev.xclient.message_type = cmd_prop;
    ev.xclient.format = 8;
    ev.xclient.data.l[0] = cmd_id;
    
    XSendEvent(dpy, root, False, SubstructureRedirectMask|SubstructureNotifyMask, &ev);
    XFlush (dpy);
}

static gboolean
moko_send_delete_message (Display *dpy, Window w) 
{
    XEvent e;
    
    e.type = ClientMessage;
    e.xclient.window = w;
    e.xclient.message_type = atoms[WM_PROTOCOLS];
    e.xclient.format = 32;
    e.xclient.data.l[0] = atoms[WM_DELETE_WINDOW];
    e.xclient.data.l[1] = CurrentTime;

    gdk_error_trap_push ();
    XSendEvent (dpy, w, False, NoEventMask, &e);
    XFlush (dpy);
    if (gdk_error_trap_pop ())
    	return FALSE;
    return TRUE;
}

static gboolean
moko_really_kill_client (Display *dpy, Window w) 
{
    gdk_error_trap_push ();

    XKillClient (dpy, w);
    XFlush (dpy);
    if (gdk_error_trap_pop ())
    	return FALSE;
    return TRUE;
}

gboolean
moko_kill_window (Display *dpy, Window w)
{
    Atom *protocols;
    int count, rc;

    gdk_error_trap_push ();
    rc = XGetWMProtocols (dpy, w, &protocols, &count);
    if (gdk_error_trap_pop ())
    	return FALSE;
    if (rc)
	{
    	int i;
    	gboolean delete_supported = FALSE;
    	for (i = 0; i < count; i++) 
		{
    		if (protocols[i] == WM_DELETE_WINDOW)
    			delete_supported = TRUE;
    	}
    	XFree (protocols);
    	if (delete_supported)
    		return moko_send_delete_message (dpy, w);
   	}
    return moko_really_kill_client (dpy, w);
}

