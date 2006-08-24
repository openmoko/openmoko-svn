/**
 * @file pkgmgr.c - Manager the package
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
 * @date 2006-8-11
 **/

#include <gtk/gtk.h>
#include <string.h>
#include <fnmatch.h>
#include <stdlib.h>

#include "pkgmgr.h"
#include "winresize.h"
#include "support.h"
#include "errorcode.h"
#include "ipkgapi.h"

#define ROW_IS_SECTION    1
#define ROW_IS_PACKAGE    2

/**
 * @brief For package tree store
 */
enum {
  COL_PACKAGE = 0,
  COL_SIZE,
  COL_STATUS,
  COL_NUMS
};

typedef struct package_list_temp_store {
  IPK_PACKAGE *ipkg;
  struct package_list_temp_store *next;
} PACKAGE_LIST_TEMP_STORE;

static gint  package_view_status;

static gint  package_button_all_status;
static gint  package_button_action_status;

static gchar package_search_string[MAX_SEARCH_ENTRY_LENGTH + 1];

/**
 * @brief Search the section to find where the package should be insert
 *
 * if return OP_FIND_CORRESPONDING_SECTION, the iter will be the section iter.
 * if return OP_REACH_SECTION_END, the iter will be invalid.
 * if return OP_FIND_THE_POSITION, the iter will be the first section iter that the section
 * name is greater than the package name of pkg.
 *
 * @param store The tree store
 * @param iter The GtkTreeIter
 * @param pkg The package
 * 
 * @return The search result
 * @retval OP_FIND_CORRESPONDING_SECTION Find the section
 * @retval OP_REACH_SECTION_END Can't find the section, and search to the end of section list
 * @retval OP_FIND_THE_POSITION Can't find the section, 
 * and the package name of iter is greater than the package name of pkg
 */
static gint
search_section (GtkTreeModel *store, GtkTreeIter *iter, IPK_PACKAGE *pkg)
{
  gchar  *secname = NULL;
  gint   ret;
  gint   res;

  ret = gtk_tree_model_get_iter_first (store, iter);

  while (ret)
    {
      gtk_tree_model_get (store, iter, COL_PACKAGE, &secname, -1);
      if (secname == NULL)
        {
          /** BUG : System error, can't get the section name 
           * The code below must not be execute.*/
          ERROR("System error, can\'t get the section name\n");
        }
    
      res = strcmp (pkg->section, secname);
      //DBG("cmp the pkg section:%s,and sec name:%s,res=%d\n", pkg->section, secname, res);
    
      if (secname != NULL)
        {
          g_free(secname);
          secname = NULL;
        }
    
      if (res == 0)
        return OP_FIND_CORRESPONDING_SECTION;
      else if (res < 0)
        return OP_FIND_THE_POSITION;
    
      ret = gtk_tree_model_iter_next (store, iter);
    }

  return OP_REACH_SECTION_END;

}

static void
insert_with_new_section (GtkTreeStore *store,
                         GtkTreeIter *sec,
                         IPK_PACKAGE *pkg)
{
  GtkTreeIter parent, iter;

  gtk_tree_store_insert_before (store, &parent, NULL, sec);

  gtk_tree_store_set (store, &parent,
                      COL_PACKAGE, (pkg->section == NULL) ? "" : pkg->section,
                      -1);

  gtk_tree_store_append (store, &iter, &parent);

  gtk_tree_store_set (store, &iter,
                      COL_PACKAGE, (pkg->name == NULL) ? "" : pkg->name,
                      COL_SIZE, (pkg->size == NULL) ? "" : pkg->size,
                      COL_STATUS, pkg->state_status,
                      -1);
                      
}

static void
insert_package_to_section (GtkTreeStore *store,
                           GtkTreeIter *parent,
                           IPK_PACKAGE *pkg)
{
  GtkTreeIter iter;

  /** BUG: There not sort when insert a package to the section */

  gtk_tree_store_append (store, &iter, parent);

  gtk_tree_store_set (store, &iter,
                      COL_PACKAGE, (pkg->name == NULL) ? "" : pkg->name,
                      COL_SIZE, (pkg->size == NULL) ? "" : pkg->size,
                      COL_STATUS, pkg->state_status,
                      -1);

}

