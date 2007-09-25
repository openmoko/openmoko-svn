/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef _MOKO_SEARCH_BAR
#define _MOKO_SEARCH_BAR

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_SEARCH_BAR            moko_search_bar_get_type()
#define MOKO_SEARCH_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_SEARCH_BAR, MokoSearchBar))
#define MOKO_SEARCH_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOKO_TYPE_SEARCH_BAR, MokoSearchBarClass))
#define MOKO_IS_SEARCH_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_SEARCH_BAR))
#define MOKO_IS_SEARCH_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOKO_TYPE_SEARCH_BAR))
#define MOKO_SEARCH_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOKO_TYPE_SEARCH_BAR, MokoSearchBarClass))

/**
 * MokoSearchBar:
 *
 * MokoSearchBar has no publicly accessible fields
 */
typedef struct _MokoSearchBar MokoSearchBar;
typedef struct _MokoSearchBarClass MokoSearchBarClass;

struct _MokoSearchBar {
	GtkHBox parent;
};

struct _MokoSearchBarClass {
	GtkHBoxClass parent_class;

	/* Signals */
	void	(*toggled)		(MokoSearchBar *self,
					 gboolean search_visible);
	void	(*text_changed)		(MokoSearchBar *self,
					 GtkEditable *editable);
	void	(*combo_changed)	(MokoSearchBar *self,
					 GtkComboBox *combo_box);
};

GType moko_search_bar_get_type (void);

GtkWidget *	moko_search_bar_new		();
GtkWidget *	moko_search_bar_new_with_combo	(GtkComboBox *combo);
GtkComboBox *	moko_search_bar_get_combo_box	(MokoSearchBar *self);
GtkEntry *	moko_search_bar_get_entry	(MokoSearchBar *self);

gboolean	moko_search_bar_search_visible	(MokoSearchBar *self);
void		moko_search_bar_toggle		(MokoSearchBar *self);

G_END_DECLS

#endif /* _MOKO_SEARCH_BAR */

