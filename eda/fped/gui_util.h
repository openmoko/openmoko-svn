/*
 * gui_util.h - GUI helper functions
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_UTIL_H
#define	GUI_UTIL_H

#include <gtk/gtk.h>

#include "coord.h"


struct draw_ctx {
	GtkWidget *widget;
	int scale;
	struct coord center;
};

struct pix_buf {
	GdkDrawable *da;
	int x, y;
	GdkPixbuf *buf;
};


extern struct draw_ctx draw_ctx;


#define DA      GDK_DRAWABLE(draw_ctx.widget->window)


GdkColor get_color(const char *spec);

void set_width(GdkGC *gc, int width);

void free_pix_buf(struct pix_buf *buf);
struct pix_buf *save_pix_buf(GdkDrawable *da, int xa, int ya, int xb, int yb,
    int border);
void restore_pix_buf(struct pix_buf *buf);

void draw_arc(GdkDrawable *da, GdkGC *gc, int fill,
    int x, int y, int r, double a1, double a2);
void draw_circle(GdkDrawable *da, GdkGC *gc, int fill,
    int x, int y, int r);

GtkWidget *label_in_box_new(const char *s);
GtkWidget *box_of_label(GtkWidget *label);
void label_in_box_fg(GtkWidget *box, const char *color);
void label_in_box_bg(GtkWidget *box, const char *color);

void vacate_widget(GtkWidget *widget);

GtkWidget *make_image(GdkDrawable *drawable, char **xpm);
GtkWidget *make_transparent_image(GdkDrawable *drawable, char **xpm);
void set_image(GtkWidget *widget, GtkWidget *image);
GtkWidget *tool_button(GtkWidget *bar, GdkDrawable *drawable, char **xpm,
    gboolean (*cb)(GtkWidget *widget, GdkEventButton *event, gpointer data),
    gpointer data);

void render_text(GdkDrawable *da, GdkGC *gc, int x, int y, double angle,
    const char *s, const char *font, double xalign, double yalign,
    int xmax, int ymax);

void destroy_all_children(GtkContainer *container);

#endif /* !GUI_UTIL_H */
