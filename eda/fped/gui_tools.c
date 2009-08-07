/*
 * gui_tools.c - GUI, tool bar
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <gtk/gtk.h>

#include "util.h"
#include "inst.h"
#include "obj.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_inst.h"
#include "gui_status.h"
#include "gui.h"
#include "gui_tools.h"


#include "icons/circ.xpm"
#include "icons/frame.xpm"
#include "icons/frame_locked.xpm"
#include "icons/frame_ready.xpm"
#include "icons/line.xpm"
#include "icons/meas.xpm"
#include "icons/pad.xpm"
#include "icons/point.xpm"
#include "icons/rect.xpm"
#include "icons/vec.xpm"


#define DA	GDK_DRAWABLE(ctx->widget->window)


struct tool_ops {
	void (*tool_selected)(void);
	struct pix_buf *(*drag_new)(struct draw_ctx *ctx, struct inst *from,
	     struct coord to);
	int (*end_new_raw)(struct draw_ctx *ctx, struct inst *from,
	    struct coord to);
	int (*end_new)(struct draw_ctx *ctx, struct inst *from,
	    struct inst *to);
};


static GtkWidget *ev_point, *ev_frame;
static GtkWidget *active_tool;
static struct tool_ops *active_ops = NULL;
static struct inst *hover_inst = NULL;
static GtkWidget *frame_image, *frame_image_locked, *frame_image_ready;

static struct drag_state {
	struct inst *inst; /* non-NULL if dragging an existing object */
	struct inst *new; /* non-NULL if dragging a new object */
	int anchors_n; /* number of anchors, 0 if no moving */
	int anchor_i; /* current anchor */
	struct vec **anchors[3];
} drag = {
	.new = NULL,
	.anchors_n = 0,
};

static struct pix_buf *pix_buf;
static struct coord last_canvas_pos;


static struct vec *new_vec(struct inst *base)
{
	struct vec *vec, **walk;

	vec = alloc_type(struct vec);
	vec->name = NULL;
	vec->base = inst_get_vec(base);
	vec->next = NULL;
	vec->frame = active_frame;
	for (walk = &active_frame->vecs; *walk; walk = &(*walk)->next);
	*walk = vec;
	return vec;
}


static struct obj *new_obj(enum obj_type type, struct inst *base)
{
	struct obj *obj, **walk;

	obj = alloc_type(struct obj);
	obj->type = type;
	obj->frame = active_frame;
	obj->base = inst_get_vec(base);
	obj->next = NULL;
	obj->lineno = 0;
	for (walk = &active_frame->objs; *walk; walk = &(*walk)->next);
	*walk = obj;
	return obj;
}


/* ----- shared functions -------------------------------------------------- */


static struct pix_buf *draw_move_line_common(struct inst *inst,
    struct coord end, struct draw_ctx *ctx, struct coord pos, int i)
{
	struct coord from, to;
	struct pix_buf *buf;

	from = translate(ctx, inst->base);
	to = translate(ctx, end);
	pos = translate(ctx, pos);
	switch (i) {
	case 0:
		from = pos;
		break;
	case 1:
		to = pos;
		break;
	default:
		abort();
	}
	buf = save_pix_buf(DA, from.x, from.y, to.x, to.y, 1);
	gdk_draw_line(DA, gc_drag, from.x, from.y, to.x, to.y);
	return buf;
}


static struct pix_buf *draw_move_rect_common(struct inst *inst,
    struct coord other, struct draw_ctx *ctx, struct coord pos, int i)
{
	struct coord min, max;
	struct pix_buf *buf;

	min = translate(ctx, inst->base);
	max = translate(ctx, other);
	pos = translate(ctx, pos);
	switch (i) {
	case 0:
		min = pos;
		break;
	case 1:
		max = pos;
		break;
	default:
		abort();
	}
	sort_coord(&min, &max);
	buf = save_pix_buf(DA, min.x, min.y, max.x, max.y, 1);
	gdk_draw_rectangle(DA, gc_drag, FALSE,
	    min.x, min.y, max.x-min.x, max.y-min.y);
	return buf;
}


/* ----- vec --------------------------------------------------------------- */


static struct coord gridify(struct coord base, struct coord pos)
{
	struct coord new;
	unit_type unit = mm_to_units(0.1);

	new.x = pos.x-((pos.x-base.x) % unit);
	new.y = pos.y-((pos.y-base.y) % unit);
	if (new.x != base.x || new.y != base.y)
		return new;
	if (fabs(pos.x-base.x) > fabs(pos.y-base.y))
		new.x += pos.x > base.x ? unit : -unit;
	else
		new.y += pos.y > base.y ? unit : -unit;
	return new;
}


