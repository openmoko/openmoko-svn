/*
 * gui_frame_drag.c - GUI, dragging of frame items
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <gtk/gtk.h>

#include "obj.h"
#include "gui_util.h"
#include "gui.h"
#include "gui_canvas.h"
#include "gui_frame_drag.h"

#if 0
#include "icons/frame.xpm"
#endif


enum {
	target_id_var,
	target_id_value,
	target_id_frame,
	target_id_canvas,
};


static GtkTargetEntry target_var = {
	.target = "var",
	.flags  = GTK_TARGET_SAME_APP,
	.info	= target_id_var,
};

static GtkTargetEntry target_value = {
	.target = "value",
	.flags  = GTK_TARGET_SAME_APP,
	.info	= target_id_value,
};

static GtkTargetEntry target_frame = {
	.target = "frame",
	.flags  = GTK_TARGET_SAME_APP,
	.info	= target_id_frame,
};


/* ----- dragging status --------------------------------------------------- */


/*
 * Pointer to whatever it is we're dragging. NULL if not dragging.
 */

static void *dragging;


int is_dragging(void *this)
{
	return this == dragging;
}


int is_dragging_anything(void)
{
	return !!dragging;
}


/* ----- helper functions for indexed list and swapping -------------------- */


#define NDX(first, item)					\
	({ typeof(first) NDX_walk;				\
	   int NDX_n = 0;					\
	   for (NDX_walk = (first); NDX_walk != (item);		\
	       NDX_walk = NDX_walk->next)			\
		NDX_n++;					\
	   NDX_n; })

#define	NTH(first, n) 						\
	({ typeof(first) *NTH_walk;				\
	   int NTH_n = (n);					\
	   for (NTH_walk = &(first); NTH_n; NTH_n--)		\
		NTH_walk = &(*NTH_walk)->next;			\
	   NTH_walk; })

#define	SWAP(a, b)						\
	({ typeof(a) SWAP_tmp;					\
	   SWAP_tmp = a;					\
	   a = b;						\
	   b = SWAP_tmp; })


/* ----- generic helper functions. maybe move to gui_util later ------------ */


static void get_cell_coords(GtkWidget *widget, guint res[4])
{
	GtkWidget *tab;

	tab = gtk_widget_get_ancestor(widget, GTK_TYPE_TABLE);
	gtk_container_child_get(GTK_CONTAINER(tab), widget,
	    "left-attach", res,
	    "right-attach", res+1,
	    "top-attach", res+2,
	    "bottom-attach", res+3, NULL);
}


static void swap_table_cells(GtkWidget *a, GtkWidget *b)
{
	GtkWidget *tab_a, *tab_b;
	guint pos_a[4], pos_b[4];

	tab_a = gtk_widget_get_ancestor(a, GTK_TYPE_TABLE);
	tab_b = gtk_widget_get_ancestor(b, GTK_TYPE_TABLE);
	get_cell_coords(a, pos_a);
	get_cell_coords(b, pos_b);
	g_object_ref(a);
	g_object_ref(b);
	gtk_container_remove(GTK_CONTAINER(tab_a), a);
	gtk_container_remove(GTK_CONTAINER(tab_b), b);
	gtk_table_attach_defaults(GTK_TABLE(tab_a), b,
	    pos_a[0], pos_a[1], pos_a[2], pos_a[3]);
	gtk_table_attach_defaults(GTK_TABLE(tab_b), a,
	    pos_b[0], pos_b[1], pos_b[2], pos_b[3]);
	g_object_unref(a);
	g_object_unref(b);
}


/* ----- swap table items -------------------------------------------------- */


static void swap_vars(struct table *table, int a, int b)
{
	struct var **var_a, **var_b;

	var_a = NTH(table->vars, a);
	var_b = NTH(table->vars, b);

	swap_table_cells(box_of_label((*var_a)->widget),
	    box_of_label((*var_b)->widget));

	SWAP(*var_a, *var_b);
	SWAP((*var_a)->next, (*var_b)->next);
}


static void swap_values(struct row *row, int a, int b)
{
	struct value **value_a, **value_b;

	value_a = NTH(row->values, a);
	value_b = NTH(row->values, b);

	swap_table_cells(box_of_label((*value_a)->widget),
	    box_of_label((*value_b)->widget));

	SWAP(*value_a, *value_b);
	SWAP((*value_a)->next, (*value_b)->next);
}



static void swap_cols(struct table *table, int a, int b)
{
	struct row *row;

	swap_vars(table, a, b);
	for (row = table->rows; row; row = row->next)
		swap_values(row, a, b);
}


