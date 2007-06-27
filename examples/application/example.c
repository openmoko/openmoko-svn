#include <gtk/gtk.h>


int
main (int argc, char **argv)
{
  GtkWidget *window, *notebook, *box, *toolbar, *details, *navigation, *icon;
  GtkToolItem *toolitem;

  gtk_init (&argc, &argv);

  /* main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete-event",
                    (GCallback) gtk_main_quit, NULL);

  /* main notebook */
  notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);

  /* navigation */
  box = gtk_vbox_new (FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box,
                            gtk_image_new_from_stock (GTK_STOCK_INDEX,
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), box, "tab-expand", TRUE,
                           NULL);


  toolbar = gtk_toolbar_new ();
  gtk_box_pack_start (GTK_BOX (box), toolbar, FALSE, FALSE, 0);

  toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_NEW);
  gtk_tool_item_set_expand (toolitem, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 0);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);

  toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_tool_item_set_expand (toolitem, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 2);



  navigation = gtk_tree_view_new ();
  gtk_box_pack_start (GTK_BOX (box), navigation, TRUE, TRUE, 0);


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
