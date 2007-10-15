/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"

int
main (int argc, char *argv[])
{
  GtkWidget *window1;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);
  gtk_rc_parse("./openmoko.rc");
  add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  window1 = create_window1 ();
  gtk_widget_set_name (window1, "main");
  gtk_window_set_resizable(GTK_WINDOW(window1),FALSE);
  gtk_widget_show (window1);
  display(GTK_WIDGET(window1));

  /*
   * if there is a input argument such as the phone number
   * it will display the number in the input entry after label "To"
   */
  if(argc==2)
  {
	  GtkWidget * entry;
	  entry = lookup_widget(GTK_WIDGET(window1),"sms_to_entry");
	  gtk_entry_set_text(GTK_ENTRY(entry),argv[1]);
  }

  gtk_main ();
  return 0;
}