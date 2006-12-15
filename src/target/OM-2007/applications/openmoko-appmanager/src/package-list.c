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
#include "filter-menu.h"
#include "errorcode.h"
#include "navigation-area.h"

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

static gint package_list_insert_node_without_check (PackageList *pkglist, IPK_PACKAGE *pkg);

/**
 * @brief Version compare
 *
 * This function is copy from ipkg.(pkg.c)
 * The verrevcmp() function compares the two version string "val" and
 * "ref". It returns an integer less than, equal to, or greater than 
 * zero if "val" is found, respectively, to be less than, to match, or
 * be greater than "ref".
 */
static int 
verrevcmp(const char *val, const char *ref)
{
  int vc, rc;
  long vl, rl;
  const char *vp, *rp;
  const char *vsep, *rsep;

  if (!val) val= "";
  if (!ref) ref= "";
  for (;;) 
    {
      vp= val;  while (*vp && !isdigit(*vp)) vp++;
      rp= ref;  while (*rp && !isdigit(*rp)) rp++;
      for (;;) 
        {
          vc= (val == vp) ? 0 : *val++;
          rc= (ref == rp) ? 0 : *ref++;
          if (!rc && !vc) break;
          if (vc && !isalpha(vc)) vc += 256;
          if (rc && !isalpha(rc)) rc += 256;
          if (vc != rc) return vc - rc;
        }
      val= vp;
      ref= rp;
      vl=0;  if (isdigit(*vp)) vl= strtol(val,(char**)&val,10);
      rl=0;  if (isdigit(*rp)) rl= strtol(ref,(char**)&ref,10);
      if (vl != rl) return vl - rl;

      vc = *val;
      rc = *ref;
      vsep = strchr(".-", vc);
      rsep = strchr(".-", rc);
      if (vsep && !rsep) return -1;
      if (!vsep && rsep) return +1;

      if (!*val && !*ref) return 0;
      if (!*val) return -1;
      if (!*ref) return +1;
    }
}

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
  PackageList *nosecpkg = NULL;

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

  // Get the nosecpkg list from the application manager data
  // If the selected list is not NULL, clear it.
  nosecpkg = (PackageList *)application_manager_data_get_upgradelist (appdata);
  if (nosecpkg != NULL)
    {
      package_list_free_package_list (nosecpkg);
      g_free (nosecpkg);
      nosecpkg = NULL;
      application_manager_data_set_upgrade_list (appdata, nosecpkg);
    }
}

/**
 * @brief Inist the SectionList struct
 */
static void 
section_list_init_node (SectionList *sec)
{
  sec->name = NULL;
  sec->next = NULL;
  sec->head.pkg = NULL;
  sec->head.pre = &(sec->head);
  sec->head.next = &(sec->head);
}

/**
 * @brief Init the PackageList struct
 */
static void 
package_list_init_node (PackageList *pkg)
{
  pkg->pkg = NULL;
  pkg->pre = pkg;
  pkg->next = pkg;
}

/**
 * @brief Check the packages, if the installed package is upgradeable,
 * put them to the "upgrade" package list.
 *
 * @param pkglist The package list
 * @param pkg The package node
 */
static gint 
check_package_upgradeable (PackageList *pkglist, IPK_PACKAGE *pkg,
                           PackageList *upgrade)
{
  IPK_PACKAGE   *tmp;
  gint   ret;

  tmp = pkglist->pkg;
  if (tmp->state_status != SS_INSTALLED)
    {
      // If the package in the list is not installed, 
      // check the other one.
      if (pkg->state_status == SS_INSTALLED)
        {
          // If the other one is installed, exchange them
          pkglist->pkg = pkg;
          pkg = tmp;
          tmp = pkglist->pkg;
        }
      else
        {
          // If the other one is not installed either, 
          // set the package with high version to the list.
          ret = verrevcmp (tmp->version, pkg->version);
          if (ret < 0)
            {
              pkglist->pkg = pkg;
            }
          return OP_SUCCESS;
        }
    }

  ret = verrevcmp (tmp->version, pkg->version);
  if (ret >= 0)
    {
      return OP_SUCCESS;
    }

  tmp->mark = PKG_STATUS_UPGRADEABLE;
  ret = package_list_insert_node_without_check (upgrade, tmp);
  if (ret == OP_SUCCESS)
    {
      return OP_SUCCESS;
    }

  return ret;
}

