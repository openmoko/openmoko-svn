
/*  moko_fixed.h
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#ifndef _MOKO_FIXED_H_
#define _MOKO_FIXED_H_

#include <gtk/gtkfixed.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_FIXED moko_fixed_get_type()

#define MOKO_FIXED(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   MOKO_TYPE_FIXED, MokoFixed))

#define MOKO_FIXED_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   MOKO_TYPE_FIXED, MokoFixedClass))

#define MOKO_IS_FIXED(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   MOKO_TYPE_FIXED))

#define MOKO_IS_FIXED_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   MOKO_TYPE_FIXED))

#define MOKO_FIXED_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   MOKO_TYPE_FIXED, MokoFixedClass))

typedef struct {
  GtkFixed parent;
} MokoFixed;

typedef struct {
  GtkFixedClass parent_class;
} MokoFixedClass;

GType moko_fixed_get_type (void);

GtkWidget* moko_fixed_new (void);

void moko_fixed_set_cargo(MokoFixed* self, GtkWidget* child);

G_END_DECLS

#endif // _MOKO_FIXED_H_

