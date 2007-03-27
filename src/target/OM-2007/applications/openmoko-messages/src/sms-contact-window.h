/*
 *  sms-contact-window.h
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
#ifndef _SMS_CONTACT_WINDOW_H_
#define _SMS_CONTACT_WINDOW_H_

#include <libmokoui/moko-window.h>
#include <libmokoui/moko-pixmap-button.h>
#include <libmokoui/moko-navigation-list.h>
#include <libmokoui/moko-application.h>
#include <libebook/e-book.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SMS_TYPE_CONTACT_WINDOW  sms_contact_window_get_type()
#define SMS_CONTACT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SMS_TYPE_CONTACT_WINDOW, SmsContactWindow))
#define SMS_CONTACT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SMS_TYPE_CONTACT_WINDOW, SmsContactWindowClass))
#define SMS_IS_CONTACT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SMS_TYPE_CONTACT_WINDOW))
#define SMS_IS_CONTACT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SMS_TYPE_CONTACT_WINDOW_CLASS)
#define SMS_GET_CONTACT_WINDOW_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SMS_TYPE_CONTACT_WINDOW, SmsContactWindowClass)

GType sms_contact_window_get_type();
GtkWidget* sms_contact_new();

enum {
  CONTACT_SEL_COL,
  CONTACT_NAME_COL,
  CONTACT_CELLPHONE_COL,
  CONTACT_LAST_COL
};

enum {
  CONTACT_SELECT_DONE_SIGNAL,
  LAST_SIGNAL
};

typedef struct
{
  MokoWindow parent;
  /* instance members */
  gchar* nameList;
}SmsContactWindow;

typedef struct
{
  MokoWindowClass parent_class;
  /* class members */
  void (*contact_select_done)(void);
}SmsContactWindowClass;

typedef struct
{
  EBook* book;
  GtkListStore *contacts_liststore;
}SmsContactData;

G_END_DECLS

#endif
