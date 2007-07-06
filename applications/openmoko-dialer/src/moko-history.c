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

#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokojournal/moko-journal.h>
#include <libmokoui/moko-stock.h>

#include "moko-history.h"

G_DEFINE_TYPE (MokoHistory, moko_history, GTK_TYPE_VBOX)

#define MOKO_HISTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_HISTORY, MokoHistoryPrivate))

#define HISTORY_MAX_ENTRIES 50

#define HISTORY_CALL_INCOMING_ICON "moko-history-call-in"
#define HISTORY_CALL_OUTGOING_ICON "moko-history-call-out"
#define HISTORY_CALL_MISSED_ICON   "moko-history-call-missed"
 
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
  
struct _MokoHistoryPrivate
{
  MokoJournal       *journal;

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
  g_print ("sms clicked\n");
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
    ;//return;

  //gtk_tree_model_get (filtered, &iter0, ENTRY_POINTER_COLUMN, &uid, -1);

  /* Create a dialog */
  dialog = gtk_message_dialog_new (GTK_WINDOW (
                                   gtk_widget_get_ancestor(GTK_WIDGET (history),
                                                            GTK_TYPE_WINDOW)),
                                   0,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_NONE,
                      "Are you sure you want to permanantly delete this call?",
                                   NULL);

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
  GdkPixbuf *icon = NULL;
  const gchar *display_text;
  time_t dstart;
  enum MessageDirection direction;
  gboolean was_missed;
  const MokoTime *time;
  MokoJournalVoiceInfo *info = NULL;
  gint type;

  uid = moko_journal_entry_get_contact_uid (entry);
  moko_journal_entry_get_direction (entry, &direction);
  time = moko_journal_entry_get_dtstart (entry);
  dstart = moko_time_as_timet (time);
  
  moko_journal_entry_get_voice_info (entry, &info);
  was_missed = moko_journal_voice_info_get_was_missed (info);
  number = moko_journal_voice_info_get_distant_number (info);

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
  /* FIXME: look up contact uid if stored */
  display_text = number;

  if ( number == NULL || display_text == NULL || uid == NULL)
  {
    //g_print ("Not adding\n");
    //return FALSE;
  }
  gtk_list_store_insert_with_values (store, &iter, 0,
    NUMBER_COLUMN, number,
    DSTART_COLUMN, dstart,
    ICON_NAME_COLUMN, icon,
    DISPLAY_TEXT_COLUMN, display_text,
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

  if (moko_journal_entry_get_type (entry) != VOICE_JOURNAL_ENTRY)
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

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (priv->filter_model));
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
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;
  MokoJournalEntry *entry;
  gint i, j, n_entries;
  GList *entries = NULL, *e;

  g_return_if_fail (MOKO_IS_HISTORY (history));
  priv = history->priv;

  /* Create the columns */
  col = gtk_tree_view_column_new ();

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer, 
                                       "pixbuf", ICON_NAME_COLUMN,
                                       NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", DISPLAY_TEXT_COLUMN,
                                       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), col);

  /* Set up the list store */
  store = gtk_list_store_new (6, G_TYPE_STRING,
                                 G_TYPE_INT,
                                 GDK_TYPE_PIXBUF,
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

  moko_journal_set_entry_added_callback (priv->journal,
                                  (MokoJournalEntryAddedFunc)on_entry_added_cb,
                                          (gpointer)history);
  
  n_entries = moko_journal_get_nb_entries (priv->journal);
  if (n_entries < 1)
  {
    g_print ("The Journal is empty\n");
    return;
  }

  i = j = 0;
  for (i = 0; i < n_entries; i++)
  {
    moko_journal_get_entry_at (priv->journal, i, &entry);
    
    /* We are not interested in anything other than voice entries */
    if (moko_journal_entry_get_type (entry) != VOICE_JOURNAL_ENTRY)
      continue;

    entries = g_list_insert_sorted (entries, 
                                    (gpointer)entry, 
                                    (GCompareFunc)sort_by_date);
  }

  for (e = entries; e != NULL; e = e->next)
  {
    if (history_add_entry (store, entry))
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
  GtkWidget *toolbar, *combo, *treeview, *image, *scroll;
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
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);
  
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/sms.png", NULL);
  image = gtk_image_new_from_pixbuf (icon);
  item = gtk_tool_button_new (image, "SMS");
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_sms_clicked), (gpointer)history); 
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 2);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 3);
  
  item = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_tool_item_set_expand (item, TRUE);
  g_signal_connect (G_OBJECT (item), "clicked", 
                    G_CALLBACK (on_delete_clicked), (gpointer)history); 
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 4);
  
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
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (history), scroll, TRUE, TRUE, 0);

  treeview = priv->treeview = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  gtk_container_add (GTK_CONTAINER (scroll), treeview);

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
