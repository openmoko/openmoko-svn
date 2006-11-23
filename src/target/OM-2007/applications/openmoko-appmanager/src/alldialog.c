/**
 * @file alldialog.c 
 * @brief Manager all dialog that be used in the application manager.
 *
 * The list of dialog
 *   - Apply dialog
 *     -# Display the packages list that marked
 *     -# The Ok button means that user confirms the choice.
 *     -# The Cancel button means that user abolishs the choice.
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-10-23
 */

#include "alldialog.h"
#include "interface.h"
#include "support.h"
#include "errorcode.h"
#include "pkglist.h"

/**
 * @brief The id of package list that display in apply dialog.
 */
enum {
  MARK_COL_NAME = 0,      ///<Package name
  MARK_NUM_COL            ///<Column number
};

/**
 * @brief Init the apply dialog, and display the mark list to user.
 * @return The apply dialog
 */
GtkWidget *
init_apply_dialog (void)
{
  GtkWidget  *dialog;
  GtkWidget  *marklist;
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore    *store;

  dialog = create_applydialog ();

  marklist = lookup_widget (dialog, "applylist");
  if (marklist == NULL)
    {
      ERROR ("Init dialog error, did not find the mark list view");
      return NULL;
    }

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Package Name"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", MARK_COL_NAME,
                                       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (marklist), col);

  store = gtk_tree_store_new (MARK_NUM_COL, G_TYPE_STRING);

  fill_store_with_marked_list (store, MARK_COL_NAME);

  gtk_tree_view_set_model (GTK_TREE_VIEW (marklist), GTK_TREE_MODEL (store));
  g_object_unref (GTK_TREE_MODEL (store));

  return dialog;
}

/**
 * @brief Show the message to user by a message dialog.
 */
void 
show_message_to_user (gchar *msg)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_OK,
                                   msg);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/**
 * @brief Create a GtkFileChooserDialog widget to choose a ".ipk" file to install.
 *
 * The name must be free by g_free().
 * @param name The name of file that user selected
 * @return TRUE User chooses a file. FALSE User cancels this operation.
 */
gboolean 
create_file_selection (gchar **name)
{
  GtkWidget      *filew;
  gint           res;
  GtkFileFilter  *filter;

  filew = gtk_file_chooser_dialog_new(_("Add application"),
                                      NULL,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filew), FALSE);
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filew), FALSE);
  gtk_window_set_default_size (GTK_WINDOW (filew), 400, 300);

  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, _("Ipk files(*.ipk)"));
  gtk_file_filter_add_pattern(filter, "*.ipk");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filew), filter);

  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, _("All files(*.*)"));
  gtk_file_filter_add_pattern(filter, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filew), filter);

  res = gtk_dialog_run(GTK_DIALOG(filew));

  if( (res == GTK_RESPONSE_ACCEPT) || (res == GTK_RESPONSE_OK) ) {
    *name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filew));
    DBG("file name of user selected is :%s\n", *name);

    gtk_widget_destroy(filew);
    return TRUE;
  }

  gtk_widget_destroy(filew);
  return FALSE;

}
