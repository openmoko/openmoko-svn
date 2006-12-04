/**
 * @file pkglist.c 
 * @brief Manage the package list and relation function.
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
#include <string.h>
#include <pthread.h>

#include "pkglist.h"
#include "widgets.h"
#include "iconmgr.h"
#include "ipkgapi.h"
#include "errorcode.h"
#include "menumgr.h"
#include "interface.h"


/**
 * @brief The structor of Package list node
 */
typedef struct package_list {
  IPK_PACKAGE *pkg;                ///<Package info
  struct package_list *pre;        ///<The previous node of package list
  struct package_list *next;       ///<The next node of package list
} PACKAGE_LIST;

/**
 * @brief Section list structure.
 */
typedef struct section_list {
  char *name;                      ///<Section name
  int  sequence;                   ///<The sequence in section list
  PACKAGE_LIST head;               ///<The first node of package list at this section
  struct section_list *next;       ///<The next section list node
} SECTION_LIST;

/**
 * Install package status
 */
enum {
  STATUS_INSTALL,
  STATUS_REINIT,
  STATUS_ERROR,
  STATUS_COMPLETE
};


static SECTION_LIST sectionhead;
static gint    scount;

static PACKAGE_LIST unkown;
static PACKAGE_LIST installed;
static PACKAGE_LIST upgrade;
static PACKAGE_LIST mark;

static PKG_LIST_HEAD pkghead;

static gchar  searchhistory[MAX_LENGTH_OF_SEARCH_STRING +1];

static pthread_t threadid;

static gchar  **installinfolist;    ///!The info list of install/remove/upgrade packages
static gchar  **prepareinfolist;    ///!The info list of prepare info
static gint   preinfo;
static gint   insinfo;
static gint   displaypreinfo;
static gint   displayinsinfo;
static gint   insstatus;
static gint   disstatus;
static gboolean  requestcancel;


static void set_search_history_string (const gchar *src);
const gchar *get_search_string (void);
static int verrevcmp(const char *val, const char *ref);
static SECTION_LIST *add_new_section_node (const char *name);
static gint search_section_node (const char *name, 
                                 SECTION_LIST **section, 
                                 SECTION_LIST *sechead);
static gint insert_package_to_section_without_check (PACKAGE_LIST *pkglist, 
                                                     IPK_PACKAGE *pkg);
static gint check_package_upgradeable (PACKAGE_LIST *pkglist, 
                                       IPK_PACKAGE *pkg);
static gint insert_package_to_section (PACKAGE_LIST *pkglist, 
                                       IPK_PACKAGE *pkg);
static gint build_index_from_package_list (PKG_LIST_HEAD *pkghead, 
                                           SECTION_LIST *sechead);
static void insert_node_to_store (GtkListStore *store, 
                                  IPK_PACKAGE *pkg);
static gint put_package_list_to_store (PACKAGE_LIST *pkglist);
static void format_depends_list (char *dest, char *depends, int size);
static void update_package_detail_info (IPK_PACKAGE *pkg);
static void update_package_name (IPK_PACKAGE *pkg);
static void remove_package_from_mark_list (IPK_PACKAGE *pkg);
static void incremental_search (const gchar *str);
static void search_from_package_list (PACKAGE_LIST *pkglist, 
                                      const gchar *str);
static void free_package_list (PACKAGE_LIST *pkglist);
static void free_section_list (SECTION_LIST *seclist);
gboolean install_timeout_event (gpointer data);
void thread_dispose_marked_package (void);


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
 * @brief Create a new node of section list.
 *
 * If the param name is NULL, return NULL;
 * If it can not malloc memory for the node of section list, it will return NULL.
 * @param name The section name.
 * @return The section pointer.
 */
