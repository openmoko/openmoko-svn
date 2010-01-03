/*
 * gui_status.h - GUI, status area
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_STATUS_H
#define GUI_STATUS_H

#include <gtk/gtk.h>

#include "coord.h"
#include "expr.h"


enum curr_unit {
	curr_unit_mm,
	curr_unit_mil,
	curr_unit_auto,
	curr_unit_n
};


extern enum curr_unit curr_unit;


void edit_pad_type(enum pad_type *type);

void edit_unique(const char **s, int (*validate)(const char *s, void *ctx),
    void *ctx, const char *tooltip);
void edit_unique_null(const char **s, int (*validate)(const char *s, void *ctx),
    void *ctx, const char *tooltip);
void edit_unique_with_values(const char **s,
    int (*validate)(const char *s, void *ctx), void *ctx,
    void (*set_values)(void *user, const struct value *values, int n_values),
    void *user, int max_values, const char *tooltip);
void edit_name(char **s, int (*validate)(const char *s, void *ctx), void *ctx,
    const char *tooltip);
void edit_expr(struct expr **expr, const char *tooltip);
void edit_expr_list(struct expr *expr,
    void (*set_values)(void *user, const struct value *values, int n_values),
    void *user, const char *tooltip);
void edit_x(struct expr **expr, const char *tooltip);
void edit_y(struct expr **expr, const char *tooltip);
void edit_nothing(void);

void set_with_units(void (*set)(const char *tooltip, const char *fmt, ...),
    const char *prefix, unit_type u, const char *tooltip);

#define SETTER(name)							\
	void status_set_##name(const char *tooltip, const char *fmt, ...) \
	   __attribute__((format(printf, 2, 3)))			\

SETTER(type_x);
SETTER(type_y);
SETTER(type_entry);
SETTER(name);
SETTER(x);
SETTER(y);
SETTER(r);
SETTER(angle);
SETTER(sys_x);
SETTER(sys_y);
SETTER(user_x);
SETTER(user_y);
SETTER(zoom);
SETTER(grid);
SETTER(unit);

#undef SETTER

void status_set_icon(GtkWidget *image);
void status_set_xy(struct coord coord);
void status_set_angle_xy(const char *tooltip, struct coord v);

void status_begin_reporting(void);

void make_status_area(GtkWidget *vbox);
void cleanup_status_area(void);

#endif /* !GUI_STATUS_H */
