/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libical/ical.h>

#include "koto-task.h"
#include "koto-field-editor-factory.h"
#include "koto-group-combo.h"
#include "koto-group-store.h"
#include "koto-task-editor.h"

G_DEFINE_TYPE (KotoTaskEditor, koto_task_editor, GTK_TYPE_TABLE);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_TASK_EDITOR, KotoTaskEditorPrivate))

typedef struct {
  KotoGroupStore *group_store;
  KotoTask *task;
  gboolean dirty;
} KotoTaskEditorPrivate;

enum {
  PROP_0,
  PROP_GROUPS,
  PROP_TASK,
  PROP_DIRTY,
  PROP_LAST,
};

typedef struct {
  const char *label;
  icalproperty_kind type;
} FieldDetails;

static const FieldDetails fields[] = {
  { N_("_Summary:"), ICAL_SUMMARY_PROPERTY },
  { N_("Ca_tegory:"), ICAL_CATEGORIES_PROPERTY },
  { N_("_Priority:"), ICAL_PRIORITY_PROPERTY },
  { N_("D_ue Date:"), ICAL_DUE_PROPERTY },
  { N_("_Web Address:"), ICAL_URL_PROPERTY },
  { N_("D_escription:"), ICAL_DESCRIPTION_PROPERTY },
};

/*
 * Called for every widget when a new icalcomponent is set.  If this widget has
 * an associated DisplayField set, then updates it with the value of the
 * relevant property.
 */
static void
update_field (GtkWidget *widget, KotoTaskEditor *editor)
{
  KotoTaskEditorPrivate *priv = GET_PRIVATE (editor);

  koto_field_editor_set (widget, priv->task);
}

/*
 * Called for every widget when a new KotoGroupStore is set.  If the widget is a
 * KotoGroupCombo, set the model property to the new store.
 */
static void
update_group_store (GtkWidget *widget, KotoTaskEditor *editor)
{
  KotoTaskEditorPrivate *priv = GET_PRIVATE (editor);

  if (KOTO_IS_GROUP_COMBO (widget)) {
    g_object_set (widget, "model", priv->group_store, NULL);
  }
}

/*
 * GObject methods.
 */

static void
koto_task_editor_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoTaskEditorPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_GROUPS:
    g_value_set_object (value, priv->group_store);
    break;
  case PROP_TASK:
    g_value_set_boxed (value, priv->task);
    break;
  case PROP_DIRTY:
    g_value_set_boolean (value, priv->dirty);
    break;
  default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_editor_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  KotoTaskEditorPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_GROUPS:
    if (priv->group_store) {
      g_object_unref (priv->group_store);
      priv->group_store = NULL;
    }
    priv->group_store = KOTO_GROUP_STORE (g_value_dup_object (value));
    gtk_container_foreach (GTK_CONTAINER (object), (GtkCallback)update_group_store, object);
    break;
  case PROP_TASK:
    priv->task = g_value_dup_boxed (value);
    priv->dirty = FALSE;
    /* Now iterate over every widget, updating any fields with values from this icalcomponent. */
    gtk_container_foreach (GTK_CONTAINER (object), (GtkCallback)update_field, object);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_editor_dispose (GObject *object)
{
  KotoTaskEditorPrivate *priv = GET_PRIVATE (object);
  
  if (priv->task) {
    koto_task_unref (priv->task);
    priv->task = NULL;
  }

  if (G_OBJECT_CLASS (koto_task_editor_parent_class)->dispose)
    G_OBJECT_CLASS (koto_task_editor_parent_class)->dispose (object);
}

static void
koto_task_editor_class_init (KotoTaskEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoTaskEditorPrivate));

  object_class->get_property = koto_task_editor_get_property;
  object_class->set_property = koto_task_editor_set_property;
  object_class->dispose = koto_task_editor_dispose;

  g_object_class_install_property (object_class, PROP_GROUPS,
                                   g_param_spec_object ("groups", "groups", NULL,
                                                        KOTO_TYPE_GROUP_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
  g_object_class_install_property (object_class, PROP_TASK,
                                   g_param_spec_boxed ("task", "task", NULL,
                                                       KOTO_TYPE_TASK,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
  g_object_class_install_property (object_class, PROP_DIRTY,
                                   g_param_spec_boolean ("dirty", "dirty", NULL,
                                                         FALSE,
                                                         G_PARAM_READABLE |
                                                         G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
}

static void
koto_task_editor_init (KotoTaskEditor *self)
{
  g_object_set (self,
                "homogeneous", FALSE,
                "row-spacing", 6,
                "column-spacing", 6,
                NULL);
}


/*
 * Public methods.
 */

GtkWidget *
koto_task_editor_new (void)
{
  return g_object_new (KOTO_TYPE_TASK_EDITOR, NULL);
}

gboolean
koto_task_editor_is_dirty (KotoTaskEditor *editor)
{
  KotoTaskEditorPrivate *priv;

  g_return_val_if_fail (KOTO_IS_TASK_EDITOR (editor), FALSE);

  priv = GET_PRIVATE (editor);

  return priv->dirty;
}

void
koto_task_editor_add_field (KotoTaskEditor *editor, icalproperty_kind kind)
{
  KotoTaskEditorPrivate *priv;
  int i;

  g_return_if_fail (KOTO_IS_TASK_EDITOR (editor));
  g_return_if_fail (kind);

  priv = GET_PRIVATE (editor);
 
  /* TODO: sort fields and use bsearch */
  for (i = 0; i < G_N_ELEMENTS (fields); i++) {
    GtkWidget *label, *widget;
    
    if (fields[i].type != kind)
      continue;

    label = gtk_label_new_with_mnemonic (_(fields[i].label));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (editor), label, 0, 1, i+1, i+2, GTK_FILL, GTK_FILL, 0, 0);

    widget = koto_field_editor_create (fields[i].type, &priv->dirty);
    gtk_widget_show (widget);

    if (GTK_IS_SCROLLED_WINDOW (widget)) {
      gtk_label_set_mnemonic_widget (GTK_LABEL (label), gtk_bin_get_child (GTK_BIN (widget)));
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
      gtk_table_attach_defaults (GTK_TABLE (editor), widget, 1, 2, i+1, i+2);
    } else {
      gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
      gtk_table_attach (GTK_TABLE (editor), widget, 1, 2, i+1, i+2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
    }
    return;
  }

  g_warning ("Could not find field for property %d", kind);
}

G_GNUC_NULL_TERMINATED void
koto_task_editor_add_fields (KotoTaskEditor *editor, ...)
{
  va_list args;
  icalproperty_kind kind;

  va_start (args, editor);

  while ((kind = va_arg (args, icalproperty_kind)) != 0) {
    koto_task_editor_add_field (editor, kind);
  }

  va_end (args);
}

