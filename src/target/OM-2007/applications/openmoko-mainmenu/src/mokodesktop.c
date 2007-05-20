#include "mokodesktop.h"
#include "mokodesktop_item.h"

static void
item_activate_cb(void *data1, void *data2);

static char* RootMatchStr = NULL;
static int ItemTypeDotDesktop = 0;


static MokoDesktopItem *
mokodesktop_get_top_level_folder(MokoDesktopItem     *top_head_item)
{
  return top_head_item;
}

static MokoDesktopItem *
get_folder_from_name ( MokoDesktopItem *top_head_item, char *name )
{
  MokoDesktopItem *item, *item_top;

  item_top = mokodesktop_get_top_level_folder(top_head_item);

  if (!strcasecmp(name, "root"))
    return item_top;

  mokodesktop_items_enumerate_siblings(mokodesktop_item_get_child(item_top), item)
    {
      if (!strcmp(name, mokodesktop_item_get_name (item)))
	    return item;
    }

  return NULL;
}

static MokoDesktopItem *
match_folder ( MokoDesktopItem *top_head_item, char *category )
{
  MokoDesktopItem *item, *item_fallback = NULL, *item_top;
  char          *match_str;

  if (category && strstr(category, "Action"))
    return NULL;

  item_top = mokodesktop_get_top_level_folder(top_head_item);

  if (RootMatchStr)
    {
      if (!strcmp("fallback", RootMatchStr))
	    {
	       item_fallback = item_top;
	    }
      else if (category && strstr(category, RootMatchStr))
	    {
	       return item_top;
	    }
    }

  mokodesktop_items_enumerate_siblings(mokodesktop_item_get_child(item_top), item)
    {
      if (mokodesktop_item_get_type (item) == ItemTypeDotDesktop)
	    {
	      match_str = (char *)mokodesktop_item_get_user_data(item);
	      if (match_str != NULL)
	      {
	        if (item_fallback == NULL && !strcmp("fallback", match_str))
		      {
		        item_fallback = item;
		        continue;
		      }
	        if (category && strstr(category, match_str))
		      {
		        return item;
		      }
	      }
	    }
    }
  return item_fallback;
}


static void
add_a_dotdesktop_item (MokoDesktopItem  *top_head_item,
		       MBDotDesktop     *dd,
		       MokoDesktopItem  *folder)
{
  MokoDesktopItem  *item_new = NULL, *item_before, *found_folder_item = NULL;
  Bool              have_attached = False;

  if (folder)
    found_folder_item = folder;
  else
    found_folder_item = match_folder( top_head_item, mb_dotdesktop_get(dd, "Categories"));

  if ( found_folder_item == NULL) return;

  item_new = mokodesktop_item_new_with_params(
					     mb_dotdesktop_get(dd, "Name"),
					     mb_dotdesktop_get(dd, "Icon"),
					     (void *)mb_dotdesktop_get_exec(dd),
					     ITEM_TYPE_DOTDESKTOP_ITEM
					     );
  if (item_new == NULL ) return;

  mokodesktop_item_set_activate_callback (item_new, item_activate_cb);

  item_before = mokodesktop_item_get_child (found_folder_item);


  if(!item_before)
  {

	mokodesktop_items_append_to_folder( found_folder_item, item_new);
	have_attached = True;
  }
  else
  {

     do
		{
         MokoDesktopItem *item_next = NULL;
         if ((item_next = mokodesktop_item_get_next_sibling(item_before)) != NULL)
		{

	      if (item_next->type == ITEM_TYPE_FOLDER
	          || item_next->type == ITEM_TYPE_PREVIOUS)
	          continue;

	      if ( (strcasecmp(item_before->name, item_new->name) < 0
		       || item_before->type == ITEM_TYPE_FOLDER
		       || item_before->type == ITEM_TYPE_PREVIOUS )
	         && strcasecmp(item_next->name, item_new->name) > 0)
	      {
	        have_attached = True;
	        mokodesktop_items_insert_after (item_before, item_new);
	        break;
	      }
	    }
       }
     while ((item_before = mokodesktop_item_get_next_sibling(item_before)) != NULL);
   }


  if (!have_attached)
    {
      mokodesktop_items_append_to_folder( found_folder_item, item_new);
    }

}



MokoDesktopItem*
mokodesktop_folder_create ( MokoDesktopItem *top_head_item,
				 char      *name,
				 char      *icon_name )
{
  MokoDesktopItem* item_folder = NULL;

  item_folder
    = mokodesktop_item_new_with_params( name,
				      icon_name,
				      NULL,
				      ITEM_TYPE_FOLDER
				      );

  mokodesktop_item_set_activate_callback (item_folder,
					mokodesktop_item_folder_activate_cb);

  return item_folder;
}

