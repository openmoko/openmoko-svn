/*
 * gui_frame_drag.h - GUI, dragging of frame items
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_FRAME_DRAG_H
#define GUI_FRAME_DRAG_H

#include <gtk/gtk.h>

#include "obj.h"


int is_dragging(void *this);

void setup_var_drag(struct var *var);
void setup_value_drag(struct value *value);
void setup_frame_drag(struct frame *frame);
void setup_canvas_drag(GtkWidget *canvas);

#endif /* !GUI_FRAME_DRAG_H */
