/*
 * gui_tools.h - GUI, tool bar
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_TOOLS_H
#define	GUI_TOOLS_H

#include <gtk/gtk.h>

#include "inst.h"


void tool_dehover(struct draw_ctx *ctx);
void tool_hover(struct draw_ctx *ctx, struct coord pos);
int tool_consider_drag(struct draw_ctx *ctx, struct coord pos);
void tool_drag(struct draw_ctx *ctx, struct coord to);
void tool_cancel_drag(struct draw_ctx *ctx);
int tool_end_drag(struct draw_ctx *ctx, struct coord to);

void tool_reset(void);

GtkWidget *gui_setup_tools(GdkDrawable *drawable);

#endif /* !GUI_TOOLS_H */
