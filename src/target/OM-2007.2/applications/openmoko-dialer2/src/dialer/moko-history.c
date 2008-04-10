/*
 *  moko-history; a Call History view; Adapted from the original 
 *  dialer-window-history code authored by Tony Guan <tonyguan@fic-sh.com.cn>
 *  and OpenedHand Ltd.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <libjana-gtk/jana-gtk.h>

#include <string.h>

#include <moko-journal.h>
#include <moko-stock.h>
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

#include "moko-contacts.h"
#include "moko-history.h"

G_DEFINE_TYPE (MokoHistory, moko_history, GTK_TYPE_VBOX)

#define MOKO_HISTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_HISTORY, MokoHistoryPrivate))

#define HISTORY_MAX_ENTRIES 50

#define HISTORY_CALL_INCOMING_ICON "moko-history-call-in"
#define HISTORY_CALL_OUTGOING_ICON "moko-history-call-out"
#define HISTORY_CALL_MISSED_ICON   "moko-history-call-missed"

#define SMS_NAMESPACE "org.openmoko.OpenmokoMessages2"
#define SMS_OBJECT "/org/openmoko/OpenmokoMessages2"

enum
{
  CALL_INCOMING = 0,
  CALL_OUTGOING,
  CALL_MISSED,

  N_CALL_TYPES
};

static gchar *icon_names[N_CALL_TYPES]  = {"moko-history-call-in",
                                           "moko-history-call-out",
                                           "moko-history-call-missed"};
static GdkPixbuf *icons[N_CALL_TYPES] = {NULL, NULL, NULL};

struct _SaveButtonInfo
{
  GtkWidget *dialog;
  gint response_id;
  gchar *number;
  MokoHistory *history;
};

typedef struct _SaveButtonInfo SaveButtonInfo;

struct _MokoHistoryPrivate
{
  MokoJournal       *journal;
  
  GtkToolItem       *save_button;
  GtkToolItem       *delete_button;
  GtkToolItem       *sms_button;
  GtkToolItem       *dial_button;
  
  GtkWidget         *save_menu;

  GtkWidget         *treeview;
  GtkWidget         *combo;

  GtkTreeModel      *main_model;
  GtkTreeModel      *sort_model;
  GtkTreeModel      *filter_model;

};

enum
{
  DIAL_NUMBER,

  LAST_SIGNAL
};

static guint history_signals[LAST_SIGNAL] = {0, };

enum
{
  PROP_JOURNAL=1
};

enum history_columns 
{
  NUMBER_COLUMN = 0,
  DSTART_COLUMN,
  ICON_NAME_COLUMN,
  DISPLAY_TEXT_COLUMN,
  CALL_DETAILS_COLUMN,
  CALL_TYPE_COLUMN,
  ENTRY_POINTER_COLUMN
};

void
moko_history_set_filter (MokoHistory *history, gint filter)
{
  MokoHistoryPrivate *priv;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo), filter);
}

static void
on_dial_clicked (GtkWidget *button, MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeView *treeview;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *number;
 
  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  treeview = GTK_TREE_VIEW (priv->treeview);
  selection = gtk_tree_view_get_selection (treeview);
  model = gtk_tree_view_get_model (treeview);

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, NUMBER_COLUMN, &number, -1);

  g_signal_emit (G_OBJECT (history), history_signals[DIAL_NUMBER], 0, number);

  g_free (number);
}

static void
on_sms_clicked (GtkWidget *button, MokoHistory *history)
{
  DBusGConnection *conn;
  DBusGProxy *proxy;
  GError *err = NULL;
  MokoHistoryPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeView *treeview;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *number;
  
  g_debug ("sms clicked");

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  treeview = GTK_TREE_VIEW (priv->treeview);
  selection = gtk_tree_view_get_selection (treeview);
  model = gtk_tree_view_get_model (treeview);

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, NUMBER_COLUMN, &number, -1);

  g_debug ("send SMS to number: %s", number);

  conn = dbus_g_bus_get (DBUS_BUS_SESSION, &err);
  if (conn == NULL)
  {
    g_warning ("Failed to make DBus connection: %s", err->message);
    g_error_free (err);
    return;
  }

  proxy = dbus_g_proxy_new_for_name (conn,
                                     SMS_NAMESPACE,
                                     SMS_OBJECT,
                                     SMS_NAMESPACE);
  if (proxy == NULL)
  {
    g_warning ("Unable to get openmoko-messages2 object");
    return;
  }

  err = NULL;
  dbus_g_proxy_call (proxy, "SendMessage", &err,
                     G_TYPE_STRING, NULL, G_TYPE_STRING, number,
                     G_TYPE_STRING, NULL,
                     G_TYPE_INVALID, G_TYPE_INVALID);

  if (err)
  {
    g_warning (err->message);
    g_error_free (err);
  }
  
}

static void
create_new_contact_from_number (gchar *number)
{
  GtkWidget *dialog, *name, *label;

  dialog = gtk_dialog_new_with_buttons ("Save as Contact",
             NULL, GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
             GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

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

    /* create contact */
    contact = e_contact_new ();
    /* add name */
    e_contact_set (contact, E_CONTACT_FULL_NAME, gtk_entry_get_text (GTK_ENTRY (name)));
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
  MokoHistory *history = info->history;
    
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

