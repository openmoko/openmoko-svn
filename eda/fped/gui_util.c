/*
 * gui_util.c - GUI helper functions
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#include "gui_style.h"
#include "gui.h"
#include "gui_util.h"


/* ----- look up a color --------------------------------------------------- */


GdkColor get_color(const char *spec)
{
	GdkColormap *cmap;
	GdkColor color;

	cmap = gdk_drawable_get_colormap(root->window);
	if (!gdk_color_parse(spec, &color))
		abort();
	if (!gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))
		abort();
	return color;
}


/* ----- lines with a width ------------------------------------------------ */


void set_width(GdkGC *gc, int width)
{
	gdk_gc_set_line_attributes(gc, width < 1 ? 1 : width,
	    GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}


/* ----- labels in a box --------------------------------------------------- */


GtkWidget *label_in_box_new(const char *s)
{
	GtkWidget *evbox, *label;

	evbox = gtk_event_box_new();
	label = gtk_label_new(s);
	gtk_misc_set_padding(GTK_MISC(label), 1, 1);
	gtk_container_add(GTK_CONTAINER(evbox), label);
	return label;
}


GtkWidget *box_of_label(GtkWidget *label)
{
	return gtk_widget_get_ancestor(label, GTK_TYPE_EVENT_BOX);
}


void label_in_box_bg(GtkWidget *label, const char *color)
{
	GtkWidget *box;
	GdkColor col = get_color(color);

	box = box_of_label(label);
	gtk_widget_modify_bg(box, GTK_STATE_NORMAL, &col);
}


/* ----- render a text string ---------------------------------------------- */


void render_text(GdkDrawable *da, GdkGC *gc, int x, int y, double angle,
    const char *s, const char *font, double xalign, double yalign,
    int xmax, int ymax)
{
	GdkScreen *screen;
	PangoRenderer *renderer;
	PangoContext *context;
	PangoLayout *layout;
	PangoFontDescription *desc;
	int width, height;
	PangoMatrix m = PANGO_MATRIX_INIT;
	double f_min, f;

	/* set up the renderer */

	screen = gdk_drawable_get_screen(da);
	renderer = gdk_pango_renderer_get_default(screen);
	gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(renderer), da);
	gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(renderer), gc);

	/* start preparing the layout */

	context = gdk_pango_context_get_for_screen(screen);
	layout = pango_layout_new(context);
	pango_layout_set_text(layout, s, -1);

	/* apply the font */

	desc = pango_font_description_from_string(font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	/* align and position the text */

	pango_layout_get_size(layout, &width, &height);
	f_min = 1.0;
	if (xmax) {
		f = xmax/((double) width/PANGO_SCALE);
		if (f < f_min)
			f_min = f;
	}
	if (ymax) {
		f = ymax/((double) height/PANGO_SCALE);
		if (f < f_min)
			f_min = f;
	}
	if (f_min < MIN_FONT_SCALE)
		f_min = MIN_FONT_SCALE;
	pango_matrix_translate(&m, x, y);
	pango_matrix_rotate(&m, angle);
	pango_matrix_translate(&m,
	    -xalign*f_min*width/PANGO_SCALE,
	    (yalign-1)*f_min*height/PANGO_SCALE);
	pango_matrix_scale(&m, f_min, f_min);

	pango_context_set_matrix(context, &m);
	pango_layout_context_changed(layout);
	pango_renderer_draw_layout(renderer, layout, 0, 0);

	/* clean up renderer */

	gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(renderer), NULL);
	gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(renderer), NULL);

	/* free objects */

	g_object_unref(layout);
	g_object_unref(context);
}


/* ----- kill the content of a container ----------------------------------- */


static void destroy_callback(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
}


void destroy_all_children(GtkContainer *container)
{
	gtk_container_foreach(container, destroy_callback, NULL);
}