static void
insert_package_without_section (GtkTreeStore *store,
                                PACKAGE_LIST_TEMP_STORE *list)
{
  GtkTreeIter parent, iter;

  gtk_tree_store_append (store, &parent, NULL);

  gtk_tree_store_set (store, &parent,
                      COL_PACKAGE, "Unknown package",
                      -1);

  while (list != NULL)
    {
      gtk_tree_store_append (store, &iter, &parent);
    
      gtk_tree_store_set (store, &iter,
                          COL_PACKAGE, (list->ipkg->name == NULL) ? "" : list->ipkg->name,
                          COL_SIZE, (list->ipkg->size == NULL) ? "" : list->ipkg->size,
                          COL_STATUS, list->ipkg->state_status,
                          -1);
    
      list = list->next;
    }
  
}

static void
free_the_list_temp_store (PACKAGE_LIST_TEMP_STORE *list)
{
  PACKAGE_LIST_TEMP_STORE *next;

  next = list;
  while (next != NULL)
    {
      list = next;
      next = list->next;
      g_free(list);
    }

}

/**
 * @brief Convert package list from IPK_PACKAGE to GtkTreeStore
 *
 * @param store The GtkTreeStore
 * @param pkg The package list
 * 
 * @return Error code
 */
static gint
init_package_store (GtkTreeStore *store,
                    IPK_PACKAGE *pkg)
{
  PACKAGE_LIST_TEMP_STORE *nosec = NULL;
  PACKAGE_LIST_TEMP_STORE *nosecend;
  GtkTreeIter   isec;

  gint    res;

  while (pkg != NULL)
    {
    
      /** if package section is unknown */
      if (pkg->section == NULL)
        {
          if (nosec == NULL)
            {
              nosec = (PACKAGE_LIST_TEMP_STORE *) g_malloc(sizeof(PACKAGE_LIST_TEMP_STORE));
              if (nosec == NULL)
                goto mem_malloc_error;
            
              nosec->next = NULL;
              nosec->ipkg = pkg;
              nosecend = nosec;
            }
          else
            {
              nosecend->next = (PACKAGE_LIST_TEMP_STORE *) g_malloc(sizeof(PACKAGE_LIST_TEMP_STORE));
              if (nosecend->next == NULL)
                goto mem_malloc_error;
            
              nosecend->next->next = NULL;
              nosecend->next->ipkg = pkg;
              nosecend = nosecend->next;
            }
          pkg = pkg->next;
          continue;
        }
    
      res = search_section (GTK_TREE_MODEL (store), &isec, pkg);
      //DBG("Search the package \"%s\" result is :%s\n", pkg->name, trans_error_code(res));
    
      if (res == OP_FIND_CORRESPONDING_SECTION)
        {
          insert_package_to_section (store, &isec, pkg);
        }
      else if (res == OP_FIND_THE_POSITION)
        {
          insert_with_new_section (store, &isec, pkg);
        }
      else if (res == OP_REACH_SECTION_END)
        {
          insert_with_new_section (store, NULL, pkg);
        }

      pkg = pkg->next;
    }

  if (nosec != NULL)
    {
      insert_package_without_section (store, nosec);
      free_the_list_temp_store (nosec);
    }

  return OP_SUCCESS;

mem_malloc_error:
  DBG("Memory malloc error\n");

  if (nosec != NULL)
    free_the_list_temp_store(nosec);
  return OP_MEMORY_MALLOC_FAIL;

}

