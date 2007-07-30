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
#include <libical/ical.h>

#ifdef HAVE_SEXY
#include <libsexy/sexy-icon-entry.h>
#endif

#include "ical-util.h"
#include "koto-task.h"
#include "koto-group.h"
#include "koto-category-group.h"
#include "koto-no-category-group.h"
#include "koto-meta-group.h"
#include "koto-date-combo.h"
#include "koto-group-combo.h"
#include "koto-priority-combo.h"
#include "koto-platform.h"

/* FieldData pointer */
#define FIELD_DATA "KotoFieldEditor:Field"

/* TODO: move to field data */
#define SIGNAL_DATA "KotoFieldEditor:Signal"

/* Function that creates a widget for a particular field */
typedef GtkWidget* (*EditorNew) (void);
/* Function that sets the widget value for a particular field/property */
typedef void (*EditorSetFunc) (GtkWidget *editor, icalproperty *prop);

/*
 * Struct defining a field to display.
 */
typedef struct {
  icalproperty_kind kind;
  EditorNew editor_new;
  EditorSetFunc set_value;
} DisplayField;

typedef struct {
  const DisplayField *field;
  gboolean *dirty;
  KotoTask *task;
  icalproperty *prop;
} FieldData;

/*
 * Plain text single-line entry fields.
 */

static void
on_entry_changed (GtkEntry *entry)
{
  FieldData *data;
  const char *text;
  
  data = g_object_get_data (G_OBJECT (entry), FIELD_DATA);

  if (!data) {
    g_warning (FIELD_DATA " property not set");
    return;
  }

  if (data->prop == NULL) {
    data->prop = icalproperty_new (data->field->kind);
    icalcomponent_add_property (data->task->comp, data->prop);
  }

  text = gtk_entry_get_text (entry);

  /* NO means use the default value type. Ugly as sin. */
  icalproperty_set_value_from_string (data->prop, text, "NO");

  *data->dirty = TRUE;

  return;
}

static GtkWidget *
entry_new (void)
{
  GtkWidget *widget;
  guint id;
  
  widget = gtk_entry_new ();
  id = g_signal_connect (widget, "changed", G_CALLBACK (on_entry_changed), NULL);
  g_object_set_data (G_OBJECT (widget), SIGNAL_DATA, GINT_TO_POINTER (id));
  return widget;
}

static void
entry_set (GtkWidget *widget, icalproperty *prop)
{
  const char *text = NULL;
  guint id;

  id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), SIGNAL_DATA));

  if (prop) {
    text = icalvalue_get_text (icalproperty_get_value (prop));
  }

  g_signal_handler_block (widget, id);
  gtk_entry_set_text (GTK_ENTRY (widget), text ?: "");
  g_signal_handler_unblock (widget, id);
}

/*
 * URL entries.
 */

#ifdef HAVE_SEXY
static void
url_entry_icon_clicked (SexyIconEntry *entry, gint icon_pos, gint button, gpointer user_data)
{
  const char *text;
  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text && text[0] != '\0') {
    koto_platform_open_url (text);
  }
}

static void
on_icon_entry_changed (SexyIconEntry *entry)
{
  const char *text;
  
  text = gtk_entry_get_text (GTK_ENTRY (entry));

  sexy_icon_entry_set_icon_highlight (entry,
                                      SEXY_ICON_ENTRY_SECONDARY,
                                      text != NULL && text[0] != '\0');
}
#endif

static GtkWidget *
url_entry_new (void)
{
  GtkWidget *entry;
  guint id;

#ifdef HAVE_SEXY
  GtkWidget *image;

  entry = sexy_icon_entry_new ();
  image = gtk_image_new_from_icon_name ("stock_internet", GTK_ICON_SIZE_MENU);

  sexy_icon_entry_set_icon (SEXY_ICON_ENTRY (entry),
                            SEXY_ICON_ENTRY_SECONDARY, GTK_IMAGE (image));

  g_signal_connect (entry, "icon-released", G_CALLBACK (url_entry_icon_clicked), NULL);
#else
  entry = gtk_entry_new ();
#endif

  /* This changed callback updates the property */
  id = g_signal_connect (entry, "changed", G_CALLBACK (on_entry_changed), NULL);
  g_object_set_data (G_OBJECT (entry), SIGNAL_DATA, GINT_TO_POINTER (id));

#ifdef HAVE_SEXY
  /* This callback enables or disables the icon highlighting */
  g_signal_connect (entry, "changed", G_CALLBACK (on_icon_entry_changed), NULL);
#endif

  return entry;
}

