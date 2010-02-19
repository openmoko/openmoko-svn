/*
 * gui_frame.h - GUI, frame window
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_FRAME_H
#define GUI_FRAME_H

#include <gtk/gtk.h>


extern int show_vars;


void make_popups(void);

void select_frame(struct frame *frame);

void gui_frame_select_inst(struct inst *inst);
void gui_frame_deselect_inst(struct inst *inst);

void build_frames(GtkWidget *vbox, int warp_width);

#endif /* !GUI_FRAME_H */