static SECTION_LIST *
add_new_section_node (const char *name)
{
  SECTION_LIST   *sect;

  g_return_val_if_fail (name != NULL, NULL);

  sect = g_malloc (sizeof (SECTION_LIST));
  if (sect == NULL)
    {
      g_warning ("ERROR: Malloc memory for section list error.");
      return NULL;
    }

  sect->name = g_malloc (strlen (name) +1);
  if (sect->name == NULL)
    {
      g_warning ("ERROR: Malloc memory for section list error.");
      g_free (sect);
      return NULL;
    }

  strcpy (sect->name, name);
  sect->head.pkg = NULL;
  sect->head.pre = &(sect->head);
  sect->head.next = &(sect->head);
  sect->next = NULL;

  scount++;

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
search_section_node (const char *name, SECTION_LIST **section, SECTION_LIST *sechead)
{
  SECTION_LIST  *tmp;
  SECTION_LIST  *pre;
  gint   ret;

  if (name == NULL)
    {
      DBG ("The name of section is NULL");
      return OP_SECTION_NAME_NULL;
    }

  if (!name[0])
    {
      DBG ("The name of section is NULL");
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

  tmp = add_new_section_node (name);
  if (tmp == NULL)
    {
      ERROR ("Can not create section node");
      return OP_MEMORY_MALLOC_FAIL;
    }

  tmp->next = pre->next;
  pre->next = tmp;

  *section = tmp;

  return OP_SUCCESS;

}

/**
 * @brief Insert a package node to the package list without check whether 
 * the package is upgradeable
 *
 * @param pkglist The package list
 * @param pkg The package node
 * @return The result code
 *
 * @retval OP_PACKAGE_NAME_NULL The package name is NULL, it is an error.
 * @retval OP_MEMORY_MALLOC_FAIL Can't malloc memory for new node. It is an fatal error.
 * @retval OP_INSERT_PACKAGE_SUCCESS Insert package to list successfully.
 * @retval OP_PACKAGE_LIST_NOT_INIT_CORRECTLY The package list do not init correctly.
 */
static gint 
insert_package_to_section_without_check (PACKAGE_LIST *pkglist, IPK_PACKAGE *pkg)
{
  PACKAGE_LIST  *tmp;
  PACKAGE_LIST  *ins;
  gint   ret;

  if (pkg->name == NULL)
    {
      //Package name is NULL.
      ERROR ("Package name is NULL, it will be ignore");
      return OP_PACKAGE_NAME_NULL;
    }

  tmp = pkglist->pre;

  while ((tmp != pkglist) && (tmp != NULL))
    {
      ret = strcmp (pkg->name, tmp->pkg->name);

      if (ret > 0) 
        {
          //The name of package is larger then the name of node
          ins = (PACKAGE_LIST *) g_malloc (sizeof (PACKAGE_LIST));
          if (ins == NULL)
            {
              ERROR ("Can not malloc memory for package node, the package name is:%s", pkg->name);
              return OP_MEMORY_MALLOC_FAIL;
            }
          ins->pkg = pkg;
          ins->pre = tmp;
          ins->next = tmp->next;

          tmp->next->pre = ins;
          tmp->next = ins;

          return OP_INSERT_PACKAGE_SUCCESS;
        }
      else if (ret == 0)
        {
          return OP_INSERT_PACKAGE_SUCCESS;
        }

      //The name of package is small then the name of node, search the pre node.
      tmp = tmp->pre;
    }

  if (tmp == NULL)
    {
      ERROR ("The package list of section not init correctly");
      return OP_PACKAGE_LIST_NOT_INIT_CORRECTLY;
    }

  ins = (PACKAGE_LIST *) g_malloc (sizeof (PACKAGE_LIST));
  if (ins == NULL)
    {
      ERROR ("Can not malloc memory for package node, the package name is:%s", pkg->name);
      return OP_MEMORY_MALLOC_FAIL;
    }
  ins->pkg = pkg;
  ins->pre = tmp;
  ins->next = tmp->next;

  tmp->next->pre = ins;
  tmp->next = ins;

  return OP_INSERT_PACKAGE_SUCCESS;
}

/**
 * @brief Check the packages, if the installed package is upgradeable,
 * put them to the "upgrade" package list.
 *
 * @param pkglist The package list
 * @param pkg The package node
 */
static gint 
check_package_upgradeable (PACKAGE_LIST *pkglist, IPK_PACKAGE *pkg)
{
  IPK_PACKAGE   *tmp;
  gint   ret;

  tmp = pkglist->pkg;
  if (tmp->state_status != SS_INSTALLED)
    {
      if (pkg->state_status == SS_INSTALLED)
        {
          pkglist->pkg = pkg;
          pkg = tmp;
          tmp = pkglist->pkg;
        }
      else
        {
          ret = verrevcmp (tmp->version, pkg->version);
          if (ret < 0)
            {
              pkglist->pkg = pkg;
            }
          return OP_PACKAGE_IS_NOT_UPGRADEABLE;
        }
    }

  ret = verrevcmp (tmp->version, pkg->version);
  if (ret >= 0)
    {
      //DBG ("The version of available package is small or equal to the version of installed package");
      return OP_PACKAGE_IS_NOT_UPGRADEABLE;
    }

  tmp->mark = PKG_STATUS_UPGRADEABLE;
  ret = insert_package_to_section_without_check (&upgrade, tmp);
  if (ret == OP_INSERT_PACKAGE_SUCCESS)
    {
      return OP_PACKAGE_IS_UPGRADEABLE;
    }

  return ret;
}

/**
 * @brief Insert a package node to the package list.
 *
 * @param pkglist The package list
 * @param pkg The package node
 * @return The result code
 * 
 * @retval OP_PACKAGE_NAME_NULL The package name is NULL, it is an error.
 * @retval OP_MEMORY_MALLOC_FAIL Can't malloc memory for new node. It is an fatal error.
 * @retval OP_INSERT_PACKAGE_SUCCESS Insert package to list successfully.
 * @retval OP_PACKAGE_IS_UPGRADEABLE Insert package to list successfully, 
 * and the package is an upgradeable.
 * @retval OP_PACKAGE_LIST_NOT_INIT_CORRECTLY The package list do not init correctly.
 */
static gint 
insert_package_to_section (PACKAGE_LIST *pkglist, IPK_PACKAGE *pkg)
{
  PACKAGE_LIST  *tmp;
  PACKAGE_LIST  *ins;
  gint   ret;

  if (pkg->name == NULL)
    {
      //Package name is NULL.
      ERROR ("Package name is NULL, it will be ignore");
      return OP_PACKAGE_NAME_NULL;
    }

  tmp = pkglist->pre;

  while ((tmp != pkglist) && (tmp != NULL))
    {
      ret = strcmp (pkg->name, tmp->pkg->name);

      if (ret > 0) 
        {
          //The name of package is larger then the name of node
          ins = (PACKAGE_LIST *) g_malloc (sizeof (PACKAGE_LIST));
          if (ins == NULL)
            {
              ERROR ("Can not malloc memory for package node, the package name is:%s", pkg->name);
              return OP_MEMORY_MALLOC_FAIL;
            }
          ins->pkg = pkg;
          ins->pre = tmp;
          ins->next = tmp->next;

          tmp->next->pre = ins;
          tmp->next = ins;

          return OP_INSERT_PACKAGE_SUCCESS;
        }
      else if (ret == 0)
        {
          //The name of package is equal to the name of node.
          //The package maybe an upgradeable package.
          DBG ("The package maybe upgradeable. Package name is:%s", pkg->name);
          DBG ("The pkg version 1 is:%s, The version 2 is:%s", tmp->pkg->version, pkg->version);
          return check_package_upgradeable (tmp, pkg);
        }

      //The name of package is small then the name of node, search the pre node.
      tmp = tmp->pre;
    }

  if (tmp == NULL)
    {
      ERROR ("The package list of section not init correctly");
      return OP_PACKAGE_LIST_NOT_INIT_CORRECTLY;
    }

  ins = (PACKAGE_LIST *) g_malloc (sizeof (PACKAGE_LIST));
  if (ins == NULL)
    {
      ERROR ("Can not malloc memory for package node, the package name is:%s", pkg->name);
      return OP_MEMORY_MALLOC_FAIL;
    }
  ins->pkg = pkg;
  ins->pre = tmp;
  ins->next = tmp->next;

  tmp->next->pre = ins;
  tmp->next = ins;

  return OP_INSERT_PACKAGE_SUCCESS;
}

/**
 * @brief Build the index form the list of package info.
 * @param pkghead The head of the list of package info.
 * @param sechead The header of section list
 * @return Error code.
 * @retval OP_SUCCESS Build the index success.
 */
static gint 
build_index_from_package_list (PKG_LIST_HEAD *pkghead, SECTION_LIST *sechead)
{
  IPK_PACKAGE  *tpkg;
  SECTION_LIST  *slist = NULL;

  gint iret, sret;

  tpkg = pkghead->pkg_list;

  while (tpkg != NULL)
    {
      if(tpkg->state_status == SS_INSTALLED)
        {
          tpkg->mark = PKG_STATUS_INSTALLED;
        }
      else
        {
          tpkg->mark = PKG_STATUS_AVAILABLE;
        }

      if (tpkg->state_status == SS_INSTALLED)
        {
          //Package has been installed. Add it to the installed list.
          iret = insert_package_to_section_without_check (&installed, tpkg);
          if (iret != OP_INSERT_PACKAGE_SUCCESS)
            {
              ERROR ("Insert to package to installed list error, the package name is:%s\n", 
                    tpkg->name); 

              //If the error is fatal error, return the error.
              //Else ignore this package.
              if (iret < OP_SPLIT_OF_SYSTEM_AND_COMMON_ERROR)
                {
                  return iret;
                }
              else
                {
                  tpkg = tpkg->next;
                  continue;
                }
            }
        }

      //Search the section node of package.
      sret = search_section_node (tpkg->section, &slist, sechead);
      if (sret == OP_SUCCESS)
        {
          //Find the section node.
          iret = insert_package_to_section (& (slist->head), tpkg);
        }
      else if (sret == OP_SECTION_NAME_NULL)
        {
          //The section name of package is NULL, Insert to "unkown" list.
          iret = insert_package_to_section (&unkown, tpkg);
        }
      else
        {
          //There is error when search the section from section list.
          return iret;
        }

      //Check the result of insert package to section.
      if (iret == OP_INSERT_PACKAGE_SUCCESS)
        {
          //Insert successfully, continue to the next.
          tpkg = tpkg->next;
          continue;
        }
      else if (iret == OP_PACKAGE_NAME_NULL)
        {
          //The package name is NULL, It is an error, only ignore it.
          tpkg = tpkg->next;
          continue;
        }
      else if (iret == OP_PACKAGE_IS_UPGRADEABLE)
        {
          //Package is upgradeable.
          //TODO:
          tpkg = tpkg->next;
          continue;
        }
      else
        {
          return iret;
        }
    } //end while (tpkg != NULL)

  return OP_SUCCESS;
}

/**
 * @brief Insert node to store
 */
static void 
insert_node_to_store (GtkListStore *store, IPK_PACKAGE *pkg)
{
  GtkTreeIter   iter;
  GdkPixbuf    *pix = NULL;

  pix = get_package_status_icon (pkg->mark);

  gtk_list_store_append (store, &iter);

  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, pkg->name,
                      COL_SIZE, pkg->size,
                      COL_POINTER, pkg,
                      -1);

}

/**
 * @brief Set the package list to package list store.
 */
static gint 
put_package_list_to_store (PACKAGE_LIST *pkglist)
{
  GtkWidget     *treeview;
  GtkListStore  *store;
  GtkTreeModel  *model;

  PACKAGE_LIST  *tmplist;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      ERROR ("Init tree view fail. The system not init the tree view of package list correctly");
      return OP_NOT_FIND_TREE_VIEW_PACKAGE;
    }

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

  store = GTK_LIST_STORE (model);
  if (store == NULL)
    {
      ERROR ("The store of package list not init correctly");
      return OP_PACKAGE_STORE_NOT_INIT_CORRECTLY;
    }

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);
  gtk_list_store_clear (store);

  tmplist = pkglist->next;

  while (tmplist != pkglist)
    {
      insert_node_to_store (store, tmplist->pkg);
      tmplist = tmplist->next;
    }

  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  g_object_unref (model);

  return OP_SUCCESS;
}

