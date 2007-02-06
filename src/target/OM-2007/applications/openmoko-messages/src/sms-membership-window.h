/*  sms-membership-window.h
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 First International Company
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
 
#ifndef _SMS_MEMBERSHIP_WINDOW_H_
#define _SMS_MEMBERSHIP_WINDOW_H_

#include <libmokoui/moko-window.h>
#include <libmokoui/moko-menu-box.h>
#include <libmokoui/moko-tree-view.h>
#include <gtk/gtklist.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtktogglebutton.h>

#include "main.h"
#include "message.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define SMS_TYPE_MEMBERSHIP_WINDOW sms_membership_window_get_type()
#define SMS_MEMBERSHIP_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   SMS_TYPE_MEMBERSHIP_WINDOW, SmsMembershipWindow))
#define SMS_MEMBERSHIP_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   SMS_TYPE_MEMBERSHIP_WINDOW, SmsMembershipWindowClass))
#define SMS_IS_MEMBERSHIP_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   SMS_TYPE_MEMBERSHIP_WINDOW))
#define SMS_IS_MEMBERSHIP_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   SMS_TYPE_MEMBERSHIP_WINDOW))
#define SMS_MEMBERSHIP_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   SMS_TYPE_MEMBERSHIP_WINDOW, SmsMembershipWindowClass))

typedef struct {
    MokoWindow parent;
} SmsMembershipWindow;

typedef struct {
    MokoWindowClass parent_class;
} SmsMembershipWindowClass;

GType sms_membership_window_get_type();
SmsMembershipWindow* sms_membership_window_new();

void sms_membership_window_set_menubox(SmsMembershipWindow* self, GtkList* folderlist);
void sms_membership_window_set_messages (SmsMembershipWindow* self, GtkListStore* liststore);
void sms_membership_window_show (SmsMembershipWindow* self);
guint sms_membership_window_run(SmsMembershipWindow* self);

G_END_DECLS

#endif // _SMS_MEMBERSHIP_WINDOW_H_
 