static void
on_save_clicked (GtkWidget *button, MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeView *treeview;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *number;
  GtkWidget *window, *btn;
  SaveButtonInfo *btn_info;
  
  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  treeview = GTK_TREE_VIEW (priv->treeview);
  selection = gtk_tree_view_get_selection (treeview);
  model = gtk_tree_view_get_model (treeview);

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;
  

  gtk_tree_model_get (model, &iter, NUMBER_COLUMN, &number, -1);
 
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_title (GTK_WINDOW (window), number);
  
  GtkWidget *vbox;
  vbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  btn = gtk_button_new_with_label ("Create New Contact");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), btn);
  btn_info = g_new0 (SaveButtonInfo, 1);
  btn_info->dialog = window;
  btn_info->response_id = 1;
  btn_info->history = history;
  btn_info->number = g_strdup (number);
  g_signal_connect (btn, "clicked", G_CALLBACK (on_btn_save_clicked), btn_info);
  g_object_weak_ref (G_OBJECT (btn), (GWeakNotify) btn_save_info_weak_notify, btn_info);
  
  btn = gtk_button_new_with_label ("Add to Contact");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), btn);
  btn_info = g_new0 (SaveButtonInfo, 1);
  btn_info->dialog = window;
  btn_info->response_id = 2;
  btn_info->history = history;
  btn_info->number = g_strdup (number);
  g_signal_connect (btn, "clicked", G_CALLBACK (on_btn_save_clicked), btn_info);
  g_object_weak_ref (G_OBJECT (btn), (GWeakNotify) btn_save_info_weak_notify, btn_info);
  
  g_free (number);
  
  gtk_widget_show_all (window);
}

static void
on_delete_clicked (GtkWidget *button, MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkWidget *dialog;
  GtkTreeIter iter0;
  GtkTreeIter iter1;
  GtkTreeIter iter2;
  GtkTreeModel *filtered;
  GtkTreeModel *sorted;
  GtkTreeModel *store;
  GtkTreeSelection *selection;
  GtkTreeView *treeview;
  GtkTreePath *path;
  const gchar *uid;
  gint result = 0;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  treeview = GTK_TREE_VIEW (priv->treeview);
  selection = gtk_tree_view_get_selection (treeview);

  if (!gtk_tree_selection_get_selected (selection, &filtered, &iter0))
    return;

  gtk_tree_model_get (filtered, &iter0, ENTRY_POINTER_COLUMN, &uid, -1);

  /* Create a dialog */
  dialog = gtk_message_dialog_new (GTK_WINDOW (
                                   gtk_widget_get_ancestor(GTK_WIDGET (history),
                                                            GTK_TYPE_WINDOW)),
                                   0,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_NONE,
                      "Are you sure you want to permanantly delete this call?"
                                   );

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          "Don't Delete", GTK_RESPONSE_CANCEL,
                          GTK_STOCK_DELETE, GTK_RESPONSE_YES,
                          NULL);
  gtk_widget_set_name (dialog, "mokomessagedialog");
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 0);

  /* Just some tests
  gtk_widget_set_size_request (dialog, 
                               GTK_WIDGET (history)->allocation.width,
                               GTK_WIDGET (history)->allocation.height);

  gtk_window_move (GTK_WINDOW (dialog),
                   GTK_WIDGET (history)->allocation.x,
                   GTK_WIDGET (history)->allocation.y);
  */
  
  result = gtk_dialog_run (GTK_DIALOG (dialog));
  switch (result)
  {
    case GTK_RESPONSE_YES:
      break;
    default:
      gtk_widget_destroy (dialog);
      return;
      break;
  }

  /* Remove the entry from the journal */
  if (moko_journal_remove_entry_by_uid (priv->journal, uid))
    moko_journal_write_to_storage (priv->journal);

  /* Remove the row from the list store */
  path = gtk_tree_model_get_path (filtered, &iter0);
  sorted = priv->sort_model;
  gtk_tree_model_filter_convert_iter_to_child_iter (
                                              GTK_TREE_MODEL_FILTER (filtered),
                                              &iter1, &iter0);

  store = priv->main_model;
  gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (sorted),
                                                  &iter2, &iter1);
  gtk_list_store_remove (GTK_LIST_STORE (store), &iter2);
  gtk_tree_view_set_cursor (treeview, path, 0, 0);

  /* Clean up */
  gtk_tree_path_free (path);
  gtk_widget_destroy (dialog);
}


