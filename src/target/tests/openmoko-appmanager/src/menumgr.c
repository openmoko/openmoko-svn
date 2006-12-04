/**
 * @file menumgr.c 
 * @brief Manage the application menu and the filter menu.
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-10-12
 */

#include "menumgr.h"
#include "support.h"
#include "widgets.h"
#include "pkglist.h"
#include "errorcode.h"
#include "iconmgr.h"

/**
 * @brief The id of current filter menu.
 */
static gint fmenuid = -1;

/**
 * @brief The id of the latest filter menu.
 *
 * Actually, the latestid does not always contain the id of the 
 * latest filter menu. The latestid must not equal to 
 * FILTER_MENU_SEARCH_RESULT. It contains the latest id that do
 * not equal to FILTER_MENU_SEARCH_RESULT.
 */
static gint latestid = -1;


/**
 * Create the filter menu. And add the static menu item.
 * @return The menu widget
 */
GtkWidget* 
create_filter_menu (void)
{
  GtkWidget *menu;
  GtkWidget *search;
  GtkWidget *installed;
  GtkWidget *upgrade;
  GtkWidget *selected;
  GtkWidget *separator;

  menu = gtk_menu_new ();

  search = gtk_menu_item_new_with_mnemonic (_("Search Results"));
  gtk_widget_show (search);
  gtk_container_add (GTK_CONTAINER (menu), search);

  installed = gtk_menu_item_new_with_mnemonic (_("Installed"));
  gtk_widget_show (installed);
  gtk_container_add (GTK_CONTAINER (menu), installed);

  upgrade = gtk_menu_item_new_with_mnemonic (_("Upgradeable"));
  gtk_widget_show (upgrade);
  gtk_container_add (GTK_CONTAINER (menu), upgrade);

  selected = gtk_menu_item_new_with_mnemonic (_("Selected"));
  gtk_widget_show (selected);
  gtk_container_add (GTK_CONTAINER (menu), selected);

  separator = gtk_separator_menu_item_new ();
  gtk_widget_show (separator);
  gtk_container_add (GTK_CONTAINER (menu), separator);
  gtk_widget_set_sensitive (separator, FALSE);

  g_signal_connect ((gpointer) search, "activate",
                    G_CALLBACK (on_search_result_activate),
                    NULL);

  g_signal_connect ((gpointer) installed, "activate",
                    G_CALLBACK (on_installed_activate),
                    NULL);

  g_signal_connect ((gpointer) upgrade, "activate",
                    G_CALLBACK (on_upgradeable_activate),
                    NULL);

  g_signal_connect ((gpointer) selected, "activate",
                    G_CALLBACK (on_selected_activate),
                    NULL);

  save_filter_menu (menu);

  return menu;
}


/**
 * Search result menu event.
 */
void
on_search_result_activate                 (GtkMenuItem     *menuitem,
                                           gpointer         user_data)
{
  search_user_input ();
}

/**
 * Installed menu event.
 */
void
on_installed_activate                 (GtkMenuItem     *menuitem,
                                       gpointer         user_data)
{
  change_package_list_status (FILTER_MENU_INSTALLED);
}

/**
 * Upgradeable menu event.
 */
void
on_upgradeable_activate                 (GtkMenuItem     *menuitem,
                                         gpointer         user_data)
{
  change_package_list_status (FILTER_MENU_UPGRADEABLE);
}

/**
 * Selected menu event.
 */
void
on_selected_activate                 (GtkMenuItem     *menuitem,
                                      gpointer         user_data)
{
  change_package_list_status (FILTER_MENU_SELECTED);
}

/**
 * Category menu event.
 */
void
on_category_activate                 (GtkMenuItem     *menuitem,
                                      gpointer         user_data)
{
  gint filter = GPOINTER_TO_INT (user_data);

  DBG ("The filter id = %d", filter);
  change_package_list_status (filter + FILTER_MENU_NUM);
}

/**
 * Destroy the filter menu.
 */
void
destroy_filter_menu (void)
{
  GtkWidget *menu;

  menu = get_widget_pointer (FIC_WIDGET_MENU_FILTER_MENU);

  if (menu != NULL)
    {
      gtk_widget_destroy (menu);
    }
  save_filter_menu (NULL);
}

/**
 * Popup the select menu.
 */
