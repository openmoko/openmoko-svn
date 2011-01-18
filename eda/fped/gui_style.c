/*
 * gui_style.c - GUI, style definitions
 *
 * Written 2009-2011 by Werner Almesberger
 * Copyright 2009-2011 by Werner Almesberger
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


GdkGC *gc_bg, *gc_bg_error;
GdkGC *gc_drag;
GdkGC *gc_highlight;
GdkGC *gc_active_frame;
GdkGC *gc_vec[mode_n];
GdkGC *gc_obj[mode_n];
GdkGC *gc_pad[mode_n];
GdkGC *gc_pad_bare[mode_n];
GdkGC *gc_pad_trace[mode_n];
GdkGC *gc_pad_mask[mode_n];
GdkGC *gc_ptext[mode_n];
GdkGC *gc_rim[mode_n];
GdkGC *gc_hole[mode_n];
GdkGC *gc_meas[mode_n];
GdkGC *gc_frame[mode_n];

PangoFontDescription *item_list_font;


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
    const char *in, const char *act, const char *sel, int width)
{
	gcs[mode_inactive]		= gc(in, width);
	gcs[mode_active]		= gc(act, width);
	gcs[mode_selected]		= gc(sel, 2*width);
}


void gui_setup_style(GdkDrawable *drawable)
{
	gc_bg = gc("#000000", 0);
	gc_bg_error = gc("#000040", 0);
	gc_drag = gc("#ffffff", 2);
	/*			inactive   active     selected	width */
	style(gc_vec,		"#202000", "#b0b050", "#ffff80", 1);
	style(gc_obj,		"#006060", "#00ffff", "#ffff80", 1);
	style(gc_pad,		"#400000", "#ff0000", "#ffff80", 1);
	style(gc_pad_bare,	"#402000", "#ff6000", "#ffff80", 1);
	style(gc_pad_trace,	"#304000", "#80c000", "#ffff80", 1);
	style(gc_pad_mask,	"#000040", "#0000ff", "#ffff80", 2);
	style(gc_ptext,		"#404040", "#ffffff", "#ffffff", 1);
	style(gc_hole,		"#000000", "#000000", "#000000", 0);
	style(gc_rim,		"#303030", "#606060", "#ffff80", 3);
	style(gc_meas,		"#280040", "#ff00ff", "#ffff80", 1);
	style(gc_frame,		"#005000", "#009000", "#ffff80", 1);

	gc_active_frame = gc("#00ff00", 2);
//	gc_highlight = gc("#ff8020", 2);
	gc_highlight = gc("#ff90d0", 2);
	gc_frame[mode_hover] = gc_vec[mode_hover] = gc("#c00000", 2);

	item_list_font = pango_font_description_from_string(ITEM_LIST_FONT);
}


void gui_cleanup_style(void)
{
	pango_font_description_free(item_list_font);
}