/**
 * @brief Init the package tree
 *
 * This function should be called once at the initial process.
 *
 * @param window The main window widget pointer
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint
init_package_view (GtkWidget *window)
{
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore        *treestore;
  GtkTreeModel        *model;

  GtkWidget           *pkgview=NULL;

  PKG_LIST_HEAD       pkglist;
  gint                ret = OP_SUCCESS;
  gint                res;

  pkglist.pkg_list = NULL;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);

  if (pkgview == NULL)
    return OP_NOT_FIND_TREE_VIEW_PACKAGE;

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Package"));
  gtk_tree_view_column_set_resizable (col, TRUE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 300);

  gtk_tree_view_append_column (GTK_TREE_VIEW (pkgview), col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_PACKAGE);

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Size"));

  gtk_tree_view_append_column (GTK_TREE_VIEW (pkgview), col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_SIZE);

  treestore = gtk_tree_store_new (COL_NUMS,
                                  G_TYPE_STRING,
                                  G_TYPE_STRING,
                                  G_TYPE_INT);

  model = GTK_TREE_MODEL (treestore);

  res = ipkg_initialize (0);
  package_view_status = PKG_STATUS_INSTALLED;
  update_button_package_status (window, PKG_STATUS_INSTALLED);
  update_button_all_status (window, PKG_VIEW_DISPLAY_ALL);
  clear_search_string ();
  package_button_action_status = -1;

  if (res == 0)
    {
      res = ipkg_list_installed_cmd (&pkglist);
    
      if (res == 0)
        {
          if (pkglist.pkg_list != NULL)
            init_package_store (treestore, pkglist.pkg_list);
        }
      else
        {
          ERROR ("Get installed package list error:%s\n", get_error_msg ());
          ret = OP_GET_INSTALLED_PACKAGE_LIST_ERROR;
        
          goto operation_error;
        }
    
      free_pkg_list(&pkglist);
    }
  else
    {
      ERROR("Init ipkg model fail\n");
      ret = OP_INIT_IPKG_MODEL_FAIL;
    
      goto operation_error;
    }

  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), model);

  //gtk_tree_view_set_enable_tree_lines (GTK_TREE_VIEW (pkgview), TRUE);

  g_object_unref (model);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pkgview), TRUE);

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (pkgview)),
                               //GTK_SELECTION_MULTIPLE);
                               GTK_SELECTION_SINGLE);


  return OP_SUCCESS;

operation_error:

  DBG("Init package list error\n");

  gtk_tree_store_clear (treestore);

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);

  g_object_unref (model);

  return ret;

}

/**
 * @brief Get the first selected package name from package tree
 *
 * If the return value is not NULL, it should be free by using g_free.
 *
 * @param window A widget pointer use to lookup the package treeview widget.
 * @return Package name. If no package that has been selected, it will return NULL.
 */
gchar *
get_first_selected_package_name (GtkWidget *window)
{
  GtkWidget   *pkgview;
  GtkTreeSelection    *selection;
  GtkTreeModel        *model;
  GtkTreeIter         iter;
  GList               *list;
  GList               *list1;
  GtkTreePath         *path;

  gchar               *pkgname = NULL;
  gint                depth;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);
  if (pkgview == NULL)
    return NULL;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pkgview));

  list = gtk_tree_selection_get_selected_rows (selection, &model);

  list1 = g_list_first (list);

  while (list1 != NULL)
  {
    depth = gtk_tree_path_get_depth ((GtkTreePath *) (list1->data));

    if(depth == 1)
      list1 = g_list_next (list1);
    else
      break;
  }

  if (list1 != NULL)
    {
      path = (GtkTreePath *) (list->data);

      if (gtk_tree_model_get_iter (model, &iter, path))
        {
          gtk_tree_model_get (model, &iter, COL_PACKAGE, &pkgname, -1);

          return pkgname;
        }
    }

  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) (gtk_tree_path_free), NULL);
      g_list_free (list);
    }

  return NULL;

}

const gchar *
get_user_input_string (GtkWidget *window)
{
  GtkWidget  *entrysearch;
  const gchar * name;

  entrysearch = lookup_widget (window, STRING_ENTRY_SEARCH);

  if (entrysearch == NULL)
    return NULL;

  name = gtk_entry_get_text ( GTK_ENTRY (entrysearch));

  DBG("user input is:%s:\n", name);

  if (strlen (name) == 0)
    return NULL;

  return name;
}

gint
display_search_package_list (GtkWidget *window, char *name)
{
  GtkWidget           *pkgview;

  GtkTreeModel        *model;
  GtkTreeStore        *store;

  PKG_LIST_HEAD       pkglist;
  gint                res;

  pkglist.pkg_list = NULL;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);

  if (pkgview == NULL)
    return OP_NOT_FIND_TREE_VIEW_PACKAGE;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));

  g_object_ref (model);

  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);

  store = GTK_TREE_STORE (model);
  gtk_tree_store_clear (store);

  res = ipkg_search_cmd (name, &pkglist);

  if (res == 0)
    {
      if ((pkglist.pkg_list != NULL) && pkglist.length != 0)
        {
          init_package_store (store, pkglist.pkg_list);
        }
    }
  else
    {

      ERROR("Can't get the available package list\n");
    }

  free_pkg_list (&pkglist);

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);

  g_object_unref (model);

  return OP_SUCCESS;

}

/**
 * @brief Update the "action" button
 *
 */
