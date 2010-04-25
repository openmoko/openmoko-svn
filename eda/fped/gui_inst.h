/*
 * gui_inst.h - GUI, instance functions
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_INST_H
#define GUI_INST_H

#include <gtk/gtk.h>

#include "coord.h"
#include "inst.h"
#include "gui_status.h"


struct coord translate(struct coord pos);
struct coord canvas_to_coord(int x, int y);

unit_type gui_dist_vec(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_vec_fallback(struct inst *self, struct coord pos,
    unit_type scale);
unit_type gui_dist_line(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_rect(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_pad(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_hole(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_arc(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_meas(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_frame(struct inst *self, struct coord pos, unit_type scale);
unit_type gui_dist_frame_eye(struct inst *self, struct coord pos,
    unit_type scale);

void project_meas(const struct inst *inst, struct coord *a1, struct coord *b1);
char *format_len(const char *label, unit_type len, enum curr_unit unit);

void gui_draw_vec(struct inst *self);
void gui_draw_line(struct inst *self);
void gui_draw_rect(struct inst *self);
void gui_draw_pad(struct inst *self);
void gui_draw_rpad(struct inst *self);
void gui_draw_hole(struct inst *self);
void gui_draw_arc(struct inst *self);
void gui_draw_meas(struct inst *self);
void gui_draw_frame(struct inst *self);

void gui_highlight_vec(struct inst *self);

#endif /* !GUI_INST_H */