/**
 * @brief Change the status of package list
 * @param id The new status id.
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint 
change_package_list_status (gint id)
{
  gint  ret = OP_SUCCESS;
  gint  cid, lid;

  get_filter_menu_id (&cid, &lid);
  if (cid == id)
    {
      DBG ("The status are same as the lastest, need not update");
      return OP_SUCCESS;
    }

  switch (id)
    {
      case FILTER_MENU_SEARCH_RESULT:
        break;

      case FILTER_MENU_INSTALLED:
        ret = put_package_list_to_store (&installed);
        set_search_history_string ("");
        break;

      case FILTER_MENU_UPGRADEABLE:
        ret = put_package_list_to_store (&upgrade);
        set_search_history_string ("");
        break;

      case FILTER_MENU_SELECTED:
        ret = put_package_list_to_store (&mark);
        set_search_history_string ("");
        break;

      default:
        if (id > (scount +FILTER_MENU_NUM))
          {
            ERROR ("The filter id is not a correct on. It is:%d. The biggest is:%d", \
                   id, scount + FILTER_MENU_NUM);
            return OP_FILTER_ID_NOT_CORRECT;
          }
        else
        {
          gint  i,j;
          SECTION_LIST  *tmp;

          j = id - FILTER_MENU_NUM;
          if (j == scount)
            {
              ret = put_package_list_to_store (&unkown);
              set_search_history_string ("");
            }
          else
            {
              tmp = sectionhead.next;
              for (i=0; i<j; i++)
                {
                  tmp = tmp->next;
                }
              ret = put_package_list_to_store (&(tmp->head));
              set_search_history_string ("");
            }
        }
        break;
    }

  change_filter_menu_id (id);

  return ret;
}

/**
 * @brief Init all data of package list
 *
 * Get the detail info of every packages, and build the index.
 * @return Error code
 * @retval OP_SUCCESS Build index success.
 */
gint 
init_package_list_data (void)
{
  int ret;

  pkghead.length = 0;
  pkghead.pkg_list = NULL;

  scount = 0;

  sectionhead.name = NULL;
  sectionhead.head.pkg = NULL;
  sectionhead.head.pre = &(sectionhead.head);
  sectionhead.head.next = &(sectionhead.head);
  sectionhead.next = NULL;

  unkown.pkg = NULL;
  unkown.pre = &unkown;
  unkown.next = &unkown;

  installed.pkg = NULL;
  installed.pre = &installed;
  installed.next = &installed;

  upgrade.pkg = NULL;
  upgrade.pre = &upgrade;
  upgrade.next = &upgrade;

  mark.pkg = NULL;
  mark.pre = &mark;
  mark.next = &mark;

  ret = ipkg_initialize (0);
  if (ret != 0)
    {
      //Can't get the available package list
      DBG ("%s", get_error_msg());
      return OP_INIT_IPKG_MODEL_FAIL;
    }

  ret = ipkg_list_available_cmd (&pkghead);
  if (ret != 0)
    {
      //Can't get the available package list
      DBG ("%s", get_error_msg());
      return OP_INIT_IPKG_MODEL_FAIL;
    }

  ret = build_index_from_package_list (&pkghead, &sectionhead);

  DBG ("The package number is: %d. The section number is: %d", pkghead.length, scount);

  if (ret != OP_SUCCESS)
    {
      ERROR ("Build package index error");
      return ret;
    }

  change_package_list_status (FILTER_MENU_INSTALLED);
  //ret = put_package_list_to_store (&(sectionhead.next->head));

  return OP_SUCCESS;
}

