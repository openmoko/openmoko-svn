/*
 * gui_meas.c - GUI, measurements
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_MEAS_H
#define GUI_MEAS_H

#include "gui_tool.h"


extern struct tool_ops tool_meas_ops;
extern struct tool_ops tool_meas_ops_x;
extern struct tool_ops tool_meas_ops_y;


void begin_drag_move_meas(struct inst *inst, int i);
struct inst *find_point_meas_move(struct inst *inst, struct coord pos);
void end_drag_move_meas(void);
void do_move_to_meas(struct inst *inst, struct inst *to, int i);

#endif /* !GUI_MEAS_H */