/**
 * @brief Insert a package node to the package list without check whether 
 * the package is upgradeable
 *
 * @param pkglist The package list
 * @param pkg The package node
 * @return The result code
 */
static gint 
package_list_insert_node_without_check (PackageList *pkglist, IPK_PACKAGE *pkg)
{
  PackageList  *tmp;
  PackageList  *ins;
  gint   ret;

  tmp = pkglist->pre;

  while ((tmp != pkglist) && (tmp != NULL))
    {
      ret = strcmp (pkg->name, tmp->pkg->name);

      if (ret > 0) 
        {
          //The name of package is larger then the name of node
          ins = (PackageList *) g_malloc (sizeof (PackageList));
          if (ins == NULL)
            {
              g_debug ("Can not malloc memory for package node, the package name is:%s", pkg->name);
              return OP_MAMORY_MALLOC_ERROR;
            }
          ins->pkg = pkg;
          ins->pre = tmp;
          ins->next = tmp->next;

          tmp->next->pre = ins;
          tmp->next = ins;

          return OP_SUCCESS;
        }
      // FIXME  Ignore the names of two packages are equal
      // At this condition, if there are two packages with the same name,
      // add every of them to the package list

      //The name of package is small then the name of node, search the pre node.
      tmp = tmp->pre;
    }

  ins = (PackageList *) g_malloc (sizeof (PackageList));
  if (ins == NULL)
    {
      g_debug ("Can not malloc memory for package node, the package name is:%s", pkg->name);
      return OP_MAMORY_MALLOC_ERROR;
    }
  ins->pkg = pkg;
  ins->pre = tmp;
  ins->next = tmp->next;

  tmp->next->pre = ins;
  tmp->next = ins;

  return OP_SUCCESS;
}

/**
 * @brief Insert a package node to the package list.
 *
 * @param pkglist The package list
 * @param pkg The package node
 * @param upgrade The package list of upgradeable packages
 * @return The result code
 */
