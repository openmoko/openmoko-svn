/*  moko-dialog.h
 *
 *  Authored by Rob Bradford <rob@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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
 */

#ifndef _MOKO_MESSAGE_DIALOG
#define _MOKO_MESSAGE_DIALOG

#include "moko-dialog.h"

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_MESSAGE_DIALOG moko_message_dialog_get_type()

#define MOKO_MESSAGE_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MOKO_TYPE_MESSAGE_DIALOG, MokoMessageDialog))

#define MOKO_MESSAGE_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MOKO_TYPE_MESSAGE_DIALOG, MokoMessageDialogClass))

#define MOKO_IS_MESSAGE_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MOKO_TYPE_MESSAGE_DIALOG))

#define MOKO_IS_MESSAGE_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MOKO_TYPE_MESSAGE_DIALOG))

#define MOKO_MESSAGE_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MOKO_TYPE_MESSAGE_DIALOG, MokoMessageDialogClass))

typedef struct {
  GtkDialog parent;
} MokoMessageDialog;

typedef struct {
  GtkDialogClass parent_class;
} MokoMessageDialogClass;

GType moko_message_dialog_get_type (void);

GtkWidget* moko_message_dialog_new (void);
void moko_message_dialog_set_message (MokoMessageDialog *dialog, const gchar *message);
void moko_message_dialog_set_image_from_stock (MokoMessageDialog *dialog,
    const gchar *stock_id);

G_END_DECLS

#endif /* _MOKO_MESSAGE_DIALOG */

