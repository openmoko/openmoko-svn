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

#include "moko-search-bar.h"

G_DEFINE_TYPE (MokoSearchBar, moko_search_bar, GTK_TYPE_HBOX)

#define SEARCH_BAR_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_SEARCH_BAR, \
	 MokoSearchBarPrivate))

typedef struct _MokoSearchBarPrivate MokoSearchBarPrivate;

struct _MokoSearchBarPrivate
{
	GtkWidget *toggle;
	GtkWidget *entry;
	GtkWidget *combo;
};

enum {
	TOGGLED,
	TEXT_CHANGED,
	CATEGORY_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
moko_search_bar_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_search_bar_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_search_bar_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (moko_search_bar_parent_class)->dispose)
		G_OBJECT_CLASS (moko_search_bar_parent_class)->dispose (object);
}

static void
moko_search_bar_finalize (GObject *object)
{
	G_OBJECT_CLASS (moko_search_bar_parent_class)->finalize (object);
}

static void
moko_search_bar_class_init (MokoSearchBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoSearchBarPrivate));

	object_class->get_property = moko_search_bar_get_property;
	object_class->set_property = moko_search_bar_set_property;
	object_class->dispose = moko_search_bar_dispose;
	object_class->finalize = moko_search_bar_finalize;

	signals[TOGGLED] =
		g_signal_new ("toggled",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (MokoSearchBarClass, toggled),
			NULL, NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	signals[TEXT_CHANGED] =
		g_signal_new ("text-changed",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (MokoSearchBarClass, text_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[CATEGORY_CHANGED] =
		g_signal_new ("category-changed",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (MokoSearchBarClass, category_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__OBJECT,
			G_TYPE_NONE, 1, GTK_TYPE_COMBO_BOX);
}

static void
toggled_cb (GtkWidget *button, MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	g_object_set (G_OBJECT (priv->entry), "visible",
		!GTK_WIDGET_VISIBLE (priv->entry), NULL);
	g_object_set (G_OBJECT (priv->combo), "visible",
		!GTK_WIDGET_VISIBLE (priv->combo), NULL);

	if (GTK_WIDGET_VISIBLE (priv->entry))
		gtk_widget_grab_focus (priv->entry);

	g_signal_emit (self, signals[TOGGLED], 0,
		GTK_WIDGET_VISIBLE (priv->entry));
}

static void
entry_changed_cb (GtkEditable *editable, MokoSearchBar *self)
{
	g_signal_emit (self, signals[TEXT_CHANGED], 0, editable);
}

static void
combo_changed_cb (GtkComboBox *combo, MokoSearchBar *self)
{
	g_signal_emit (self, signals[CATEGORY_CHANGED], 0, combo);
}

static void
moko_search_bar_init (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);
	
	/* Create toggle button */
	priv->toggle = gtk_toggle_button_new ();
	gtk_widget_set_name (priv->toggle, "mokosearchbutton");
	gtk_button_set_image (GTK_BUTTON (priv->toggle),
		gtk_image_new_from_stock (GTK_STOCK_FIND,
			GTK_ICON_SIZE_SMALL_TOOLBAR));
	gtk_box_pack_start (GTK_BOX (self), priv->toggle, FALSE, FALSE, 0);
	gtk_widget_show_all (priv->toggle);
	
	/* Create entry */
	priv->entry = gtk_entry_new ();
	gtk_widget_set_name (priv->entry, "mokosearchentry");
	g_object_set (G_OBJECT (priv->entry), "no-show-all", TRUE, NULL);
	gtk_box_pack_start (GTK_BOX (self), priv->entry, TRUE, TRUE, 0);

	priv->combo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (self), priv->combo, TRUE, TRUE, 0);
	gtk_widget_show (priv->combo);

	/* Connect up signals */
	g_signal_connect (G_OBJECT (priv->toggle), "toggled",
		G_CALLBACK (toggled_cb), self);
	g_signal_connect (G_OBJECT (priv->entry), "changed",
		G_CALLBACK (entry_changed_cb), self);
	g_signal_connect (G_OBJECT (priv->combo), "changed",
		G_CALLBACK (combo_changed_cb), self);
}

GtkWidget *
moko_search_bar_new (void)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_SEARCH_BAR, NULL));
}

gboolean
moko_search_bar_search_visible (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);
	
	return GTK_WIDGET_VISIBLE (priv->entry);
}

void
moko_search_bar_toggle (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->toggle),
		!gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (priv->toggle)));
}

void
moko_search_bar_append_category (MokoSearchBar *self, const gchar *text)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo), text);
}

void
moko_search_bar_insert_category (MokoSearchBar *self, gint position,
				 const gchar *text)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_combo_box_insert_text (GTK_COMBO_BOX (priv->combo), position, text);
}

void
moko_search_bar_prepend_category (MokoSearchBar *self, const gchar *text)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_combo_box_prepend_text (GTK_COMBO_BOX (priv->combo), text);
}

void
moko_search_bar_remove_category (MokoSearchBar *self, gint position)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_combo_box_remove_text (GTK_COMBO_BOX (priv->combo), position);
}

gint
moko_search_bar_count_categories (MokoSearchBar *self)
{
	gint rows = 0;
	GtkTreeIter iter;
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	GtkTreeModel *model = gtk_combo_box_get_model (
		GTK_COMBO_BOX (priv->combo));
	
	if (gtk_tree_model_get_iter_first (model, &iter))
		do { rows ++; } while (gtk_tree_model_iter_next (model, &iter));
	
	return rows;
}

void
moko_search_bar_clear_categories (MokoSearchBar *self)
{
	gint i;
	
	for (i = moko_search_bar_count_categories (self); i > 0; i--) {
		moko_search_bar_remove_category (self, 0);
	}
}

gint
moko_search_bar_get_active (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	return gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo));
}

gchar *
moko_search_bar_get_active_category (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	return gtk_combo_box_get_active_text (GTK_COMBO_BOX (priv->combo));
}

void
moko_search_bar_set_active (MokoSearchBar *self, gint position)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo), position);
}

const gchar *
moko_search_bar_get_text (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	return gtk_entry_get_text (GTK_ENTRY (priv->entry));
}

void
moko_search_bar_set_text (MokoSearchBar *self, const gchar *text)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_entry_set_text (GTK_ENTRY (priv->entry), text);
}

