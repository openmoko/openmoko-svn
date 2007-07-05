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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "koto-field-editor-factory.h"
#include "koto-task-editor.h"
#include "koto-task-editor-dialog.h"

G_DEFINE_TYPE (KotoTaskEditorDialog, koto_task_editor_dialog, GTK_TYPE_DIALOG);

/* TODO: make a TaskEditorIface? */

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_TASK_EDITOR_DIALOG, KotoTaskEditorDialogPrivate))

enum {
  PROP_0,
  PROP_TASK,
  PROP_DIRTY,
  PROP_GROUPS,
  PROP_CAL,
};

typedef struct {
  GtkWidget *editor;
  ECal *cal;
  KotoTask *task;
  GtkWidget *desc, *desc_label;
  gboolean desc_dirty;
} KotoTaskEditorDialogPrivate;

/* TODO: probably best to implement this as normal button callbacks and emit
   response ourself. */
static void
response_cb (GtkDialog *dialog, int response)
{
  KotoTaskEditorDialogPrivate *priv = GET_PRIVATE (dialog);
  GError *error = NULL;
  GtkWidget *confirm;

  if (!priv->task) {
    return;
  }
  
  switch (response) {
  case KOTO_TASK_EDITOR_DIALOG_RESPONSE_DELETE:
    confirm = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                     _("Are you sure you want to delete \"%s\"?"),
                                     icalcomponent_get_summary (priv->task->comp));
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (confirm),
                                              _("If you delete an item, it is permanently lost."));
    gtk_dialog_add_buttons (GTK_DIALOG (confirm),
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_DELETE, GTK_RESPONSE_ACCEPT,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (confirm), GTK_RESPONSE_ACCEPT);
    
    if (gtk_dialog_run (GTK_DIALOG (confirm)) == GTK_RESPONSE_ACCEPT) {
      if (!e_cal_remove_object (priv->cal, icalcomponent_get_uid (priv->task->comp), &error)) {
        g_warning ("Cannot remove object: %s", error->message);
        g_error_free (error);
      }
    } else {
      /* Stop the response signal from getting anywhere. */
      g_signal_stop_emission_by_name (dialog, "response");
    }
    
    gtk_widget_destroy (confirm);
  default:
    if (koto_task_editor_dialog_is_dirty (KOTO_TASK_EDITOR_DIALOG (dialog))) {
      if (!e_cal_modify_object (priv->cal, priv->task->comp, CALOBJ_MOD_THIS, &error)) {
        g_warning ("Cannot modify object: %s", error->message);
        g_error_free (error);
      }
    }
  }
}

