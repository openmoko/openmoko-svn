/*
 *  @file appmanager-data.h
 *  @brief The all data that the application manager will used
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
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#ifndef _FIC_APPMANAGER_DATA_H
#define _FIC_APPMANAGER_DATA_H

#include <gtk/gtk.h>

#include "pixbuf-list.h"

G_BEGIN_DECLS

#define MOKO_TYPE_APPLICATION_MANAGER_DATA  (application_manager_data_get_type ())
#define MOKO_APPLICATION_MANAGER_DATA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                            MOKO_TYPE_APPLICATION_MANAGER_DATA, \
                                            ApplicationManagerData))
#define MOKO_APPLICATION_MANAGER_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                    MOKO_TYPE_APPLICATION_MANAGER_DATA, \
                                                    ApplicationManagerDataClass))
#define MOKO_IS_APPLICATION_MANAGER_DATA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                               MOKO_TYPE_APPLICATION_MANAGER_DATA))
#define MOKO_IS_APPLICATION_MANAGER_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                       MOKO_TYPE_APPLICATION_MANAGER_DATA))
#define MOKO_APPLICATION_MANAGER_DATA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                      MOKO_TYPE_APPLICATION_MANAGER_DATA, \
                                                      ApplicationManagerDataClass))


/*
 * @brief The all data that will be used in the application manager
 *
 * Acturally, it need not load the pixbuf to memory and keep them.
 * But these pixbuf are used so many times. So it is worthy to load them
 * and keep them in the memory.
 * FIXME Maybe use a GObject as the base class is the better.
 */
typedef struct _ApplicationManagerData {
  GObject          parent;             /* The parent of the struct */

  GtkWidget        *mwindow;           /* The main window */
  GtkWidget        *menubox;           /* The menubox */
  GtkMenu          *selectmenu;        /* The select menu */
  GtkEntry         *searchentry;       /* The search entry */
  GtkWidget        *tvpkglist;         /* The treeview of the package list */
  GtkWidget        *tvdetail;          /* The textview of the details info */
  gpointer          pkglist;           /* The package list get from lib ipkg */
  gpointer          sectionlist;       /* The section list parse from the package list */
  gpointer          installedlist;     /* The list of all installed packages */
  gpointer          upgradelist;       /* The list of all upgradeable packages */
  gpointer          selectedlist;      /* The list of packages that user selected */
  gpointer          nosecpkglist;      /* The list of packages whose section name is NULL */
  gpointer          currentlist;       /* The current list that display on the navigation list */
  GdkPixbuf        *statuspix[N_COUNT_PKG_STATUS];    /* The all pixbufs that need by the package list store */
  gchar            *searchhistory;     /* The search history */
  GtkWidget        *installdialog;     /* The install dialog */
  
  GtkTreeModel     *filter_store;      /* GtkListStore for filter menu */
  
  GtkToolItem      *install_btn; /* Install toolbar button */
  GtkToolItem      *remove_btn;  /* Remove toolbar button */
  
  GtkWidget        *searchbar;
} ApplicationManagerData;

/*
 * @brief The class struct of application manager data
 */
typedef struct _ApplicationManagerDataClass {
  GObjectClass    parent_class;        /* The parent class */
} ApplicationManagerDataClass;

GType application_manager_data_get_type (void);


ApplicationManagerData *application_manager_data_new (void);

void application_manager_data_set_main_window (ApplicationManagerData *appdata, 
                                               GtkWindow *window);

void application_manager_data_set_filter_menu (ApplicationManagerData *appdata,
                                               GtkMenu *filtermenu);

void application_manager_data_set_menubox (ApplicationManagerData *appdata,
                                           GtkWidget *menubox);

void application_manager_data_set_select_menu (ApplicationManagerData *appdata,
                                               GtkMenu *selectmenu);

void application_manager_data_set_search_entry (ApplicationManagerData *appdata,
                                                GtkEntry *entry);

void application_manager_data_set_tvpkglist (ApplicationManagerData *appdata,
                                             GtkWidget *tvpkglist);

void application_manager_data_set_tvdetail (ApplicationManagerData *appdata,
                                            GtkWidget *tvdetail);

void application_manager_data_set_pkglist (ApplicationManagerData *appdata,
                                           gpointer pkglist);

void application_manager_data_set_section_list (ApplicationManagerData *appdata,
                                                gpointer sectionlist);

void application_manager_data_set_installed_list (ApplicationManagerData *appdata,
                                                  gpointer installedlist);

void application_manager_data_set_upgrade_list (ApplicationManagerData *appdata,
                                                gpointer upgradelist);

void application_manager_data_set_selected_list (ApplicationManagerData *appdata,
                                                 gpointer selectedlist);

void application_manager_data_set_current_list (ApplicationManagerData *appdata,
                                                gpointer currentlist);

void application_manager_data_set_nosecpkg_list (ApplicationManagerData *appdata,
                                                 gpointer nosecpkglist);

void init_pixbuf_list (ApplicationManagerData *appdata);

void application_manager_data_set_search_history (ApplicationManagerData *appdata,
                                                 gchar *searchhistory);

void application_manager_data_set_install_dialog (ApplicationManagerData *appdata,
                                                  GtkWidget *installdialog);

GtkWidget *
     application_manager_get_main_window (ApplicationManagerData *appdata);

GtkMenu *
     application_manager_get_filter_menu (ApplicationManagerData *appdata);

GtkWidget *
     application_manager_get_menubox (ApplicationManagerData *appdata);

GtkMenu *
     application_manager_get_select_menu (ApplicationManagerData *appdata);

GtkEntry *
     application_manager_get_search_entry (ApplicationManagerData *appdata);

GtkWidget *
     application_manager_get_tvpkglist (ApplicationManagerData *appdata);

GtkWidget *
     application_manager_get_tvdetail (ApplicationManagerData *appdata);

gpointer 
     application_manager_data_get_pkglist (ApplicationManagerData *appdata);

gpointer 
     application_manager_data_get_sectionlist (ApplicationManagerData *appdata);

gpointer 
     application_manager_data_get_installedlist (ApplicationManagerData *appdata);

gpointer 
     application_manager_data_get_upgradelist (ApplicationManagerData *appdata);

gpointer 
     application_manager_data_get_selectedlist (ApplicationManagerData *appdata);

gpointer
     application_manager_data_get_nosecpkglist (ApplicationManagerData *appdata);

gpointer
     application_manager_data_get_currentlist (ApplicationManagerData *appdata);

GdkPixbuf *
     application_manager_data_get_status_pixbuf (ApplicationManagerData *appdata, 
                                                 guint id);

gchar *
     application_manager_data_get_search_history (ApplicationManagerData *appdata);

GtkWidget *
     application_manager_data_get_install_dialog (ApplicationManagerData *appdata);

G_END_DECLS

#endif