static struct pix_buf *drag_new_vec(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;

	pos = inst_get_point(from);
	to = gridify(pos, to);
	status_set_type_x("dX =");
	status_set_type_y("dX =");
	status_set_x("%lg mm", units_to_mm(to.x-pos.x));
	status_set_y("%lg mm", units_to_mm(to.y-pos.y));
	pos = translate(ctx, pos);
	to = translate(ctx, to);
	buf = save_pix_buf(DA, pos.x, pos.y, to.x, to.y, 1);
	gdk_draw_line(DA, gc_drag, pos.x, pos.y, to.x, to.y);
	return buf;
}


struct pix_buf *draw_move_vec(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	return draw_move_line_common(inst,
	    add_vec(sub_vec(inst->u.rect.end, inst->base), pos), ctx, pos, i);
}


static int end_new_raw_vec(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct vec *vec;
	struct coord pos;

	vec = new_vec(from);
	pos = inst_get_point(from);
	to = gridify(pos, to);
	vec->x = new_num(make_mm(units_to_mm(to.x-pos.x)));
	vec->y = new_num(make_mm(units_to_mm(to.y-pos.y)));
	return 1;
}


static struct tool_ops vec_ops = {
	.drag_new	= drag_new_vec,
	.end_new_raw	= end_new_raw_vec,
};


/* ----- line -------------------------------------------------------------- */


static struct pix_buf *drag_new_line(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	buf = save_pix_buf(DA, pos.x, pos.y, to.x, to.y, 1);
	gdk_draw_line(DA, gc_drag, pos.x, pos.y, to.x, to.y);
	return buf;
}


struct pix_buf *draw_move_line(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	return draw_move_line_common(inst, inst->u.rect.end, ctx, pos, i);
}


static int end_new_line(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_line, from);
	obj->u.line.other = inst_get_vec(to);
	obj->u.line.width = NULL;
	return 1;
}


static struct tool_ops line_ops = {
	.drag_new	= drag_new_line,
	.end_new	= end_new_line,
};


/* ----- rect -------------------------------------------------------------- */


static struct pix_buf *drag_new_rect(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	sort_coord(&pos, &to);
	buf = save_pix_buf(DA, pos.x, pos.y, to.x, to.y, 1);
	gdk_draw_rectangle(DA, gc_drag, FALSE,
	    pos.x, pos.y, to.x-pos.x, to.y-pos.y);
	return buf;
}


struct pix_buf *draw_move_rect(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	return draw_move_rect_common(inst, inst->u.rect.end, ctx, pos, i);
}


static int end_new_rect(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_rect, from);
	obj->u.rect.other = inst_get_vec(to);
	obj->u.rect.width = NULL;
	return 1;
}


static struct tool_ops rect_ops = {
	.drag_new	= drag_new_rect,
	.end_new	= end_new_rect,
};


/* ----- pad --------------------------------------------------------------- */


static int end_new_pad(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_pad, from);
	obj->u.pad.other = inst_get_vec(to);
	obj->u.pad.name = stralloc("?");
	return 1;
}


struct pix_buf *draw_move_pad(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	return draw_move_rect_common(inst, inst->u.pad.other, ctx, pos, i);
}


static struct tool_ops pad_ops = {
	.drag_new	= drag_new_rect,
	.end_new	= end_new_pad,
};


/* ----- circ -------------------------------------------------------------- */


static struct pix_buf *drag_new_circ(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;
	double r;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	r = hypot(to.x-pos.x, to.y-pos.y);
	buf = save_pix_buf(DA, pos.x-r, pos.y-r, pos.x+r, pos.y+r, 1);
	draw_circle(DA, gc_drag, FALSE, pos.x, pos.y, r);
	return buf;
}


static int end_new_circ(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_arc, from);
	obj->u.arc.start = inst_get_vec(to);
	obj->u.arc.end = inst_get_vec(to);
	obj->u.arc.width = NULL;
	return 1;
}