/*
 * Priority combo
 */

static void
on_priority_changed (KotoPriorityCombo *combo)
{
  FieldData *data;
  int priority;

  data = g_object_get_data (G_OBJECT (combo), FIELD_DATA);

  if (!data) {
    g_warning (FIELD_DATA " property not set");
    return;
  }

  if (data->prop == NULL) {
    data->prop = icalproperty_new (ICAL_PRIORITY_PROPERTY);
    icalcomponent_add_property (data->task->comp, data->prop);
  }

  priority = koto_priority_combo_get_priority (combo);
  if (priority != PRIORITY_NONE) {
    icalproperty_set_priority (data->prop, priority);
  } else {
    icalcomponent_remove_property (data->task->comp, data->prop);
    icalproperty_free (data->prop);
    data->prop = NULL;
  }

  *data->dirty = TRUE;
}

static GtkWidget *
priority_new (void)
{
  GtkWidget *widget;
  guint id;

  widget = koto_priority_combo_new ();
  id = g_signal_connect (widget, "changed", G_CALLBACK (on_priority_changed), NULL);
  g_object_set_data (G_OBJECT (widget), SIGNAL_DATA, GINT_TO_POINTER (id));
  return widget;
}

static void
priority_set (GtkWidget *widget, icalproperty *prop)
{
  int priority = 0;
  guint id;

  id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), SIGNAL_DATA));

  if (prop) {
    priority = icalproperty_get_priority (prop);
  }
  
  g_signal_handler_block (widget, id);
  koto_priority_combo_set_priority (KOTO_PRIORITY_COMBO (widget), priority);
  g_signal_handler_unblock (widget, id);
}

/*
 * Summary editor.
 */

static void
on_text_changed (GtkTextBuffer *buffer)
{
  FieldData *data;
  GtkWidget *widget;
  GtkTextIter start, end;
  const char *text;

  widget = g_object_get_data (G_OBJECT (buffer), "koto-real-widget");
  if (!widget) {
    g_warning ("Cannot find view for buffer");
    return;
  }

  data = g_object_get_data (G_OBJECT (widget), FIELD_DATA);

  if (!data) {
    g_warning (FIELD_DATA " property not set");
    return;
  }

  if (data->prop == NULL) {
    data->prop = icalproperty_new (data->field->kind);
    icalcomponent_add_property (data->task->comp, data->prop);
  }

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  /* NO means use the default value type. Ugly as sin. */
  icalproperty_set_value_from_string (data->prop, text, "NO");
  
  *data->dirty = TRUE;

  return;
}

static GtkWidget *
text_new (void)
{
  GtkWidget *scrolled, *textview;
  GtkTextBuffer *buffer;
  guint id;
  
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_IN);

  textview = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview), FALSE);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
  g_object_set_data (G_OBJECT (buffer), "koto-real-widget", scrolled);
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (scrolled), textview);

  id = g_signal_connect (buffer, "changed", G_CALLBACK (on_text_changed), NULL);
  g_object_set_data (G_OBJECT (scrolled), SIGNAL_DATA, GINT_TO_POINTER (id));
  
  return scrolled;
}

