/*
 *  Messages -- An messages application for OpenMoko Framework
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

#ifndef __MAIN__H_
#define __MAIN__H_

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-tree-view.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreemodelfilter.h>
//#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include "foldersdb.h"
#include "detail-area.h"

typedef struct _MessengerData{
    MokoApplication* app;
    MokoPanedWindow* window;
    GtkWidget* menu;
    GtkWidget* filtmenu;
    FoldersDB* foldersdb;
    GtkWidget* toolbox;
    GtkListStore* liststore;
    GtkTreeModel* filter;
    GtkWidget* view;
    GtkWidget* details;
    GSList* folderlist;
    gchar* currentfolder;
    gchar* s_key;
    gint   msg_num;
    gboolean searchOn;
    GtkWidget* nfEntry;
    GtkWidget* frEntry;
    GtkWidget* mmWin;
    GtkWidget* mmitem;
    GtkWidget* fnitem;
    DBusConnection *bus;
}MessengerData;

enum {
    COLUMN_ICON,
    COLUMN_FROM,
    COLUMN_SUBJECT,
    COLUMN_CONTENT,
    COLUMN_STATUS,
    COLUMN_FOLDER,
    NUM_COLS,
};

enum {
    PAGE_EDIT_MODE,
    PAGE_NEW_MAIL,
    PAGE_MODE_READ,
    PAGE_MODE_REPLY,
    PAGE_MODE_FORWARD,
    PAGE_MODE_MMSHIP,
    NUM_PAGES,
};

GtkWidget* reload_filter_menu (MessengerData* d, GSList* folderlist);
void setup_ui( MessengerData* d );
void populate_navigation_area( MessengerData* d );
void populate_detail_area( MessengerData* d );
void main_quit(GtkWidget* widget, GdkEvent* event, MessengerData* d);
void update_folder_sensitive (MessengerData* d, GSList* folderlist);
gboolean init_dbus (MessengerData* d);

#endif