struct pix_buf *draw_move_arc(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	struct coord c, from, to, end;
	double r, r_save, a1, a2;
	struct pix_buf *buf;

	c = translate(ctx, inst->base);
	from =
	    translate(ctx, rotate_r(inst->base, inst->u.arc.r, inst->u.arc.a1));
	to =
	    translate(ctx, rotate_r(inst->base, inst->u.arc.r, inst->u.arc.a2));
	pos = translate(ctx, pos);
	switch (i) {
	case 0:
		c = pos;
		break;
	case 1:
		from = pos;
		if (inst->obj->u.arc.start != inst->obj->u.arc.end)
			break;
		/* fall through */
	case 2:
		to = pos;
		break;
	default:
		abort();
	}
	r = hypot(from.x-c.x, from.y-c.y);
	/*
	 * the screen coordinate system is reversed with y growing downward,
	 * so we have to negate the angles.
	 */
	a1 = -theta(c, from);
	a2 = -theta(c, to);
	if (a2 < a1)
		a2 += 360.0;

	if (i != 2)
		r_save = r;
	else {
		r_save = hypot(to.x-c.x, to.y-c.y);
		if (r > r_save)
			r_save = r;
	}
	buf = save_pix_buf(DA,
	    c.x-r_save, c.y-r_save, c.x+r_save, c.y+r_save, 1);
	draw_arc(DA, gc_drag, FALSE, c.x, c.y, r, a1, a2);
	if (i == 2) {
		end = rotate_r(c, r_save, -a2);
		gdk_draw_line(DA, gc_drag, c.x, c.y, end.x, end.y);
	}
	return buf;
}


void do_move_to_arc(struct inst *inst, struct vec *vec, int i)
{
	struct obj *obj = inst->obj;
	int is_circle;

	is_circle = obj->u.arc.start == obj->u.arc.end;
	switch (i) {
	case 0:
		obj->base = vec;
		break;
	case 1:
		obj->u.arc.start = vec;
		if (!is_circle)
			break;
		/* fall through */
	case 2:
		obj->u.arc.end = vec;
		break;
	default:
		abort();
	}
}


static struct tool_ops circ_ops = {
	.drag_new	= drag_new_circ,
	.end_new	= end_new_circ,
};


/* ----- meas -------------------------------------------------------------- */


struct pix_buf *draw_move_meas(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	return draw_move_line_common(inst, inst->u.meas.end, ctx, pos, i);
}


static int end_new_meas(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_meas, from);
	obj->u.meas.other = inst_get_vec(to);
	obj->u.meas.offset = parse_expr("0mm");
	return 1;
}


static struct tool_ops meas_ops = {
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
};


/* ----- frame helper ------------------------------------------------------ */


static int is_parent_of(const struct frame *p, const struct frame *c)
{
	const struct obj *obj;

	if (p == c)
		return 1;
	for (obj = p->objs; obj; obj = obj->next)
		if (obj->type == ot_frame)
			if (is_parent_of(obj->u.frame.ref, c))
				return 1;
	return 0;
}


/* ----- frame cache ------------------------------------------------------- */


static struct frame *locked_frame = NULL;


static void remove_child(GtkWidget *widget, gpointer data)
{
	gtk_container_remove(GTK_CONTAINER(data), widget);
}


static void set_frame_image(GtkWidget *image)
{
	gtk_container_foreach(GTK_CONTAINER(ev_frame), remove_child, ev_frame);
	gtk_container_add(GTK_CONTAINER(ev_frame), image);
	gtk_widget_show_all(ev_frame);
}


void tool_frame_update(void)
{
	set_frame_image(!locked_frame ? frame_image :
	    is_parent_of(locked_frame, active_frame) ?
	    frame_image_locked : frame_image_ready);
}


void tool_frame_deleted(const struct frame *frame)
{
	if (frame == locked_frame) {
		locked_frame = NULL;
		set_frame_image(frame_image);
	}
}


static void tool_selected_frame(void)
{
	if (active_frame != root_frame) {
		locked_frame = active_frame;
		set_frame_image(frame_image_locked);
	}
}


/* ----- frame ------------------------------------------------------------- */


struct pix_buf *draw_move_frame(struct inst *inst, struct draw_ctx *ctx,
    struct coord pos, int i)
{
	struct pix_buf *buf;
	int r = FRAME_EYE_R2;

	pos = translate(ctx, pos);
	buf = save_pix_buf(DA, pos.x-r, pos.y-r, pos.x+r, pos.y+r, 1);
	draw_arc(DA, gc_drag, FALSE, pos.x, pos.y, r, 0, 360);
	return buf;
}


static int end_new_frame(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (!locked_frame || is_parent_of(locked_frame, active_frame))
		return 0;
	obj = new_obj(ot_frame, from);
	obj->u.frame.ref = locked_frame;
	obj->u.frame.lineno = 0;
	if (!locked_frame->active_ref)
		locked_frame->active_ref = obj;
	locked_frame = NULL;
	tool_frame_update();
	return 1;
}