/**
 * @brief Init the filter menu.
 */
void
init_filter_menu (void)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  gint  i;
  SECTION_LIST  *tmp;

  menu = create_filter_menu ();
  tmp = sectionhead.next;

  for (i=0; i<scount; i++)
    {
      menuitem = gtk_menu_item_new_with_mnemonic (tmp->name);
      gtk_widget_show (menuitem);
      gtk_container_add (GTK_CONTAINER (menu), menuitem);

      g_signal_connect ((gpointer) menuitem, "activate",
                        G_CALLBACK (on_category_activate),
                        GINT_TO_POINTER (i));
      tmp = tmp->next;
    }

  if (unkown.next != (&unkown))
    {
      menuitem = gtk_menu_item_new_with_mnemonic ("unkown");
      gtk_widget_show (menuitem);
      gtk_container_add (GTK_CONTAINER (menu), menuitem);

      g_signal_connect ((gpointer) menuitem, "activate",
                        G_CALLBACK (on_category_activate),
                        GINT_TO_POINTER (scount));
    }
}

/**
 * @brief Format the depends list of package.
 * @param depends The depends list
 * @param The dest string
 */
static void 
format_depends_list (char *dest, char *depends, int size)
{
  int i = 0;
  char *src;

  src = depends;
  dest[i++] = '\t';
  dest[i++] = '*';
  while (*src)
    {
      if (*src == ',')
        {
          dest[i++] = '\n';
          dest[i++] = '\t';
          dest[i++] = '*';
          src++;
        }
      dest[i++] = *src;
      src++;
    }
  dest[i] = 0;
}

/**
 * @brief Update the package detail infomation in the detail area.
 * @param pkg The package that selected
 */
static void 
update_package_detail_info (IPK_PACKAGE *pkg)
{
  GtkWidget   *detailinfo;
  GtkTextBuffer  *buffer;
  GtkTextIter    start, insert;
  gint           positions, positione;
  GtkTextTagTable   *tagtable;

  detailinfo = get_widget_pointer (FIC_WIDGET_TEXT_VIEW_DETAIL);
  if (detailinfo == NULL)
    {
      ERROR ("Can not get the textdetail widget.");
      return;
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (detailinfo));
  gtk_text_buffer_set_text (buffer, pkg->description, -1);

  tagtable = gtk_text_buffer_get_tag_table (buffer);
  if (gtk_text_tag_table_lookup (tagtable, "bold") == NULL)
    {
      gtk_text_buffer_create_tag (buffer, "bold",
                                  "weight", PANGO_WEIGHT_BOLD,
                                  "scale", 1.1,
                                  NULL);
    }

  if (pkg->version != NULL)
    {
      gtk_text_buffer_get_end_iter (buffer, &insert);
      positions = gtk_text_iter_get_offset (&insert);
      gtk_text_buffer_insert (buffer, &insert, "\nVersion:\n\t", -1);

      gtk_text_buffer_get_end_iter (buffer, &insert);
      positione = gtk_text_iter_get_offset (&insert);
      gtk_text_buffer_insert (buffer, &insert, pkg->version, -1);

      gtk_text_buffer_get_iter_at_offset (buffer, &start, positions);
      gtk_text_buffer_get_iter_at_offset (buffer, &insert, positione);
      gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &insert);
    }

  if (pkg->depends != NULL)
    {
      char  *dep;
      int   size = strlen (pkg->depends) *2;
      dep = g_malloc (size);
      if (dep == NULL)
        {
          ERROR ("Can not malloc space for depends list.");
          return;
        }

      gtk_text_buffer_get_end_iter (buffer, &insert);
      positions = gtk_text_iter_get_offset (&insert);
      gtk_text_buffer_insert (buffer, &insert, "\nDepends:\n", -1);

      gtk_text_buffer_get_end_iter (buffer, &insert);
      positione = gtk_text_iter_get_offset (&insert);
      format_depends_list (dep, pkg->depends, size);
      gtk_text_buffer_insert (buffer, &insert, dep, -1);

      gtk_text_buffer_get_iter_at_offset (buffer, &start, positions);
      gtk_text_buffer_get_iter_at_offset (buffer, &insert, positione);
      gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &insert);

      g_free (dep);
    }
}

/**
 * @brief Update the package name and package description in the detail area.
 * @param pkg The package that selected
 */
static void 
update_package_name (IPK_PACKAGE *pkg)
{
  GtkWidget   *appname;
  GtkTextBuffer  *buffer;
  GtkTextIter   start,insert;
  GtkTextTagTable   *tagtable;

  appname = get_widget_pointer (FIC_WIDGET_TEXT_VIEW_TEXTAPPNAME);
  if (appname == NULL)
    {
      ERROR ("Can not get the textappname widget.");
      return;
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appname));
  gtk_text_buffer_set_text (buffer, pkg->name, -1);

  tagtable = gtk_text_buffer_get_tag_table (buffer);
  if (gtk_text_tag_table_lookup (tagtable, "bold") == NULL)
    {
      gtk_text_buffer_create_tag (buffer, "bold",
                                  "weight", PANGO_WEIGHT_BOLD,
                                  "scale", 1.1,
                                  NULL);
    }

  gtk_text_buffer_get_end_iter (buffer, &insert);
  gtk_text_buffer_insert (buffer, &insert, "\n", -1);

  gtk_text_buffer_get_end_iter (buffer, &insert);
  gtk_text_buffer_insert (buffer, &insert, pkg->maintainer, -1);

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_start_iter (buffer, &insert);
  gtk_text_iter_forward_line (&insert);
  gtk_text_buffer_apply_tag_by_name (buffer, "bold", &start, &insert);
}

/**
 * @brief Update the Detail info with the selected row.
 */
void 
update_detail_info (GtkWidget *widget)
{
  GtkTreeModel     *model;
  GtkTreeIter      iter;
  GtkTreeSelection *selection;
  IPK_PACKAGE      *pkg;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
      update_package_name (pkg);     
      update_package_detail_info (pkg);     
    }
}

/**
 * @brief Get the select status from package infomation.
 * @param data The package infomation
 * @return The select status
 */
gint 
get_select_status (gpointer data)
{
  IPK_PACKAGE *tmp;

  tmp = (IPK_PACKAGE *)data;
  return tmp->mark;
}

/**
 * @brief Remove the appointed package from marked package list.
 * @param pkg The package that will be removed.
 */
