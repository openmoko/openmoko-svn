/*
 * gui.c - Editor GUI core
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
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

#include "util.h"
#include "inst.h"
#include "obj.h"
#include "unparse.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_status.h"
#include "gui_canvas.h"
#include "gui_icons.h"
#include "gui.h"


GtkWidget *root;

static GtkWidget *frames_box;


/* ----- menu bar ---------------------------------------------------------- */


static void make_menu_bar(GtkWidget *vbox)
{
	GtkWidget *bar;
	GtkWidget *file_menu, *file, *quit;

	bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

	file_menu = gtk_menu_new();

	file = gtk_menu_item_new_with_label("File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(bar), file);

	quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit);

	g_signal_connect(G_OBJECT(quit), "activate",
	    G_CALLBACK(gtk_main_quit), NULL);
}


/* ----- variable list ----------------------------------------------------- */


static void add_sep(GtkWidget *box, int size)
{
	GtkWidget *sep;
	GdkColor black = { 0, 0, 0, 0 };

	sep = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(box), sep, FALSE, TRUE, size);
	gtk_widget_modify_bg(sep, GTK_STATE_NORMAL, &black);
}


/* ----- variable name editor ---------------------------------------------- */


static int find_var_in_frame(const struct frame *frame, const char *name)
{
	const struct table *table;
	const struct loop *loop;
	const struct var *var;

	for (table = frame->tables; table; table = table->next)
		for (var = table->vars; var; var = var->next)
			if (!strcmp(var->name, name))
				return 1;
	for (loop = frame->loops; loop; loop = loop->next)
		if (!strcmp(loop->var.name, name))
			return 1;
        return 0;
}


static int validate_var_name(const char *s, void *ctx)
{
	struct var *var = ctx;

	if (!is_id(s))
		return 0;
	return !find_var_in_frame(var->frame, s);
}


static void unselect_var(void *data)
{
	struct var *var = data;

        label_in_box_bg(var->widget, COLOR_VAR_PASSIVE);
}


static void edit_var(struct var *var)
{
	inst_select_outside(var, unselect_var);
        label_in_box_bg(var->widget, COLOR_VAR_EDITING);
	status_set_name(var->name);
	edit_unique(&var->name, validate_var_name, var);
}


/* ----- value editor ------------------------------------------------------ */


static void unselect_value(void *data)
{
	struct value *value = data;

        label_in_box_bg(value->widget,
	    value->row && value->row->table->active_row == value->row ?
	     COLOR_CHOICE_SELECTED : COLOR_EXPR_PASSIVE);
}


static void edit_value(struct value *value)
{
	inst_select_outside(value, unselect_value);
        label_in_box_bg(value->widget, COLOR_EXPR_EDITING);
	edit_expr(&value->expr);
}


/* ----- activator --------------------------------------------------------- */


static GtkWidget *add_activator(GtkWidget *hbox, int active,
    gboolean (*cb)(GtkWidget *widget, GdkEventButton *event, gpointer data),
    gpointer user, const char *fmt, ...)
{
	GtkWidget *label;
	va_list ap;
	char buf[100];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	label = label_in_box_new(buf);
	gtk_misc_set_padding(GTK_MISC(label), 2, 2);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	label_in_box_bg(label,
	    active ? COLOR_CHOICE_SELECTED : COLOR_CHOICE_UNSELECTED);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(label),
	    FALSE, FALSE, 2);
	g_signal_connect(G_OBJECT(box_of_label(label)),
	    "button_press_event", G_CALLBACK(cb), user);
	return label;
}


/* ----- assignments ------------------------------------------------------- */


static gboolean assignment_var_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	edit_var(data);
	return TRUE;
}


static gboolean assignment_value_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	edit_value(data);
	return TRUE;
}


static void build_assignment(GtkWidget *vbox, struct frame *frame,
     struct table *table)
{
	GtkWidget *hbox, *field;
	char *expr;