static gint 
package_list_insert_node (PackageList *pkglist, IPK_PACKAGE *pkg, PackageList *upgrade)
{
  PackageList  *tmp;
  PackageList  *ins;
  gint         ret;

  tmp = pkglist->pre;

  while ((tmp != pkglist) && (tmp != NULL))
    {
      ret = strcmp (pkg->name, tmp->pkg->name);

      if (ret > 0) 
        {
          //The name of package is larger then the name of node
          ins = (PackageList *) g_malloc (sizeof (PackageList));
          if (ins == NULL)
            {
              g_debug ("Can not malloc memory for package node, the package name is:%s", pkg->name);
              return OP_MAMORY_MALLOC_ERROR;
            }
          ins->pkg = pkg;
          ins->pre = tmp;
          ins->next = tmp->next;

          tmp->next->pre = ins;
          tmp->next = ins;

          return OP_SUCCESS;
        }
      else if (ret == 0)
        {
          //The name of package is equal to the name of node.
          //The package maybe an upgradeable package.
          g_debug ("The package maybe upgradeable. Package name is:%s", pkg->name);
          g_debug ("The pkg version 1 is:%s, The version 2 is:%s", tmp->pkg->version, pkg->version);
          return check_package_upgradeable (tmp, pkg, upgrade);
        }

      //The name of package is small then the name of node, search the pre node.
      tmp = tmp->pre;
    }

  ins = (PackageList *) g_malloc (sizeof (PackageList));
  if (ins == NULL)
    {
      g_debug ("Can not malloc memory for package node, the package name is:%s", pkg->name);
      return OP_MAMORY_MALLOC_ERROR;
    }
  ins->pkg = pkg;
  ins->pre = tmp;
  ins->next = tmp->next;

  tmp->next->pre = ins;
  tmp->next = ins;

  return OP_SUCCESS;
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
  IPK_PACKAGE   *pkg;

  SectionList   *sectionlist = NULL;
  PackageList   *installed = NULL;
  PackageList   *upgrade = NULL;
  PackageList   *selected = NULL;
  PackageList   *nosecpkg = NULL;

  SectionList   *tmpsec = NULL;
  gint          ret;

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

  // Clear the old data
  package_list_clear_old_index (appdata);

  // Malloc memory for the head
  sectionlist = g_malloc (sizeof (SectionList));
  if (sectionlist == NULL)
    {
      g_debug ("Can not malloc memory for the section list");
      return OP_MAMORY_MALLOC_ERROR;
    }

  installed = g_malloc (sizeof (PackageList));
  if (installed == NULL)
    {
      g_debug ("Can not malloc memory for the package list");
      g_free (sectionlist);
      return OP_MAMORY_MALLOC_ERROR;
    }

  upgrade = g_malloc (sizeof (PackageList));
  if (upgrade == NULL)
    {
      g_debug ("Can not malloc memory for the package list");
      g_free (sectionlist);
      g_free (installed);
      return OP_MAMORY_MALLOC_ERROR;
    }

  selected = g_malloc (sizeof (PackageList));
  if (selected == NULL)
    {
      g_debug ("Can not malloc memory for the package list");
      g_free (sectionlist);
      g_free (installed);
      g_free (upgrade);
      return OP_MAMORY_MALLOC_ERROR;
    }

  nosecpkg = g_malloc (sizeof (PackageList));
  if (nosecpkg == NULL)
    {
      g_debug ("Can not malloc memory for the package list");
      g_free (sectionlist);
      g_free (installed);
      g_free (upgrade);
      g_free (selected);
      return OP_MAMORY_MALLOC_ERROR;
    }

  // Init each list
  g_debug ("Begin init each list");

  section_list_init_node (sectionlist);

  package_list_init_node (installed);
  package_list_init_node (upgrade);
  package_list_init_node (selected);
  package_list_init_node (nosecpkg);

  // Set the header of each list to the application manager data
  application_manager_data_set_section_list (appdata, sectionlist);
  application_manager_data_set_installed_list (appdata, installed);
  application_manager_data_set_upgrade_list (appdata, upgrade);
  application_manager_data_set_selected_list (appdata, selected);
  application_manager_data_set_nosecpkg_list (appdata, nosecpkg);

  // Start to build the index for all packages
  pkg = pkglist->pkg_list;

  while (pkg != NULL)
    {
      // Check wheather the package was installed
      if (pkg->state_status == SS_INSTALLED)
        {
          pkg->mark = PKG_STATUS_INSTALLED;
          ret = package_list_insert_node_without_check (installed, pkg);
          if (ret != OP_SUCCESS)
            {
              return ret;
            }
        }
      else
        {
          pkg->mark = PKG_STATUS_AVAILABLE;
        }

      //Search the section node of package.
      ret = package_list_search_section_node (pkg->section, &tmpsec, sectionlist);
      if (ret == OP_SUCCESS)
        {
          ret = package_list_insert_node (&(tmpsec->head), pkg, upgrade);
          if (ret != OP_SUCCESS)
            {
              return ret;
            }
        }
      else if (ret == OP_SECTION_NAME_NULL)
        {
          ret = package_list_insert_node (nosecpkg, pkg, upgrade);
          if (ret != OP_SUCCESS)
            {
              return ret;
            }
        }
      else
        {
          return ret;
        }

      pkg = pkg->next;
    }

  return OP_SUCCESS;
}

/**
 * @brief Add the sections to the filter menu
 *
 * @param appdata The application manager data
 */
