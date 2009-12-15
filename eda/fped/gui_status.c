/*
 * gui_status.c - GUI, status area
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "util.h"
#include "coord.h"
#include "error.h"
#include "unparse.h"
#include "obj.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_canvas.h"
#include "gui.h"
#include "gui_status.h"


enum edit_status {
	es_unchanged,
	es_good,
	es_bad,
};


struct edit_ops {
	char *(*retrieve)(void *ctx);
	enum edit_status (*status)(const char *s, void *ctx);
	void (*store)(const char *s, void *ctx);
};


enum curr_unit curr_unit = curr_unit_mm;


static GtkWidget *open_edits = NULL;
static GtkWidget *last_edit = NULL;


/* ----- setter functions -------------------------------------------------- */


static GtkWidget *status_icon;
static GtkWidget *status_name, *status_entry;
static GtkWidget *status_type_x, *status_type_y, *status_type_entry;
static GtkWidget *status_box_x, *status_entry_y;
static GtkWidget *status_x, *status_y;
static GtkWidget *status_r, *status_angle;
static GtkWidget *status_sys_x, *status_sys_y;
static GtkWidget *status_user_x, *status_user_y;
static GtkWidget *status_zoom, *status_grid, *status_unit;
static GtkWidget *status_msg;

/* The x entry area serves multiple purposes */

static GtkWidget *status_entry_x;


static void set_label(GtkWidget *label, const char *fmt, va_list ap)
{
	char *s;

	s = stralloc_vprintf(fmt, ap);
	gtk_label_set_text(GTK_LABEL(label), s);
	free(s);
}


#define	SETTER(name)					\
	void status_set_##name(const char *fmt, ...)	\
	{						\
		va_list ap;				\
							\
		va_start(ap, fmt);			\
		set_label(status_##name, fmt, ap);	\
		va_end(ap);				\
	}

SETTER(type_x)
SETTER(type_y)
SETTER(type_entry)
SETTER(name)
SETTER(x)
SETTER(y)
SETTER(r)
SETTER(angle)
SETTER(sys_x)
SETTER(sys_y)
SETTER(user_x)
SETTER(user_y)
SETTER(zoom)
SETTER(grid)
SETTER(unit)


/* ----- set things with units --------------------------------------------- */


void set_with_units(void (*set)(const char *fmt, ...), const char *prefix,
    unit_type u)
{
	double n;
	int mm;

	switch (curr_unit) {
	case curr_unit_mm:
		n = units_to_mm(u);
		mm = 1;
		break;
	case curr_unit_mil:
		n = units_to_mil(u);
		mm = 0;
		break;
	case curr_unit_auto:
		n = units_to_best(u, &mm);
		break;
	default:
		abort();
	}
	if (mm) {
		/* -NNN.NNN mm */
		set("%s" MM_FORMAT_FIXED " mm", prefix, n);
	} else {
		/* -NNNN.N mil */
		set("%s" MIL_FORMAT_FIXED " mil", prefix, n);
	}
}


/* ----- complex status updates -------------------------------------------- */


void status_set_icon(GtkWidget *image)
{
	vacate_widget(status_icon);
	if (image)
		gtk_container_add(GTK_CONTAINER(status_icon), image);
	gtk_widget_show_all(status_icon);
}


void status_set_xy(struct coord coord)
{
	/* do dX/dY etc. stuff later */
	status_set_type_x("X =");
	status_set_type_y("Y =");

	set_with_units(status_set_x, "", coord.x);
	set_with_units(status_set_y, "", coord.y);
}


void status_set_angle_xy(struct coord v)
{
	if (!v.x && !v.y)
		status_set_angle("a = 0 deg");
	else
		status_set_angle("a = %3.1f deg", theta_vec(v));

}


static void entry_color(GtkWidget *widget, const char *color)
{
	GdkColor col;

	col = get_color(color);
	gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &col);
}


/* ----- pad type display and change --------------------------------------- */


static enum pad_type *curr_pad_type;
static GtkWidget *pad_type;