static gboolean
history_add_entry (GtkListStore *store, MokoJournalEntry *entry)
{
  GtkTreeIter iter;
  const gchar *uid, *number;
  MokoContactEntry *contacts;
  GdkPixbuf *icon = NULL;
  const gchar *display_text;
  const gchar *details;
  time_t dstart, dend, duration;
  MessageDirection direction;
  gboolean was_missed;
  const MokoTime *time;
  gchar dstart_str[256];
  gchar duration_str[9];
  gint type;

  uid = moko_journal_entry_get_uid (entry);
  moko_journal_entry_get_direction (entry, &direction);
  time = moko_journal_entry_get_dtstart (entry);
  dstart = moko_time_as_timet (time);
  time = moko_journal_entry_get_dtend (entry);
  dend = moko_time_as_timet (time);
  duration = dend - dstart;
  
  was_missed = moko_journal_voice_info_get_was_missed (entry);
  number = moko_journal_voice_info_get_distant_number (entry);

  /* Load the correct icon */
  if (direction == DIRECTION_OUT)
  {
    icon = icons[CALL_OUTGOING];
    type = HISTORY_FILTER_DIALED;
  }
  else
  {
    if (was_missed)
    {
      icon = icons[CALL_MISSED];
      type = HISTORY_FILTER_MISSED;
    }
    else
    {
      icon = icons[CALL_INCOMING];
      type = HISTORY_FILTER_RECEIVED;
    }
  }

  /* display text should be the contact name or the number dialed */
  contacts = moko_contacts_lookup (moko_contacts_get_default (), number);   
  if (contacts)
    display_text = contacts->contact->name;
  else
  {
    if (number == NULL || !strcmp(number, "") || !strcmp(number, "NULL"))
      display_text = "Unknown number";
    else
      display_text = number;
  }

  strftime (dstart_str, sizeof (dstart_str), "%d/%m/%Y %H:%M:%S",
	    localtime (&dstart));
  strftime (duration_str, sizeof (duration_str), "%H:%M:%S", gmtime (&duration));
  details = g_strdup_printf ("%s\t\t%s", dstart_str, duration_str);

  if (display_text == NULL || uid == NULL)
  {
    /*g_debug ("Not adding");
    return FALSE;*/
  }
  gtk_list_store_insert_with_values (store, &iter, 0,
    NUMBER_COLUMN, number,
    DSTART_COLUMN, dstart,
    ICON_NAME_COLUMN, icon,
    DISPLAY_TEXT_COLUMN, display_text,
    CALL_DETAILS_COLUMN, details,
    CALL_TYPE_COLUMN, type,
    ENTRY_POINTER_COLUMN, uid,
    -1);

  return TRUE;
}

static void
on_entry_added_cb (MokoJournal *journal,
                   MokoJournalEntry *entry,
                   MokoHistory *history)
{
  MokoHistoryPrivate *priv;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  if (moko_journal_entry_get_entry_type (entry) != VOICE_JOURNAL_ENTRY)
    return;

  history_add_entry (GTK_LIST_STORE (priv->main_model), entry);
}

static gboolean
moko_history_filter_visible_func (GtkTreeModel *model, 
                                  GtkTreeIter  *iter,
                                  MokoHistory  *history)
{
  MokoHistoryPrivate *priv;
  gint type;
  gint active;

  g_return_val_if_fail (MOKO_IS_HISTORY (history), TRUE);
  priv = history->priv;

  active = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo));

  if (active == HISTORY_FILTER_ALL)
    return TRUE;

  gtk_tree_model_get (model, iter, CALL_TYPE_COLUMN, &type, -1);

  if (active == type)
    return TRUE;

  return FALSE;
}

static void
on_filter_changed (GtkWidget *combo, MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkTreeView *treeview;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *path;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (priv->filter_model));


  treeview = GTK_TREE_VIEW (priv->treeview);
  model = gtk_tree_view_get_model (treeview);

  if (!gtk_tree_model_get_iter_first (model, &iter))
    return;
  path = gtk_tree_model_get_path (model, &iter);
  gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
  gtk_tree_path_free (path);

}