static void 
remove_package_from_mark_list (IPK_PACKAGE *pkg)
{
  PACKAGE_LIST  *tmp;

  tmp = mark.next;

  while (tmp != &mark)
    {
      if(tmp->pkg == pkg)
        {
          tmp->next->pre = tmp->pre;
          tmp->pre->next = tmp->next;
          g_free (tmp);
        }
      tmp = tmp->next;
    }
}

/**
 * @brief The unmark event. Unmark the selected package.
 */
void 
package_unmark_event (void)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  IPK_PACKAGE      *pkg;
  GtkWidget        *treeview;
  GdkPixbuf        *pixbuf;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      ERROR ("Can not get the treeview widget.");
      return;
    }

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  switch (pkg->mark)
    {
      case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
        pkg->mark = PKG_STATUS_AVAILABLE;
        break;

      case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
        pkg->mark = PKG_STATUS_INSTALLED;
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        pkg->mark = PKG_STATUS_UPGRADEABLE;
        break;

      default:
        pkg->mark = PKG_STATUS_AVAILABLE;
    }
  pixbuf = get_package_status_icon (pkg->mark);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pixbuf,
                      -1);
  remove_package_from_mark_list (pkg);

}

/**
 * @brief The mark for install event. Mark the selected package for install.
 * And add the package to marked package list.
 */
void 
package_mark_install_event (void)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  IPK_PACKAGE      *pkg;
  GtkWidget        *treeview;
  GdkPixbuf        *pixbuf;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      ERROR ("Can not get the treeview widget.");
      return;
    }

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  switch (pkg->mark)
    {
      case PKG_STATUS_AVAILABLE:
        pkg->mark = PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL;
        insert_package_to_section_without_check (&mark, pkg);
        break;

      default:
        pkg->mark = PKG_STATUS_AVAILABLE;
    }
  pixbuf = get_package_status_icon (pkg->mark);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pixbuf,
                      -1);
}

/**
 * @brief The mark for upgrade event. Mark the selected package for upgrade.
 * And add the package to marked package list.
 */
void 
package_mark_upgrade_event (void)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  IPK_PACKAGE      *pkg;
  GtkWidget        *treeview;
  GdkPixbuf        *pixbuf;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      ERROR ("Can not get the treeview widget.");
      return;
    }

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  switch (pkg->mark)
    {
      case PKG_STATUS_UPGRADEABLE:
        insert_package_to_section_without_check (&mark, pkg);

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        pkg->mark = PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE;
        break;

      default:
        pkg->mark = PKG_STATUS_AVAILABLE;
    }
  pixbuf = get_package_status_icon (pkg->mark);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pixbuf,
                      -1);
}

/**
 * @brief The mark for remove event. Mark the selected package for remove.
 * And add the package to marked package list.
 */
void 
package_mark_remove_event (void)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  IPK_PACKAGE      *pkg;
  GtkWidget        *treeview;
  GdkPixbuf        *pixbuf;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      ERROR ("Can not get the treeview widget.");
      return;
    }

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  switch (pkg->mark)
    {
      case PKG_STATUS_INSTALLED:
        insert_package_to_section_without_check (&mark, pkg);
        pkg->mark = PKG_STATUS_INSTALLED_MARK_FOR_REMOVE;
        break;

      case PKG_STATUS_UPGRADEABLE:
        insert_package_to_section_without_check (&mark, pkg);
      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
        DBG ("Mark an upgradeable package to remove");
        pkg->mark = PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE;
        break;

      default:
        pkg->mark = PKG_STATUS_AVAILABLE;
    }
  pixbuf = get_package_status_icon (pkg->mark);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pixbuf,
                      -1);
}

/**
 * @brief Check whether the id is a valid dynamic section id
 *
 * If the id is bigger than the count of section, It will be an
 * "unkown" id. The return value is reversed. Ture means the id
 * is not a valid id. False means the id is a valid id.
 * 
 * @param id The section id
 * @return TRUE The id is "unkown" section id. FALSE The id is not
 * "unkown" id
 */
gboolean 
check_section_id (gint id)
{
  if (id >= scount)
    return TRUE;
  return FALSE;
}

/**
 * @brief Get the section name by section id.
 * @param id The section id
 * @return The section name
 */
gchar *
get_section_name (gint id)
{
  gint  i;
  SECTION_LIST *tmp;

  tmp = sectionhead.next;

  for (i=0; i<id; i++)
    {
      tmp = tmp->next;
    }
  return tmp->name;
}

/**
 * @brief Check the status of the marked list
 *
 * Takecare, the return value of this function is reversed. 
 * The marked list is empty when it returns TRUE. The marked list is not 
 * empty when it returns FALSE.
 * @return FALSE the marked list is not empty.\n
 *         TRUE the marked list is empty.
 */
gboolean 
check_marked_list_empty (void)
{
  if (mark.next == &mark)
    return TRUE;
  return FALSE;
}

/**
 * @brief Insert each package name in the marked list to the tree store.
 * @param store The tree store
 * @param column The column id that the package name will insert to.
 */
void 
fill_store_with_marked_list (GtkTreeStore *store, gint column)
{
  PACKAGE_LIST *tmp;
  GtkTreeIter  iter;
  gboolean     mup, min, mre;
  GtkTreeIter  iup, iin, ire;

  mup = min = mre = FALSE;
  tmp = mark.next;
  while (tmp != &mark)
    {
      switch (tmp->pkg->mark)
        {
          case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
            min = TRUE;
            break;

          case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
            mup = TRUE;
            break;

          case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
          case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
            mre = TRUE;
            break;

          default:
            ERROR ("The status of package in the marked list is error, the package name is:%s", tmp->pkg->name);
            break;
        }
      tmp = tmp->next;
    }

  if (min)
    {
      gtk_tree_store_append (store, &iin, NULL);
      gtk_tree_store_set (store, &iin, column, "To be installed", -1);
    }

  if (mup)
    {
      gtk_tree_store_append (store, &iup, NULL);
      gtk_tree_store_set (store, &iup, column, "To be upgraded", -1);
    }

  if (mre)
    {
      gtk_tree_store_append (store, &ire, NULL);
      gtk_tree_store_set (store, &ire, column, "To be removed", -1);
    }

  tmp = mark.next;
  while (tmp != &mark)
    {
      switch (tmp->pkg->mark)
        {
          case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
            gtk_tree_store_append (store, &iter, &iin);
            break;

          case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
            gtk_tree_store_append (store, &iter, &iup);
            break;

          case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
          case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
            gtk_tree_store_append (store, &iter, &ire);
            break;

          default:
            continue;
        }
      gtk_tree_store_set (store, &iter, column, tmp->pkg->name, -1);
      tmp = tmp->next;
    }
}