void
update_button_action_status (GtkWidget *window, gint status)
{
  GtkWidget *action;

  action = lookup_widget (window, STRING_BUTTON_ACTION);
  
  if (action == NULL)
    return;

  package_button_action_status = status;

  if (status == SS_NOT_INSTALLED)
    {
      gtk_button_set_label (GTK_BUTTON (action), _("Install"));
    }
  else if (status == SS_INSTALLED)
    {
      gtk_button_set_label (GTK_BUTTON (action), _("Remove"));
    }
  else
    {
      gtk_button_set_label (GTK_BUTTON (action), _("+Actions"));
    }
}

gint
get_button_action_status (void)
{
  return package_button_action_status;
}


gint
get_first_selected_package_status (GtkWidget *window)
{
  GtkWidget   *pkgview;
  GtkTreeSelection    *selection;
  GtkTreeModel        *model;
  GtkTreeIter         iter;
  GList               *list;
  GList               *list1;
  GtkTreePath         *path;

  gint                depth;
  gint                status;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);
  if (pkgview == NULL)
    return -1;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pkgview));

  list = gtk_tree_selection_get_selected_rows (selection, &model);

  list1 = g_list_first (list);

  while (list1 != NULL)
  {
    depth = gtk_tree_path_get_depth ((GtkTreePath *) (list1->data));

    if(depth == 1)
      list1 = g_list_next (list1);
    else
      break;
  }

  if (list1 != NULL)
    {
      path = (GtkTreePath *) (list->data);

      if (gtk_tree_model_get_iter (model, &iter, path))
        {
          gtk_tree_model_get (model, &iter, COL_STATUS, &status, -1);

          return status;
        }
    }

  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) (gtk_tree_path_free), NULL);
      g_list_free (list);
    }

  return -1;

}

void
update_button_package_status (GtkWidget *window, gint status)
{
  GtkWidget  *package;

  package = lookup_widget (window, STRING_BUTTON_PACKAGE_LABEL);

  if (package == NULL)
    return;

  switch (status)
    {
      case PKG_STATUS_INSTALLED:
        //gtk_button_set_label (GTK_BUTTON (package), _("Installed"));
        gtk_label_set_text (GTK_LABEL (package), _("Installed"));
        break;

      case PKG_STATUS_AVAILABLE:
        //gtk_button_set_label (GTK_BUTTON (package), _("Available"));
        gtk_label_set_text (GTK_LABEL (package), _("Available"));
        break;

      case PKG_STATUS_UPDATES:
        //gtk_button_set_label (GTK_BUTTON (package), _("Updates"));
        gtk_label_set_text (GTK_LABEL (package), _("Updates"));
        break;

      case PKG_STATUS_SELECTED:
        //gtk_button_set_label (GTK_BUTTON (package), _("Selected"));
        gtk_label_set_text (GTK_LABEL (package), _("Selected"));
        break;

      default:
        //gtk_button_set_label (GTK_BUTTON (package), _("Package"));
        gtk_label_set_text (GTK_LABEL (package), _("Package"));
        break;
    }
}

void
update_button_all_status (GtkWidget *window, gint status)
{
  GtkWidget  *package;

  package = lookup_widget (window, STRING_BUTTON_ALL_LABEL);

  if (package == NULL)
    return;

  package_button_all_status = status;

  switch (status)
    {
      case PKG_VIEW_DISPLAY_ALL:
        gtk_label_set_text (GTK_LABEL (package), _("All"));
        break;

      case PKG_VIEW_DISPLAY_SEARCH:
        gtk_label_set_text (GTK_LABEL (package), _("Search"));
        break;

      default:
        gtk_label_set_text (GTK_LABEL (package), _("ALL"));
        break;
    }
}

gint
change_package_list (GtkWidget *window, gint status)
{
  GtkWidget           *pkgview;

  GtkTreeModel        *model;
  GtkTreeStore        *store;

  PKG_LIST_HEAD       pkglist;
  gint                res;

  pkglist.pkg_list = NULL;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);

  if (pkgview == NULL)
    return OP_NOT_FIND_TREE_VIEW_PACKAGE;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));

  g_object_ref (model);

  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);

  store = GTK_TREE_STORE (model);
  gtk_tree_store_clear (store);

  clear_search_string ();

  switch (status)
    {
      case PKG_STATUS_INSTALLED:
        res = ipkg_list_installed_cmd (&pkglist);
        update_button_package_status (window, status);
        break;

      case PKG_STATUS_AVAILABLE:
        res = ipkg_list_available_cmd(&pkglist);
        update_button_package_status (window, status);
        break;

      case PKG_STATUS_UPDATES:
        res = -1;
        break;

      case PKG_STATUS_SELECTED:
        res = -1;
        break;

      default :
        res = -1;
        break;
    }

  if (res == 0)
    {
      if (pkglist.pkg_list != NULL)
        {
          init_package_store (store, pkglist.pkg_list);
        }
    }
  else
    {
      ERROR("Can't get package list\n");
    }

  free_pkg_list (&pkglist);

  update_button_all_status (window, PKG_VIEW_DISPLAY_ALL);
  update_button_action_status (window, -1);

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);

  g_object_unref (model);

  return OP_SUCCESS;

}