static void show_pad_type(void)
{
	const char *s;

	switch (*curr_pad_type) {
	case pt_normal:
		s = "normal";
		break;
	case pt_bare:
		s = "bare";
		break;
	case pt_paste:
		s = "paste";
		break;
	case pt_mask:
		s = "mask";
		break;
	default:
		abort();
	}
	gtk_label_set_text(GTK_LABEL(pad_type), s);
}


static gboolean pad_type_button_press_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
	switch (event->button) {
	case 1:
		*curr_pad_type = (*curr_pad_type+1) % pt_n;
		show_pad_type();
		break;
	}
	/*
	 * We can't just redraw() here, because changing the pad type may also
	 * affect the visual stacking. So we change the world and hope we end
	 * up selecting the same pad afterwards.
	 */
	change_world_reselect();
	return TRUE;
}


void edit_pad_type(enum pad_type *type)
{
	vacate_widget(status_box_x);
	curr_pad_type = type;
	pad_type = label_in_box_new(NULL);
	gtk_container_add(GTK_CONTAINER(status_box_x), box_of_label(pad_type));
	label_in_box_bg(pad_type, COLOR_SELECTOR);
	g_signal_connect(G_OBJECT(box_of_label(pad_type)),
	    "button_press_event", G_CALLBACK(pad_type_button_press_event),
	    NULL);
	show_pad_type();
	gtk_widget_show_all(status_box_x);
}


/* ----- edit helper functions --------------------------------------------- */


static void reset_edit(GtkWidget *widget)
{
	struct edit_ops *ops;
	void *ctx;
	char *s;

	ops = gtk_object_get_data(GTK_OBJECT(widget), "edit-ops");
	ctx = gtk_object_get_data(GTK_OBJECT(widget), "edit-ctx");
	assert(ops);
	s = ops->retrieve(ctx);
	gtk_object_set_data(GTK_OBJECT(widget), "edit-ops", NULL);
	gtk_entry_set_text(GTK_ENTRY(widget), s);
	free(s);
	entry_color(widget, COLOR_EDIT_ASIS);
	gtk_object_set_data(GTK_OBJECT(widget), "edit-ops", ops);
}


static void reset_edits(void)
{
	GtkWidget *edit;

	for (edit = open_edits; edit;
	    edit = gtk_object_get_data(GTK_OBJECT(edit), "edit-next"))
		reset_edit(edit);
	gtk_widget_grab_focus(GTK_WIDGET(open_edits));
}


static gboolean edit_key_press_event(GtkWidget *widget, GdkEventKey *event,
    gpointer data)
{
	GtkWidget *next = gtk_object_get_data(GTK_OBJECT(widget), "edit-next");

	switch (event->keyval) {
	case GDK_Tab:
		gtk_widget_grab_focus(GTK_WIDGET(next ? next : open_edits));
		return TRUE;
	case GDK_Escape:
		reset_edits();
		return TRUE;
	default:
		return FALSE;
	}
}


static void setup_edit(GtkWidget *widget, struct edit_ops *ops, void *ctx)
{
	gtk_object_set_data(GTK_OBJECT(widget), "edit-ops", ops);
	gtk_object_set_data(GTK_OBJECT(widget), "edit-ctx", ctx);
	gtk_object_set_data(GTK_OBJECT(widget), "edit-next", NULL);

	reset_edit(widget);

	if (!open_edits)
		gtk_widget_grab_focus(GTK_WIDGET(widget));
	gtk_widget_show(widget);

	g_signal_connect(G_OBJECT(widget), "key_press_event",
	    G_CALLBACK(edit_key_press_event), open_edits);

	if (last_edit)
		gtk_object_set_data(GTK_OBJECT(last_edit), "edit-next", widget);
	else
		open_edits = widget;
	last_edit = widget;
}


/* ----- identifier fields ------------------------------------------------- */


struct edit_unique_ctx {
	const char **s;
	int (*validate)(const char *s, void *ctx);
	void *ctx;
};


/*
 * Handle NULL so that we can also use it for unique_null
 */

