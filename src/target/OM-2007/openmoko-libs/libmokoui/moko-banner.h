/*  moko-banner.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2007/04/23 22:27:36 $) [$Author: mickey $]
 */

#ifndef _MOKO_BANNER_H_
#define _MOKO_BANNER_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_BANNER moko_banner_get_type()
#define MOKO_BANNER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_BANNER, MokoBanner))
#define MOKO_BANNER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_BANNER, MokoBannerClass))
#define MOKO_IS_BANNER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_BANNER))
#define MOKO_IS_BANNER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_BANNER))
#define MOKO_BANNER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_BANNER, MokoBannerClass))

typedef struct {
    GObject parent;
} MokoBanner;

typedef struct {
    GObjectClass parent_class;
} MokoBannerClass;

GType moko_banner_get_type(void);
MokoBanner* moko_banner_get_instance(void);

void moko_banner_show_text(MokoBanner* self, const gchar* text, gint timeout);
void moko_banner_hide(MokoBanner* self);


G_END_DECLS

#endif // _MOKO_BANNER_H_