void 
popup_select_menu (GtkWidget *treeview, 
                   GdkEventButton *event,
                   gpointer pkg)
{
  GtkWidget  *menu;
  GList    *item;
  gboolean  mask[NU_SELECT_MENU];
  gint   select;
  gint    i;

  menu = get_widget_pointer (FIC_WIDGET_MENU_SELECT_MENU);
  if ( menu == NULL)
    {
      ERROR ("Can not find the select menu widget.");
      return;
    }

  select = get_select_status (pkg);
  switch (select)
    {
      case PKG_STATUS_AVAILABLE:
        mask[SELECT_MENU_UNMARK] = FALSE;
        mask[SELECT_MENU_MARK_INSTALL] = TRUE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = FALSE;
        break;

      case PKG_STATUS_INSTALLED:
        mask[SELECT_MENU_UNMARK] = FALSE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = TRUE;
        break;

      case PKG_STATUS_UPGRADEABLE:
        mask[SELECT_MENU_UNMARK] = FALSE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = TRUE;
        mask[SELECT_MENU_MARK_REMOVE] = TRUE;
        break;

      case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
        mask[SELECT_MENU_UNMARK] = TRUE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = FALSE;
        break;

      case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
        mask[SELECT_MENU_UNMARK] = TRUE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = FALSE;
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
        mask[SELECT_MENU_UNMARK] = TRUE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = TRUE;
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        mask[SELECT_MENU_UNMARK] = TRUE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = TRUE;
        mask[SELECT_MENU_MARK_REMOVE] = FALSE;
        break;

      default :
        mask[SELECT_MENU_UNMARK] = FALSE;
        mask[SELECT_MENU_MARK_INSTALL] = FALSE;
        mask[SELECT_MENU_MARK_UPGRADE] = FALSE;
        mask[SELECT_MENU_MARK_REMOVE] = FALSE;
        break;

    }
  item = gtk_container_get_children (GTK_CONTAINER (menu));
  for (i=0; item != NULL; item = g_list_next (item), i++)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (item->data), mask[i]);
    }

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, 0, 
                  (event != NULL) ? event->button : 0,
                  gdk_event_get_time((GdkEvent *) event));
}

/**
 * @brief Change current id to the new id.
 * @param id The new filter menu id
 */
void 
change_filter_menu_id (gint id)
{
  GtkWidget *bfilter;

  if (fmenuid != FILTER_MENU_SEARCH_RESULT)
    {
      latestid = fmenuid;
    }
  fmenuid = id;

  bfilter = get_widget_pointer (FIC_WIDGET_BUTTON_FILTER);
  if (bfilter == NULL)
    {
      ERROR ("Not find the button widget filter, Can not update the lable");
      return;
    }

  if (id < FILTER_MENU_NUM)
    {
      switch (id)
        {
          case FILTER_MENU_SEARCH_RESULT:
            gtk_button_set_label (GTK_BUTTON (bfilter), _("Search Result"));
            break;

          case FILTER_MENU_INSTALLED:
            gtk_button_set_label (GTK_BUTTON (bfilter), _("Installed"));
            break;

          case FILTER_MENU_UPGRADEABLE:
            gtk_button_set_label (GTK_BUTTON (bfilter), _("Upgradeable"));
            break;

          case FILTER_MENU_SELECTED:
            gtk_button_set_label (GTK_BUTTON (bfilter), _("Selected"));
            break;

          default:
            ERROR ("filter id is error. The id is: %d", id);
            return;
        }
    }
  else
    {
      gchar *name;
      if (check_section_id (id - FILTER_MENU_NUM))
        {
          gtk_button_set_label (GTK_BUTTON (bfilter), "unkown");
          return;
        }

      name = get_section_name (id - FILTER_MENU_NUM);
      gtk_button_set_label (GTK_BUTTON (bfilter), name);
    }
}

/**
 * @brief Get current id and latest id
 *
 * Set the current id to "cid", and set the latest id to "lid"
 * @param cid The current id
 * @param lid The latest id
 */
void 
get_filter_menu_id (gint *cid, gint *lid)
{
  *cid = fmenuid;
  *lid = latestid;
}

/**
 * @brief Set the sensitive of each item on filter menu
 * @param menu The filter menu
 */
void 
set_sensitive_filter_menu (GtkWidget *menu)
{
  GList    *item;
  gint     i;

  item = gtk_container_get_children (GTK_CONTAINER (menu));
  for (i=0; i<FILTER_MENU_NUM; i++, item = g_list_next (item))
    {
      gtk_widget_set_sensitive (GTK_WIDGET (item->data), 
                                get_sensitive_of_item(i));
    }
}

/**
 * @brief Re-init the id of filter menu
 */
void 
reinit_id_of_filter_menu (void)
{
  fmenuid = -1;
  latestid = -1;
}