static char *unique_retrieve(void *ctx)
{
	struct edit_unique_ctx *unique_ctx = ctx;

	return stralloc(*unique_ctx->s ? *unique_ctx->s : "");
}


static enum edit_status unique_status(const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	if (!strcmp(s, *unique_ctx->s))
		return es_unchanged;
	return !unique_ctx->validate ||
	    unique_ctx->validate(s, unique_ctx->ctx) ? es_good : es_bad;
}


static void unique_store(const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	*unique_ctx->s = unique(s);
}


static struct edit_ops edit_ops_unique = {
	.retrieve	= unique_retrieve,
	.status		= unique_status,
	.store		= unique_store,
};


void edit_unique(const char **s, int (*validate)(const char *s, void *ctx), 
    void *ctx)
{
	static struct edit_unique_ctx unique_ctx;

	unique_ctx.s = s;
	unique_ctx.validate = validate;
	unique_ctx.ctx = ctx;
	setup_edit(status_entry, &edit_ops_unique, &unique_ctx);
}


/* ----- identifier fields with NULL --------------------------------------- */


static enum edit_status unique_null_status(const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	if (!strcmp(s, *unique_ctx->s ? *unique_ctx->s : ""))
		return es_unchanged;
	if (!*s)
		return es_good;
	return !unique_ctx->validate ||
	    unique_ctx->validate(s, unique_ctx->ctx) ? es_good : es_bad;
}


static void unique_null_store(const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	if (!*s)
		*unique_ctx->s = NULL;
	else
		*unique_ctx->s = unique(s);
}


static struct edit_ops edit_ops_null_unique = {
	.retrieve	= unique_retrieve,
	.status		= unique_null_status,
	.store		= unique_null_store,
};


void edit_unique_null(const char **s,
    int (*validate)(const char *s, void *ctx), void *ctx)
{
	static struct edit_unique_ctx unique_ctx;

	unique_ctx.s = s;
	unique_ctx.validate = validate;
	unique_ctx.ctx = ctx;
	setup_edit(status_entry, &edit_ops_null_unique, &unique_ctx);
}


/* ----- unique field (variable) optionally followed by values ------------- */


struct edit_unique_with_values_ctx {
	const char **s;
	int (*validate)(const char *s, void *ctx);
	void *ctx;
	void (*set_values)(void *user, const struct value *values,
	    int n_values);
	void *user;
	int max_values;
};


static enum edit_status unique_with_values_status(const char *s, void *ctx)
{
	const struct edit_unique_with_values_ctx *unique_ctx = ctx;
	const char *id;
	struct value *values;
	int n;

	if (!strcmp(s, *unique_ctx->s))
		return es_unchanged;
	status_begin_reporting();
	n = parse_var(s, &id, &values, unique_ctx->max_values);
	if (n < 0)
		return es_bad;
	free_values(values, 0);
	return !unique_ctx->validate ||
	    unique_ctx->validate(id, unique_ctx->ctx) ? es_good : es_bad;
}


static void unique_with_values_store(const char *s, void *ctx)
{
	const struct edit_unique_with_values_ctx *unique_ctx = ctx;
	struct value *values;
	int n;

	status_begin_reporting();
	n = parse_var(s, unique_ctx->s, &values, unique_ctx->max_values);
	if (!n)
		return;
	assert(n >= 0);
	assert(unique_ctx->max_values == -1 || n <= unique_ctx->max_values);
	unique_ctx->set_values(unique_ctx->user, values, n);
	free_values(values, 1);
}


static struct edit_ops edit_ops_unique_with_values = {
	.retrieve	= unique_retrieve,
	.status		= unique_with_values_status,
	.store		= unique_with_values_store,
};