static struct tool_ops frame_ops = {
	.tool_selected	= tool_selected_frame,
	.drag_new	= NULL,
	.end_new	= end_new_frame,
};


/* ----- moving references ------------------------------------------------- */


static int may_move(struct inst *curr)
{
	if (!selected_inst)
		return 0;
	if (drag.anchors_n)
		return 0; /* already moving something else */
	drag.anchors_n = inst_anchors(selected_inst, drag.anchors);
	for (drag.anchor_i = 0; drag.anchor_i != drag.anchors_n;
	    drag.anchor_i++)
		if (*drag.anchors[drag.anchor_i] == inst_get_vec(curr))
			return 1;
	drag.anchors_n = 0;
	return 0;
}


static int would_be_equal(const struct drag_state *state,
    int a, int b, struct inst *curr)
{
	const struct vec *va;
	const struct vec *vb;

	va = a == state->anchor_i ? inst_get_vec(curr) : *state->anchors[a];
	vb = b == state->anchor_i ? inst_get_vec(curr) : *state->anchors[b];
	return va == vb;
}


static int may_move_to(const struct drag_state *state, struct inst *curr)
{
	assert(drag.inst);
	assert(state->anchors_n);
	switch (state->anchors_n) {
	case 3:
		if (would_be_equal(state, 0, 2, curr))
			return 0;
		/* fall through */
	case 2:
		if (would_be_equal(state, 0, 1, curr))
			return 0;
		/* fall through */
	case 1:
		return 1;
	default:
		abort();
	}
}


static void do_move_to(struct drag_state *state, struct inst *curr)
{
	assert(may_move_to(state, curr));
	*state->anchors[state->anchor_i] = inst_get_vec(curr);
}


/* ----- hover ------------------------------------------------------------- */


void tool_dehover(struct draw_ctx *ctx)
{
	if (hover_inst)
		inst_hover(hover_inst, ctx, 0);
	hover_inst = NULL;
}


void tool_hover(struct draw_ctx *ctx, struct coord pos)
{
	struct inst *curr;

	curr = inst_find_point(ctx, pos);
	if (curr && !active_ops) {
		if (drag.anchors_n) {
			if (!may_move_to(&drag, curr))
				curr = NULL;
		} else {
			if (!may_move(curr))
				curr = NULL;
			drag.anchors_n = 0;
		}
	}
	if (curr != hover_inst)
		tool_dehover(ctx);
	if (curr) {
		inst_hover(curr, ctx, 1);
		hover_inst = curr;
	}
}


/* ----- mouse actions ----------------------------------------------------- */


int tool_consider_drag(struct draw_ctx *ctx, struct coord pos)
{
	struct inst *curr;

	assert(!drag.new);
	assert(!drag.anchors_n);
	last_canvas_pos = translate(ctx, pos);
	curr = inst_find_point(ctx, pos);
	if (!curr)
		return 0;
	pix_buf = NULL;
	if (active_ops) {
		if (active_ops->drag_new) {
			drag.inst = NULL;
			drag.new = curr;
			return 1;
		} else {
			/* object is created without dragging */
			if (active_ops->end_new(ctx, curr, NULL)) {
				tool_cancel_drag(ctx);
				return -1;
			}
			return 0;
		}
	}
	if (!may_move(curr))
		return 0;
	drag.inst = selected_inst;
	drag.new = NULL;
	return 1;
}


void tool_drag(struct draw_ctx *ctx, struct coord to)
{
	if (pix_buf)
		restore_pix_buf(pix_buf);
	last_canvas_pos = translate(ctx, to);
	tool_hover(ctx, to);
	pix_buf = drag.new ? active_ops->drag_new(ctx, drag.new, to) :
	    inst_draw_move(drag.inst, ctx, to, drag.anchor_i);
}


void tool_cancel_drag(struct draw_ctx *ctx)
{
	tool_dehover(ctx);
	tool_reset();
	if (pix_buf) {
		restore_pix_buf(pix_buf);
		pix_buf = NULL;
	}
	drag.new = NULL;
	active_ops = NULL;
	drag.anchors_n = 0;
}


