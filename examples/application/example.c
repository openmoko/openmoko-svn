/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <gtk/gtk.h>


/* type definitions */

typedef struct
{
  GtkWidget *search_entry;
  GtkWidget *filter_combo;
} ApplicationData;

/* signal callbacks */

static void search_toggle_cb (GtkWidget * button, ApplicationData * data);

int
main (int argc, char **argv)
{
  GtkWidget *window, *notebook, *icon;
  GtkWidget *box, *hbox, *toolbar, *details, *navigation, *w;
  GtkTreeViewColumn *column;
  GtkWidget *widget;
  GtkToolItem *toolitem;
  GtkListStore *liststore;
  GtkTreeIter it;
  ApplicationData *data;

  gtk_init (&argc, &argv);

  data = g_new0 (ApplicationData, 1);

  /* main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete-event",
                    (GCallback) gtk_main_quit, NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Example");

  /* main notebook */
  notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);

  /* navigation */
  box = gtk_vbox_new (FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box,
                            gtk_image_new_from_stock (GTK_STOCK_INDEX,
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), box, "tab-expand",
                           TRUE, NULL);

  /* toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (box), toolbar, FALSE, FALSE, 0);

  toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_NEW);
  gtk_tool_item_set_expand (toolitem, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (),
                      1);

  toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_tool_item_set_expand (toolitem, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 2);

  /* search/filter bar */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  w = gtk_toggle_button_new ();
  g_signal_connect (G_OBJECT (w), "toggled", (GCallback) search_toggle_cb,
                    data);
  gtk_button_set_image (GTK_BUTTON (w),
                        gtk_image_new_from_stock (GTK_STOCK_FIND,
                                                  GTK_ICON_SIZE_SMALL_TOOLBAR));
  gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);

  data->search_entry = gtk_entry_new ();
  g_object_set (G_OBJECT (data->search_entry), "no-show-all", TRUE, NULL);
  gtk_box_pack_start (GTK_BOX (hbox), data->search_entry, TRUE, TRUE, 0);

  data->filter_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (data->filter_combo),
                             "Filter Menu");
  gtk_combo_box_append_text (GTK_COMBO_BOX (data->filter_combo), "Small");
  gtk_combo_box_append_text (GTK_COMBO_BOX (data->filter_combo), "Medium");
  gtk_combo_box_append_text (GTK_COMBO_BOX (data->filter_combo), "Large");
  gtk_combo_box_set_active (GTK_COMBO_BOX (data->filter_combo), 0);
  gtk_box_pack_start (GTK_BOX (hbox), data->filter_combo, TRUE, TRUE, 0);


  /* list */
  w = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (w),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box), w, TRUE, TRUE, 0);


  liststore = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_list_store_insert_with_values (liststore, &it, 0, 0, "One", -1);
  gtk_list_store_insert_with_values (liststore, &it, 1, 0, "Two", -1);
  gtk_list_store_insert_with_values (liststore, &it, 2, 0, "Skip a few", -1);
  gtk_list_store_insert_with_values (liststore, &it, 3, 0, "Ninety Nine", -1);
  gtk_list_store_insert_with_values (liststore, &it, 4, 0, "One Hundred", -1);

  navigation = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore));
  column = gtk_tree_view_column_new_with_attributes ("Counting",
                                                     gtk_cell_renderer_text_new
                                                     (), "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (navigation), column);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (navigation), FALSE);
  gtk_container_add (GTK_CONTAINER (w), navigation);


  /* details */
  details = gtk_vbox_new (FALSE, 6);
  
  widget = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (widget), "Hello, I am an entry");
  gtk_box_pack_start (GTK_BOX (details), widget, FALSE, FALSE, 0);

  widget = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_box_pack_start (GTK_BOX (details), widget, FALSE, FALSE, 0);

  widget = gtk_check_button_new_with_label ("Checkbutton");
  gtk_box_pack_start (GTK_BOX (details), widget, FALSE, FALSE, 0);

  widget = gtk_spin_button_new_with_range (0, 100, 1);
  gtk_box_pack_start (GTK_BOX (details), widget, FALSE, FALSE, 0);
  
  widget = gtk_radio_button_new_with_label (NULL, "RadioButton");
  gtk_box_pack_start (GTK_BOX (details), widget, FALSE, FALSE, 0);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), details,
                            gtk_image_new_from_stock (GTK_STOCK_EDIT,
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), details, "tab-expand",
                           TRUE, NULL);


  /* let's go! */
  gtk_widget_show_all (window);
  gtk_main ();

  g_free (data);
  return 0;
}

/* signal callbacks */

static void
search_toggle_cb (GtkWidget * button, ApplicationData * data)
{
  gboolean search;

  search = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

  if (search)
    {
      gtk_widget_show (data->search_entry);
      gtk_widget_hide (data->filter_combo);
    }
  else
    {
      gtk_widget_show (data->filter_combo);
      gtk_widget_hide (data->search_entry);
    }
}