	if (!table->vars || table->vars->next)
		return;
	if (!table->rows || table->rows->next)
		return;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	field = label_in_box_new(table->vars->name);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(field), FALSE, FALSE, 0);
	label_in_box_bg(field, COLOR_VAR_PASSIVE);
	table->vars->widget = field;
	g_signal_connect(G_OBJECT(box_of_label(field)),
	    "button_press_event",
	    G_CALLBACK(assignment_var_select_event), table->vars);

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(" = "),
	    FALSE, FALSE, 0);

	expr = unparse(table->rows->values->expr);
	field = label_in_box_new(expr);
	free(expr);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(field), FALSE, FALSE, 0);
	label_in_box_bg(field, COLOR_EXPR_PASSIVE);
	table->rows->values->widget = field;
	g_signal_connect(G_OBJECT(box_of_label(field)),
	    "button_press_event",
	    G_CALLBACK(assignment_value_select_event), table->rows->values);
}


/* ----- tables ------------------------------------------------------------ */


static void select_row(struct row *row)
{
	struct table *table = row->table;
	struct value *value;

	for (value = table->active_row->values; value; value = value->next)
		label_in_box_bg(value->widget, COLOR_ROW_UNSELECTED);
	table->active_row = row;
	for (value = table->active_row->values; value; value = value->next)
		label_in_box_bg(value->widget, COLOR_ROW_SELECTED);
}


static gboolean table_var_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	edit_var(data);
	return TRUE;
}


static gboolean table_value_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	struct value *value = data;

	if (!value->row || value->row->table->active_row == value->row)
		edit_value(value);
	else {
		select_row(value->row);
		change_world();
	}
	return TRUE;
}


static void build_table(GtkWidget *vbox, struct frame *frame,
     struct table *table)
{
	GtkWidget *tab, *field;
	GtkWidget *evbox;
	struct var *var;
	struct row *row;
	struct value *value;
	int n_vars = 0, n_rows = 0;
	char *expr;
	GdkColor col;

	for (var = table->vars; var; var = var->next)
		n_vars++;
	for (row = table->rows; row; row = row->next)
		n_rows++;

	if (n_vars == 1 && n_rows == 1)
		return;

	evbox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox), evbox, FALSE, FALSE, 0);

	tab = gtk_table_new(n_rows+1, n_vars, FALSE);
	gtk_container_add(GTK_CONTAINER(evbox), tab);
	col = get_color(COLOR_VAR_TABLE_SEP);
	gtk_widget_modify_bg(GTK_WIDGET(evbox),
            GTK_STATE_NORMAL, &col);

	gtk_table_set_row_spacings(GTK_TABLE(tab), 1);
	gtk_table_set_col_spacings(GTK_TABLE(tab), 1);

	n_vars = 0;
	for (var = table->vars; var; var = var->next) {
		field = label_in_box_new(var->name);
		gtk_table_attach_defaults(GTK_TABLE(tab), box_of_label(field),
		    n_vars, n_vars+1, 0, 1);
		label_in_box_bg(field, COLOR_VAR_PASSIVE);
		g_signal_connect(G_OBJECT(box_of_label(field)),
		    "button_press_event",
		    G_CALLBACK(table_var_select_event), var);
		var->widget = field;
		n_vars++;
	}
	n_rows = 0;
	for (row = table->rows; row; row = row->next) {
		n_vars = 0;
		for (value = row->values; value; value = value->next) {
			expr = unparse(value->expr);
			field = label_in_box_new(expr);
			free(expr);
			gtk_table_attach_defaults(GTK_TABLE(tab),
			    box_of_label(field),
			    n_vars, n_vars+1,
			    n_rows+1, n_rows+2);
			label_in_box_bg(field, table->active_row == row ?
			    COLOR_ROW_SELECTED : COLOR_ROW_UNSELECTED);
			g_signal_connect(G_OBJECT(box_of_label(field)),
			    "button_press_event",
			    G_CALLBACK(table_value_select_event), value);
			value->widget = field;
			n_vars++;
		}
		n_rows++;
	}
}


/* ----- loops ------------------------------------------------------------- */


static gboolean loop_var_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	struct loop *loop = data;

	edit_var(&loop->var);
	return TRUE;
}


static gboolean loop_from_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	struct loop *loop = data;

	edit_value(&loop->from);
	return TRUE;
}


static gboolean loop_to_select_event(GtkWidget *widget,
     GdkEventButton *event, gpointer data)
{
	struct loop *loop = data;

	edit_value(&loop->to);
	return TRUE;
}


static gboolean loop_select_event(GtkWidget *widget, GdkEventButton *event,
     gpointer data)
{
	struct loop *loop = data;

	loop->active = (long) gtk_object_get_data(GTK_OBJECT(widget), "value");
	change_world();
	return TRUE;
}


