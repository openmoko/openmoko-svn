#include <gtk/gtk.h>


int
main (int argc, char **argv)
{
  GtkWidget *window, *notebook, *toolbar, *details, *navigation, *icon;

  gtk_init (&argc, &argv);

  /* main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete-event", (GCallback) gtk_main_quit, NULL);

  /* main notebook */
  notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);

  /* navigation */
  navigation = gtk_tree_view_new ();
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), navigation, gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), navigation, "tab-expand", TRUE, NULL);

  /* details */
  details = gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_DIALOG);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), details, gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), details, "tab-expand", TRUE, NULL);


  /* let's go! */
  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}
