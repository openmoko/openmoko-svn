/*
 * delete.h - Object deletion
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef DELETE_H
#define DELETE_H


#include "obj.h"


void delete_vec(struct vec *vec);
void delete_obj(struct obj *obj);
void delete_row(struct row *row);
void delete_column(struct table *table, int n);
void delete_table(struct table *table);
void delete_loop(struct loop *loop);
void delete_frame(struct frame *frame);
int destroy(void);
int undelete(void);

#endif /* !DELETE_H */
