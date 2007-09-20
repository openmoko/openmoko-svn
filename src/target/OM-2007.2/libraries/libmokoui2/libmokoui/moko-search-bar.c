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
	PROP_COMBO = 1,
	PROP_ENTRY,
};

enum {
	TOGGLED,
	TEXT_CHANGED,
	COMBO_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };


static void
combo_changed_cb (GtkComboBox *combo, MokoSearchBar *self)
{
	g_signal_emit (self, signals[COMBO_CHANGED], 0, combo);
}

static void
moko_search_bar_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (object);

	switch (property_id) {
	    case PROP_COMBO :
		g_value_set_object (value, priv->combo);
		break;
	    case PROP_ENTRY :
		g_value_set_object (value, priv->entry);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_search_bar_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (object);

	switch (property_id) {
	    case PROP_COMBO :
		priv->combo = g_value_get_object (value);
		gtk_box_pack_start (GTK_BOX (object), priv->combo,
			TRUE, TRUE, 0);
		gtk_widget_show (priv->combo);

		g_signal_connect (G_OBJECT (priv->combo), "changed",
			G_CALLBACK (combo_changed_cb), object);
		break;
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

	g_object_class_install_property (
		object_class,
		PROP_COMBO,
		g_param_spec_object (
			"combo",
			"GtkComboBox *",
			"The GtkComboBox to place inside the widget.",
			GTK_TYPE_COMBO_BOX,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (
		object_class,
		PROP_ENTRY,
		g_param_spec_object (
			"entry",
			"GtkEntry *",
			"The GtkEntry created for this widget.",
			GTK_TYPE_ENTRY,
			G_PARAM_READABLE));

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

	signals[COMBO_CHANGED] =
		g_signal_new ("combo-changed",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (MokoSearchBarClass, combo_changed),
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

	/* Connect up signals */
	g_signal_connect (G_OBJECT (priv->toggle), "toggled",
		G_CALLBACK (toggled_cb), self);
	g_signal_connect (G_OBJECT (priv->entry), "changed",
		G_CALLBACK (entry_changed_cb), self);
}

/**
 * moko_search_bar_new:
 *
 * Create a new MokoSearBar widget
 *
 * returns: The newly created MokoSearchBar
 */

GtkWidget *
moko_search_bar_new (void)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_SEARCH_BAR,
		"combo", gtk_combo_box_new (), NULL));
}

/**
 * moko_search_bar_new_with_combo:
 * @comno: a GtkComboBox to use as the combobox
 *
 * Creates a MokoSearchBar with the specified combo box
 *
 * returns: the newly created MokoSearchBar
 */

GtkWidget *
moko_search_bar_new_with_combo (GtkComboBox *combo)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_SEARCH_BAR,
		"combo", combo, NULL));
}

/**
 * moko_search_bar_get_combo_box:
 * @self: a MokoSearchBar
 *
 * Get a pointer to the GtkComboBox being used in the MokoSearchBar
 *
 * returns: the GtkComboBox
 */
GtkComboBox *
moko_search_bar_get_combo_box (MokoSearchBar *self)
{
	GtkComboBox *combo;
	g_object_get (G_OBJECT (self), "combo", &combo, NULL);
	return combo;
}


/**
 * moko_search_bar_get_entry:
 * @self: a MokoSearchBar
 *
 * Retrieve the GtkEntry widget being used in the MokoSearchBar
 *
 * returns: the GtkEntry
 */
GtkEntry *
moko_search_bar_get_entry (MokoSearchBar *self)
{
	GtkEntry *entry;
	g_object_get (G_OBJECT (self), "entry", &entry, NULL);
	return entry;
}

/**
 * moko_search_bar_search_visible:
 * @self: a MokoSaerchBar
 *
 * Determine the visibility of the search entry.
 *
 * returns: TRUE if the search entry is visible
 */
gboolean
moko_search_bar_search_visible (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);
	
	return GTK_WIDGET_VISIBLE (priv->entry);
}


/**
 * moko_search_bar_toggle:
 * @self: a MokoSearchBar
 *
 * Toggle the search button on the MokoSearchBar.
 * This toggles the visibility of the combo box and entry widgets.
 */
void
moko_search_bar_toggle (MokoSearchBar *self)
{
	MokoSearchBarPrivate *priv = SEARCH_BAR_PRIVATE (self);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->toggle),
		!gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (priv->toggle)));
}


