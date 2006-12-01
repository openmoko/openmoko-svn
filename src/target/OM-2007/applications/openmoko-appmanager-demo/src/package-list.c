/**
 *  @file package-list.c
 *  @brief The package list that get from the lib ipkg
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include "appmanager-data.h"
#include "package-list.h"
#include "ipkgapi.h"
#include "errorcode.h"

/**
 * @brief The structor of Package list node
 */
typedef struct package_list {
  IPK_PACKAGE *pkg;                ///<Package info
  struct package_list *pre;        ///<The previous node of package list
  struct package_list *next;       ///<The next node of package list
} PackageList;

/**
 * @brief Section list structure.
 */
typedef struct section_list {
  char *name;                      ///<Section name
  int  sequence;                   ///<The sequence in section list
  PackageList head;               ///<The first node of package list at this section
  struct section_list *next;       ///<The next section list node
} SectionList;

/**
 * @brief Get the list of all packages from lib ipkg
 * @param appdata The application manager data
 * @return If success, return OP_SUCCESS, else return error code
 */
gint 
init_package_list (ApplicationManagerData *appdata)
{
  PKG_LIST_HEAD *head;
  int ret;

  head = g_malloc (sizeof (PKG_LIST_HEAD));
  if (head == NULL)
    {
      g_debug ("Can not malloc memory for the package list header");
      return OP_MAMORY_MALLOC_ERROR;
    }

  head->length = 0;
  head->pkg_list = NULL;

  ret = ipkg_initialize (0);
  if (ret != 0)
    {
      //Can't initialize the lib ipkg
      g_debug ("Can not initialize the libipkg, the result is %d\nthe error message is:%s",
               ret, get_error_msg());
      return OP_ERROR;
    }

  ret = ipkg_list_available_cmd (head);
  if (ret != 0)
    {
      //Can't get the package list correctly
      g_debug ("Can not get the package list, the result is %d\nthe error message is:%s",
               ret, get_error_msg());
      g_free (head);
      return OP_ERROR;
    }

  application_manager_data_set_pkglist (appdata, head);
  return OP_SUCCESS;
}

/**
 * @brief Create a new node of section list.
 *
 * If the param name is NULL, return NULL;
 * If it can not malloc memory for the node of section list, it will return NULL.
 * @param name The section name.
 * @return The section pointer.
 */
static SectionList *
package_list_create_new_section_node (const char *name)
{
  SectionList   *sect;

  g_return_val_if_fail (name != NULL, NULL);

  sect = g_malloc (sizeof (SectionList));
  if (sect == NULL)
    {
      g_debug ("ERROR: Malloc memory for section list error.");
      return NULL;
    }

  sect->name = g_malloc (strlen (name) +1);
  if (sect->name == NULL)
    {
      g_debug ("ERROR: Malloc memory for section list error.");
      g_free (sect);
      return NULL;
    }

  strcpy (sect->name, name);
  sect->head.pkg = NULL;
  sect->head.pre = &(sect->head);
  sect->head.next = &(sect->head);
  sect->next = NULL;

  return sect;
}

/**
 * @brief Search section node.
 *
 * Search the section node for a package. If the section of package isn't
 * in the section list, add a new section node for it. Only when the 
 * return value is OP_SUCCESS, the value of parameter "section" is valid.
 *
 * @param name The name of section
 * @param section The section pointer
 * @param sechead The header of section list
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
static gint 
package_list_search_section_node (const char *name, 
                                  SectionList **section, 
                                  SectionList *sechead)
{
  SectionList  *tmp;
  SectionList  *pre;
  gint   ret;

  if (name == NULL)
    {
      g_debug ("The name of section is NULL");
      return OP_SECTION_NAME_NULL;
    }
  if (name[0] == 0)
    {
      g_debug ("The name of section is NULL");
      return OP_SECTION_NAME_NULL;
    }

  pre = sechead;
  tmp = sechead->next;
  while (tmp != NULL)
    {
      ret = strcmp (tmp->name, name);

      if (ret == 0)
        {
          //Find it.
          *section = tmp;
          return OP_SUCCESS;
        }
      if (ret > 0)
        {
          break;
        }

      pre = tmp;
      tmp = pre->next;
    }

  tmp = package_list_create_new_section_node (name);
  if (tmp == NULL)
    {
      g_debug ("Can not create section node");
      return OP_MAMORY_MALLOC_ERROR;
    }
  tmp->next = pre->next;
  pre->next = tmp;

  *section = tmp;

  return OP_SUCCESS;
}

/**
 * @brief Free the package list
 * @param pkglist The package list
 */