static void
koto_task_editor_dialog_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoTaskEditorDialog *editor = KOTO_TASK_EDITOR_DIALOG (object);
  KotoTaskEditorDialogPrivate *priv = GET_PRIVATE (editor);
  
  switch (property_id) {
  case PROP_TASK:
    g_value_set_boxed (value, priv->task);
    break;
  case PROP_DIRTY:
    g_value_set_boolean (value, koto_task_editor_dialog_is_dirty (editor));
    break;
  case PROP_GROUPS:
    {
      KotoGroupStore *groups;
      g_object_get (priv->editor, "groups", &groups, NULL);
      g_value_set_object (value, groups);
    }
    break;
  case PROP_CAL:
    g_value_set_object (value, priv->cal);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_editor_dialog_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  KotoTaskEditorDialog *dialog = KOTO_TASK_EDITOR_DIALOG (object);
  KotoTaskEditorDialogPrivate *priv = GET_PRIVATE (dialog);

  switch (property_id) {
  case PROP_TASK:
    koto_task_editor_dialog_set_task (dialog, g_value_get_boxed (value));
    break;
  case PROP_GROUPS:
    g_object_set (priv->editor, "groups", g_value_get_object (value), NULL);
    break;
  case PROP_CAL:
    if (priv->cal)
      g_object_unref (priv->cal);
    priv->cal = E_CAL (g_value_dup_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_editor_dialog_dispose (GObject *object)
{
  KotoTaskEditorDialogPrivate *priv = GET_PRIVATE (object);
  
  if (priv->cal) {
    g_object_unref (priv->cal);
    priv->cal = NULL;
  }
  
  if (priv->task) {
    koto_task_unref (priv->task);
    priv->task = NULL;
  }

  if (G_OBJECT_CLASS (koto_task_editor_dialog_parent_class)->dispose)
    G_OBJECT_CLASS (koto_task_editor_dialog_parent_class)->dispose (object);
}

static void
koto_task_editor_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (koto_task_editor_dialog_parent_class)->finalize (object);
}

static void
koto_task_editor_dialog_class_init (KotoTaskEditorDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoTaskEditorDialogPrivate));

  object_class->get_property = koto_task_editor_dialog_get_property;
  object_class->set_property = koto_task_editor_dialog_set_property;
  object_class->dispose = koto_task_editor_dialog_dispose;
  object_class->finalize = koto_task_editor_dialog_finalize;

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

  g_object_class_install_property (object_class, PROP_GROUPS,
                                   g_param_spec_object ("groups", "groups", NULL,
                                                        KOTO_TYPE_GROUP_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));

  g_object_class_install_property (object_class, PROP_CAL,
                                   g_param_spec_object ("cal", "cal", NULL,
                                                        E_TYPE_CAL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
}

static void
koto_task_editor_dialog_init (KotoTaskEditorDialog *self)
{
  KotoTaskEditorDialogPrivate *priv = GET_PRIVATE (self);
  GtkWidget *notebook;

  g_object_set (self,
                "title", "Tasks",
                "has-separator", FALSE,
                NULL);

  /* Make the window a bit wider by default */
  gtk_window_set_default_size (GTK_WINDOW (self), 300, -1);

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (self)->vbox), notebook);

  priv->editor = koto_task_editor_new ();
  koto_task_editor_add_fields (KOTO_TASK_EDITOR (priv->editor),
                               ICAL_SUMMARY_PROPERTY,
                               ICAL_CATEGORIES_PROPERTY,
                               ICAL_PRIORITY_PROPERTY,
                               ICAL_DUE_PROPERTY,
                               ICAL_URL_PROPERTY,
                               NULL);

  gtk_container_set_border_width (GTK_CONTAINER (priv->editor), 6);
  gtk_widget_show (priv->editor);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            priv->editor,
                            gtk_label_new_with_mnemonic (_("Det_ails")));

  priv->desc = koto_field_editor_create (ICAL_DESCRIPTION_PROPERTY, &priv->desc_dirty);
  gtk_container_set_border_width (GTK_CONTAINER (priv->desc), 6);
  gtk_widget_show (priv->desc);
  priv->desc_label = gtk_label_new_with_mnemonic ("");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            priv->desc, priv->desc_label);


  gtk_dialog_add_buttons (GTK_DIALOG (self),
                          GTK_STOCK_DELETE, KOTO_TASK_EDITOR_DIALOG_RESPONSE_DELETE,
                          GTK_STOCK_CLOSE, KOTO_TASK_EDITOR_DIALOG_RESPONSE_CLOSE,
                          NULL);

  g_signal_connect (self, "response", G_CALLBACK (response_cb), NULL);
}

GtkWidget *
koto_task_editor_dialog_new (void)
{
  return g_object_new (KOTO_TYPE_TASK_EDITOR_DIALOG, NULL);
}

gboolean
koto_task_editor_dialog_is_dirty (KotoTaskEditorDialog *dialog)
{
  KotoTaskEditorDialogPrivate *priv;

  g_return_val_if_fail (KOTO_IS_TASK_EDITOR_DIALOG (dialog), FALSE);

  priv = GET_PRIVATE (dialog);

  return koto_task_editor_is_dirty (KOTO_TASK_EDITOR (priv->editor)) | priv->desc_dirty;
}

void
koto_task_editor_dialog_set_task (KotoTaskEditorDialog *dialog, KotoTask *task)
{
  KotoTaskEditorDialogPrivate *priv;
  char *title;

  g_return_if_fail (KOTO_IS_TASK_EDITOR_DIALOG (dialog));
  g_return_if_fail (task != NULL);

  priv = GET_PRIVATE (dialog);

  if (priv->task)
    koto_task_unref (priv->task);
  priv->task = koto_task_ref (task);

  g_object_set (priv->editor, "task", priv->task, NULL);
  
  koto_field_editor_set (priv->desc, task);
  if (icalcomponent_get_description (priv->task->comp)) {
    gtk_label_set_markup (GTK_LABEL (priv->desc_label), _("<b>_Notes</b>"));
  } else {
    gtk_label_set_text (GTK_LABEL (priv->desc_label), _("_Notes"));
  }
  gtk_label_set_use_underline (GTK_LABEL (priv->desc_label), TRUE);
  
  title = g_strdup_printf (_("%s - Tasks"), icalcomponent_get_summary (task->comp));
  gtk_window_set_title (GTK_WINDOW (dialog), title);
  g_free (title);
}

void
koto_task_editor_dialog_set_group_store (KotoTaskEditorDialog *dialog, KotoGroupStore *store)
{
  g_return_if_fail (KOTO_IS_TASK_EDITOR_DIALOG (dialog));

  g_object_set (dialog, "groups", store, NULL);
}


G_GNUC_DEPRECATED void
koto_task_editor_dialog_run (KotoTaskEditorDialog *dialog, ECal *cal)
{
  g_return_if_fail (KOTO_IS_TASK_EDITOR_DIALOG (dialog));
  g_return_if_fail (E_IS_CAL (cal));

  g_object_set (dialog, "cal", cal, NULL);

  gtk_dialog_run (GTK_DIALOG (dialog));
}
