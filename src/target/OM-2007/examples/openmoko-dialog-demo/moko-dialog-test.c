#include <libmokoui/moko-dialog.h>


int
main (int argc, char *argv[])
{
  GtkWidget *d, *b1, *b2, *b3;
  GtkSettings *s;

  gtk_init (&argc, &argv);

  s = gtk_settings_get_default ();
  g_object_set (G_OBJECT(s), "gtk-theme-name", "openmoko-standard", NULL);

  d = moko_dialog_new ();

  moko_dialog_add_buttons (d, "Open", 0, NULL);

  b1 = moko_dialog_add_button (d, GTK_STOCK_GO_BACK, 0);
  b2 = moko_dialog_add_button (d, GTK_STOCK_GO_FORWARD, 0);

  gtk_button_box_set_child_secondary (MOKO_DIALOG (d)->action_area, b1, TRUE);
  gtk_button_box_set_child_secondary (MOKO_DIALOG (d)->action_area, b2, TRUE);

  moko_dialog_run (d);


}
