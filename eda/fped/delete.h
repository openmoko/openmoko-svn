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


int delete_vec(struct vec *vec);
int delete_obj(struct obj *obj);
int destroy(void);
int undelete(void);

#endif /* !OBJ_H */
