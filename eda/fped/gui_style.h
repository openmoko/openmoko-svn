/*
 * gui_style.h - GUI, style definitions
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_STYLE_H
#define	GUI_STYLE_H

#include <gtk/gtk.h>

#include "inst.h"


/* ----- screen distances, etc. -------------------------------------------- */


#define	CANVAS_CLEARANCE	10

#define	ZOOM_STOP_BORDER	50	/* stop zoom if we have at least a 50
					   pixel border */

#define	VEC_ARROW_LEN		10
#define	VEC_ARROW_ANGLE		20
#define	VEC_EYE_R		5

#define	PAD_FONT		"Sans Bold 24"
#define	PAD_BORDER		2

#define	MEAS_FONT		"Sans 8"
#define	MEAS_BASELINE_OFFSET	0.1
#define	MEAS_ARROW_LEN		9
#define MEAS_ARROW_ANGLE	30

#define	FRAME_FONT		"Sans 8"
#define	FRAME_BASELINE_OFFSET	0.1
#define	FRAME_SHORT_X		100
#define	FRAME_SHORT_Y		20
#define	FRAME_CLEARANCE		5
#define	FRAME_EYE_R1		3
#define	FRAME_EYE_R2		5

#define	ITEM_LIST_FONT		"Liberation Mono 8"
//#define	ITEM_LIST_FONT		"Courier Bold 8"

#define	SELECT_R		6	/* pixels within which we select */

#define	DRAG_MIN_R		5

#define	MIN_FONT_SCALE		0.20	/* don't scale fonts below this */

#define	MM_FORMAT_FIXED		"%8.3f"	/* -NNN.NNN */
#define	MIL_FORMAT_FIXED	"%7.1f"	/* -NNNN.N */
#define	MM_FORMAT_SHORT		"%.4g"
#define	MIL_FORMAT_SHORT	"%.4g"

#define	DEFAULT_FRAME_AREA_WIDTH 250
#define	DEFAULT_FRAME_AREA_HEIGHT 100
#define	FRAME_AREA_MISC_WIDTH	26	/* pane, scroll bar, slack */


/* ----- assorted colors --------------------------------------------------- */


#define COLOR_EDIT_ASIS	"#ffffff"
#define COLOR_EDIT_GOOD	"#a0ffa0"
#define COLOR_EDIT_BAD	"#ffa0a0"

#define	COLOR_EDITING	"#ff00ff"

#define	COLOR_PART_NAME		"#ffa050"
#define	COLOR_PART_NAME_EDITING	COLOR_EDITING

#define	COLOR_FRAME_UNSELECTED	"#c0c0c0"
#define COLOR_FRAME_SELECTED	"#fff0a0"
#define COLOR_FRAME_EDITING	COLOR_EDITING

#define	COLOR_VAR_PASSIVE	COLOR_FRAME_UNSELECTED
#define	COLOR_VAR_EDITING	COLOR_EDITING
#define	COLOR_EXPR_PASSIVE	"#f0f0ff"
#define	COLOR_EXPR_EDITING	COLOR_EDITING
#define	COLOR_CHOICE_UNSELECTED	COLOR_EXPR_PASSIVE
#define	COLOR_CHOICE_SELECTED	"#a0a0ff"
#define	COLOR_ROW_UNSELECTED	COLOR_CHOICE_UNSELECTED
#define	COLOR_ROW_SELECTED	COLOR_CHOICE_SELECTED

#define	COLOR_VAR_TABLE_SEP	"black"

#define	COLOR_TOOL_UNSELECTED	"#dcdad5"
#define	COLOR_TOOL_SELECTED	"red"

#define	COLOR_ITEM_NORMAL	"#dcdad5"
#define	COLOR_ITEM_SELECTED	COLOR_FRAME_SELECTED
#define	COLOR_ITEM_ERROR	"red"

#define	COLOR_SELECTOR		"white"


/* ----- canvas drawing styles --------------------------------------------- */


extern GdkGC *gc_bg, *gc_bg_error;
extern GdkGC *gc_drag;
extern GdkGC *gc_highlight;
extern GdkGC *gc_active_frame;
extern GdkGC *gc_vec[mode_n];
extern GdkGC *gc_obj[mode_n];
extern GdkGC *gc_pad[mode_n];
extern GdkGC *gc_pad_bare[mode_n];
extern GdkGC *gc_pad_mask[mode_n];
extern GdkGC *gc_ptext[mode_n];
extern GdkGC *gc_meas[mode_n];
extern GdkGC *gc_frame[mode_n];

extern PangoFontDescription *item_list_font;

void gui_setup_style(GdkDrawable *drawable);
void gui_cleanup_style(void);

#endif /* !GUI_STYLE_H */
