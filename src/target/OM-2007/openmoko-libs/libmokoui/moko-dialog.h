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

#ifndef _MOKO_DIALOG
#define _MOKO_DIALOG

#include "moko-window.h"

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_DIALOG moko_dialog_get_type()

#define MOKO_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MOKO_TYPE_DIALOG, MokoDialog))

#define MOKO_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MOKO_TYPE_DIALOG, MokoDialogClass))

#define MOKO_IS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MOKO_TYPE_DIALOG))

#define MOKO_IS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MOKO_TYPE_DIALOG))

#define MOKO_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MOKO_TYPE_DIALOG, MokoDialogClass))

typedef struct {
  MokoWindow parent;
  GtkWidget *vbox;
  GtkWidget *action_area;
} MokoDialog;

typedef struct {
  MokoWindowClass parent_class;
  void (* response) (MokoDialog *self, gint response_id);
} MokoDialogClass;

GType moko_dialog_get_type (void);

gint moko_dialog_get_button_response_id (MokoDialog *self, GtkButton *button);


void moko_dialog_add_buttons (MokoDialog *self, gchar *first_button_text, ...);
GtkWidget* moko_dialog_add_button (MokoDialog *self, gchar *text, gint response_id);
void moko_dialog_add_button_widget (MokoDialog *self, GtkButton *button, gint response_id);
void moko_dialog_response (MokoDialog *self, gint response_id);
gint moko_dialog_run (MokoDialog *self);

GtkWidget* moko_dialog_new (void);

G_END_DECLS

#endif /* _MOKO_DIALOG */