void edit_unique_with_values(const char **s,
    int (*validate)(const char *s, void *ctx), void *ctx,
    void (*set_values)(void *user, const struct value *values, int n_values),
    void *user, int max_values)
{
	static struct edit_unique_with_values_ctx unique_ctx;

	unique_ctx.s = s;
	unique_ctx.validate = validate;
	unique_ctx.ctx = ctx;
	unique_ctx.set_values = set_values;
	unique_ctx.user = user;
	unique_ctx.max_values = max_values;
	setup_edit(status_entry, &edit_ops_unique_with_values, &unique_ctx);
}


/* ----- string fields ----------------------------------------------------- */


struct edit_name_ctx {
	char **s;
	int (*validate)(const char *s, void *ctx);
	void *ctx;
};


static char *name_retrieve(void *ctx)
{
	struct edit_name_ctx *name_ctx = ctx;

	return stralloc(*name_ctx->s ? *name_ctx->s : "");
}


static enum edit_status name_status(const char *s, void *ctx)
{
	const struct edit_name_ctx *name_ctx = ctx;

	if (!strcmp(s, *name_ctx->s))
		return es_unchanged;
	return !name_ctx->validate || name_ctx->validate(s, name_ctx->ctx) ?
	    es_good : es_bad;
}


static void name_store(const char *s, void *ctx)
{
	const struct edit_name_ctx *name_ctx = ctx;

	free(*name_ctx->s);
	*name_ctx->s = stralloc(s);
}


static struct edit_ops edit_ops_name = {
	.retrieve	= name_retrieve,
	.status		= name_status,
	.store		= name_store,
};


void edit_name(char **s, int (*validate)(const char *s, void *ctx), void *ctx)
{
	static struct edit_name_ctx name_ctx;

	name_ctx.s = s;
	name_ctx.validate = validate;
	name_ctx.ctx = ctx;
	setup_edit(status_entry, &edit_ops_name, &name_ctx);
}


/* ----- expression fields ------------------------------------------------- */


static struct expr *try_parse_expr(const char *s)
{
	status_begin_reporting();
	return parse_expr(s);
}


static char *expr_retrieve(void *ctx)
{
	struct expr **expr = ctx;

	return unparse(*expr);
}


static enum edit_status expr_status(const char *s, void *ctx)
{
	struct expr *expr;

	expr = try_parse_expr(s);
	if (!expr)
		return es_bad;
	free_expr(expr);
	return es_good;
}


static void expr_store(const char *s, void *ctx)
{
	struct expr **anchor = ctx;
	struct expr *expr;

	expr = try_parse_expr(s);
	assert(expr);
	if (*anchor)
		free_expr(*anchor);
	*anchor = expr;
}


static struct edit_ops edit_ops_expr = {
	.retrieve	= expr_retrieve,
	.status		= expr_status,
	.store		= expr_store,
};


static void edit_any_expr(GtkWidget *widget, struct expr **expr)
{
	setup_edit(widget, &edit_ops_expr, expr);
}


void edit_expr(struct expr **expr)
{
	edit_any_expr(status_entry, expr);
}


void edit_x(struct expr **expr)
{
	vacate_widget(status_box_x);
	gtk_container_add(GTK_CONTAINER(status_box_x), status_entry_x);
	gtk_widget_show(status_box_x);
	edit_any_expr(status_entry_x, expr);
}


void edit_y(struct expr **expr)
{
	edit_any_expr(status_entry_y, expr);
}


/* ----- expression list --------------------------------------------------- */


struct edit_expr_list_ctx {
	struct expr *expr;
	void (*set_values)(void *user, const struct value *values,
	    int n_values);
	void *user;
};


static char *expr_list_retrieve(void *ctx)
{
	struct edit_expr_list_ctx *expr_list_ctx = ctx;

	return unparse(expr_list_ctx->expr);
}


static enum edit_status expr_list_status(const char *s, void *ctx)
{
	struct value *values;
	int n;

	status_begin_reporting();
	n = parse_values(s, &values);
	if (n < 0)
		return es_bad;
	free_values(values, 0);
	return es_good;
}


static void expr_list_store(const char *s, void *ctx)
{
	struct edit_expr_list_ctx *expr_list_ctx = ctx;
	struct value *values;
	int n;

	status_begin_reporting();
	n = parse_values(s, &values);
	assert(n >= 0);
	expr_list_ctx->set_values(expr_list_ctx->user, values, n);
	free_values(values, 1);
}


