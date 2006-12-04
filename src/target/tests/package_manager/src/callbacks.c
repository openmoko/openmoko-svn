#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "errorcode.h"
#include "winresize.h"
#include "ipkgapi.h"
#include "pkgmgr.h"
#include "detareamgr.h"

static GtkWidget *entrysearch;

void
on_winappmgr_size_allocate             (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data)
{
}


void
on_tvpackage_cursor_changed            (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
  gint status;

  update_details_area (GTK_WIDGET (treeview));

  status = get_first_selected_package_status (GTK_WIDGET (treeview));

  update_button_action_status (GTK_WIDGET (treeview), status);
}

void
on_winappmgr_destroy                   (GtkObject       *object,
                                        gpointer         user_data)
{
  ipkg_uninitialize();

  gtk_main_quit();
}



void
on_buttonall_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
}


void
on_entrysearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
  gchar *name;

  name = gtk_editable_get_chars (editable, 0, -1);

  search_package_from_package_list (entrysearch, name);
/*
  if (strlen (name) == 0)
    {
      change_package_list (entrysearch, get_package_view_status());
    }
  else
    {
      display_search_package_list (entrysearch, name);
    }
*/

  if (name != NULL)
    g_free (name);
}


gboolean
on_entrysearch_focus                   (GtkWidget       *widget,
                                        GtkDirectionType  direction,
                                        gpointer         user_data)
{
  DBG("on_entrysearch_focus\n");
  return FALSE;
}


gboolean
on_entrysearch_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
  //entrysearch = NULL;

  //DBG("on_entrysearch_focus_out_event\n");
  return FALSE;
}


gboolean
on_entrysearch_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
  entrysearch = widget;
  //DBG("on_entrysearch_focus_in_event\n");

  return FALSE;
}


void
on_buttonsearch_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
/*
  const gchar *name;
  gchar *pkgname;

  name = get_user_input_string (GTK_WIDGET (button));

  pkgname = g_malloc (strlen(name) +1);

  strcpy (pkgname, name);

  display_search_package_list (GTK_WIDGET (button), pkgname);

  g_free (pkgname);
*/
}

void
on_buttonpackage_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  roll_package_view_status (GTK_WIDGET (button));
}


void
on_buttonaction_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  add_remove_package (GTK_WIDGET (button));
}