void
on_tree_selection_changed (GtkTreeSelection *selection, MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean selected;
  
  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  
  selected = gtk_tree_selection_get_selected (selection, &model, &iter);

  gtk_widget_set_sensitive (GTK_WIDGET (priv->dial_button), selected);
  gtk_widget_set_sensitive (GTK_WIDGET (priv->sms_button), selected);
  gtk_widget_set_sensitive (GTK_WIDGET (priv->save_button), selected);
  gtk_widget_set_sensitive (GTK_WIDGET (priv->delete_button), selected);
}

static gint
sort_by_date (MokoJournalEntry *a, MokoJournalEntry *b)
{
  const MokoTime *at, *bt;
  time_t ta, tb;

  at = moko_journal_entry_get_dtstart (a);
  bt = moko_journal_entry_get_dtstart (b);

  ta = moko_time_as_timet (at);
  tb = moko_time_as_timet (bt);

  return (gint)difftime (ta, tb);
}

static void
moko_history_load_entries (MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkListStore *store;
  GtkTreeModel *sorted;
  GtkTreeModel *filtered;
  GtkCellRenderer *renderer;
  MokoJournalEntry *entry;
  gint i, j, n_entries;
  GList *entries = NULL, *e;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  /* Create renderer and column */
  renderer = jana_gtk_cell_renderer_note_new ();
  g_object_set (G_OBJECT (renderer), "show_created", FALSE,
	            "show_recipient", FALSE, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (priv->treeview),
		0, NULL, renderer, "author", DISPLAY_TEXT_COLUMN, "body",
		CALL_DETAILS_COLUMN, "icon", ICON_NAME_COLUMN, NULL);

  g_signal_connect (priv->treeview, "size-allocate",
		G_CALLBACK (jana_gtk_utils_treeview_resize), renderer);

  /* Set up the list store */
  store = gtk_list_store_new (7, G_TYPE_STRING,
                                 G_TYPE_INT,
                                 GDK_TYPE_PIXBUF,
                                 G_TYPE_STRING,
                                 G_TYPE_STRING,
                                 G_TYPE_INT,
                                 G_TYPE_STRING);
  priv->main_model = GTK_TREE_MODEL (store);

  sorted = gtk_tree_model_sort_new_with_model (priv->main_model);
  priv->sort_model = sorted;
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sorted), 
                                       DSTART_COLUMN,
                                       GTK_SORT_DESCENDING);

  /* Set up the filtered column */
  filtered = gtk_tree_model_filter_new (sorted, NULL);
  priv->filter_model = filtered;
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filtered),
               (GtkTreeModelFilterVisibleFunc) moko_history_filter_visible_func,
                                          history,
                                          NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview), filtered);

  g_signal_connect (priv->journal, "entry_added",
                    G_CALLBACK (on_entry_added_cb), (gpointer)history);
  
  n_entries = moko_journal_get_nb_entries (priv->journal);
  if (n_entries < 1)
  {
    g_debug ("The Journal is empty");
    return;
  }

  i = j = 0;
  for (i = 0; i < n_entries; i++)
  {
    moko_journal_get_entry_at (priv->journal, i, &entry);
    
    /* We are not interested in anything other than voice entries */
    if (moko_journal_entry_get_entry_type (entry) != VOICE_JOURNAL_ENTRY)
      continue;

    entries = g_list_insert_sorted (entries, 
                                    (gpointer)entry, 
                                    (GCompareFunc)sort_by_date);
  }

  for (e = entries; e != NULL; e = e->next)
  {
    if (history_add_entry (store, e->data))
      j++;
  }
}

/* GObject functions */
static void
moko_history_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_history_parent_class)->dispose (object);
}

static void
moko_history_finalize (GObject *history)
{
  G_OBJECT_CLASS (moko_history_parent_class)->finalize (history);
}

