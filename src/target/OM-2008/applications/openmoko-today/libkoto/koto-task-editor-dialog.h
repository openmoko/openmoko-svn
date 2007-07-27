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

#ifndef _KOTO_TASK_EDITOR_DIALOG
#define _KOTO_TASK_EDITOR_DIALOG

#include <gtk/gtkdialog.h>
#include <libecal/e-cal.h>
#include "koto-task.h"
#include "koto-group-store.h"

G_BEGIN_DECLS

#define KOTO_TYPE_TASK_EDITOR_DIALOG koto_task_editor_dialog_get_type()

#define KOTO_TASK_EDITOR_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_TASK_EDITOR_DIALOG, KotoTaskEditorDialog))

#define KOTO_TASK_EDITOR_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_TASK_EDITOR_DIALOG, KotoTaskEditorDialogClass))

#define KOTO_IS_TASK_EDITOR_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_TASK_EDITOR_DIALOG))

#define KOTO_IS_TASK_EDITOR_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_TASK_EDITOR_DIALOG))

#define KOTO_TASK_EDITOR_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_TASK_EDITOR_DIALOG, KotoTaskEditorDialogClass))

typedef struct {
  GtkDialog parent;
} KotoTaskEditorDialog;

typedef struct {
  GtkDialogClass parent_class;
} KotoTaskEditorDialogClass;

enum {
  KOTO_TASK_EDITOR_DIALOG_RESPONSE_CLOSE = GTK_RESPONSE_CLOSE,
  KOTO_TASK_EDITOR_DIALOG_RESPONSE_DELETE = 0
};

GType koto_task_editor_dialog_get_type (void);

GtkWidget * koto_task_editor_dialog_new (void);

gboolean koto_task_editor_dialog_is_dirty (KotoTaskEditorDialog *dialog);

void koto_task_editor_dialog_set_group_store (KotoTaskEditorDialog *dialog, KotoGroupStore *store);

void koto_task_editor_dialog_set_task (KotoTaskEditorDialog *dialog, KotoTask *task);

G_GNUC_DEPRECATED void koto_task_editor_dialog_run (KotoTaskEditorDialog *dialog, ECal *cal);

G_END_DECLS

#endif /* _KOTO_TASK_EDITOR_DIALOG */
