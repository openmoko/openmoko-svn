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
#include <gtk/gtk.h>

#include "util.h"
#include "coord.h"
#include "error.h"
#include "unparse.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui.h"
#include "gui_status.h"


struct edit_ops {
	int (*changed)(GtkWidget *widget, const char *s, void *ctx);
	int (*activate)(GtkWidget *widget, const char *s, void *ctx);
};

static struct edit_ops *edit_ops = NULL;
static void *edit_ctx;


/* ----- setter functions -------------------------------------------------- */


static GtkWidget *status_name, *status_entry;
static GtkWidget *status_x, *status_y;
static GtkWidget *status_r, *status_angle;
static GtkWidget *status_sys_pos, *status_user_pos;
static GtkWidget *status_zoom, *status_grid;
static GtkWidget *status_msg;


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

SETTER(name)
SETTER(x)
SETTER(y)
SETTER(r)
SETTER(angle)
SETTER(sys_pos)
SETTER(user_pos)
SETTER(zoom)
SETTER(grid)


/* ----- complex status updates -------------------------------------------- */


void status_set_xy(struct coord coord)
{
	status_set_x("x = %5.2f mm", units_to_mm(coord.x));
	status_set_y("y = %5.2f mm", units_to_mm(coord.y));
}


static void entry_color(const char *color)
{
	GdkColor col;

	col = get_color(color);
	gtk_widget_modify_base(GTK_WIDGET(status_entry),
	    GTK_STATE_NORMAL, &col);
}


/* ----- identifier fields ------------------------------------------------- */


struct edit_unique_ctx {
	const char **s;
	int (*validate)(const char *s, void *ctx);
	void *ctx;
};


static int unique_changed(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;
	int ok;

	if (!strcmp(s, *unique_ctx->s)) {
		entry_color(COLOR_EDIT_ASIS);
		return 1;
	}
	ok = !unique_ctx->validate || unique_ctx->validate(s, unique_ctx->ctx);
	entry_color(ok ? COLOR_EDIT_GOOD : COLOR_EDIT_BAD);
	return ok;
}


static int unique_activate(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	if (strcmp(s, *unique_ctx->s) &&
	     unique_ctx->validate && !unique_ctx->validate(s, unique_ctx->ctx))
		return 0;
	*unique_ctx->s = unique(s);
	entry_color(COLOR_EDIT_ASIS);
	return 1;
}


static struct edit_ops edit_ops_unique = {
	.changed	= unique_changed,
	.activate	= unique_activate,
};


void edit_unique(const char **s, int (*validate)(const char *s, void *ctx), 
    void *ctx)
{
	static struct edit_unique_ctx unique_ctx;

	unique_ctx.s = s;
	unique_ctx.validate = validate;
	unique_ctx.ctx = ctx;
	edit_ops = &edit_ops_unique;
	edit_ctx = &unique_ctx;
	gtk_entry_set_text(GTK_ENTRY(status_entry), *s);
	entry_color(COLOR_EDIT_ASIS);
	gtk_widget_show(status_entry);
}


/* ----- identifier fields with NULL --------------------------------------- */


static int unique_null_changed(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;
	int ok;

	if (!strcmp(s, *unique_ctx->s ? *unique_ctx->s : "")) {
		entry_color(COLOR_EDIT_ASIS);
		return 1;
	}
	ok = !*s;
	if (!ok)
		ok = !unique_ctx->validate ||
		     unique_ctx->validate(s, unique_ctx->ctx);
	entry_color(ok ? COLOR_EDIT_GOOD : COLOR_EDIT_BAD);
	return ok;
}


static int unique_null_activate(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_unique_ctx *unique_ctx = ctx;

	if (!*s)
		 *unique_ctx->s = NULL;
	else {
		if (unique_ctx->validate &&
		    !unique_ctx->validate(s, unique_ctx->ctx))
			return 0;
		*unique_ctx->s = unique(s);
	}
	entry_color(COLOR_EDIT_ASIS);
	return 1;
}


static struct edit_ops edit_ops_null_unique = {
	.changed	= unique_null_changed,
	.activate	= unique_null_activate,
};


void edit_unique_null(const char **s,
    int (*validate)(const char *s, void *ctx), void *ctx)
{
	static struct edit_unique_ctx unique_ctx;

	unique_ctx.s = s;
	unique_ctx.validate = validate;
	unique_ctx.ctx = ctx;
	edit_ops = &edit_ops_null_unique;
	edit_ctx = &unique_ctx;
	gtk_entry_set_text(GTK_ENTRY(status_entry), *s ? *s : "");
	entry_color(COLOR_EDIT_ASIS);
	gtk_widget_show(status_entry);
}


/* ----- string fields ----------------------------------------------------- */


struct edit_name_ctx {
	char **s;
	int (*validate)(const char *s, void *ctx);
	void *ctx;
};


static int name_changed(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_name_ctx *name_ctx = ctx;
	int ok;

	if (!strcmp(s, *name_ctx->s)) {
		entry_color(COLOR_EDIT_ASIS);
		return 1;
	}
	ok = !name_ctx->validate || name_ctx->validate(s, name_ctx->ctx);
	entry_color(ok ? COLOR_EDIT_GOOD : COLOR_EDIT_BAD);
	return ok;
}


