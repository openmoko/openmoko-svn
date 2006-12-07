
/*  moko_search_bar.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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

#ifndef _MOKO_SEARCH_BAR_H_
#define _MOKO_SEARCH_BAR_H_

#include <glib-object.h>
#include <gtk/gtktoolbar.h>

G_BEGIN_DECLS

#define MOKO_TYPE_SEARCH_BAR (moko_search_bar_get_type())

#define MOKO_SEARCH_BAR(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   MOKO_TYPE_SEARCH_BAR, MokoSearchBar))

#define MOKO_SEARCH_BAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   MOKO_TYPE_SEARCH_BAR, MokoSearchBarClass))

#define MOKO_IS_SEARCH_BAR(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   MOKO_TYPE_SEARCH_BAR))

#define MOKO_IS_SEARCH_BAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   MOKO_TYPE_SEARCH_BAR))

#define MOKO_SEARCH_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   MOKO_TYPE_SEARCH_BAR, MokoSearchBarClass))

typedef struct {
  GtkToolbar parent;
} MokoSearchBar;

typedef struct {
  GtkToolbarClass parent_class;
} MokoSearchBarClass;

GType moko_search_bar_get_type (void);

MokoSearchBar* moko_search_bar_new (void);

G_END_DECLS

#endif // _MOKO_SEARCH_BAR_H_

