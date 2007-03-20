/*
 *  @file install-dialog.c
 *  @brief It is the dialog that displays the process of install/remove/upgrade
 *  packages.
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
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
#include <libmokoui/moko-tree-view.h>
#include <string.h>

#include "install-dialog.h"
#include "appmanager-window.h"
#include "package-list.h"

static void install_dialog_class_init (InstallDialogClass *klass);
static void install_dialog_init (InstallDialog *self);
static gboolean install_dialog_time_out (gpointer dialog);

G_DEFINE_TYPE (InstallDialog, install_dialog, GTK_TYPE_DIALOG)

#define MOKO_INSTALL_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_INSTALL_DIALOG, InstallDialogPriv));

/*
 * @brief The private data of the Install dialog
 */
typedef struct _InstallDialogPriv {
  ApplicationManagerData   *maindata;    /* The main data of the application manager */
  gchar    *installinfolist;            /* The list of install/remove/upgrade infomation */
  gchar    *prepareinfolist;            /* The list of prepareinfomation */
  gint     preparenum;                   /* Prepare to install/remove/upgrade the _number_ package */
  gint     installnum;                   /* Installing/removing/upgrading the _number_ package */
  gint     displaypreparenum;            /* Prepare info of the _number_ package has been displayed */
  gint     displayinstallnum;            /* Installed info of the _number_ package has been displayed */
  gint     installstatus;                /* The status of the install process */
  gint     displaystatus;                /* The status of the display process */
  gboolean requestcancel;                /* Cancel the install status by user choice */
  GtkWidget  *textview;                  /* The textview in the dialog */
  gint     pkgnum;                       /* The number of packages that need be installed/upgraded/removed */
} InstallDialogPriv;

static void 
install_dialog_class_init (InstallDialogClass *klass)
{
  g_type_class_add_private (klass, sizeof (InstallDialogPriv));
}

static void 
install_dialog_init (InstallDialog *self)
{
  GtkWidget  *vbox;
  GtkWidget  *scrollwindow;
  GtkWidget  *closebutton;
  InstallDialogPriv *priv;

  priv = MOKO_INSTALL_DIALOG_GET_PRIVATE(self);

  /* Init the data */
  priv->maindata = NULL;
  priv->installinfolist = NULL;
  priv->prepareinfolist = NULL;
  priv->preparenum = 0;
  priv->installnum = 0;
  priv->displaypreparenum = 0;
  priv->displayinstallnum = 0;
  priv->installstatus = STATUS_INSTALL;
  priv->displaystatus = STATUS_INSTALL;
  priv->requestcancel = FALSE;

  /* Init the dialog */
  gtk_widget_set_size_request (GTK_WIDGET (self), 480, 480);
  gtk_window_set_title (GTK_WINDOW (self), _("dialog1"));
  gtk_window_set_type_hint (GTK_WINDOW (self), GDK_WINDOW_TYPE_HINT_DIALOG);

  vbox = GTK_DIALOG (self)->vbox;
  gtk_widget_show (vbox);

  scrollwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrollwindow);
  gtk_box_pack_start (GTK_BOX (vbox), scrollwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwindow), GTK_SHADOW_IN);

  priv->textview = gtk_text_view_new ();
  gtk_widget_show (priv->textview);
  gtk_container_add (GTK_CONTAINER (scrollwindow), priv->textview);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->textview), FALSE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (priv->textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (priv->textview), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (priv->textview), FALSE);

  closebutton = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (closebutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (self), closebutton, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (closebutton, GTK_CAN_DEFAULT);
}

/*
 * @brief Create a new install dialog
 * @param appdata The application manager data
 * @param pkgnum The number of package that will be disposed
 * @return The install dialog
 */
InstallDialog *
install_dialog_new (ApplicationManagerData *appdata, gint pkgnum)
{
  InstallDialog  *dialog = MOKO_INSTALL_DIALOG (g_object_new (MOKO_TYPE_INSTALL_DIALOG, NULL));
  InstallDialogPriv *priv = MOKO_INSTALL_DIALOG_GET_PRIVATE (dialog);

  priv->installinfolist = (gchar *)g_malloc (sizeof(gchar *) * pkgnum);
  if (priv->installinfolist == NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (dialog));
      g_debug ("Can not malloc memory for the install dialog");
      return NULL;
    }
  priv->prepareinfolist = (gchar *)g_malloc (sizeof(gchar *) * pkgnum);
  if (priv->prepareinfolist == NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (dialog));
      g_free (priv->installinfolist);
      g_debug ("Can not malloc memory for the install dialog");
      return NULL;
    }
  priv->pkgnum = pkgnum;
  g_timeout_add (100, (GSourceFunc)install_dialog_time_out, dialog);

  return dialog;
}