gint
get_package_view_status (void)
{
  return package_view_status;
}

gint
roll_package_view_status (GtkWidget *window)
{
  gint res;

  package_view_status ++;

  if (package_view_status >= PKG_STATUS_UPDATES)
    {
      package_view_status = PKG_STATUS_INSTALLED;
    }

  res = change_package_list (window, package_view_status);

  return res;
}

void
clear_search_string (void)
{
  strcpy (package_search_string, "");
}

void
update_search_string (gchar *name)
{
  strncpy (package_search_string, name, MAX_SEARCH_ENTRY_LENGTH);
}



gint
search_package_from_section (GtkTreeModel *model,
                             GtkTreeIter *parent,
                             gchar *name)
{
  GtkTreeIter   iter, next;

  gchar        *pkgname = NULL;
  gint         count = 0;
  gboolean     res;
  gint         match;

  res = gtk_tree_model_iter_children (model, &next, parent);

  while (res)
    {
      iter = next;
      res = gtk_tree_model_iter_next (model, &next);

      gtk_tree_model_get (model, &iter, COL_PACKAGE, &pkgname, -1);

      if (pkgname != NULL)
        {
          match = fnmatch (name, pkgname, 0);

          if (match != 0)
            {
              gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
            }
          else
            {
              count ++;
            }

          g_free (pkgname);
          pkgname = NULL;
        }
      else
        {
          ERROR ("Can not get the package name\n");
          count ++;
        }
    }

  return count;
}

void
search_package_incremental (GtkWidget *window, gchar *name)
{
  GtkWidget           *pkgview;

  GtkTreeModel        *model;
  GtkTreeStore        *store;
  GtkTreeIter         iter;
  GtkTreeIter         next;

  gint                count;
  gboolean            res;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);
  if (pkgview == NULL)
    {
      ERROR ("Find package list view error\n");
      return;
    }

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));
  store = GTK_TREE_STORE (model);

  update_button_all_status (window, PKG_VIEW_DISPLAY_SEARCH);

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);

  res = gtk_tree_model_get_iter_first (model, &next);

  while (res)
    {
      iter = next;
      res = gtk_tree_model_iter_next (model, &next);

      count = search_package_from_section (model, &iter, name);

      if (count == 0)
        {
          /** if the section has not contain any package, delete it */
          gtk_tree_store_remove (store, &iter);
        }

    }

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);
  g_object_unref (model);
}

static gint
search_and_init_package_store (GtkTreeStore *store,
                               IPK_PACKAGE *pkg,
                               gchar *name)
{
  PACKAGE_LIST_TEMP_STORE *nosec = NULL;
  PACKAGE_LIST_TEMP_STORE *nosecend;
  GtkTreeIter   isec;

  gint    res;

  while (pkg != NULL)
    {
      if (pkg->name != NULL)
        {
          res = fnmatch (name, pkg->name, 0);
          if (res != 0)
            {
              pkg = pkg->next;
              continue;
            }
        }

      /** if package section is unknown */
      if (pkg->section == NULL)
        {
          if (nosec == NULL)
            {
              nosec = (PACKAGE_LIST_TEMP_STORE *) g_malloc(sizeof(PACKAGE_LIST_TEMP_STORE));
              if (nosec == NULL)
                goto mem_malloc_error;
            
              nosec->next = NULL;
              nosec->ipkg = pkg;
              nosecend = nosec;
            }
          else
            {
              nosecend->next = (PACKAGE_LIST_TEMP_STORE *) g_malloc(sizeof(PACKAGE_LIST_TEMP_STORE));
              if (nosecend->next == NULL)
                goto mem_malloc_error;
            
              nosecend->next->next = NULL;
              nosecend->next->ipkg = pkg;
              nosecend = nosecend->next;
            }
          pkg = pkg->next;
          continue;
        }
    
      res = search_section (GTK_TREE_MODEL (store), &isec, pkg);
      //DBG("Search the package \"%s\" result is :%s\n", pkg->name, trans_error_code(res));
    
      if (res == OP_FIND_CORRESPONDING_SECTION)
        {
          insert_package_to_section (store, &isec, pkg);
        }
      else if (res == OP_FIND_THE_POSITION)
        {
          insert_with_new_section (store, &isec, pkg);
        }
      else if (res == OP_REACH_SECTION_END)
        {
          insert_with_new_section (store, NULL, pkg);
        }

      pkg = pkg->next;
    }

  if (nosec != NULL)
    {
      insert_package_without_section (store, nosec);
      free_the_list_temp_store (nosec);
    }

  return OP_SUCCESS;

mem_malloc_error:
  DBG("Memory malloc error\n");

  if (nosec != NULL)
    free_the_list_temp_store(nosec);
  return OP_MEMORY_MALLOC_FAIL;

}