/**
 * @brief Dispose all marked package.
 */
void 
dispose_marked_package (void)
{
  GtkWidget  *dia;
  GtkTextBuffer *buffer;
  GtkWidget  *textview;
  PACKAGE_LIST *tmp;
  int i;
  int j;
  int ret;

  preinfo = 0;
  insinfo = 0;
  displaypreinfo = 0;
  displayinsinfo = 0;
  insstatus = STATUS_INSTALL;
  requestcancel = FALSE;
  disstatus = STATUS_INSTALL;
  tmp = mark.next;
  i = 0;
  while (tmp != &mark)
    {
      tmp = tmp->next;
      i++;
    }
  installinfolist = (gchar **) g_malloc (sizeof(gchar *) * i);
  if (installinfolist == NULL)
    {
      ERROR ("Malloc memory error");
      return;
    }
  prepareinfolist = (gchar **) g_malloc (sizeof (gchar *) *i);
  if (prepareinfolist == NULL)
    {
      ERROR ("Malloc memory error");
      g_free (installinfolist);
      installinfolist = NULL;
      return;
    }
  for (j=0; j<i; j++)
    {
      installinfolist[j] = NULL;
      prepareinfolist[j] = NULL;
    }

  ret = pthread_create (&threadid, NULL, 
                        (void *)thread_dispose_marked_package,
                        NULL);
  if (ret != 0)
    {
      ERROR ("Can not create a new thread to install/update/remove packages");
      return;
    }

  dia = get_widget_pointer (FIC_WIDGET_DIALOG_APPLYING_DIALOG);
  if (dia == NULL)
    {
      dia = create_disposedialog ();
      save_applying_dialog (dia);
    }

  textview = get_widget_pointer (FIC_WIDGET_DIALOG_APPLYING_TEXT);
  if (textview == NULL)
    {
      ERROR ("Can not find the widget of applying text");
      return ;
    }
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
  gtk_text_buffer_set_text (buffer, "", -1);

  g_timeout_add (100, (GSourceFunc)install_timeout_event, NULL);

  gtk_dialog_run (GTK_DIALOG (dia));

  gtk_widget_hide (dia);
}

/*
void 
dispose_marked_package (void)
{
  PACKAGE_LIST *tmp;
  PACKAGE_LIST *tmp1;
  gchar        *newname;
  gint         res;
  gboolean     change;

  change = FALSE;
  tmp = mark.next;
  while (tmp != &mark)
    {
      switch (tmp->pkg->mark)
        {
          case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
          case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
            res = ipkg_install_cmd (tmp->pkg->name, "root",  &newname);
            DBG ("Install package:%s, the result is %d", tmp->pkg->name, res);
            break;

          case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
          case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
            res = ipkg_remove_cmd (tmp->pkg->name);
            DBG ("Remove package:%s, the result is %d", tmp->pkg->name, res);
            break;

          default:
            continue;
        }

      if (res != 0)
        {
          ERROR ("%s", get_error_msg());
          tmp = tmp->next;
        }
      else
        {
          change = TRUE;
          tmp1 = tmp->next;
          remove_package_from_mark_list (tmp->pkg);
          tmp = tmp1;
          if (newname != NULL)
            {
              free (newname);
              newname = NULL;
            }
        }

    }
  if (change)
    {
      DBG ("Dispose success, Begin re-init the package list");
      free_all_dynamic_memory ();
      reinit_package_list_data ();
    }
}
*/

/**
 * @brief Get the sensitive of the item on the filter menu
 * @param id The id of menuitem
 * @return The sensitive of the menuitem
 */
gboolean 
get_sensitive_of_item (gint id)
{
  switch (id)
    {
      case FILTER_MENU_SEARCH_RESULT:
        if (check_search_string_empty ())
          return FALSE;
        return TRUE;

      case FILTER_MENU_INSTALLED:
        if (installed.next == &installed)
          return FALSE;
        return TRUE;

      case FILTER_MENU_UPGRADEABLE:
        if (upgrade.next == &upgrade)
          return FALSE;
        return TRUE;

      case FILTER_MENU_SELECTED:
        if (mark.next == &mark)
          return FALSE;
        return TRUE;

      default:
        break;
    }

  return FALSE;
}

/**
 * @brief Copy the input string to the search history.
 * @param src The input string
 */
static void 
set_search_history_string (const gchar *src)
{
  strncpy (searchhistory, src, MAX_LENGTH_OF_SEARCH_STRING);
  searchhistory[MAX_LENGTH_OF_SEARCH_STRING] = 0;
}

/**
 * @brief Get the search string.
 */
const gchar *
get_search_string (void)
{
  GtkWidget  *entry;

  entry = get_widget_pointer (FIC_WIDGET_ENTRY_SEARCH);
  if (entry == NULL)
    {
      ERROR ("Can not find the entry search widget");
      return NULL;
    }
  return gtk_entry_get_text (GTK_ENTRY (entry));
}

/**
 * @brief Check whether the search string is empty
 *
 * Take care, the result is reversed. TRUE means the search
 * string is empty. FALSE means the search string is not empty.
 * @return TRUE The search string is empty.
 *  FALSE The search string is not empty.
 */
gboolean 
check_search_string_empty (void)
{
  const gchar *str;

  str = get_search_string ();

  if (str == NULL)
    return TRUE;

  if (str[0] == 0)
    return TRUE;

  return FALSE;
}

/**
 * @brief Incremental search from current view
 * @param str The search string
 */
static void 
incremental_search (const gchar *str)
{
  GtkWidget    *pkgview;
  GtkTreeModel *model = NULL;
  GtkTreeIter  iter, next;
  IPK_PACKAGE  *pkg;
  gboolean     res;

  pkgview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (pkgview == NULL)
    {
      ERROR ("Error to find the widget treeview package list");
      return;
    }

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));
  //g_object_ref (model);
  //gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);

  res = gtk_tree_model_get_iter_first (model, &next);
  while (res)
    {
      iter = next;
      res = gtk_tree_model_iter_next (model, &next);

      pkg = NULL;
      gtk_tree_model_get (model, &iter, COL_POINTER, &pkg, -1);
      if (pkg == NULL)
        {
          ERROR ("Can not get the pointer in a row");
          continue;
        }
      
      if (strstr (pkg->name, (char *)str) != NULL)
        continue;

      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }

  //gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);
  //g_object_unref (model);
}

/**
 * @brief Search from a package list and put the result to store
 * @param pkglist The package list
 * @param str The search string
 */