static void 
package_list_free_package_list (PackageList *pkglist)
{
  PackageList *tmp;
  PackageList *next;

  tmp = pkglist->next;
  while (tmp != pkglist)
    {
      next = tmp->next;
      g_free (tmp);
      tmp = next;
    }
  pkglist->next = pkglist;
  pkglist->pre = pkglist;
}

/**
 * @brief Free the section list
 * @param seclist The section list
 */
static void 
packaeg_list_free_section_list (SectionList *seclist)
{
  SectionList *tmp;
  SectionList *next;

  tmp = seclist->next;
  while (tmp != NULL)
    {
      next = tmp->next;
      package_list_free_package_list (&(tmp->head));
      g_free (tmp->name);
      g_free (tmp);
      tmp = next;
    }
  g_free (seclist);
}

/**
 * @brief Clear the old index
 */
static void 
package_list_clear_old_index (ApplicationManagerData *appdata)
{
  SectionList *sectionlist = NULL;
  PackageList *installed = NULL;
  PackageList *upgrade = NULL;
  PackageList *selected = NULL;

  // Get the section list from the application manager data
  // If the section list is not NULL, clear it.
  sectionlist = (SectionList *) application_manager_data_get_sectionlist (appdata);
  if (sectionlist != NULL)
    {
      packaeg_list_free_section_list (sectionlist);
      sectionlist = NULL;
      application_manager_data_set_section_list (appdata, sectionlist);
    }

  // Get the installed list from the application manager data
  // If the installed list is not NULL, clear it.
  installed = (PackageList *)application_manager_data_get_installedlist (appdata);
  if (installed != NULL)
    {
      package_list_free_package_list (installed);
      g_free (installed);
      installed = NULL;
      application_manager_data_set_installed_list (appdata, installed);
    }

  // Get the upgrade list from the application manager data
  // If the upgrade list is not NULL, clear it.
  upgrade = (PackageList *)application_manager_data_get_upgradelist (appdata);
  if (upgrade != NULL)
    {
      package_list_free_package_list (upgrade);
      g_free (upgrade);
      upgrade = NULL;
      application_manager_data_set_upgrade_list (appdata, upgrade);
    }

  // Get the selected list from the application manager data
  // If the selected list is not NULL, clear it.
  selected = (PackageList *)application_manager_data_get_upgradelist (appdata);
  if (selected != NULL)
    {
      package_list_free_package_list (selected);
      g_free (selected);
      selected = NULL;
      application_manager_data_set_upgrade_list (appdata, selected);
    }
}


/**
 * @brief Build a detailed index for the packages list in the application
 * manager data
 * @param appdata The application manager data
 * @return If success, return OP_SUCCESS, else return error code
 */
gint 
package_list_build_index (ApplicationManagerData *appdata)
{
  PKG_LIST_HEAD *pkglist;

  SectionList *sectionlist = NULL;
  //PackageList *installed = NULL;
  //PackageList *upgrade = NULL;
  //PackageList *selected = NULL;

  // Get the package list from application manager data
  pkglist = (PKG_LIST_HEAD *) application_manager_data_get_pkglist (appdata);
  if (pkglist == NULL)
    {
      g_debug ("The package list is not available");
      return OP_ERROR;
    }

  if (pkglist->pkg_list == NULL)
    {
      g_debug ("The package list is not available");
      return OP_ERROR;
    }

  package_list_clear_old_index (appdata);

  sectionlist = g_malloc (sizeof (SectionList));

  return OP_SUCCESS;
}