static struct edit_ops edit_ops_expr_list = {
	.retrieve	= expr_list_retrieve,
	.status		= expr_list_status,
	.store		= expr_list_store,
};


void edit_expr_list(struct expr *expr,
    void (*set_values)(void *user, const struct value *values, int n_values),
    void *user)
{
	static struct edit_expr_list_ctx expr_list_ctx;

	expr_list_ctx.expr = expr;
	expr_list_ctx.set_values = set_values;
	expr_list_ctx.user = user;
	setup_edit(status_entry, &edit_ops_expr_list, &expr_list_ctx);
}


/* ----- text entry -------------------------------------------------------- */


static enum edit_status get_status(GtkWidget *widget)
{
	struct edit_ops *ops;
	void *ctx;
	const char *s;

	ops = gtk_object_get_data(GTK_OBJECT(widget), "edit-ops");
	if (!ops)
		return es_unchanged;
	ctx = gtk_object_get_data(GTK_OBJECT(widget), "edit-ctx");
	s = gtk_entry_get_text(GTK_ENTRY(widget));
	return ops->status(s, ctx);
}


static void set_edit(GtkWidget *widget)
{
	struct edit_ops *ops;
	void *ctx;
	const char *s;

	ops = gtk_object_get_data(GTK_OBJECT(widget), "edit-ops");
	if (!ops)
		return;
	ctx = gtk_object_get_data(GTK_OBJECT(widget), "edit-ctx");
	s = gtk_entry_get_text(GTK_ENTRY(widget));
	if (ops->store)
		ops->store(s, ctx);
}


static gboolean changed(GtkWidget *widget, GdkEventMotion *event,
    gpointer data)
{
	switch (get_status(widget)) {
	case es_unchanged:
		entry_color(widget, COLOR_EDIT_ASIS);
		break;
	case es_good:
		entry_color(widget, COLOR_EDIT_GOOD);
		break;
	case es_bad:
		entry_color(widget, COLOR_EDIT_BAD);
		break;
	default:
		abort();
	}
	return TRUE;
}


static gboolean activate(GtkWidget *widget, GdkEventMotion *event,
    gpointer data)
{
	GtkWidget *edit;
	enum edit_status status;
	int unchanged = 1;

	for (edit = open_edits; edit;
	    edit = gtk_object_get_data(GTK_OBJECT(edit), "edit-next")) {
		status = get_status(edit);
		if (status == es_bad)
			return TRUE;
		if (status == es_good)
			unchanged = 0;
	}
	if (unchanged)
		return TRUE;
	for (edit = open_edits; edit;
	    edit = gtk_object_get_data(GTK_OBJECT(edit), "edit-next"))
		if (get_status(edit) == es_good) {
			entry_color(edit, COLOR_EDIT_ASIS);
			set_edit(edit);
		}
	inst_deselect();
	change_world();
	return TRUE;
}


void edit_nothing(void)
{
	gtk_widget_hide(status_entry);
	gtk_widget_hide(status_box_x);
	gtk_widget_hide(status_entry_y);
	open_edits = NULL;
	last_edit = NULL;
}


/* ----- status reports ---------------------------------------------------- */


static gint context_id;
static int have_msg = 0;


static void clear_status_msg(void)
{
	if (have_msg) {
		gtk_statusbar_pop(GTK_STATUSBAR(status_msg), context_id);
		have_msg = 0;
	}
}


static void report_to_gui(const char *s)
{
	if (!have_msg)
		gtk_statusbar_push(GTK_STATUSBAR(status_msg), context_id, s);
	have_msg = 1;
}


void status_begin_reporting(void)
{
	clear_status_msg();
	reporter = report_to_gui;
}


/* ----- unit selection ---------------------------------------------------- */


