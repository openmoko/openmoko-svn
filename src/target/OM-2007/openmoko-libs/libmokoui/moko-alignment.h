/*  moko-alignment.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#ifndef _MOKO_ALIGNMENT_H_
#define _MOKO_ALIGNMENT_H_

#include <gtk/gtkalignment.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_ALIGNMENT moko_alignment_get_type()
#define MOKO_ALIGNMENT(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_ALIGNMENT, MokoAlignment))
#define MOKO_ALIGNMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_ALIGNMENT, MokoAlignmentClass))
#define MOKO_IS_ALIGNMENT(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_ALIGNMENT))
#define MOKO_IS_ALIGNMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_ALIGNMENT))
#define MOKO_ALIGNMENT_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_ALIGNMENT, MokoAlignmentClass))

typedef struct {
    GtkAlignment parent;
} MokoAlignment;

typedef struct {
    GtkAlignmentClass parent_class;
} MokoAlignmentClass;

GType moko_alignment_get_type (void);

GtkWidget* moko_alignment_new (void);

G_END_DECLS

#endif // _MOKO_ALIGNMENT_H_

