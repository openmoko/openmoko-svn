/*
 *  moko-save-number.c: save a phone number to a contact by creating a new
 *                      contact or adding to an existing one.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2008 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 */


#include <gtk/gtk.h>

#include <moko-finger-scroll.h>

#include <libebook/e-book.h>

#include "hito-contact-view.h"
#include "hito-contact-store.h"
#include "hito-group-store.h"
#include "hito-group-combo.h"
#include "hito-all-group.h"
#include "hito-separator-group.h"
#include "hito-group.h"
#include "hito-no-category-group.h"
#include "hito-vcard-util.h"


struct _SaveButtonInfo
{
  GtkWidget *dialog;
  gint response_id;
  gchar *number;
};
typedef struct _SaveButtonInfo SaveButtonInfo;



static void
create_new_contact_from_number (gchar *number)
{
  GtkWidget *dialog, *name, *label;

  dialog = gtk_dialog_new_with_buttons ("Save as Contact",
             NULL, GTK_DIALOG_MODAL,
             GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
             GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
             NULL);

  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

  label = gtk_label_new ("Enter a name for the contact");
  name = gtk_entry_new ();

  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG(dialog)->vbox), label);
  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG(dialog)->vbox), name);

  gtk_widget_show (label);
  gtk_widget_show (name);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    EContact *contact;
    EBook *book;
    EVCardAttribute *attr;
    const gchar *s;

    /* create contact */
    contact = e_contact_new ();
    /* add name */
    s = gtk_entry_get_text (GTK_ENTRY (name));
    e_contact_set (contact, E_CONTACT_FULL_NAME, (const gpointer) s);
    /* add number */
    attr = e_vcard_attribute_new ("", EVC_TEL);
    e_vcard_add_attribute_with_value (E_VCARD (contact), attr, number);
    hito_vcard_attribute_set_type (attr, "Other");

    /* open address book */
    /* TODO: check GErrors */
    book = e_book_new_system_addressbook (NULL);
    e_book_open (book, FALSE, NULL);

    /* add contact to address book, and close */
    e_book_add_contact (book, contact, NULL);

    g_object_unref (book);
    g_object_unref (contact);
  }
  gtk_widget_destroy (dialog);
}

static void
add_number_to_contact (gchar *number)
{
    EBook *book;
    EBookQuery *query;
    EBookView *view;
    GtkWidget *window, *contacts_treeview, *scroll, *groups_combo;
    GtkTreeModel *store, *group_store, *contact_filter;
    GError *err = NULL;
    
    window = gtk_dialog_new_with_buttons ("Add to Contact", NULL, 0,
					  "Cancel", GTK_RESPONSE_CANCEL,
					  "Add", GTK_RESPONSE_OK,
					  NULL);

    gtk_dialog_set_has_separator (GTK_DIALOG (window), FALSE);

    book = e_book_new_system_addressbook (&err);
    if (err)
      return;
    e_book_open (book, FALSE, &err);
    if (err)
     return;
    query = e_book_query_any_field_contains (NULL);
    e_book_get_book_view (book, query, NULL, 0, &view, &err);
    if (err)
      return;

    e_book_query_unref (query);  
    e_book_view_start (view);


    store = hito_contact_store_new (view);

    group_store = hito_group_store_new ();
    hito_group_store_set_view (HITO_GROUP_STORE (group_store), view);
    hito_group_store_add_group (HITO_GROUP_STORE (group_store), hito_all_group_new ());
    hito_group_store_add_group (HITO_GROUP_STORE (group_store), hito_separator_group_new (-99));
    hito_group_store_add_group (HITO_GROUP_STORE (group_store), hito_separator_group_new (99));
    hito_group_store_add_group (HITO_GROUP_STORE (group_store), hito_no_category_group_new ());

    contact_filter = hito_contact_model_filter_new (HITO_CONTACT_STORE (store));

    groups_combo = hito_group_combo_new (HITO_GROUP_STORE (group_store));
    hito_group_combo_connect_filter (HITO_GROUP_COMBO (groups_combo),
                                   HITO_CONTACT_MODEL_FILTER (contact_filter));
    gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (window)->vbox), groups_combo);
    gtk_combo_box_set_active (GTK_COMBO_BOX (groups_combo), 0);


    
    contacts_treeview = hito_contact_view_new (HITO_CONTACT_STORE (store), HITO_CONTACT_MODEL_FILTER (contact_filter));
    
    scroll = moko_finger_scroll_new ();
    gtk_widget_set_size_request (scroll, -1, 300);
    gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (window)->vbox), scroll);
    
    gtk_container_add (GTK_CONTAINER (scroll), contacts_treeview);
    
    gtk_widget_show_all (scroll);
    gtk_widget_show_all (groups_combo);
    
    if (gtk_dialog_run (GTK_DIALOG (window)) == GTK_RESPONSE_OK)
    {
      GtkTreeIter iter;
      EContact *contact;
      EVCardAttribute *attr;
      GtkTreeModel *model;
      GtkTreeSelection *selection;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (contacts_treeview));

      if (gtk_tree_selection_get_selected (selection, &model, &iter))
      {
        gtk_tree_model_get (model, &iter, COLUMN_CONTACT, &contact, -1);
        if (contact)
        {
          attr = e_vcard_attribute_new ("", EVC_TEL);
          e_vcard_add_attribute_with_value (E_VCARD (contact), attr, number);
          hito_vcard_attribute_set_type (attr, "Other");
          e_book_async_commit_contact (book, contact, NULL, NULL);
          g_object_unref (contact);
        }
      }
    }

    gtk_widget_destroy (window);
    g_object_unref (book);
}

static void
on_btn_save_clicked (GtkWidget *button, SaveButtonInfo *info)
{
  gint action = info->response_id;
  gchar *number = g_strdup (info->number);
    
  /* this also destroys info data */
  gtk_widget_destroy (info->dialog);
  
  if (action == 1)
  {
    /* create new contact */
    create_new_contact_from_number (number);
  }
  else
  {
    add_number_to_contact (number);
  }
  g_free (number);
}

static void
btn_save_info_weak_notify (SaveButtonInfo *info, GObject *object)
{
  g_free (info->number);
  g_free (info);
}

void
moko_save_number (const gchar *number)
{
  GtkWidget *window, *btn, *vbox;
  SaveButtonInfo *btn_info;

  if (!number || !strcmp (number, ""))
  {
    GtkWidget *dlg;
    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  "No number available");
    gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);
    return;
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_title (GTK_WINDOW (window), number);
  
  vbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  btn = gtk_button_new_with_label ("Create New Contact");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), btn);
  btn_info = g_new0 (SaveButtonInfo, 1);
  btn_info->dialog = window;
  btn_info->response_id = 1;
  btn_info->number = g_strdup (number);
  g_signal_connect (btn, "clicked", G_CALLBACK (on_btn_save_clicked), btn_info);
  g_object_weak_ref (G_OBJECT (btn), (GWeakNotify) btn_save_info_weak_notify, btn_info);
  
  btn = gtk_button_new_with_label ("Add to Contact");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), btn);
  btn_info = g_new0 (SaveButtonInfo, 1);
  btn_info->dialog = window;
  btn_info->response_id = 2;
  btn_info->number = g_strdup (number);
  g_signal_connect (btn, "clicked", G_CALLBACK (on_btn_save_clicked), btn_info);
  g_object_weak_ref (G_OBJECT (btn), (GWeakNotify) btn_save_info_weak_notify, btn_info);
  
  gtk_widget_show_all (window);
}

