#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "widgets.h"
#include "errorcode.h"
#include "pkglist.h"
#include "menumgr.h"
#include "alldialog.h"
#include "initwidgets.h"


void
on_showstatus_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_showsource_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_Install_single_application_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gchar *name = NULL;
  if (!create_file_selection (&name))
    {
      return;
    }
  install_from_ipk_file (name);

  g_free (name);
}


void
on_showhelp_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  free_all_dynamic_memory ();
  uninit_libipkg ();
  gtk_main_quit ();
}


void
on_appmgr_destroy                      (GtkObject       *object,
                                        gpointer         user_data)
{
  free_all_dynamic_memory ();
  uninit_libipkg ();
  gtk_main_quit ();
}

void
on_bmainmenu_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *menu;

  menu = get_widget_pointer (FIC_WIDGET_MENU_APPLICATION_MENU);
  if (menu == NULL)
    {
      g_error ("Can not find the main menu widget, fatal error.");
    }

  gtk_menu_popup ( GTK_MENU (menu), NULL, GTK_WIDGET (button),  NULL, 0, 0, 0);
}


void
on_bfilter_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *menu;

  menu = get_widget_pointer (FIC_WIDGET_MENU_FILTER_MENU);
  if (menu == NULL)
    {
      ERROR ("Can not find the filter menu widget, fatal error.");
      return;
    }
  set_sensitive_filter_menu (menu);

  gtk_menu_popup ( GTK_MENU (menu), NULL, GTK_WIDGET (button),  NULL, 0, 0, 0);

}


void
on_unmark_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  package_unmark_event ();
}


void
on_markinstall_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  package_mark_install_event ();
}


void
on_markupgrade_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  package_mark_upgrade_event ();
}


void
on_markremove_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  package_mark_remove_event ();
}


gboolean
on_treepkglist_button_press_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  if (event->type == GDK_BUTTON_PRESS)
    {
      GtkTreeSelection *selection;
      GtkTreeIter iter;
      GtkTreePath *path;
      GtkTreeModel  *model;
      GtkTreeViewColumn *column;
      GtkTreeViewColumn *firstcol;
      gpointer  pkg;
      

      if (!(event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))))
        {
          DBG ("Not a package list view event");
          return FALSE;
        }

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
                                         (int)event->x, (int)event->y,
                                         &path, &column, NULL, NULL))
        {
          firstcol = gtk_tree_view_get_column (GTK_TREE_VIEW (widget), 0);
          gtk_tree_selection_unselect_all (selection);
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (widget), path, NULL, FALSE);
          //gtk_tree_selection_select_path (selection, path);
          //update_detail_info (widget);

          if (!((event->button == 1) && (strcmp(firstcol->title, column->title) == 0)))
            {
              return TRUE;
            }
          DBG ("The column is the first column");

          if (gtk_tree_selection_get_selected (selection, &model, &iter))
            {
              gtk_tree_model_get (model, &iter, COL_POINTER, &pkg, -1);
              popup_select_menu (widget, event, pkg);
            }
          return TRUE;
        }
    }

  return FALSE;
}

void
on_treepkglist_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
  update_detail_info (GTK_WIDGET (treeview));
}

void
on_bapply_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *dialog;
  gint   res;

  if (check_marked_list_empty ())
    {
      show_message_to_user (_("No package that has been selected"));
      return;
    }

  dialog = init_apply_dialog ();

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_OK)
    {
      dispose_marked_package ();
    }

  gtk_widget_destroy (dialog);
}


void
on_entrysearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
  search_user_input ();
}

void
on_bupgrade_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  mark_all_upgrade ();
}


void
on_bdown_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *swpkg;
  gint  width, height;

  swpkg = get_widget_pointer (FIC_WIDGET_SCROLLED_WINDOW_PKG_LIST);
  if (swpkg == NULL)
    {
      DBG ("Can not find the scrolled window of package list");
      return;
    }
  gtk_widget_get_size_request (swpkg, &width, &height);
  DBG ("The height of scrolled window of pkg is:%d", height);

  if (height >= 390)
    {
      return;
    }

  height += 50;
  gtk_widget_set_size_request (swpkg, width, height);
}


void
on_bup_clicked                         (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *swpkg;
  gint  width, height;

  swpkg = get_widget_pointer (FIC_WIDGET_SCROLLED_WINDOW_PKG_LIST);
  if (swpkg == NULL)
    {
      DBG ("Can not find the scrolled window of package list");
      return;
    }
  gtk_widget_get_size_request (swpkg, &width, &height);

  if (height <= 140)
    {
      return;
    }

  height -= 50;
  gtk_widget_set_size_request (swpkg, width, height);

}


void
on_bsearchon_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget  *bsearch;
  GtkWidget  *bapply;
  GtkWidget  *bmode;
  GtkWidget  *bupgrade;
  GtkWidget  *entrysearch;
  GtkWidget  *fixedtoolbar;

  fixedtoolbar = get_widget_pointer (FIC_WIDGET_FIXED_TOOL_BAR);

  bsearch = get_widget_pointer (FIC_WIDGET_BUTTON_SEARCH);
  bapply = get_widget_pointer (FIC_WIDGET_BUTTON_APPLY);
  bmode = get_widget_pointer (FIC_WIDGET_BUTTON_MODE);
  bupgrade = get_widget_pointer (FIC_WIDGET_BUTTON_UPGRADE);
  entrysearch = get_widget_pointer (FIC_WIDGET_ENTRY_SEARCH);

  gtk_widget_hide (GTK_WIDGET (button));

  gtk_widget_show (bapply);
  gtk_widget_show (bmode);
  gtk_widget_show (bupgrade);

  gtk_widget_hide (entrysearch);
  gtk_widget_show (bsearch);

  gtk_widget_set_name (fixedtoolbar, "bg_normal_toolbar");
}