static int name_activate(GtkWidget *widget, const char *s, void *ctx)
{
	const struct edit_name_ctx *name_ctx = ctx;

	if (name_ctx->validate && !name_ctx->validate(s, name_ctx->ctx))
		return 0;
	free(*name_ctx->s);
	*name_ctx->s = stralloc(s);
	entry_color(COLOR_EDIT_ASIS);
	return 1;
}


static struct edit_ops edit_ops_name = {
	.changed	= name_changed,
	.activate	= name_activate,
};


void edit_name(char **s, int (*validate)(const char *s, void *ctx), void *ctx)
{
	static struct edit_name_ctx name_ctx;

	name_ctx.s = s;
	name_ctx.validate = validate;
	name_ctx.ctx = ctx;
	edit_ops = &edit_ops_name;
	edit_ctx = &name_ctx;
	gtk_entry_set_text(GTK_ENTRY(status_entry), *s);
	entry_color(COLOR_EDIT_ASIS);
	gtk_widget_show(status_entry);
}


/* ----- expression fields ------------------------------------------------- */


static struct expr *try_parse_expr(const char *s)
{
	status_begin_reporting();
	return parse_expr(s);
}


static int expr_changed(GtkWidget *widget, const char *s, void *ctx)
{
	struct expr *expr;

	expr = try_parse_expr(s);
	if (!expr) {
		entry_color(COLOR_EDIT_BAD);
		return 0;
	}
	entry_color(COLOR_EDIT_GOOD);
	free_expr(expr);
	return 1;
}


static int expr_activate(GtkWidget *widget, const char *s, void *ctx)
{
	struct expr **anchor = ctx;
	struct expr *expr;

	expr = try_parse_expr(s);
	if (!expr)
		return 0;
	if (*anchor)
		free_expr(*anchor);
	*anchor = expr;
	entry_color(COLOR_EDIT_ASIS);
	return 1;
}


static struct edit_ops edit_ops_expr = {
	.changed	= expr_changed,
	.activate	= expr_activate,
};


void edit_expr(struct expr **expr)
{
	char *s;

	edit_ops = &edit_ops_expr;
	edit_ctx = expr;
	s = unparse(*expr);
	gtk_entry_set_text(GTK_ENTRY(status_entry), s);
	free(s);
	entry_color(COLOR_EDIT_ASIS);
	gtk_widget_show(status_entry);
}


/* ----- text entry -------------------------------------------------------- */


static gboolean changed(GtkWidget *widget, GdkEventMotion *event,
     gpointer data)
{
	if (edit_ops && edit_ops->changed)
		edit_ops->changed(widget,
		    gtk_entry_get_text(GTK_ENTRY(widget)), edit_ctx);
	return TRUE;
}


static gboolean activate(GtkWidget *widget, GdkEventMotion *event,
     gpointer data)
{
	if (edit_ops && edit_ops->activate)
		if (edit_ops->activate(widget,
		    gtk_entry_get_text(GTK_ENTRY(widget)), edit_ctx)) {
			inst_deselect();
			change_world();
		}
	return TRUE;
}


void edit_nothing(void)
{
	edit_ops = NULL;
	gtk_widget_hide(status_entry);
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


/* ----- setup ------------------------------------------------------------- */


static GtkWidget *add_label(GtkWidget *tab, int col, int row)
{
	GtkWidget *label;

	label = label_in_box_new(NULL);
	gtk_table_attach_defaults(GTK_TABLE(tab), box_of_label(label),
	    col, col+1, row, row+1);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	return label;
}


void make_status_area(GtkWidget *vbox)
{
	GtkWidget *tab, *hbox;

	tab = gtk_table_new(5, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), tab, FALSE, TRUE, 0);

	/* name and input */

	status_name = add_label(tab, 0, 0);

	/*
	 * @@@ this should make the entry consume all available space. alas, it
	 * doesn't work like that :-(
	 */
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tab), hbox,
	    0, 1, 1, 2);
	
	status_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), status_entry, TRUE, TRUE, 0);

	gtk_entry_set_has_frame(GTK_ENTRY(status_entry), FALSE);

	g_signal_connect(G_OBJECT(status_entry), "changed",
            G_CALLBACK(changed), status_entry);
	g_signal_connect(G_OBJECT(status_entry), "activate",
            G_CALLBACK(activate), status_entry);

	/* x / y */

	status_x = add_label(tab, 1, 0);
	status_y = add_label(tab, 1, 1);

	/* r / angle */

	status_r = add_label(tab, 2, 0);
	status_angle = add_label(tab, 2, 1);

	/* sys / user pos */

	status_sys_pos = add_label(tab, 3, 0);
	status_user_pos = add_label(tab, 3, 1);

	/* zoom / grid */

	status_zoom = add_label(tab, 4, 0);
	status_grid = add_label(tab, 4, 1);

	/* message bar */

	status_msg = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), status_msg, FALSE, FALSE, 0);

	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_msg),
	    "messages");
}