static void
text_set (GtkWidget *widget, icalproperty *prop)
{
  const char *text = NULL;
  char *s;
  GtkWidget *textview;
  GtkTextBuffer *buffer;
  guint id;

  id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), SIGNAL_DATA));

  textview = gtk_bin_get_child (GTK_BIN (widget));
  if (!GTK_IS_TEXT_VIEW (textview)) {
    g_warning ("Child widget was not GtkTextView");
    return;
  }
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

  if (prop) {
    text = icalproperty_get_value_as_string (prop);
  }

  g_signal_handler_block (buffer, id);
  if (text) {
    s = g_strcompress (text);
    gtk_text_buffer_set_text (buffer, s, -1);
    g_free (s);
  } else {
    gtk_text_buffer_set_text (buffer, "", -1);
  }
  g_signal_handler_unblock (buffer, id);
}

/*
 * Category widget
 */

static void
category_set (GtkWidget *widget, icalproperty *prop)
{
  char **categories;
  GtkTreeIter iter;
  KotoGroupStore *store;
  guint id;

  id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), SIGNAL_DATA));

  g_signal_handler_block (widget, id);
  if (prop) {
    categories = g_strsplit (icalproperty_get_categories (prop), ",", 0);
    store = KOTO_GROUP_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (widget)));
    koto_group_store_get_iter_for_group (store, categories[0], &iter);
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (widget), &iter);
    g_strfreev (categories);
  } else {
    gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
  }
  g_signal_handler_unblock (widget, id);
}

char *
run_new_category_dialog (GtkWidget *widget)
{
  GtkWidget *parent, *dialog, *box, *label, *entry;
  char *name = NULL;

  /* Magically get the top-level window */
  parent = gtk_widget_get_toplevel (widget);
  parent = GTK_WIDGET_TOPLEVEL (parent) ? parent : NULL;
  
  dialog = gtk_dialog_new_with_buttons (NULL, GTK_WINDOW (parent),
                                        GTK_DIALOG_NO_SEPARATOR,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  box = GTK_DIALOG (dialog)->vbox;
  gtk_box_set_spacing (GTK_BOX (box), 8);
  
  label = gtk_label_new (_("Enter the name of the new category:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  
  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  
  gtk_widget_show_all (dialog);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
    name = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
  }

  gtk_widget_destroy (dialog);

  return name;
}

static void
on_category_changed (KotoGroupCombo *combo)
{
  FieldData *data;
  KotoGroup *group;
  GType type;

  data = g_object_get_data (G_OBJECT (combo), FIELD_DATA);

  if (!data) {
    g_warning (FIELD_DATA " property not set");
    return;
  }

  group = koto_group_combo_get_active_group (combo);
  type = G_OBJECT_TYPE (group);
  
  if (g_type_is_a (type, KOTO_TYPE_CATEGORY_GROUP)) {
    /* Create the property if it didn't exist */
    if (data->prop == NULL) {
      data->prop = icalproperty_new (ICAL_CATEGORIES_PROPERTY);
      icalcomponent_add_property (data->task->comp, data->prop);
    }
    /* Now set its value */
    icalproperty_set_categories (data->prop, koto_group_get_name (group));
  }
  
  else if (g_type_is_a (type, KOTO_TYPE_META_GROUP)) {
    KotoMetaGroupKind kind;
    kind = koto_meta_group_get_kind (KOTO_META_GROUP (group));

    switch (kind) {
    case KOTO_META_GROUP_NONE:
      icalcomponent_remove_property (data->task->comp, data->prop);
      icalproperty_free (data->prop);
      data->prop = NULL;
      break;
    case KOTO_META_GROUP_NEW: {
      char *name;
      KotoGroupStore *store;
      GtkTreeIter iter;

      name = run_new_category_dialog (GTK_WIDGET (combo));
      
      if (name) {
        store = KOTO_GROUP_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)));
        koto_group_store_add_new_category (store, &iter, name);
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
        g_free (name);
      } else {
        /* Revert the group */
        category_set (GTK_WIDGET (combo), data->prop);
      }
      break;
    }
    case KOTO_META_GROUP_SEPERATOR:
      /* Do nothing */
      break;
    }
  } else {
    g_warning ("Unhandled group type %s", g_type_name (type));
  }
  
  g_object_unref (group);

  *data->dirty = TRUE;
}

