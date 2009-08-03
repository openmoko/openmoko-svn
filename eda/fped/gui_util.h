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


GdkColor get_color(const char *spec);

void set_width(GdkGC *gc, int width);

GtkWidget *label_in_box_new(const char *s);
GtkWidget *box_of_label(GtkWidget *label);
void label_in_box_bg(GtkWidget *box, const char *color);

void render_text(GdkDrawable *da, GdkGC *gc, int x, int y, double angle,
    const char *s, const char *font, double xalign, double yalign,
    int xmax, int ymax);

void destroy_all_children(GtkContainer *container);

#endif /* !GUI_UTIL_H */