static void swap_rows(struct row **a, struct row **b)
{
	struct value *value_a, *value_b;

	value_a = (*a)->values;
	value_b = (*b)->values;
	while (value_a) {
		swap_table_cells(box_of_label(value_a->widget),
		    box_of_label(value_b->widget));
		value_a = value_a->next;
		value_b = value_b->next;
	}
	SWAP(*a, *b);
	SWAP((*a)->next, (*b)->next);
}


/* ----- common functions -------------------------------------------------- */


/*
 * according to
 * http://www.pubbs.net/201004/gtk/22819-re-drag-and-drop-drag-motion-cursor-lockup-fixed-.html
 * http://www.cryingwolf.org/articles/gtk-dnd.html
 */

static int has_target(GtkWidget *widget, GdkDragContext *drag_context,
    const char *name)
{
	GdkAtom target;

	target = gtk_drag_dest_find_target(widget, drag_context, NULL);

	/*
	 * Force allocation so that we don't have to check for GDK_NONE.
	 */
	return target == gdk_atom_intern(name, FALSE);
}


static void drag_begin(GtkWidget *widget,
    GtkTextDirection previous_direction, gpointer user_data)
{
	GdkPixbuf *pixbuf;

	/*
	 * Suppress the icon. PixBufs can't be zero-sized, but nobody will
	 * notice a lone pixel either :-)
	 */
	pixbuf =
	    gdk_pixbuf_get_from_drawable(NULL, DA, NULL, 0, 0, 0, 0, 1, 1);
	gtk_drag_source_set_icon_pixbuf(widget, pixbuf);
	g_object_unref(pixbuf);

	dragging = user_data;
}


static void drag_end(GtkWidget *widget, GdkDragContext *drag_context,
    gpointer user_data)
{
	dragging = NULL;
}


static void setup_drag_common(GtkWidget *widget, void *user_arg)
{
	g_signal_connect(G_OBJECT(widget), "drag-begin",
	    G_CALLBACK(drag_begin), user_arg);
	g_signal_connect(G_OBJECT(widget), "drag-end",
	    G_CALLBACK(drag_end), user_arg);
}


/* ----- drag variables ---------------------------------------------------- */


static gboolean drag_var_motion(GtkWidget *widget,
    GdkDragContext *drag_context, gint x, gint y, guint time_,
    gpointer user_data)
{
	struct var *from = dragging;
	struct var *to = user_data;
	int from_n, to_n, i;

	if (!has_target(widget, drag_context, "var"))
		return FALSE;
	if (from == to || from->table != to->table)
		return FALSE;
	from_n = NDX(from->table->vars, from);
	to_n = NDX(to->table->vars, to);
	for (i = from_n < to_n ? from_n : to_n;
	    i != (from_n < to_n ? to_n : from_n); i++)
		swap_cols(from->table, i, i+1);
	return FALSE;
}


void setup_var_drag(struct var *var)
{
	GtkWidget *box;

	box = box_of_label(var->widget);
	gtk_drag_source_set(box, GDK_BUTTON1_MASK,
	    &target_var, 1, GDK_ACTION_PRIVATE);
	gtk_drag_dest_set(box, GTK_DEST_DEFAULT_MOTION,
	    &target_var, 1, GDK_ACTION_PRIVATE);
	setup_drag_common(box, var);
	g_signal_connect(G_OBJECT(box), "drag-motion",
	    G_CALLBACK(drag_var_motion), var);
}


/* ----- drag values ------------------------------------------------------- */


static gboolean drag_value_motion(GtkWidget *widget,
    GdkDragContext *drag_context, gint x, gint y, guint time_,
    gpointer user_data)
{
	struct value *from = dragging;
	struct value *to = user_data;
	struct table *table;
	struct row **row, *end;
	int from_n, to_n, i;

	if (!has_target(widget, drag_context, "value"))
		return FALSE;
	table = from->row->table;
	if (table != to->row->table)
		return FALSE;

	/* columns */

	from_n = NDX(from->row->values, from);
	to_n = NDX(to->row->values, to);
	for (i = from_n < to_n ? from_n : to_n;
	    i != (from_n < to_n ? to_n : from_n); i++)
		swap_cols(table, i, i+1);

	/* rows */

	if (from->row == to->row)
		return FALSE;
	row = &table->rows;
	while (1) {
		if (*row == from->row) {
			end = to->row;
			break;
		}
		if (*row == to->row) {
			end = from->row;
			break;
		}
		row = &(*row)->next;
	}
	while (1) {
		swap_rows(row, &(*row)->next);
		if (*row == end)
			break;
		row = &(*row)->next;
	}

	return FALSE;
}


