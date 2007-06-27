#include <gtk/gtk.h>


int
main (int argc, char **argv)
{
  GtkWidget *window, *notebook, *icon;
  GtkWidget *box, *toolbar, *details, *navigation, *w;
  GtkTreeViewColumn *column;
  GtkToolItem *toolitem;
  GtkListStore *liststore;
  GtkTreeIter it;

  gtk_init (&argc, &argv);

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

  w = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (w),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box), w, TRUE, TRUE, 0);

  /* list */
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
  details = gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_DIALOG);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), details,
                            gtk_image_new_from_stock (GTK_STOCK_EDIT,
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), details, "tab-expand",
                           TRUE, NULL);


  /* let's go! */
  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}