gint
search_from_original_package_list (GtkWidget *window, gchar *name)
{
  GtkWidget           *pkgview;

  GtkTreeModel        *model;
  GtkTreeStore        *store;

  PKG_LIST_HEAD       pkglist;
  gint                res;

  pkglist.pkg_list = NULL;

  pkgview = lookup_widget (window, STRING_TREE_VIEW_PACKAGE);

  if (pkgview == NULL)
    return OP_NOT_FIND_TREE_VIEW_PACKAGE;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));

  g_object_ref (model);

  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);

  store = GTK_TREE_STORE (model);
  gtk_tree_store_clear (store);

  switch (get_package_view_status ())
    {
      case PKG_STATUS_INSTALLED:
        res = ipkg_list_installed_cmd (&pkglist);
        break;

      case PKG_STATUS_AVAILABLE:
        res = ipkg_list_available_cmd(&pkglist);
        break;

      case PKG_STATUS_UPDATES:
        res = -1;
        break;

      case PKG_STATUS_SELECTED:
        res = -1;
        break;

      default :
        res = -1;
        break;
    }

  if (res == 0)
    {
      if (pkglist.pkg_list != NULL)
        {
          if (strlen (name) <= 2)
            {
              init_package_store (store, pkglist.pkg_list);
              update_button_all_status (window, PKG_VIEW_DISPLAY_ALL);
            }
          else
            {
              search_and_init_package_store (store, pkglist.pkg_list, name);
              update_button_all_status (window, PKG_VIEW_DISPLAY_SEARCH);
            }
        }
    }
  else
    {
      ERROR("Can't get package list\n");
    }

  free_pkg_list (&pkglist);

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);

  g_object_unref (model);

  return OP_SUCCESS;

}

void
search_package_from_package_list (GtkWidget *window, gchar *name)
{
  gchar pattern[MAX_SEARCH_ENTRY_LENGTH +3];
  gint  res;


  sprintf (pattern, "*%s*", package_search_string);

  res = fnmatch (pattern, name, 0);

  if (res == 0)
    {
      /** incremental search */
      snprintf (pattern, MAX_SEARCH_ENTRY_LENGTH, "*%s*", name);
      search_package_incremental (window, pattern);
    }
  else
    {
      snprintf (pattern, MAX_SEARCH_ENTRY_LENGTH, "*%s*", name);
      search_from_original_package_list (window, pattern);
    }

  update_search_string (name);
}



void
update_all_view (GtkWidget *window)
{
  change_package_list (window, get_package_view_status ());
}

void
add_remove_package (GtkWidget *window)
{
  gint  status;
  gchar *pkgname = NULL;
  gchar *newname = NULL;

  gint  res;

  status = get_button_action_status ();

  if ((status == SS_NOT_INSTALLED) || (status == SS_INSTALLED))
    {
      pkgname = get_first_selected_package_name (window);

      if (pkgname != NULL)
        {
          if (status == SS_NOT_INSTALLED)
            {
              res = ipkg_install_cmd (pkgname, &newname);
              DBG ("Install package:%s, the result is:%d\n", pkgname, res);
              if (newname != NULL)
                free (newname);
            }
          else
            {
              res = ipkg_remove_cmd (pkgname);
              DBG ("Install package:%s, the result is:%d\n", pkgname, res);
            }
          update_all_view (window);

          g_free (pkgname);
        }
    }
}
