/*
 * gui_style.h - GUI, style definitions
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
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

#define	SELECT_R		6	/* pixels within which we select */

#define	MIN_FONT_SCALE		0.20	/* don't scale fonts below this */


/* ----- assorted colors --------------------------------------------------- */


#define COLOR_EDIT_ASIS	"#ffffff"
#define COLOR_EDIT_GOOD	"#a0ffa0"
#define COLOR_EDIT_BAD	"#ffa0a0"

#define	COLOR_EDITING	"#ff00ff"

#define	COLOR_FRAME_UNSELECTED	"#c0c0c0"
#define COLOR_FRAME_SELECTED	"#fff0a0"
#define COLOR_FRAME_EDITING	COLOR_EDITING

#define	COLOR_VAR_PASSIVE	COLOR_FRAME_UNSELECTED
#define	COLOR_VAR_EDITING	COLOR_EDITING
#define	COLOR_EXPR_PASSIVE	"#f0f0ff"
#define	COLOR_EXPR_EDITING	COLOR_EDITING
#define	COLOR_CHOICE_UNSELECTED	COLOR_EXPR_PASSIVE
#define	COLOR_CHOICE_SELECTED	"#9090ff"
#define	COLOR_ROW_UNSELECTED	COLOR_CHOICE_UNSELECTED
#define	COLOR_ROW_SELECTED	COLOR_CHOICE_SELECTED

#define	COLOR_VAR_TABLE_SEP	"black"


/* ----- canvas drawing styles --------------------------------------------- */


extern GdkGC *gc_bg;
extern GdkGC *gc_vec[mode_n];
extern GdkGC *gc_obj[mode_n];
extern GdkGC *gc_pad[mode_n];
extern GdkGC *gc_ptext[mode_n];
extern GdkGC *gc_meas[mode_n];
extern GdkGC *gc_frame[mode_n];


void gui_setup_style(GdkDrawable *drawable);

#endif /* !GUI_STYLE_H */