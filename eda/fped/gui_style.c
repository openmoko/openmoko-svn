/*
 * gui_style.c - GUI, style definitions
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <gtk/gtk.h>

#include "inst.h"
#include "gui_util.h"
#include "gui.h"
#include "gui_style.h"


#define	INVALID	"#00ffff"


GdkGC *gc_bg;
GdkGC *gc_drag;
GdkGC *gc_active_frame;
GdkGC *gc_vec[mode_n];
GdkGC *gc_obj[mode_n];
GdkGC *gc_pad[mode_n];
GdkGC *gc_ptext[mode_n];
GdkGC *gc_meas[mode_n];
GdkGC *gc_frame[mode_n];


static GdkGC *gc(const char *spec, int width)
{
	GdkGCValues gc_values = {
		.background = get_color("black"),
		.foreground = get_color(spec),
		.line_width = width,
	};

	return gdk_gc_new_with_values(root->window, &gc_values,
	     GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH);
}


static void style(GdkGC *gcs[mode_n],
    const char *in, const char *in_path, const char *act, const char *act_path,
    const char *sel)
{
	gcs[mode_inactive]		= gc(in, 1);
	gcs[mode_inactive_in_path]	= gc(in_path, 1);
	gcs[mode_active]		= gc(act, 1);
	gcs[mode_active_in_path]	= gc(act_path, 1);
	gcs[mode_selected]		= gc(sel, 2);
}


void gui_setup_style(GdkDrawable *drawable)
{
	gc_bg = gc("#000000", 0);
	gc_drag = gc("#ffffff", 2);
	/*		inactive   in+path    active     act+path   selected */
	style(gc_vec,	"#202000", "#404020", "#909040", "#c0c080", "#ffff80");
	style(gc_obj,	"#006060", INVALID,   "#00ffff", INVALID,   "#ffff80");
	style(gc_pad,	"#400000", INVALID,   "#ff0000", INVALID,   "#ffff80");
	style(gc_ptext,	"#404040", INVALID,   "#ffffff", INVALID,   "#ffffff");
	style(gc_meas,	"#280040", INVALID,   "#ff00ff", INVALID,   "#ffff80");
	style(gc_frame,	"#004000", "#205020", "#009000", INVALID,   "#ffff80");
	gc_active_frame = gc("#00ff00", 2);

	gc_frame[mode_hover] = gc_vec[mode_hover] = gc("#c00000", 1);
}
