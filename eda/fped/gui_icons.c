/*
 * gui_icons.c - GUI, icon bar
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <gtk/gtk.h>

#include "gui_util.h"
#include "gui_icons.h"


#include "icons/arc.xpm"
#include "icons/circ.xpm"
#include "icons/frame.xpm"
#include "icons/line.xpm"
#include "icons/meas.xpm"
#include "icons/pad.xpm"
#include "icons/point.xpm"
#include "icons/rect.xpm"
#include "icons/vec.xpm"


static GtkToolItem *icon_button(GtkWidget *bar, GdkDrawable *drawable,
     char **xpm, GtkToolItem *last)
{
	GdkPixmap *pixmap;
	GtkWidget *image;	
	GtkToolItem *item;

	pixmap = gdk_pixmap_create_from_xpm_d(drawable, NULL, NULL, xpm);
	image = gtk_image_new_from_pixmap(pixmap, NULL);

/*
 * gtk_radio_tool_button_new_from_widget is *huge*. we try to do things in a
 * more compact way.
 */
#if 0
	if (last)
		item = gtk_radio_tool_button_new_from_widget(
		    GTK_RADIO_TOOL_BUTTON(last));
	else
		item = gtk_radio_tool_button_new(NULL);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item), image);
#else
	item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(item), image);

	gtk_container_set_border_width(GTK_CONTAINER(item), 1);
#endif

	gtk_toolbar_insert(GTK_TOOLBAR(bar), item, -1);

	return item;
}


GtkWidget *gui_setup_icons(GdkDrawable *drawable)
{
	GtkWidget *bar;
	GtkToolItem *last;

	bar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(bar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(bar),
	    GTK_ORIENTATION_VERTICAL);
//gtk_container_set_border_width(GTK_CONTAINER(bar), 5);

	last = icon_button(bar, drawable, xpm_point, NULL);
	last = icon_button(bar, drawable, xpm_vec, last);
	last = icon_button(bar, drawable, xpm_frame, last);
	last = icon_button(bar, drawable, xpm_pad, last);
	last = icon_button(bar, drawable, xpm_line, last);
	last = icon_button(bar, drawable, xpm_rect, last);
	last = icon_button(bar, drawable, xpm_circ, last);
	last = icon_button(bar, drawable, xpm_arc, last);
	last = icon_button(bar, drawable, xpm_meas, last);

	return bar;
}