static void 
search_from_package_list (PACKAGE_LIST *pkglist, const gchar *str)
{
  GtkWidget    *pkgview;
  GtkTreeModel *model;
  GtkListStore *store;
  PACKAGE_LIST *tmp;

  pkgview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (pkgview == NULL)
    {
      ERROR ("Error to find the widget treeview package list");
      return;
    }

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pkgview));
  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (pkgview), NULL);
  store = GTK_LIST_STORE (model);
  gtk_list_store_clear (store);

  tmp = pkglist->next;

  while (tmp != pkglist)
    {
      if (strstr (tmp->pkg->name, (char *)str) != NULL)
        {
          insert_node_to_store (store, tmp->pkg);
        }
      tmp = tmp->next;
    }

  gtk_tree_view_set_model (GTK_TREE_VIEW(pkgview), model);
  g_object_unref (model);
}

/**
 * @brief Search string that user input from the corresponding 
 * package list.
 */
void 
search_user_input (void)
{
  GtkWidget  *entry;
  const gchar *str;
  gint  cid, lid;
  PACKAGE_LIST  *pkglist = NULL;

  entry = get_widget_pointer (FIC_WIDGET_ENTRY_SEARCH);
  if (entry == NULL)
    {
      ERROR ("Can not find the entry widget");
      return;
    }

  str = gtk_entry_get_text (GTK_ENTRY (entry));
  if (str == NULL)
    {
      get_filter_menu_id (&cid, &lid);
      if (cid == FILTER_MENU_SEARCH_RESULT)
        cid = lid;
      change_package_list_status (cid);
      return;
    }
  if (str[0] == 0)
    {
      get_filter_menu_id (&cid, &lid);
      if (cid == FILTER_MENU_SEARCH_RESULT)
        cid = lid;
      change_package_list_status (cid);
      return;
    }

  if (strstr ((char *)str, (char *)searchhistory) != NULL)
    {
      incremental_search (str);
    }
  else
    {
      get_filter_menu_id (&cid, &lid);
      if (cid == FILTER_MENU_SEARCH_RESULT)
        cid = lid;
      switch (cid)
        {
          case FILTER_MENU_SEARCH_RESULT:
            ERROR ("The status of filter is error");
            return;

          case FILTER_MENU_INSTALLED:
            pkglist = &installed;
            break;

          case FILTER_MENU_UPGRADEABLE:
            pkglist = &upgrade;
            break;

          case FILTER_MENU_SELECTED:
            pkglist = &mark;
            break;

          default:
            if (cid >= (scount + FILTER_MENU_NUM))
              {
                pkglist = &unkown;
              }
            else
              {
                int  i, j;
                SECTION_LIST *tmp;
                j = cid - FILTER_MENU_NUM;
                tmp = sectionhead.next;
                for (i=0; i<j; i++)
                  {
                    tmp = tmp->next;
                  }
                pkglist = &(tmp->head);
              }
            break;

        } // end switch (cid)
      search_from_package_list (pkglist, str);
    }

  change_package_list_status (FILTER_MENU_SEARCH_RESULT);
  set_search_history_string (str);
}

/**
 * @brief Free the package list
 * @param pkglist The package list
 */
