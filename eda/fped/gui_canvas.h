/*
 * gui_canvas.h - GUI, canvas
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <gtk/gtk.h>


/*
 * "highlight" is invoked at the end of each redraw, for optional highlighting
 * of objects.
 */

extern void (*highlight)(struct draw_ctx *ctx);


void redraw(void);

GtkWidget *make_canvas(void);
void init_canvas(void);

#endif /* !GUI_CANVAS_H */