static void build_loop(GtkWidget *vbox, struct frame *frame,
     struct loop *loop)
{
	GtkWidget *hbox, *field, *label;
	char *expr;
	int i;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	field = label_in_box_new(loop->var.name);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(field), FALSE, FALSE, 0);
	label_in_box_bg(field, COLOR_VAR_PASSIVE);
	g_signal_connect(G_OBJECT(box_of_label(field)),
	    "button_press_event",
	    G_CALLBACK(loop_var_select_event), loop);
	loop->var.widget = field;

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(" = "),
	    FALSE, FALSE, 0);

	expr = unparse(loop->from.expr);
	field = label_in_box_new(expr);
	free(expr);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(field), FALSE, FALSE, 0);
	label_in_box_bg(field, COLOR_EXPR_PASSIVE);
	g_signal_connect(G_OBJECT(box_of_label(field)),
	    "button_press_event",
	    G_CALLBACK(loop_from_select_event), loop);
	loop->from.widget = field;

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(" ... "),
	    FALSE, FALSE, 0);

	expr = unparse(loop->to.expr);
	field = label_in_box_new(expr);
	free(expr);
	gtk_box_pack_start(GTK_BOX(hbox), box_of_label(field), FALSE, FALSE, 0);
	label_in_box_bg(field, COLOR_EXPR_PASSIVE);
	g_signal_connect(G_OBJECT(box_of_label(field)),
	    "button_press_event",
	    G_CALLBACK(loop_to_select_event), loop);
	loop->to.widget = field;

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(" ("),
	    FALSE, FALSE, 0);

	for (i = 0; i != loop->iterations; i++) {
		label = add_activator(hbox, loop->active == i,
		    loop_select_event, loop, "%d", i);
		gtk_object_set_data(GTK_OBJECT(box_of_label(label)), "value",
		    (gpointer) (long) i);
	}

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(")"),
	    FALSE, FALSE, 0);
}


/* ----- the list of variables, tables, and loops -------------------------- */


static GtkWidget *build_vars(struct frame *frame)
{
	GtkWidget *vbox;
	struct table *table;
	struct loop *loop;

	vbox= gtk_vbox_new(FALSE, 0);
	for (table = frame->tables; table; table = table->next) {
		add_sep(vbox, 3);
		build_assignment(vbox, frame, table);
		build_table(vbox, frame, table);
	}
	for (loop = frame->loops; loop; loop = loop->next) {
		add_sep(vbox, 3);
		build_loop(vbox, frame, loop);
	}
	return vbox;
}


/* ----- frame labels ------------------------------------------------------ */


static int validate_frame_name(const char *s, void *ctx)
{
	struct frame *f;

	if (!is_id(s))
		return 0;
	for (f = frames; f; f = f->next)
		if (f->name && !strcmp(f->name, s))
			return 0;
	return 1;
}


static void unselect_frame(void *data)
{
	struct frame *frame= data;

	/*
	 * "unselect" means in this context that the selection has moved
	 * elsewhere. However, this does not necessarily change the frame.
	 * (And, in fact, since we rebuild the frame list anyway, the color
	 * change here doesn't matter if selecting a different frame.)
	 * So we revert from "editing" to "selected".
	 */
        label_in_box_bg(frame->label, COLOR_FRAME_SELECTED);
}


static void edit_frame(struct frame *frame)
{
	inst_select_outside(frame, unselect_frame);
	label_in_box_bg(frame->label, COLOR_FRAME_EDITING);
	status_set_name(frame->name);
	edit_unique(&frame->name, validate_frame_name, frame);
}


static void select_frame(struct frame *frame)
{
	if (active_frame)
		label_in_box_bg(active_frame->label, COLOR_FRAME_UNSELECTED);
	active_frame = frame;
	change_world();
}


static gboolean frame_select_event(GtkWidget *widget, GdkEventButton *event,
     gpointer data)
{
	if (active_frame != data)
		select_frame(data);
	else {
		if (active_frame->name)
			edit_frame(data);
	}
	return TRUE;
}


