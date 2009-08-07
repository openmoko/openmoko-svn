/*
 * gui_status.h - GUI, status area
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
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


void edit_unique(const char **s, int (*validate)(const char *s, void *ctx),
    void *ctx, int focus);
void edit_unique_null(const char **s, int (*validate)(const char *s, void *ctx),
    void *ctx, int focus);
void edit_name(char **s, int (*validate)(const char *s, void *ctx), void *ctx,
    int focus);
void edit_expr(struct expr **expr, int focus);
void edit_x(struct expr **expr);
void edit_y(struct expr **expr);
void edit_nothing(void);

void status_set_type_x(const char *fmt, ...);
void status_set_type_y(const char *fmt, ...);
void status_set_type_entry(const char *fmt, ...);
void status_set_name(const char *fmt, ...);
void status_set_x(const char *fmt, ...);
void status_set_y(const char *fmt, ...);
void status_set_r(const char *fmt, ...);
void status_set_angle(const char *fmt, ...);
void status_set_sys_x(const char *fmt, ...);
void status_set_sys_y(const char *fmt, ...);
void status_set_user_x(const char *fmt, ...);
void status_set_user_y(const char *fmt, ...);
void status_set_zoom(const char *fmt, ...);
void status_set_grid(const char *fmt, ...);

void status_set_xy(struct coord coord);

void status_begin_reporting(void);

void make_status_area(GtkWidget *vbox) ;

#endif /* !GUI_STATUS_H */
