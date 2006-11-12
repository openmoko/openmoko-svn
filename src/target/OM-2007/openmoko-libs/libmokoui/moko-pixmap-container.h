
/*  moko_pixmap_container.h
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

#ifndef _MOKO_PIXMAP_CONTAINER_H_
#define _MOKO_PIXMAP_CONTAINER_H_

#include <gtk/gtkfixed.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_PIXMAP_CONTAINER moko_pixmap_container_get_type()

#define MOKO_PIXMAP_CONTAINER(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   MOKO_TYPE_PIXMAP_CONTAINER, MokoPixmapContainer))

#define MOKO_PIXMAP_CONTAINER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   MOKO_TYPE_PIXMAP_CONTAINER, MokoPixmapContainerClass))

#define MOKO_IS_PIXMAP_CONTAINER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   MOKO_TYPE_PIXMAP_CONTAINER))

#define MOKO_IS_PIXMAP_CONTAINER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   MOKO_TYPE_PIXMAP_CONTAINER))

#define MOKO_PIXMAP_CONTAINER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   MOKO_TYPE_PIXMAP_CONTAINER, MokoPixmapContainerClass))

typedef struct {
  GtkFixed parent;
} MokoPixmapContainer;

typedef struct {
  GtkFixedClass parent_class;
} MokoPixmapContainerClass;

GType moko_pixmap_container_get_type (void);

MokoPixmapContainer* moko_pixmap_container_new (void);

void moko_pixmap_container_set_cargo(MokoPixmapContainer* self, GtkWidget* child);

G_END_DECLS

#endif // _MOKO_PIXMAP_CONTAINER_H_