static void 
free_package_list (PACKAGE_LIST *pkglist)
{
  PACKAGE_LIST *tmp;
  PACKAGE_LIST *next;

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
free_section_list (SECTION_LIST *seclist)
{
  SECTION_LIST *tmp;
  SECTION_LIST *next;

  tmp = seclist->next;
  while (tmp != NULL)
    {
      next = tmp->next;
      free_package_list (& (tmp->head));
      g_free (tmp->name);
      g_free (tmp);
      tmp = next;
    }
  seclist->next = NULL;
}

/**
 * @brief Free all dynamic memory
 */
void 
free_all_dynamic_memory (void)
{
  GtkWidget * treeview;
  GtkTreeModel *model;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  //g_object_ref (model);
  //gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);
  gtk_list_store_clear (GTK_LIST_STORE (model));
  //gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  //g_object_unref (model);

  free_section_list (&sectionhead);
  scount = 0;

  free_package_list (&unkown);
  free_package_list (&installed);
  free_package_list (&upgrade);
  free_package_list (&mark);

  if (pkghead.pkg_list != NULL)
    {
      free_pkg_list (&pkghead);
      pkghead.pkg_list = NULL;
      pkghead.length = 0;
    }
}

/**
 * @brief Uninit the libipkg
 */
void 
uninit_libipkg (void)
{
  ipkg_uninitialize ();
}

/**
 * @brief Re-init the data of package list
 */
gint 
reinit_package_list_data (void)
{
  int ret;

  ret = ipkg_list_available_cmd (&pkghead);
  if (ret != 0)
    {
      //Can't get the available package list
      DBG ("%s", get_error_msg());
      return OP_INIT_IPKG_MODEL_FAIL;
    }

  ret = build_index_from_package_list (&pkghead, &sectionhead);

  if (ret != OP_SUCCESS)
    {
      ERROR ("Build package index error");
      return ret;
    }
  reinit_id_of_filter_menu ();
  change_package_list_status (FILTER_MENU_INSTALLED);
  return OP_SUCCESS;
}

/**
 * @brief Install package from ".ipk" file.
 * @param filename The name and path of the ".ipk" file
 */
void 
install_from_ipk_file (gchar *filename)
{
  int res;
  char *newname = NULL;

  res = ipkg_install_cmd (filename, "root", &newname);
  if (res != 0)
    {
      ERROR ("Failed to install package. The error message is:%s", get_error_msg());
      return;
    }

  free_all_dynamic_memory ();
  reinit_package_list_data ();
  if (newname != NULL)
    {
      free (newname);
    }
}

/**
 * @brief Mark all upgradeable package for upgrade
 */
void 
mark_all_upgrade (void)
{
  PACKAGE_LIST *tmp;

  tmp = upgrade.next;
  while (tmp != &upgrade)
    {
      switch (tmp->pkg->mark)
        {
          case PKG_STATUS_UPGRADEABLE:
            tmp->pkg->mark = PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE;
            insert_package_to_section_without_check (&mark, tmp->pkg);
            break;

          case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
            tmp->pkg->mark = PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE;
            break;

          default:
            break;
        }
      tmp = tmp->next;
    }

  reinit_id_of_filter_menu ();
  change_package_list_status (FILTER_MENU_SELECTED);
}

/**
 * @brief Turn on a new thread to dispose the all marked package.
 */
void
thread_dispose_marked_package (void)
{
  PACKAGE_LIST  *tmp;
  PACKAGE_LIST  *tmp1;
  gchar        *newname;
  gint         res;
  gboolean     change;
  int          i;
  int          ret;

  change = FALSE;
  tmp = mark.next;
  i = 0;
  while (tmp != &mark)
    {
      switch (tmp->pkg->mark)
        {
          case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:

            prepareinfolist[preinfo] = g_malloc (256);
            if (prepareinfolist[preinfo] == NULL)
              {
                ERROR ("Can not malloc memory");
              }
            else
              {
                ret = snprintf (prepareinfolist[preinfo], 255, 
                                "Begin install package \"%s\"\n",
                                tmp->pkg->name);
                prepareinfolist[preinfo][ret] = 0;
              }
            preinfo ++;

            res = ipkg_install_cmd (tmp->pkg->name, "root",  &newname);

            if (res == 0)
              {
                installinfolist[insinfo] = g_malloc (256);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], 255, 
                                    "Install package \"%s\" success.\n",
                                    tmp->pkg->name);
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            else
              {
                ret = strlen (get_error_msg ());
                installinfolist[insinfo] = g_malloc (ret + 128);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], ret + 127, 
                                    "Install package \"%s\" error, the error message is:%s\n",
                                    tmp->pkg->name, 
                                    get_error_msg ());
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            insinfo ++;
            break;

          case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:

            prepareinfolist[preinfo] = g_malloc (1025);
            ret = snprintf (prepareinfolist[preinfo], 1024, 
                            "Begin upgrade package \"%s\"\n",
                            tmp->pkg->name);
            prepareinfolist[preinfo][ret] = 0;
            preinfo ++;

            res = ipkg_install_cmd (tmp->pkg->name, "root",  &newname);

            if (res == 0)
              {
                installinfolist[insinfo] = g_malloc (256);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], 255, 
                                    "Upgrade package \"%s\" success.\n",
                                    tmp->pkg->name);
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            else
              {
                ret = strlen (get_error_msg ());
                installinfolist[insinfo] = g_malloc (ret + 128);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], ret + 127, 
                                    "Upgrade package \"%s\" error, the error message is:%s\n",
                                    tmp->pkg->name, 
                                    get_error_msg ());
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            insinfo ++;
            break;

          case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
          case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:

            prepareinfolist[preinfo] = g_malloc (1025);
            ret = snprintf (prepareinfolist[preinfo], 1024, 
                            "Begin remove package \"%s\"\n",
                            tmp->pkg->name);
            prepareinfolist[preinfo][ret] = 0;
            preinfo ++;

            res = ipkg_remove_cmd (tmp->pkg->name);

            if (res == 0)
              {
                installinfolist[insinfo] = g_malloc (256);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], 255, 
                                    "Remove package \"%s\" success.\n",
                                    tmp->pkg->name);
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            else
              {
                ret = strlen (get_error_msg ());
                installinfolist[insinfo] = g_malloc (ret + 128);
                if (installinfolist[insinfo] == NULL)
                  {
                    ERROR ("Can not malloc memory");
                  }
                else
                  {
                    ret = snprintf (installinfolist[insinfo], ret + 127, 
                                    "Remove package \"%s\" error, the error message is:%s\n",
                                    tmp->pkg->name, 
                                    get_error_msg ());
                    installinfolist[insinfo][ret] = 0;
                  }
              }
            insinfo ++;
            break;

          default:
            continue;
        }

      if (res != 0)
        {
          ERROR ("%s", get_error_msg());
          tmp = tmp->next;
        }
      else
        {
          change = TRUE;
          tmp1 = tmp->next;
          remove_package_from_mark_list (tmp->pkg);
          tmp = tmp1;
          if (newname != NULL)
            {
              free (newname);
              newname = NULL;
            }
        }

    }

  if (change)
    {
      DBG ("Dispose success, Begin re-init the package list");
      insstatus = STATUS_REINIT;
      free_all_dynamic_memory ();
      reinit_package_list_data ();
    }

  insstatus = STATUS_COMPLETE;
}

/**
 * @brief Time out event, update the infomation of install/remove/upgrade with
 * package.
 */
gboolean 
install_timeout_event (gpointer data)
{
  GtkWidget  *textview;
  GtkTextBuffer  *buffer;
  GtkTextIter   iter;
  gboolean   change;

  textview = get_widget_pointer (FIC_WIDGET_DIALOG_APPLYING_TEXT);
  if (textview == NULL)
    {
      ERROR ("Can not find the widget of applying text");
      return FALSE;
    }
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

  change = FALSE;
  while ((displaypreinfo < preinfo) || (displayinsinfo < insinfo))
    {
      change = TRUE;
      DBG ("dis pre=%d,dis ins=%d, ins pre=%d, ins ins=%d", 
           displaypreinfo, displayinsinfo, preinfo, insinfo);
      if (displaypreinfo == displayinsinfo)
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter, 
                                  prepareinfolist[displaypreinfo], -1);
          DBG ("The displaypreinfo is:%d", displaypreinfo);
          DBG ("The display info is:%s", prepareinfolist[displaypreinfo]);
          g_free (prepareinfolist[displaypreinfo]);
          prepareinfolist[displaypreinfo] = NULL;
          displaypreinfo ++;
        }
      else
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter,
                                  installinfolist[displayinsinfo], -1);
          DBG ("The display info is:%s", installinfolist[displayinsinfo]);
          g_free (installinfolist[displayinsinfo]);
          installinfolist[displayinsinfo] = NULL;
          displayinsinfo ++;
        }
    }
  if (change)
    {
      return TRUE;
    }

  if (insstatus == STATUS_REINIT)
    {
      if (disstatus != STATUS_REINIT)
        {
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter,
                                  "Reload the package list, please wait\n", 
                                  -1);
          DBG ("Display the reinit infomation");
          disstatus = STATUS_REINIT;
        }
      return TRUE;
    }

  if (insstatus == STATUS_ERROR)
    {
      //There is error when install/remove/upgrade packages
      //Add code later.
    }

  if (insstatus == STATUS_COMPLETE)
    {
      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert (buffer, &iter,
                              "Completely, press the close button to finish.\n",
                              -1);
      DBG ("Complete the install/upgrade/remove process");
      if (installinfolist != NULL)
        {
          g_free (installinfolist);
          installinfolist = NULL;
        }
      if (prepareinfolist != NULL)
        {
          g_free (prepareinfolist);
          prepareinfolist = NULL;
        }
      
      return FALSE;
    }
  return TRUE;
}