int tool_end_drag(struct draw_ctx *ctx, struct coord to)
{
	struct drag_state state = drag;
	struct inst *end;
	struct tool_ops *ops = active_ops;

	tool_cancel_drag(ctx);
	if (state.new && ops->end_new_raw)
		return ops->end_new_raw(ctx, state.new, to);
	end = inst_find_point(ctx, to);
	if (!end)
		return 0;
	if (state.new)
		return ops->end_new(ctx, state.new, end);
	if (!may_move_to(&state, end))
		return 0;
	if (!inst_do_move_to(drag.inst, inst_get_vec(end), state.anchor_i))
		do_move_to(&state, end);
	return 1;
}


void tool_redraw(struct draw_ctx *ctx)
{
	if (!drag.new && !drag.anchors_n)
		return;
	if (pix_buf)
		free_pix_buf(pix_buf);
	pix_buf = NULL;
	tool_drag(ctx, canvas_to_coord(ctx,
	    last_canvas_pos.x, last_canvas_pos.y));
}


/* ----- tool bar creation ------------------------------------------------- */


static void tool_select(GtkWidget *evbox, struct tool_ops *ops)
{
	GdkColor col;

	if (active_tool) {
		col = get_color(TOOL_UNSELECTED);
		gtk_widget_modify_bg(active_tool, GTK_STATE_NORMAL, &col);
		active_tool = NULL;
	}
	col = get_color(TOOL_SELECTED);
	gtk_widget_modify_bg(evbox, GTK_STATE_NORMAL, &col);
	active_tool = evbox;
	active_ops = ops;
	if (ops && ops->tool_selected)
		ops->tool_selected();
}


void tool_reset(void)
{
	tool_select(ev_point, NULL);
}


static gboolean tool_button_press_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
	tool_select(widget, data);
	return TRUE;
}


static GtkWidget *make_image(GdkDrawable *drawable,  char **xpm)
{
	GdkPixmap *pixmap;
	GtkWidget *image;

	pixmap = gdk_pixmap_create_from_xpm_d(drawable, NULL, NULL, xpm);
	image = gtk_image_new_from_pixmap(pixmap, NULL);
	gtk_misc_set_padding(GTK_MISC(image), 1, 1);
	return image;
}


static GtkWidget *tool_button(GtkWidget *bar, GdkDrawable *drawable,
    char **xpm, GtkWidget *last_evbox, struct tool_ops *ops)
{
	GtkWidget *image, *evbox;	
	GtkToolItem *item;
	GtkToolItem *last = NULL;

	if (last_evbox)
		last = GTK_TOOL_ITEM(gtk_widget_get_ancestor(last_evbox,
		    GTK_TYPE_TOOL_ITEM));

/*
 * gtk_radio_tool_button_new_from_widget is *huge*. we try to do things in a
 * more compact way.
 */
#if 0
	if (last)
		item = gtk_radio_tool_button_new_from_widget(
		    GTK_RADIO_TOOL_BUTTON(last));
	else
		item = gtk_radio_tool_button_new(NULL);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item), image);
#else
	evbox = gtk_event_box_new();
	if (xpm) {
		image = make_image(drawable, xpm);
		gtk_container_add(GTK_CONTAINER(evbox), image);
	}
	g_signal_connect(G_OBJECT(evbox), "button_press_event",
            G_CALLBACK(tool_button_press_event), ops);

	item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(item), evbox);

	gtk_container_set_border_width(GTK_CONTAINER(item), 0);
#endif

	gtk_toolbar_insert(GTK_TOOLBAR(bar), item, -1);

	return evbox;
}


GtkWidget *gui_setup_tools(GdkDrawable *drawable)
{
	GtkWidget *bar;
	GtkWidget *last;

	bar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(bar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(bar),
	    GTK_ORIENTATION_VERTICAL);

	ev_point = tool_button(bar, drawable, xpm_point, NULL, NULL);
	last = tool_button(bar, drawable, xpm_vec, ev_point, &vec_ops);
	ev_frame = tool_button(bar, drawable, NULL, last, &frame_ops);
	last = ev_frame;
	last = tool_button(bar, drawable, xpm_pad, last, &pad_ops);
	last = tool_button(bar, drawable, xpm_line, last, &line_ops);
	last = tool_button(bar, drawable, xpm_rect, last, &rect_ops);
	last = tool_button(bar, drawable, xpm_circ, last, &circ_ops);
	last = tool_button(bar, drawable, xpm_meas, last, &meas_ops);

	frame_image = gtk_widget_ref(make_image(drawable, xpm_frame));
	frame_image_locked =
	    gtk_widget_ref(make_image(drawable, xpm_frame_locked));
	frame_image_ready =
	    gtk_widget_ref(make_image(drawable, xpm_frame_ready));
	set_frame_image(frame_image);

	tool_reset();

	return bar;
}
