/*
 *  @file install-dialog.h
 *  @brief It is the dialog that displays the process of install/remove/upgrade
 *  packages.
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
#ifndef _FIC_INSTALL_DIALOG_H
#define _FIC_INSTALL_DIALOG_H

#include <gtk/gtk.h>

#include "appmanager-data.h"

G_BEGIN_DECLS

#define MOKO_TYPE_INSTALL_DIALOG  (install_dialog_get_type ())
#define MOKO_INSTALL_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                  MOKO_TYPE_INSTALL_DIALOG, \
                                  InstallDialog))
#define MOKO_INSTALL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                          MOKO_TYPE_INSTALL_DIALOG, \
                                          InstallDialogClass))
#define MOKO_IS_INSTALL_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                     MOKO_TYPE_INSTALL_DIALOG))
#define MOKO_IS_INSTALL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                             MOKO_TYPE_INSTALL_DIALOG))
#define MOKO_INSTALL_DIALOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                            MOKO_TYPE_INSTALL_DIALOG, \
                                            InstallDialogClass))

/*
 * @brief Process status of the install dialog
 */
enum {
  STATUS_INSTALL,       /* Install/remove/upgrade packages */
  STATUS_REINIT,        /* Reinit the package list and navigation view */
  STATUS_ERROR,         /* There is error when install/remove/upgrade packages */
  STATUS_COMPLETE       /* The process of install/remove/upgrade packages completed */
};

/*
 * @brief The install dialog struct
 */
typedef struct _InstallDialog {
  GtkDialog          parent;             /* The parent of the struct */
} InstallDialog;

/*
 * @brief The install dialog class struct
 */
typedef struct _InstallDialogClass {
  GtkDialogClass     parent_class;       /* The parent of the struct */
} InstallDialogClass;

GType install_dialog_get_type (void);

InstallDialog *install_dialog_new (ApplicationManagerData *appdata, gint pkgnum);
void install_dialog_set_install_status (InstallDialog *dialog, 
                                        gint status);
void install_dialog_add_prepare_info (InstallDialog *dialog, 
                                      const gchar *info);

void install_dialog_add_install_info (InstallDialog *dialog, 
                                      const gchar *info);

G_END_DECLS

#endif