void
on_bsearch_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget  *bsearchon;
  GtkWidget  *bapply;
  GtkWidget  *bmode;
  GtkWidget  *bupgrade;
  GtkWidget  *entrysearch;
  GtkWidget  *fixedtoolbar;

  fixedtoolbar = get_widget_pointer (FIC_WIDGET_FIXED_TOOL_BAR);

  bsearchon = get_widget_pointer (FIC_WIDGET_BUTTON_SEARCHON);
  bapply = get_widget_pointer (FIC_WIDGET_BUTTON_APPLY);
  bmode = get_widget_pointer (FIC_WIDGET_BUTTON_MODE);
  bupgrade = get_widget_pointer (FIC_WIDGET_BUTTON_UPGRADE);
  entrysearch = get_widget_pointer (FIC_WIDGET_ENTRY_SEARCH);

  gtk_widget_hide (GTK_WIDGET (button));
  gtk_widget_hide (bapply);
  gtk_widget_hide (bmode);
  gtk_widget_hide (bupgrade);

  gtk_widget_show (entrysearch);
  gtk_widget_show (bsearchon);

  gtk_widget_grab_focus (entrysearch);
}


gboolean
on_entrysearch_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
  GtkWidget  *fixedtoolbar;

  gtk_widget_set_name (widget, "entry_search_toolbar");

  fixedtoolbar = get_widget_pointer (FIC_WIDGET_FIXED_TOOL_BAR);
  gtk_widget_set_name (fixedtoolbar, "bg_search_toolbar");

  return FALSE;
}


gboolean
on_entrysearch_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
  GtkWidget  *fixedtoolbar;

  gtk_widget_set_name (widget, "entry_search_toolbar_focusout");

  fixedtoolbar = get_widget_pointer (FIC_WIDGET_FIXED_TOOL_BAR);
  gtk_widget_set_name (fixedtoolbar, "bg_search_toolbar_focusout");

  return FALSE;
}


gboolean
on_bbarsep_button_release_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  if (event->type == GDK_BUTTON_RELEASE)
    {
      GtkWidget *viewportpkglist;
      GtkWidget *viewportdetail;
      gint x,y;
      gint stepper;

      gtk_widget_get_size_request (GTK_WIDGET (widget), &x, &y);
      y = x/2;

      viewportpkglist = lookup_widget (widget, "viewportpkglist");
      if (viewportpkglist == NULL)
        {
          ERROR ("Can not find the widget of viewportpkglist");
          return FALSE;
        }

      if (!GTK_WIDGET_VISIBLE (viewportpkglist))
        {
          gtk_widget_show (viewportpkglist);
          resize_navigation_detail_area (viewportpkglist, 0);
          return FALSE;
        }

      if ((int)event->x < y)
        {
          //The button is on the left
          gtk_widget_get_size_request (viewportpkglist, &x, &y);

          if ((y - STEPER_HEIGHT) < MIN_NAVIGATION_LIST_HEIGHT)
            {
              if (y == MIN_NAVIGATION_LIST_HEIGHT)
                {
                  return FALSE;
                }
              stepper = MIN_NAVIGATION_LIST_HEIGHT - NAVIGATION_LIST_HEIGHT;
            }
          else
            {
              stepper = y - NAVIGATION_LIST_HEIGHT - STEPER_HEIGHT;
            }
        }
      else
        {
          //The button is on the right
          viewportdetail = lookup_widget (widget, "viewportdetail");
          if (viewportdetail == NULL)
            {
              ERROR ("Can not find the widget of viewportdetail");
              return FALSE;
            }
          gtk_widget_get_size_request (viewportdetail, &x, &y);

          if ((y - STEPER_HEIGHT) < MIN_DETAIL_AREA_HEIGHT)
            {
              if (y == MIN_DETAIL_AREA_HEIGHT)
                {
                  return FALSE;
                }
              stepper = MAIN_WINDOW_HEIGHT - MENUBAR_HEIGHT - TOOLBAR_HEIGHT
                        - NAVIGATION_LIST_HEIGHT - BAR_SEPARATE_HEIGTH
                        - MIN_DETAIL_AREA_HEIGHT;
            }
          else
            {
              stepper = MAIN_WINDOW_HEIGHT - MENUBAR_HEIGHT - TOOLBAR_HEIGHT
                        - NAVIGATION_LIST_HEIGHT - BAR_SEPARATE_HEIGTH
                        - y + STEPER_HEIGHT;
            }
        }
      DBG ("The cstepper is %d", stepper);

      resize_navigation_detail_area (widget, stepper);
    }

  return FALSE;
}

void
on_bfullscreen_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *viewportpkglist;

  viewportpkglist = lookup_widget (GTK_WIDGET (button), "viewportpkglist");
  if (viewportpkglist == NULL)
    {
      ERROR ("Can not find the widget of viewportdetail");
      return;
    }

  if (GTK_WIDGET_VISIBLE (viewportpkglist))
    {
      full_screen_detail (GTK_WIDGET (button));
    }
  else
    {
      gtk_widget_show (viewportpkglist);
      resize_navigation_detail_area (viewportpkglist, 0);
    }
}