void 
package_list_add_section_to_filter_menu (ApplicationManagerData *appdata)
{
  SectionList  *seclist;
  SectionList  *tmpsec;
  GtkMenu      *filtermenu;
  PackageList  *tmppkg;

  seclist = application_manager_data_get_sectionlist (appdata);
  if (seclist == NULL)
    {
      g_debug ("Section list is empty, not need add anything to filter menu");
      return;
    }

  filtermenu = application_manager_get_filter_menu (appdata);
  if (filtermenu == NULL)
    {
      g_debug ("Filter menu not init correctly");
      return;
    }

  tmpsec = seclist->next;

  while (tmpsec != NULL)
    {
      filter_menu_add_item (filtermenu, tmpsec->name, appdata);
      tmpsec = tmpsec->next;
    }

  tmppkg = application_manager_data_get_nosecpkglist (appdata);
  if (tmppkg == NULL)
    {
      return;
    }

  if (tmppkg->next != tmppkg)
    {
      filter_menu_add_item (filtermenu, PACKAGE_LIST_NO_SECTION_STRING, appdata);
    }
}

/**
 * @brief Insert node to store
 */
static void 
insert_node_to_store (ApplicationManagerData *appdata, 
                      GtkListStore *store, 
                      IPK_PACKAGE *pkg)
{
  GtkTreeIter   iter;
  GdkPixbuf    *pix = NULL;

  pix = application_manager_data_get_status_pixbuf (appdata, pkg->mark);

  gtk_list_store_append (store, &iter);

  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, pkg->name,
                      COL_SIZE, pkg->size,
                      COL_POINTER, pkg,
                      -1);

}

/**
 * @brief Put the nodes in the package list to the GtkListStore
 *
 * @param appdata The application manager data
 * @param store The list store
 * @param pkglist The package list
 */
void 
translate_package_list_to_store (ApplicationManagerData *appdata, 
                                 GtkListStore *store, 
                                 gpointer pkglist)
{
  PackageList *pkglisthead = (PackageList *)pkglist;
  PackageList *tmplist;

  tmplist = pkglisthead->next;
  while (tmplist != pkglisthead)
    {
      insert_node_to_store (appdata, store, tmplist->pkg);
      tmplist = tmplist->next;
    }
}

/**
 * @brief Get the package list which section name equals the "name"
 * from the dynamic section list
 * @param appdata The application manager data
 * @param name The section name
 */
gpointer 
package_list_get_with_name (ApplicationManagerData *appdata,
                            const gchar *name)
{
  SectionList  *seclist;

  g_return_val_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata), NULL);

  seclist = application_manager_data_get_sectionlist (appdata);
  if (seclist == NULL)
    {
      g_debug ("Section list is NULL");
      return NULL;
    }

  seclist = seclist->next;
  while (seclist != NULL)
    {
      if ( 0 == strcmp (name, seclist->name))
        {
          return &(seclist->head);
        }
      seclist = seclist->next;
    }

  return NULL;
}

/**
 * @brief Get the select status from package infomation.
 * @param data The package infomation
 * @return The select status
 */
gint 
package_list_get_package_status (gpointer data)
{
  IPK_PACKAGE *tmp;

  g_return_val_if_fail (data != NULL, -1);

  tmp = (IPK_PACKAGE *)data;
  return tmp->mark;
}

/**
 * @brief Set the select status to package infomation
 * @param data The package infomation
 * @param status The new select status
 */
void 
package_list_set_package_status (gpointer data, gint status)
{
  IPK_PACKAGE *tmp;

  g_return_if_fail (data != NULL);
  g_return_if_fail ((status >= PKG_STATUS_AVAILABLE) && (status < N_COUNT_PKG_STATUS));

  tmp = (IPK_PACKAGE *)data;
  tmp->mark = status;
}

/**
 * @brief Remove a package node from the selected package list
 * @brief appdata The application manager data
 * @param pkg The package infomation
 */