/*
 * @brief The timeout event, update the display infomation of 
 * install/remove/upgrade packages
 * @param dialog The install dialog
 */
static gboolean 
install_dialog_time_out (gpointer dialog)
{
  InstallDialog  *installdialog;
  InstallDialogPriv *priv;
  GtkTextBuffer  *buffer;
  gboolean       change = FALSE;
  GtkTextIter   iter;

  g_return_val_if_fail (MOKO_IS_INSTALL_DIALOG (dialog), FALSE);
  installdialog = MOKO_INSTALL_DIALOG (dialog);

  priv = MOKO_INSTALL_DIALOG_GET_PRIVATE (installdialog);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview));

  while ((priv->displaypreparenum < priv->preparenum) 
         || (priv->displayinstallnum < priv->installnum))
    {
      change = TRUE;
      if (priv->displaypreparenum == priv->displayinstallnum)
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter, 
                                  priv->prepareinfolist[priv->displaypreparenum],
                                  -1);
          g_free (priv->prepareinfolist[priv->displaypreparenum]);
          priv->prepareinfolist[priv->displaypreparenum] = NULL;
          priv->displaypreparenum ++;
        }
      else
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter, 
                                  priv->installinfolist[priv->displayinstallnum],
                                  -1);
          g_free (priv->installinfolist[priv->displayinstallnum]);
          priv->installinfolist[priv->displayinstallnum] = NULL;
          priv->displayinstallnum ++;
        }
    }

  if (change)
    return TRUE;

  if (priv->installstatus == STATUS_REINIT)
    {
      if (priv->displaystatus != STATUS_REINIT)
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter,
                                  "Reload the package list, please wait\n", 
                                  -1);
          priv->displaystatus = STATUS_REINIT;
        }
      return TRUE;
    }

  if (priv->installstatus == STATUS_ERROR)
    {
      /* FIXME An error appeared in the install process. Add code later. */
      return FALSE;
    }

  if (priv->installstatus == STATUS_COMPLETE)
    {
      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert (buffer, &iter,
                              "Completely, press the close button to finish.\n",
                              -1);

      if (priv->installinfolist != NULL)
        {
          g_free (priv->installinfolist);
          priv->installinfolist = NULL;
        }

      if (priv->prepareinfolist != NULL)
        {
          g_free (priv->prepareinfolist);
          priv->prepareinfolist = NULL;
        }
      return FALSE;
    }

  return TRUE;
}

/*
 * @brief Set the install status of the install dialog
 * @param dialog The install dialog
 * @param status The status
 */
void 
install_dialog_set_install_status (InstallDialog *dialog, 
                                   gint status)
{
  InstallDialogPriv *priv;

  g_return_if_fail (MOKO_IS_INSTALL_DIALOG (dialog));
  priv = MOKO_INSTALL_DIALOG_GET_PRIVATE (dialog);

  g_return_if_fail ((status >= STATUS_INSTALL) && (status <= STATUS_COMPLETE));
  priv->installstatus = status;
}

/*
 * @brief Add a prepare infomation to the dialog infomation
 * @param dialog The install dialog
 * @param info The infomation string
 */
void 
install_dialog_add_prepare_info (InstallDialog *dialog, 
                                 const gchar *info)
{
  InstallDialogPriv *priv;

  g_return_if_fail (MOKO_IS_INSTALL_DIALOG (dialog));
  priv = MOKO_INSTALL_DIALOG_GET_PRIVATE (dialog);

  g_return_if_fail (priv->preparenum < priv->pkgnum);

  priv->prepareinfolist[priv->preparenum] = g_malloc (strlen (info) +1);
  g_return_if_fail (priv->prepareinfolist[priv->preparenum] != NULL);

  strcpy (priv->prepareinfolist[priv->preparenum], info);
  priv->preparenum ++;
}

/*
 * @brief Add a install infomation to the dialog infomation
 * @param dialog The install dialog
 * @param info The infomation string
 */
void 
install_dialog_add_install_info (InstallDialog *dialog, 
                                 const gchar *info)
{
  InstallDialogPriv *priv;

  g_return_if_fail (MOKO_IS_INSTALL_DIALOG (dialog));
  priv = MOKO_INSTALL_DIALOG_GET_PRIVATE (dialog);

  g_return_if_fail (priv->installnum < priv->pkgnum);

  priv->installinfolist[priv->installnum] = g_malloc (strlen (info) +1);
  g_return_if_fail (priv->installinfolist[priv->installnum] != NULL);

  strcpy (priv->installinfolist[priv->installnum], info);
  priv->installnum ++;
}

