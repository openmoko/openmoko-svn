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
};

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

static void
moko_history_load_entries (MokoHistory *history)
{
  g_print ("Loading entries\n");
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
  GtkWidget *toolbar, *combo, *treeview;
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

  item = gtk_tool_button_new_from_stock (MOKO_STOCK_CALL_DIAL);
  gtk_tool_item_set_expand (item, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);
  
  item = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_tool_item_set_expand (item, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 2);
  
  /* Test combobox */
  store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT);
  
  icon = gdk_pixbuf_new_from_file (PKGDATADIR"/all.png", NULL);
  gtk_list_store_insert_with_values (store, &iter, 0, 
                                     0, icon, 
                                     1, "Call History - All",
                                     2, HISTORY_FILTER_ALL,
                                     -1);

  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer, 
                                  "pixbuf", 0,
                                  NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                  "text", 1,
                                  NULL);

  gtk_box_pack_start (GTK_BOX (history), combo, FALSE, FALSE, 0);

  treeview = gtk_tree_view_new ();
  gtk_box_pack_start (GTK_BOX (history), treeview, TRUE, TRUE, 0);
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
