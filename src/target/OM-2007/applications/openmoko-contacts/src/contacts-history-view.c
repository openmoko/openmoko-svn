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
 
#include "contacts-history-view.h"


#include "contacts-ui.h"
#include "contacts-main.h"
#include "contacts-utils.h"
#include "contacts-history.h"

void 
contacts_history_pane_show (GtkWidget *button, 
                            ContactsData *data)
{
  GtkTreeSelection *selection;
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW 
                                             (data->ui->contacts_treeview));
  gtk_notebook_set_current_page (GTK_NOTEBOOK (data->ui->main_notebook),
                                 CONTACTS_HISTORY_PANE);
  contacts_history_pane_update_selection (selection, data);
}

void 
contacts_history_pane_update_selection (GtkTreeSelection *selection, 
                                        ContactsData *data)
{
  EContact *contact;
  gchar *uid = NULL;

  if (!selection) {
    gtk_widget_set_sensitive (GTK_WIDGET (data->ui->history), FALSE);
    return;
  }

  /* Get the currently selected contact and update the history view */
  contact = contacts_contact_from_selection (selection, data->contacts_table);
  if (!contact) {
    gtk_widget_set_sensitive (GTK_WIDGET (data->ui->history), FALSE);
    return;
  }
  gtk_widget_set_sensitive (GTK_WIDGET (data->ui->history), TRUE);
  
  /* Get the contacts uid and update the history widget */
  uid = (gchar*) e_contact_get (contact, E_CONTACT_UID);
  if (uid)
    contacts_history_update_uid (CONTACTS_HISTORY (data->ui->history), uid);
  
  if (uid)
    g_free (uid);
}

static void
on_moko_journal_entry_activated (ContactsHistory *history, 
                                 MokoJournalEntry *entry)
{
  g_print ("Launch Viewer\n");
}

GtkWidget*
contacts_history_pane_new (void)
{
  GtkWidget *history = contacts_history_new ();
  
  g_signal_connect (G_OBJECT (history), "entry-activated",
                    G_CALLBACK (on_moko_journal_entry_activated), NULL);
  
  return history;
}
