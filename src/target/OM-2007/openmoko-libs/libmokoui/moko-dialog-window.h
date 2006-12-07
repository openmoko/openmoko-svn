/*  moko-dialog-window.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2006/12/06 23:15:19 $) [$Author: mickey $]
 */

#ifndef _MOKO_DIALOG_WINDOW_H_
#define _MOKO_DIALOG_WINDOW_H_

#include "moko-window.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_DIALOG_WINDOW moko_dialog_window_get_type()
#define MOKO_DIALOG_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   MOKO_TYPE_DIALOG_WINDOW, MokoDialogWindow))
#define MOKO_DIALOG_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   MOKO_TYPE_DIALOG_WINDOW, MokoDialogWindowClass))
#define MOKO_IS_DIALOG_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   MOKO_TYPE_DIALOG_WINDOW))
#define MOKO_IS_DIALOG_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   MOKO_TYPE_DIALOG_WINDOW))
#define MOKO_DIALOG_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   MOKO_TYPE_DIALOG_WINDOW, MokoDialogWindowClass))

typedef struct {
    MokoWindow parent;
} MokoDialogWindow;

typedef struct {
    MokoWindowClass parent_class;
} MokoDialogWindowClass;

GType moko_dialog_window_get_type();
MokoDialogWindow* moko_dialog_window_new();

void moko_dialog_window_set_title(MokoDialogWindow* self, const gchar* title);

G_END_DECLS

#endif // _MOKO_DIALOG_WINDOW_H_