static void
moko_history_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  MokoHistoryPrivate *priv;

  g_return_if_fail (MOKO_IS_HISTORY (object));
  priv = (MOKO_HISTORY (object))->priv;

  switch (prop_id)
  {
    case PROP_JOURNAL:
      priv->journal = (MokoJournal *)g_value_get_pointer (value);
      if (priv->journal)
        moko_history_load_entries (MOKO_HISTORY (object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
moko_history_get_property (GObject    *object, 
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  MokoHistoryPrivate *priv;

  g_return_if_fail (MOKO_IS_HISTORY (object));
  priv = (MOKO_HISTORY (object))->priv;

  switch (prop_id)
  {
    case PROP_JOURNAL:
      g_value_set_pointer (value, priv->journal);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
moko_history_class_init (MokoHistoryClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_history_finalize;
  obj_class->dispose = moko_history_dispose;
  obj_class->set_property = moko_history_set_property;
  obj_class->get_property = moko_history_get_property;

  g_object_class_install_property (
    obj_class,
    PROP_JOURNAL,
    g_param_spec_pointer ("journal",
                         "MokoJournal",
                         "A MokoJournal Object",
                         G_PARAM_CONSTRUCT|G_PARAM_READWRITE));
  history_signals[DIAL_NUMBER] =
    g_signal_new ("dial_number", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoHistoryClass, dial_number),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  g_type_class_add_private (obj_class, sizeof (MokoHistoryPrivate)); 
}

static void
moko_history_init (MokoHistory *history)
{
  MokoHistoryPrivate *priv;
  GtkIconTheme *theme;
  gint i;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *toolbar, *combo, *image, *scroll;
  GtkToolItem *item;
  GtkCellRenderer *renderer;
  GdkPixbuf *icon;


  priv = history->priv = MOKO_HISTORY_GET_PRIVATE (history);

  /* Create the icons */
  theme = gtk_icon_theme_get_default ();
  for (i = 0; i < N_CALL_TYPES; i++)
  {
    icons[i] = gtk_icon_theme_load_icon (theme,
                                         icon_names[i],
                                         GTK_ICON_SIZE_MENU,
                                         0, NULL);
  }

  /* Toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (history), toolbar, FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/phone.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "Dial");
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_dial_clicked), (gpointer)history);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  priv->dial_button = item;

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/sms.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "SMS");
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_sms_clicked), (gpointer)history); 
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  priv->sms_button = item;

  item = gtk_tool_button_new (NULL, NULL);
  gtk_tool_button_set_icon_name (item, "contact-new");
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (item, "clicked", 
                    G_CALLBACK (on_save_clicked), history); 
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  priv->save_button = item;
  
  item = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_delete_clicked), (gpointer)history); 
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  priv->delete_button = item;
  
  /* Filter combo */
  store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT);
  
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/received.png", NULL);
  gtk_list_store_insert_with_values (store, &iter, 0, 
                                     0, icon, 
                                     1, "Call History - Received",
                                     2, HISTORY_FILTER_RECEIVED,
                                     -1);  
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/dialed.png", NULL);
  gtk_list_store_insert_with_values (store, &iter, 0, 
                                     0, icon, 
                                     1, "Call History - Dialed",
                                     2, HISTORY_FILTER_DIALED,
                                     -1);  

  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/missed.png", NULL);
  gtk_list_store_insert_with_values (store, &iter, 0, 
                                     0, icon, 
                                     1, "Call History - Missed",
                                     2, HISTORY_FILTER_MISSED,
                                     -1);
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/all.png", NULL);
  gtk_list_store_insert_with_values (store, &iter, 0, 
                                     0, icon, 
                                     1, "Call History - All",
                                     2, HISTORY_FILTER_ALL,
                                     -1);
  
  
  /* add to contact/save menu */
  GtkWidget *menu_item;
  
  priv->save_menu = gtk_menu_new ();
  gtk_menu_attach_to_widget (GTK_MENU (priv->save_menu), GTK_WIDGET (priv->save_button), NULL);

  menu_item = gtk_menu_item_new_with_label ("New Contact");
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->save_menu), menu_item);
  
  menu_item = gtk_menu_item_new_with_label ("Add to Contact");
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->save_menu), menu_item);
  
  gtk_widget_show_all (priv->save_menu);
  
  
  /* filter combo */
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  priv->combo  = combo;
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (G_OBJECT (combo), "changed", 
                    G_CALLBACK (on_filter_changed), history);
  
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer, 
                                  "pixbuf", 0,
                                  NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer), "xpad", 10, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                  "text", 1,
                                  NULL);

  gtk_box_pack_start (GTK_BOX (history), combo, FALSE, FALSE, 0);

  /* Treeview */
  scroll = moko_finger_scroll_new ();
  gtk_box_pack_start (GTK_BOX (history), scroll, TRUE, TRUE, 0);

  priv->treeview = gtk_tree_view_new ();
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->treeview), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_container_add (GTK_CONTAINER (scroll), priv->treeview);
  
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview)),
		    "changed", G_CALLBACK (on_tree_selection_changed), history);

  gtk_widget_show_all (GTK_WIDGET (history));
}

GtkWidget*
moko_history_new (MokoJournal *journal)
{
  MokoHistory *history = NULL;
    
  history = g_object_new (MOKO_TYPE_HISTORY,
                          "journal", journal,
                          NULL);

  return GTK_WIDGET (history);
}