int
mokodesktop_init ( MokoDesktopItem *top_head_item,
		              int              type_reg_cnt )
{
#define APP_PATHS_N 4

  DIR *dp;
  struct stat    stat_info;

  char vfolder_path_root[512];
  char vfolder_path[512];
  char orig_wd[256];

  int   desktops_dirs_n  = APP_PATHS_N;

  int   i = 0;

  MBDotDesktopFolders     *ddfolders;
  MBDotDesktopFolderEntry *ddentry;
  MokoDesktopItem         *item_new = NULL;
  MBDotDesktop            *dd, *user_overides = NULL;

  char                     app_paths[APP_PATHS_N][256];
  struct dirent          **namelist;
/*
	top_head_item  = mokodesktop_item_new_with_params ("Home",
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );
*/
	ItemTypeDotDesktop  = type_reg_cnt;

  snprintf( vfolder_path_root, 512, "%s/.matchbox/vfolders/Root.directory",
	    mb_util_get_homedir());
  snprintf( vfolder_path, 512, "%s/.matchbox/vfolders",
	    mb_util_get_homedir());

 if (stat(vfolder_path_root, &stat_info))
    {
      snprintf(vfolder_path_root, 512, VFOLDERDIR"/vfolders/Root.directory");
      snprintf(vfolder_path, 512, VFOLDERDIR "/vfolders" );
    }

fprintf(stdout, "moko: vfolder_path_root=[%s]\n", vfolder_path_root);
fprintf(stdout, "moko: vfolder_path=[%s]\n", vfolder_path);

  dd = mb_dotdesktop_new_from_file(vfolder_path_root);

  if (!dd)
    {
      fprintf( stderr, "mokodesktop: cant open %s\n", vfolder_path );
      return -1;
    }

  RootMatchStr = mb_dotdesktop_get(dd, "Match");

fprintf(stdout, "moko: RootMatchStr=[%s]\n", RootMatchStr);

  mokodesktop_item_set_name (top_head_item,
			   mb_dotdesktop_get(dd, "Name"));
  ddfolders = mb_dot_desktop_folders_new(vfolder_path);

  mb_dot_desktop_folders_enumerate(ddfolders, ddentry)
    {
      item_new
	         = mokodesktop_folder_create ( top_head_item,
					   mb_dot_desktop_folder_entry_get_name(ddentry),
					   mb_dot_desktop_folder_entry_get_icon(ddentry));

      mokodesktop_item_set_user_data (item_new,
				    (void *)mb_dot_desktop_folder_entry_get_match(ddentry));

      mokodesktop_item_set_type (item_new, ItemTypeDotDesktop);

      mokodesktop_items_append_to_top_level (top_head_item, item_new);
    }


  snprintf(app_paths[0], 256, "%s/applications", DATADIR);
  snprintf(app_paths[1], 256, "/usr/share/applications");
  snprintf(app_paths[2], 256, "/usr/local/share/applications");
  snprintf(app_paths[3], 256, "%s/.applications", mb_util_get_homedir());

  if (getcwd(orig_wd, 255) == (char *)NULL)
    {
      fprintf(stderr, "Cant get current directory\n");
      return -1;
    }

  for (i = 0; i < desktops_dirs_n; i++)
    {
      int   n = 0, j = 0;

      if (i > 0 && !strcmp(app_paths[0], app_paths[i]))
	        continue;

      if ((dp = opendir(app_paths[i])) == NULL)
	    {
	       fprintf(stderr, "mokodesktop: failed to open %s\n", app_paths[i]);
	       continue;
	    }

      chdir(app_paths[i]);

      //n = scandir(".", &namelist, 0, alphasort);
      n = scandir(".", &namelist, 0, NULL);


      while (j < n && n > 0)
	    {
		  if (namelist[j]->d_name[0] ==  '.')
	         goto end;

	      if (strcmp(namelist[j]->d_name+strlen(namelist[j]->d_name)-8,".desktop"))
	         goto end;

	      lstat(namelist[j]->d_name, &stat_info);
	      if (!(S_ISDIR(stat_info.st_mode)))
	      {
	         MBDotDesktop *dd;
	         dd = mb_dotdesktop_new_from_file(namelist[j]->d_name);
	         if (dd)
		       {
		          if (mb_dotdesktop_get(dd, "Type") 
		              && !strcmp(mb_dotdesktop_get(dd, "Type"), "Application")
		              && mb_dotdesktop_get(dd, "Name")
		              && mb_dotdesktop_get(dd, "Exec"))
		          {
		              MokoDesktopItem *folder = NULL;
		              char             full_path[512];
		              char          *folder_name = NULL;

				      add_a_dotdesktop_item (top_head_item, dd, folder);
		          }
		          mb_dotdesktop_free(dd);
		       }
	      }
	      end:
	         free(namelist[j]);
	         ++j;
	    }

      closedir(dp);
      free(namelist);
    }

  chdir(orig_wd);

  return 1;
}



static void
item_activate_cb(void *data1, void *data2)
{
  MokoDesktopItem *top_head_item = (MokoDesktopItem *)data1;
  MokoDesktopItem *item = (MokoDesktopItem *)data2;

  switch (fork())
    {
    case 0:
      mb_exec((char *)item->data);
      fprintf(stderr, "exec failed, cleaning up child\n");
      exit(1);
    case -1:
      fprintf(stderr, "can't fork\n");
      break;
    }
}

