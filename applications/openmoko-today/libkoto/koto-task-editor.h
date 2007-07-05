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

#ifndef _KOTO_TASK_EDITOR
#define _KOTO_TASK_EDITOR

#include <gtk/gtktable.h>
#include <libical/icalderivedproperty.h>

G_BEGIN_DECLS

#define KOTO_TYPE_TASK_EDITOR koto_task_editor_get_type()

#define KOTO_TASK_EDITOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_TASK_EDITOR, KotoTaskEditor))

#define KOTO_TASK_EDITOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_TASK_EDITOR, KotoTaskEditorClass))

#define KOTO_IS_TASK_EDITOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_TASK_EDITOR))

#define KOTO_IS_TASK_EDITOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_TASK_EDITOR))

#define KOTO_TASK_EDITOR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_TASK_EDITOR, KotoTaskEditorClass))

typedef struct {
  GtkTable parent;
} KotoTaskEditor;

typedef struct {
  GtkTableClass parent_class;
} KotoTaskEditorClass;

GType koto_task_editor_get_type (void);

GtkWidget* koto_task_editor_new (void);

gboolean koto_task_editor_is_dirty (KotoTaskEditor *editor);

void koto_task_editor_add_field (KotoTaskEditor *editor, icalproperty_kind kind);

G_GNUC_NULL_TERMINATED void koto_task_editor_add_fields (KotoTaskEditor *editor, ...);

G_END_DECLS

#endif /* _KOTO_TASK_EDITOR */