void setup_value_drag(struct value *value)
{
	GtkWidget *box;

	box = box_of_label(value->widget);
	gtk_drag_source_set(box, GDK_BUTTON1_MASK,
	    &target_value, 1, GDK_ACTION_PRIVATE);
	gtk_drag_dest_set(box, GTK_DEST_DEFAULT_MOTION,
	    &target_value, 1, GDK_ACTION_PRIVATE);
	setup_drag_common(box, value);
	g_signal_connect(G_OBJECT(box), "drag-motion",
	    G_CALLBACK(drag_value_motion), value);
}


/* ----- frame to canvas helper functions ---------------------------------- */


static int frame_on_canvas = 0;


static void leave_canvas(void)
{
	if (frame_on_canvas)
		canvas_frame_end();
	frame_on_canvas = 0;
}


/* ----- drag frame labels ------------------------------------------------- */


#if 0

/*
 * Setting our own icon looks nice but it slows things down to the point where
 * cursor movements can lag noticeable and it adds yet another element to an
 * already crowded cursor.
 */

static void drag_frame_begin(GtkWidget *widget,
    GtkTextDirection previous_direction, gpointer user_data)
{
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	GdkColormap *cmap;

	pixmap = gdk_pixmap_create_from_xpm_d(DA, &mask, NULL, xpm_frame);
	cmap = gdk_drawable_get_colormap(root->window);
	gtk_drag_source_set_icon(widget, cmap, pixmap, mask);
	g_object_unref(pixmap);
	g_object_unref(mask);

	dragging = user_data;
}

#endif


static gboolean drag_frame_motion(GtkWidget *widget,
    GdkDragContext *drag_context, gint x, gint y, guint time_,
    gpointer user_data)
{
	if (!has_target(widget, drag_context, "frame"))
		return FALSE;
	/* nothing else to do yet */
	return FALSE;
}


static void drag_frame_end(GtkWidget *widget, GdkDragContext *drag_context,
    gpointer user_data)
{
	leave_canvas();
	drag_end(widget, drag_context, user_data);
}


void setup_frame_drag(struct frame *frame)
{
	GtkWidget *box;

	box = box_of_label(frame->label);
	gtk_drag_source_set(box, GDK_BUTTON1_MASK,
	    &target_frame, 1, GDK_ACTION_COPY);
	setup_drag_common(box, frame);

	/* override */
#if 0
	g_signal_connect(G_OBJECT(box), "drag-begin",
	    G_CALLBACK(drag_frame_begin), frame);
#endif
	g_signal_connect(G_OBJECT(box), "drag-end",
	    G_CALLBACK(drag_frame_end), frame);

	g_signal_connect(G_OBJECT(box), "drag-motion",
	    G_CALLBACK(drag_frame_motion), frame);
}


/* ----- drag to the canvas ------------------------------------------------ */


static gboolean drag_canvas_motion(GtkWidget *widget,
    GdkDragContext *drag_context, gint x, gint y, guint time_,
    gpointer user_data)
{
	if (!has_target(widget, drag_context, "frame"))
		return FALSE;
gtk_drag_finish(drag_context, FALSE, FALSE, time_);
return FALSE;
	if (!frame_on_canvas) {
		frame_on_canvas = 1;
		canvas_frame_begin(dragging);
	}
	if (canvas_frame_motion(dragging, x, y)) {
		gdk_drag_status(drag_context, GDK_ACTION_COPY, time_);
		return TRUE;
	} else {
		gdk_drag_status(drag_context, 0, time_);
		return FALSE;
	}
}


static void drag_canvas_leave(GtkWidget *widget, GdkDragContext *drag_context,
    guint time_, gpointer user_data)
{
	leave_canvas();
}


static gboolean drag_canvas_drop(GtkWidget *widget,
    GdkDragContext *drag_context, gint x, gint y, guint time_,
    gpointer user_data)
{
	if (!has_target(widget, drag_context, "frame"))
		return FALSE;
	if (!canvas_frame_drop(dragging, x, y))
		return FALSE;
	gtk_drag_finish(drag_context, TRUE, FALSE, time_);
	return TRUE;
}


void setup_canvas_drag(GtkWidget *canvas)
{
	gtk_drag_dest_set(canvas,
	    GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
	    &target_frame, 1, GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(canvas), "drag-motion",
	    G_CALLBACK(drag_canvas_motion), NULL);
	g_signal_connect(G_OBJECT(canvas), "drag-leave",
	    G_CALLBACK(drag_canvas_leave), NULL);
	g_signal_connect(G_OBJECT(canvas), "drag-drop",
	    G_CALLBACK(drag_canvas_drop), NULL);
}