static void show_curr_unit(void)
{
	switch (curr_unit) {
	case curr_unit_mm:
		status_set_unit("mm");
		break;
	case curr_unit_mil:
		status_set_unit("mil");
		break;
	case curr_unit_auto:
		status_set_unit("auto");
		break;
	default:
		abort();
	}
}


static gboolean unit_button_press_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
	switch (event->button) {
	case 1:
		curr_unit = (curr_unit+1) % curr_unit_n;
		show_curr_unit();
		break;
	}
	refresh_pos();
	change_world();
	return TRUE;
}


/* ----- setup ------------------------------------------------------------- */


static GtkWidget *add_vbox(GtkWidget *tab, int col, int row)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tab), vbox,
	    col, col+1, row, row+1);
	return vbox;
}


static GtkWidget *add_label_basic(GtkWidget *tab, int col, int row)
{
	GtkWidget *label;

	label = label_in_box_new(NULL);
	gtk_table_attach(GTK_TABLE(tab), box_of_label(label),
	    col, col+1, row, row+1,
	    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 1);
	    /* 0 , 1 - padding */
	return label;
}


static GtkWidget *add_label(GtkWidget *tab, int col, int row)
{
	GtkWidget *label;

	label = add_label_basic(tab, col, row);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	return label;
}


static GtkWidget *make_entry(void)
{
	GtkWidget *entry;

	entry = gtk_entry_new();
	gtk_entry_set_has_frame(GTK_ENTRY(entry), FALSE);

	g_signal_connect(G_OBJECT(entry), "changed",
	    G_CALLBACK(changed), entry);
	g_signal_connect(G_OBJECT(entry), "activate",
	    G_CALLBACK(activate), entry);

	return entry;
}


static GtkWidget *add_entry(GtkWidget *tab, int col, int row)
{
	GtkWidget *entry;

	entry = make_entry();
	gtk_table_attach_defaults(GTK_TABLE(tab), entry,
	    col, col+1, row, row+1);
	return entry;
}


void make_status_area(GtkWidget *vbox)
{
	GtkWidget *tab, *sep;
	GtkWidget *hbox, *vbox2;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 1);

	status_icon = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2), status_icon, FALSE, FALSE, 0);

	tab = gtk_table_new(7, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), tab, TRUE, TRUE, 0);

	/* types */

	status_type_x = add_label(tab, 0, 0);
	status_type_y = add_label(tab, 0, 1);
	status_type_entry = add_label(tab, 0, 2);

	/* x / y */

	status_x = add_label(tab, 1, 0);
	status_box_x = add_vbox(tab, 2, 0);
	status_y = add_label(tab, 1, 1);
	status_entry_y = add_entry(tab, 2, 1);

	status_entry_x = gtk_widget_ref(make_entry());
	
	/* name and input */

	status_name = add_label(tab, 1, 2);
	status_entry = add_entry(tab, 2, 2);

	/* separator */

	sep = gtk_vseparator_new();
	gtk_table_attach_defaults(GTK_TABLE(tab), sep, 3, 4, 0, 3);

	/* sys / user pos */

	status_sys_x = add_label(tab, 4, 0);
	status_sys_y = add_label(tab, 4, 1);
	status_user_x = add_label(tab, 5, 0);
	status_user_y = add_label(tab, 5, 1);

	/* r / angle */

	status_r = add_label(tab, 4, 2);
	status_angle = add_label(tab, 5, 2);

	/* zoom / grid / unit */

	status_zoom = add_label(tab, 6, 0);
	status_grid = add_label(tab, 6, 1);
	status_unit = add_label_basic(tab, 6, 2);

	/* unit selection */

	label_in_box_bg(status_unit, COLOR_SELECTOR);
	show_curr_unit();
	g_signal_connect(G_OBJECT(box_of_label(status_unit)),
	    "button_press_event", G_CALLBACK(unit_button_press_event), NULL);

	/* message bar */

	status_msg = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), status_msg, FALSE, FALSE, 0);

	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_msg),
	    "messages");
}


void cleanup_status_area(void)
{
	gtk_widget_unref(status_entry_x);
}
