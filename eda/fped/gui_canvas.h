/*
 * gui_canvas.h - GUI, canvas
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
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

extern void (*highlight)(void);


void refresh_pos(void);

void redraw(void);

void zoom_in_center(void);
void zoom_out_center(void);
void zoom_to_frame(void);
void zoom_to_extents(void);

void canvas_frame_begin(struct frame *frame);
int canvas_frame_motion(struct frame *frame, int x, int y);
void canvas_frame_end(void);
int canvas_frame_drop(struct frame *frame, int x, int y);

GtkWidget *make_canvas(void);
void init_canvas(void);

#endif /* !GUI_CANVAS_H */
