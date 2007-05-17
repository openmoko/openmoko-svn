/*
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libmokoui/moko-application.h>
#include <libmokoui/moko-details-window.h>
#include <libmokoui/moko-dialog-window.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-navigation-list.h>
#include <libmokoui/moko-scrolled-pane.h>
#include <libmokoui/moko-stock.h>


#ifndef OMOKO_H
#define OMOKO_H

void contacts_ui_create (ContactsData *data);
void contacts_ui_update_groups_list (ContactsData *data);

GtkWidget * create_contacts_list (ContactsData *data);

#endif