static GtkWidget *
category_new (void)
{
  GtkWidget *widget;
  guint id;

  widget = koto_group_combo_new (NULL);
  id = g_signal_connect (widget, "changed", G_CALLBACK (on_category_changed), NULL);
  g_object_set_data (G_OBJECT (widget), SIGNAL_DATA, GINT_TO_POINTER (id));
  return widget;
}

/*
 * Due widget
 */

static void
on_date_notify (KotoDateCombo *combo, GParamSpec *spec)
{
  FieldData *data;
  GDate *date;
  icaltimetype due;

  data = g_object_get_data (G_OBJECT (combo), FIELD_DATA);

  if (!data) {
    g_warning (FIELD_DATA " property not set");
    return;
  }

  if (data->prop == NULL) {
    data->prop = icalproperty_new (ICAL_DUE_PROPERTY);
    icalcomponent_add_property (data->task->comp, data->prop);
  }

  date = koto_date_combo_get_date (combo);
  if (g_date_valid (date)) {
    /* This may be the fastest way from GDate to icaltimetype */
    due = icaltime_from_day_of_year (g_date_get_day_of_year (date),
                                     g_date_get_year (date));
    icalproperty_set_due (data->prop, due);
  } else {
    icalcomponent_remove_property (data->task->comp, data->prop);
    icalproperty_free (data->prop);
    data->prop = NULL;
  }

  *data->dirty = TRUE;
}

static GtkWidget *
date_new ()
{
  GtkWidget *widget;
  guint id;

  widget = koto_date_combo_new ();
  id = g_signal_connect (widget, "notify::date", G_CALLBACK (on_date_notify), NULL);
  g_object_set_data (G_OBJECT (widget), SIGNAL_DATA, GINT_TO_POINTER (id));
  return widget;
}

static void
date_set (GtkWidget *widget, icalproperty *prop)
{
  GDate date;
  struct icaltimetype due;
  guint id;

  id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), SIGNAL_DATA));

  g_date_clear (&date, 1);
  
  if (prop) {
    due = icalproperty_get_due (prop);
    g_date_set_dmy (&date, due.day, due.month, due.year);
  }
  
  g_signal_handler_block (widget, id);
  koto_date_combo_set_date (KOTO_DATE_COMBO (widget), &date);
  g_signal_handler_unblock (widget, id);
}

/*
 * Public methods.
 */

static const DisplayField fields[] = {
  { ICAL_SUMMARY_PROPERTY, entry_new, entry_set },
  { ICAL_PRIORITY_PROPERTY, priority_new, priority_set },
  { ICAL_DESCRIPTION_PROPERTY, text_new, text_set },
  { ICAL_URL_PROPERTY, url_entry_new, entry_set },
  { ICAL_CATEGORIES_PROPERTY, category_new, category_set },
  { ICAL_DUE_PROPERTY, date_new, date_set },
};

GtkWidget *
koto_field_editor_create (icalproperty_kind kind, gboolean *dirty)
{
  FieldData *data;
  GtkWidget *widget;
  int i;

  g_return_val_if_fail (kind, NULL);

  /* TODO: sort array and use bsearch */

  for (i = 0; i < G_N_ELEMENTS (fields); i++) {
    if (fields[i].kind == kind) {
      widget = fields[i].editor_new ();

      data = g_new0 (FieldData, 1);
      data->field = &fields[i];
      data->dirty = dirty;

      g_object_set_data_full (G_OBJECT (widget), FIELD_DATA, data, g_free);
      
      return widget;
    }
  }
  return NULL;
}

void
koto_field_editor_set (GtkWidget *widget, KotoTask *task)
{
  FieldData *data;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (task);

  data = g_object_get_data (G_OBJECT (widget), FIELD_DATA);
  if (!data)
    return;
  
  g_return_if_fail (data->field->set_value);

  data->task = task;
  data->prop = icalcomponent_get_first_property (task->comp, data->field->kind);
  data->field->set_value (widget, data->prop);
}
