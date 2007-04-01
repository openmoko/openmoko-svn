/*
 *  sms-dialog-window.h
 *  
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *  
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: alex $]
 */

#ifndef _SMS_DIALOG_WINDOW_H_
#define _SMS_DIALOG_WINDOW_H_

#include <libmokoui/moko-window.h>
#include <libmokoui/moko-tool-box.h>
#include "message.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define SMS_TYPE_DIALOG_WINDOW sms_dialog_window_get_type()
#define SMS_DIALOG_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   SMS_TYPE_DIALOG_WINDOW, SmsDialogWindow))
#define SMS_DIALOG_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   SMS_TYPE_DIALOG_WINDOW, SmsDialogWindowClass))
#define SMS_IS_DIALOG_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   SMS_TYPE_DIALOG_WINDOW))
#define SMS_IS_DIALOG_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   SMS_TYPE_DIALOG_WINDOW))
#define SMS_DIALOG_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   SMS_TYPE_DIALOG_WINDOW, SmsDialogWindowClass))

typedef struct
{ 
  MokoWindow parent;
  GtkWidget* addressBtn;
  GtkWidget* toEntry;
}
SmsDialogWindow;

typedef struct
{
  MokoWindowClass parent_class;
}
SmsDialogWindowClass;

GType sms_dialog_window_get_type();
SmsDialogWindow* sms_dialog_window_new();

void sms_dialog_window_set_title(SmsDialogWindow* self, const gchar* title);
void mail_dialog_window_set_title(SmsDialogWindow* self, const gchar* title);
void sms_dialog_window_set_contents(SmsDialogWindow* self, GtkWidget* contents);
void sms_dialog_reply_message(SmsDialogWindow* self, message* msg);
void sms_dialog_forward_message(SmsDialogWindow* self, message* msg);

G_END_DECLS

#endif // _SMS_DIALOG_WINDOW_H_

