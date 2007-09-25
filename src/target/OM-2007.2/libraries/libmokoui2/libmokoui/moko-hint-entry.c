/*
 * moko-hint-history.c - taken from koto-hint-history.c
 *   <http://svn.o-hand.com/repos/tasks/trunk/libkoto/koto-hint-entry.c>
 *
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Relicensed with permission from GPL version 2 to LGPL version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include "moko-hint-entry.h"

G_DEFINE_TYPE (MokoHintEntry, moko_hint_entry, GTK_TYPE_ENTRY)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_HINT_ENTRY, MokoHintEntryPrivate))

typedef enum {
  STATE_HINTING,
  STATE_ENTRY,
} EntryState;

typedef struct {
  EntryState state;
  char *hint;
} MokoHintEntryPrivate;

/*
 * Private methods.
 */

static void
update (MokoHintEntry *entry)
{
  MokoHintEntryPrivate *priv;
  GtkWidget *widget;
  const char *text;

  g_assert (MOKO_IS_HINT_ENTRY (entry));

  priv = GET_PRIVATE (entry);
  widget = GTK_WIDGET (entry);

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (GTK_WIDGET_HAS_FOCUS (entry)) {
    if (priv->state == STATE_HINTING) {
      priv->state = STATE_ENTRY;
      gtk_widget_modify_text (widget, GTK_STATE_NORMAL, NULL);
      gtk_entry_set_text (GTK_ENTRY (entry), "");
    }
  } else {
    if (text == NULL || text[0] == '\0') {
      priv->state = STATE_HINTING;
      gtk_entry_set_text (GTK_ENTRY (entry), priv->hint);
      gtk_widget_modify_text (widget, GTK_STATE_NORMAL,
                              &gtk_widget_get_style (widget)->text[GTK_STATE_INSENSITIVE]);
    } else {
      priv->state = STATE_ENTRY;
    }
  }
}

/*
 * Object methods.
 */

enum {
  PROP_0,
  PROP_HINT,
};

static gboolean
focus_in_event (GtkWidget *widget, GdkEventFocus *event)
{
  update (MOKO_HINT_ENTRY (widget));

  GTK_WIDGET_CLASS (moko_hint_entry_parent_class)->focus_in_event (widget, event);

  return FALSE;
}

static gboolean
focus_out_event (GtkWidget *widget, GdkEventFocus *event)
{
  update (MOKO_HINT_ENTRY (widget));

  GTK_WIDGET_CLASS (moko_hint_entry_parent_class)->focus_out_event (widget, event);

  return FALSE;
}

static void
moko_hint_entry_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  MokoHintEntryPrivate *priv = GET_PRIVATE (object);
  
  switch (property_id) {
  case PROP_HINT:
    g_value_set_string (value, priv->hint);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_hint_entry_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  MokoHintEntryPrivate *priv = GET_PRIVATE (object);
  
  switch (property_id) {
  case PROP_HINT:
    if (priv->hint) {
      g_free (priv->hint);
    }
    priv->hint = g_value_dup_string (value);
    update (MOKO_HINT_ENTRY (object));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_hint_entry_finalize (GObject *object)
{
  MokoHintEntryPrivate *priv = GET_PRIVATE (object);

  g_free (priv->hint);

  if (G_OBJECT_CLASS (moko_hint_entry_parent_class)->finalize)
    G_OBJECT_CLASS (moko_hint_entry_parent_class)->finalize (object);
}


static void
moko_hint_entry_class_init (MokoHintEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoHintEntryPrivate));

  object_class->set_property = moko_hint_entry_set_property;
  object_class->get_property = moko_hint_entry_get_property;
  object_class->finalize = moko_hint_entry_finalize;

  widget_class->focus_in_event = focus_in_event;
  widget_class->focus_out_event = focus_out_event;
  
  g_object_class_install_property (object_class, PROP_HINT, g_param_spec_string
                                   ("hint", "hint", "Text to display when the entry is empty", "",
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                                    G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void
moko_hint_entry_init (MokoHintEntry *self)
{
}


/*
 * Public methods.
 */

/**
 * moko_hint_entry_new:
 * @hint: The hint to display when the widget is not focused
 *
 * Create a new #MokoHintEntry widget with the specified hint.
 *
 * Returns: a newly created #MokoHintEntry
 */
GtkWidget *
moko_hint_entry_new (const char *hint)
{
  return g_object_new (MOKO_TYPE_HINT_ENTRY,
                       "hint", hint,
                       NULL);
}

/**
 * moko_hint_entry_clear:
 * @entry: a #MokoHintEntry
 *
 * Clear the text in the entry and if the widget is not focused, display the
 * hint text.
 */
void
moko_hint_entry_clear (MokoHintEntry *entry)
{
  g_return_if_fail (MOKO_IS_HINT_ENTRY (entry));
  
  gtk_entry_set_text (GTK_ENTRY (entry), "");

  update (entry);
}

/**
 * moko_hint_entry_is_empty:
 * @entry: a #MokoHintyEntry
 *
 * Check if the entry has a user entered value. 
 *
 * returns: TRUE if the hint is currently displayed. FALSE if the user has
 * entered a value
 */
gboolean
moko_hint_entry_is_empty (MokoHintEntry *entry)
{
  MokoHintEntryPrivate *priv;

  g_return_val_if_fail (MOKO_IS_HINT_ENTRY (entry), TRUE);
  
  priv = GET_PRIVATE (entry);

  if (priv->state == STATE_HINTING)
    return TRUE;

  return strlen (gtk_entry_get_text (GTK_ENTRY (entry))) == 0;
}
