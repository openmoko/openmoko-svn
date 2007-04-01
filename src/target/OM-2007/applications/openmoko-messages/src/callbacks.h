/*
 *  callbacks.h 
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>
#include "sms-dialog-window.h"
#include "sms-membership-window.h"
#include "main.h"

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, MessengerData* d);
void send_signal_to_footer (DBusConnection* bus, gchar* message_str);

void cb_new_sms (GtkMenuItem* item, MessengerData* d);
void cb_new_mail (GtkMenuItem* item, MessengerData* d);
void cb_new_folder (GtkMenuItem* item, MessengerData* d);
void cb_mode_read (GtkMenuItem* item, MessengerData* d);
void cb_mode_reply (GtkMenuItem* item, MessengerData* d);
void cb_mode_forward (GtkMenuItem* item, MessengerData* d);
void cb_delete_folder (GtkMenuItem* item, MessengerData* d);
void cb_delete_message (GtkMenuItem* item, MessengerData* d);
void cb_mmitem_activate (GtkMenuItem* item, MessengerData* d);
void cb_fnitem_activate (GtkMenuItem* item, MessengerData* d);
void cb_cursor_changed(GtkTreeSelection* selection, MessengerData* d);

void cb_nfBtn_clicked (GtkButton* button, MessengerData* d);
void cb_nfResetBtn_clicked (GtkButton* button, MessengerData* d);
void cb_frBtn_clicked (GtkButton* button, MessengerData* d);
void cb_frResetBtn_clicked (GtkButton* button, GtkWidget* entry);
void cb_dfBtn_clicked (GtkButton* button, MessengerData* d);
void on_mmode_rdo_btn_clicked (gchar* folder);
void on_btnsend_clicked                (GtkButton       *button,
                                        gpointer         user_data);
void on_btn_address_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void cb_search_entry_changed (GtkEditable* editable, MessengerData* d);
void cb_search_on (MessengerData* d);
void cb_search_off (MessengerData* d);
void delete_folder (MessengerData* d, gchar* oldName);

#endif

