/*
 * gui_util.c - GUI helper functions
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#include "util.h"
#include "gui_style.h"
#include "gui.h"
#include "gui_util.h"


struct draw_ctx draw_ctx;


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


/* ----- backing store ----------------------------------------------------- */


void free_pix_buf(struct pix_buf *buf)
{
	g_object_unref(G_OBJECT(buf->buf));
	free(buf);
}


struct pix_buf *save_pix_buf(GdkDrawable *da, int xa, int ya, int xb, int yb,
    int border)
{
	struct pix_buf *buf;
	int tmp;
	int w, h;

	if (xa > xb) {
		tmp = xa;
		xa = xb;
		xb = tmp;
	}
	if (ya > yb) {
		tmp = ya;
		ya = yb;
		yb = tmp;
	}
	buf = alloc_type(struct pix_buf);
	buf->da = da;
	buf->x = xa-border;
	buf->y = ya-border;
	w = xb-xa+1+2*border;
	h = yb-ya+1+2*border;
	if (buf->x < 0) {
		w += buf->x;
		buf->x = 0;
	}
	if (buf->y < 0) {
		h += buf->y;
		buf->y = 0;
	}
	buf->buf = gdk_pixbuf_get_from_drawable(NULL, da, NULL,
	    buf->x, buf->y, 0, 0, w, h);
	return buf;
}


void restore_pix_buf(struct pix_buf *buf)
{
	gdk_draw_pixbuf(buf->da, NULL, buf->buf, 0, 0, buf->x, buf->y, -1, -1,
	    GDK_RGB_DITHER_NORMAL, 0, 0);
	free_pix_buf(buf);
}


/* ----- arcs and circles -------------------------------------------------- */


void draw_arc(GdkDrawable *da, GdkGC *gc, int fill,
    int x, int y, int r, double a1, double a2)
{
	/*
	 * This adjustment handles two distinct cases:
	 * - if a1 == a2, we make sure we draw a full circle
	 * - the end angle a2 must always be greater than the start angle a1
	 */
	if (a2 <= a1)
		a2 += 360;
        gdk_draw_arc(da, gc, fill, x-r, y-r, 2*r, 2*r, a1*64, (a2-a1)*64);
}


void draw_circle(GdkDrawable *da, GdkGC *gc, int fill,
    int x, int y, int r)
{
        draw_arc(da, gc, fill, x, y, r, 0, 360);
}


/* ----- labels in a box --------------------------------------------------- */


GtkWidget *label_in_box_new(const char *s, const char *tooltip)
{
	GtkWidget *evbox, *label;

	evbox = gtk_event_box_new();
	label = gtk_label_new(s);
	gtk_misc_set_padding(GTK_MISC(label), 1, 1);
	gtk_container_add(GTK_CONTAINER(evbox), label);
	if (tooltip)
		gtk_widget_set_tooltip_markup(evbox, tooltip);
	return label;
}


GtkWidget *box_of_label(GtkWidget *label)
{
	return gtk_widget_get_ancestor(label, GTK_TYPE_EVENT_BOX);
}


void label_in_box_fg(GtkWidget *label, const char *color)
{
	GdkColor col = get_color(color);

	gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &col);
}


void label_in_box_bg(GtkWidget *label, const char *color)
{
	GtkWidget *box;
	GdkColor col = get_color(color);

	box = box_of_label(label);
	gtk_widget_modify_bg(box, GTK_STATE_NORMAL, &col);
}


/* ----- generate a tool button with an XPM image -------------------------- */


GtkWidget *make_image(GdkDrawable *drawable, char **xpm, const char *tooltip)
{
	GdkPixmap *pixmap;
	GtkWidget *image;
	GdkColor white = get_color("white");

	pixmap = gdk_pixmap_create_from_xpm_d(drawable, NULL, &white, xpm);
	image = gtk_image_new_from_pixmap(pixmap, NULL);
	gtk_misc_set_padding(GTK_MISC(image), 1, 1);
	if (tooltip)
		gtk_widget_set_tooltip_markup(image, tooltip);
	return image;
}


GtkWidget *make_transparent_image(GdkDrawable *drawable, char **xpm,
    const char *tooltip)
{
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	GtkWidget *image;

	pixmap = gdk_pixmap_create_from_xpm_d(drawable, &mask, NULL, xpm);
	image = gtk_image_new_from_pixmap(pixmap, mask);
	gtk_misc_set_padding(GTK_MISC(image), 1, 1);
	if (tooltip)
		gtk_widget_set_tooltip_markup(image, tooltip);
	return image;
}


static void remove_child(GtkWidget *widget, gpointer data)
{
	gtk_container_remove(GTK_CONTAINER(data), widget);
}


void vacate_widget(GtkWidget *widget)
{
	gtk_container_foreach(GTK_CONTAINER(widget), remove_child, widget);
}


void set_image(GtkWidget *widget, GtkWidget *image)
{
	vacate_widget(widget);
	gtk_container_add(GTK_CONTAINER(widget), image);
	gtk_widget_show_all(widget);
}


GtkWidget *tool_button(GtkWidget *bar, GdkDrawable *drawable,
    char **xpm, const char *tooltip,
    gboolean (*cb)(GtkWidget *widget, GdkEventButton *event, gpointer data),
    gpointer data)
{
	GtkWidget *image, *evbox;	
	GtkToolItem *item;

	/*
	 * gtk_radio_tool_button_new_from_widget is *huge*. We try to do things
	 * in a
	 * more compact way.
	 */

	evbox = gtk_event_box_new();
	if (xpm) {
		image = make_image(drawable, xpm, tooltip);
		gtk_container_add(GTK_CONTAINER(evbox), image);
	}
	g_signal_connect(G_OBJECT(evbox), "button_press_event",
            G_CALLBACK(cb), data);

	item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(item), evbox);

	gtk_container_set_border_width(GTK_CONTAINER(item), 0);

	gtk_toolbar_insert(GTK_TOOLBAR(bar), item, -1);

	return evbox;
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


/* ----- Debugging support ------------------------------------------------- */


/*
 * View with  make montage  or something like
 *
 * montage -label %f -frame 3 __dbg????.png png:- | display -
 */

void debug_save_pixbuf(GdkPixbuf *buf)
{
	static int buf_num = 0;
	char name[20]; /* plenty */

	sprintf(name, "__dbg%04d.png", buf_num++);
	gdk_pixbuf_save(buf, name, "png", NULL, NULL);
	fprintf(stderr, "saved to %s\n", name);
}


/*
 * gtk_widget_get_snapshot seems to use an expose event to do the drawing. This
 * means that we can't call debug_save_widget from the expose event handler of
 * the widget being dumped.
 */

void debug_save_widget(GtkWidget *widget)
{
	GdkPixmap *pixmap;
	GdkPixbuf *pixbuf;
	gint w, h;

	pixmap = gtk_widget_get_snapshot(widget, NULL);
	gdk_drawable_get_size(GDK_DRAWABLE(pixmap), &w, &h);
	pixbuf = gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(pixmap),
	    NULL, 0, 0, 0, 0, w, h);
	debug_save_pixbuf(pixbuf);
	gdk_pixmap_unref(pixmap);
	g_object_unref(pixbuf);
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


/* ----- get a widget's desired width -------------------------------------- */


int get_widget_width(GtkWidget *widget)
{
	GtkRequisition req;

	gtk_widget_show_all(widget);
	gtk_widget_size_request(widget, &req);
	return req.width;
}