void 
package_list_remove_package_from_selected_list (ApplicationManagerData *appdata,
                                                gpointer pkg)
{
  PackageList  *selectedlist;
  PackageList  *tmplist;

  selectedlist = (PackageList *)application_manager_data_get_selectedlist (appdata);
  g_return_if_fail (selectedlist != NULL);

  tmplist = selectedlist;

  while (tmplist != selectedlist)
    {
      if(tmplist->pkg == pkg)
        {
          tmplist->next->pre = tmplist->pre;
          tmplist->pre->next = tmplist->next;
          g_free (tmplist);
        }
      tmplist = tmplist->next;
    }
}

/**
 * @brief Add a new package node to the selected package list
 * @param appdata The application manager data
 * @param pkg The package infomation
 */
void 
package_list_add_node_to_selected_list (ApplicationManagerData *appdata,
                                        gpointer pkg)
{
  PackageList  *selectedlist;

  selectedlist = (PackageList *)application_manager_data_get_selectedlist (appdata);
  g_return_if_fail (selectedlist != NULL);

  package_list_insert_node_without_check (selectedlist, (IPK_PACKAGE *)pkg);
}

/**
 * @brief Get the package version from package node
 *
 * @param pkg The package infomation
 * @return A pointer to the version of the package. The string points to the 
 *  internally allocated storage and must not be free, modified or stored.
 */
char *
package_list_get_package_version (gpointer pkg)
{
  IPK_PACKAGE *package;

  g_return_val_if_fail (pkg != NULL, NULL);

  package = (IPK_PACKAGE *)pkg;

  return package->version;
}

/**
 * @brief Get the package name from package node
 *
 * @param pkg The package infomation
 * @return A pointer to the name of the package. The string points to the 
 *  internally allocated storage and must not be free, modified or stored.
 */
char *
package_list_get_package_name (gpointer pkg)
{
  IPK_PACKAGE *package;

  g_return_val_if_fail (pkg != NULL, NULL);

  package = (IPK_PACKAGE *)pkg;

  return package->name;
}

/**
 * @brief Get the package depends from package node
 *
 * @param pkg The package infomation
 * @return A pointer to the depends of the package. The string points to the 
 *  internally allocated storage and must not be free, modified or stored.
 */
char *
package_list_get_package_depends (gpointer pkg)
{
  IPK_PACKAGE *package;

  g_return_val_if_fail (pkg != NULL, NULL);

  package = (IPK_PACKAGE *)pkg;

  return package->depends;
}

/**
 * @brief Get the package description from package node
 *
 * @param pkg The package infomation
 * @return A pointer to the description of the package. The string points to the 
 *  internally allocated storage and must not be free, modified or stored.
 */
char *
package_list_get_package_description (gpointer pkg)
{
  IPK_PACKAGE *package;

  g_return_val_if_fail (pkg != NULL, NULL);

  package = (IPK_PACKAGE *)pkg;

  return package->description;
}

/**
 * @brief Get the package maintainer from package node
 *
 * @param pkg The package infomation
 * @return A pointer to the maintainer of the package. The string points to the 
 *  internally allocated storage and must not be free, modified or stored.
 */
char *
package_list_get_package_maintainer (gpointer pkg)
{
  IPK_PACKAGE *package;

  g_return_val_if_fail (pkg != NULL, NULL);

  package = (IPK_PACKAGE *)pkg;

  return package->maintainer;
}

/**
 * @brief Put the nodes in the package list to the GtkListStore
 *
 * @param appdata The application manager data
 * @param store The list store
 * @param pkglist The package list
 * @param str The search string
 */
void 
search_and_translate_package_list_to_store (ApplicationManagerData *appdata, 
                                            GtkListStore *store, 
                                            gpointer pkglist,
                                            const gchar *str)
{
  PackageList *pkglisthead = (PackageList *)pkglist;
  PackageList *tmplist;

  tmplist = pkglisthead->next;
  while (tmplist != pkglisthead)
    {
      if (strstr (tmplist->pkg->name, str) != NULL)
        insert_node_to_store (appdata, store, tmplist->pkg);
      tmplist = tmplist->next;
    }
}