static GtkWidget *build_frame_label(struct frame *frame)
{
	GtkWidget *label;

	label = label_in_box_new(frame->name ? frame->name : "(root)");
	gtk_misc_set_padding(GTK_MISC(label), 2, 2);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);

	label_in_box_bg(label, active_frame == frame ?
	    COLOR_FRAME_SELECTED : COLOR_FRAME_UNSELECTED);

	g_signal_connect(G_OBJECT(box_of_label(label)),
	    "button_press_event", G_CALLBACK(frame_select_event), frame);
	frame->label = label;

	return box_of_label(label);
}


/* ----- frame references -------------------------------------------------- */


static gboolean frame_ref_select_event(GtkWidget *widget, GdkEventButton *event,
     gpointer data)
{
	struct obj *obj = data;

	obj->u.frame.ref->active_ref = data;
	change_world();
	return TRUE;
}


static GtkWidget *build_frame_refs(const struct frame *frame)
{
	GtkWidget *hbox;
	struct obj *obj;

	hbox = gtk_hbox_new(FALSE, 0);
	for (obj = frame->objs; obj; obj = obj->next)
		if (obj->type == ot_frame && obj->u.frame.ref == active_frame)
			add_activator(hbox,
			    obj == obj->u.frame.ref->active_ref,
			    frame_ref_select_event, obj,
			    "%d", obj->u.frame.lineno);
	return hbox;
}


/* ----- frames ------------------------------------------------------------ */


static void build_frames(GtkWidget *vbox)
{
	struct frame *frame;
	GtkWidget *tab, *label, *refs, *vars;
	int n;

	destroy_all_children(GTK_CONTAINER(vbox));
	for (frame = frames; frame; frame = frame->next)
		n++;

	tab = gtk_table_new(n*2, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tab), 1);
	gtk_table_set_col_spacings(GTK_TABLE(tab), 1);
	gtk_box_pack_start(GTK_BOX(vbox), tab, FALSE, FALSE, 0);

	n = 0;
	for (frame = root_frame; frame; frame = frame->prev) {
		label = build_frame_label(frame);
		gtk_table_attach_defaults(GTK_TABLE(tab), label,
                    0, 1, n*2, n*2+1);
		refs = build_frame_refs(frame);
		gtk_table_attach_defaults(GTK_TABLE(tab), refs,
                    1, 2, n*2, n*2+1);
		vars = build_vars(frame);
		gtk_table_attach_defaults(GTK_TABLE(tab), vars,
                    1, 2, n*2+1, n*2+2);
		n++;
	}
	gtk_widget_show_all(tab);
}


/* ----- central screen area ----------------------------------------------- */


static void make_center_area(GtkWidget *vbox)
{
	GtkWidget *hbox, *frames_area, *paned;
	GtkWidget *icons;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	paned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(hbox), paned, TRUE, TRUE, 0);
	
	/* Frames */

	frames_area = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add1(GTK_PANED(paned), frames_area);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frames_area),
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(frames_area, 250, 100);

	frames_box = gtk_vbox_new(FALSE, 0);
	build_frames(frames_box);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(frames_area),
	    frames_box);

	/* Canvas */

	gtk_paned_add2(GTK_PANED(paned), make_canvas());

	/* Icon bar */

	icons = gui_setup_icons(root->window);
	gtk_box_pack_end(GTK_BOX(hbox), icons, FALSE, FALSE, 0);
}


/* ----- GUI construction -------------------------------------------------- */


void change_world(void)
{
	inst_deselect();
	status_begin_reporting();
	instantiate();
	label_in_box_bg(active_frame->label, COLOR_FRAME_SELECTED);
	build_frames(frames_box);
	redraw();
}


static void make_screen(GtkWidget *window)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	make_menu_bar(vbox);
	make_center_area(vbox);
	make_status_area(vbox);
}


int gui_init(int *argc, char ***argv)
{
	gtk_init(argc, argv);
	return 0;
}


int gui_main(int argc, char **argv)
{
	root = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(root), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(root), 600, 400);
	gtk_window_set_title(GTK_WINDOW(root), "fped");

	/* get root->window */
	gtk_widget_show_all(root);

	g_signal_connect_swapped(G_OBJECT(root), "destroy",
	    G_CALLBACK(gtk_main_quit), NULL);

	make_screen(root);

	gtk_widget_show_all(root);

	gui_setup_style(root->window);
	init_canvas();
	edit_nothing();
	select_frame(root_frame);

	gtk_main();

	return 0;
}
