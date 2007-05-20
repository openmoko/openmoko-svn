#include "mokodesktop_item.h"


#define WARN(txt) fprintf(stderr, "%s:%i %s %s", __FILE__, __LINE__, __func__, txt )


MokoDesktopItem *
mokodesktop_item_new()
{
  MokoDesktopItem *ditem;
  ditem = malloc(sizeof(MokoDesktopItem));
  memset(ditem, 0, sizeof(MokoDesktopItem));

  ditem->type = ITEM_TYPE_UNKNOWN;

  return ditem;
}

void
mokodesktop_item_free(MokoDesktopItem *item)
{
  if (item->name)			free(item->name);
  if (item->name_extended)	free(item->name_extended);
  if (item->comment)		free(item->comment);
  if (item->icon_name)		free(item->icon_name);

  free(item);
}

Bool
mokodesktop_item_folder_has_contents(MokoDesktopItem *folder)
{
  if (folder->item_child && folder->item_child->item_next_sibling)
    return True;
  return False;
}

void
mokodesktop_item_folder_contents_free( MokoDesktopItem *top_head_item,
                                       MokoDesktopItem *item)
{
  MokoDesktopItem *item_tmp = NULL, *item_cur = NULL;

  if (item->item_child && item->item_child->item_next_sibling)
    {
      if (item == top_head_item )
	    {
	       item_cur = item->item_child;
	       top_head_item->item_child = NULL;
	    }
      else item_cur = item->item_child->item_next_sibling;

      while (item_cur != NULL)
	    {
	       item_tmp = item_cur->item_next_sibling;

	       if (item_cur->item_child)
	            mokodesktop_item_folder_contents_free(top_head_item, item_cur->item_child);

	       mokodesktop_item_free(item_cur);
	       item_cur = item_tmp;
	    }

      if ( item != top_head_item)
	       item->item_child->item_next_sibling = NULL;
    }

  // fprintf(stdout, "moko:  free ok!\n");
}

MokoDesktopItem *
mokodesktop_item_new_with_params (  const char    *name,
			                              const char    *icon_name,
			                              void          *data,
			                              int            type)
{
  MokoDesktopItem *ditem;
  ditem = mokodesktop_item_new();

  if (name) ditem->name           = strdup(name);
  if (icon_name)
    {
      if (strlen(icon_name) > 5
	  && icon_name[strlen(icon_name)-4] != '.'
	  && icon_name[strlen(icon_name)-5] != '.')
	  {
	      ditem->icon_name = malloc(sizeof(char)*(strlen(icon_name)+5));
	      sprintf(ditem->icon_name, "%s.png", icon_name);
	    }
      else
	    {
	      ditem->icon_name = strdup(icon_name);
	    }
    }

  if (data) ditem->data           = data;

  if (type) ditem->type           = type;

  return ditem;
}


void
mokodesktop_items_append ( MokoDesktopItem *item_head,
			                     MokoDesktopItem *item )
{
  MokoDesktopItem *item_tmp = NULL;

  item_tmp = item_head;

  while ( item_tmp->item_next_sibling != NULL )
    item_tmp = item_tmp->item_next_sibling;

  item_tmp->item_next_sibling = item;
  item->item_prev_sibling = item_tmp;

  item->item_parent = item_head->item_parent;

}

void
mokodesktop_items_insert_after ( MokoDesktopItem *suffix_item,
			                         MokoDesktopItem *item )
{
  if (!suffix_item->item_next_sibling)
    {
      mokodesktop_items_append (suffix_item, item);
      return;
    }

  item->item_next_sibling = suffix_item->item_next_sibling;

  suffix_item->item_next_sibling->item_prev_sibling = item;

  suffix_item->item_next_sibling = item;

  item->item_prev_sibling = suffix_item;
}

void
mokodesktop_items_append_to_folder ( MokoDesktopItem  *item_folder,
				                             MokoDesktopItem  *item )
{
  if (!item_folder->item_child)
	{
	   item_folder->item_child = item;
	   item_folder->item_child->item_parent = item_folder;
	   return;
	}


  mokodesktop_items_append (item_folder->item_child, item);

}

void
mokodesktop_items_append_to_top_level ( MokoDesktopItem  *top_head_item,
                                        MokoDesktopItem  *item )
{
  MokoDesktopItem *top_level = top_head_item;

  item->item_parent = top_level;

  if (top_level->item_child == NULL)
    top_level->item_child = item;
  else
    mokodesktop_items_append (top_level->item_child, item);
}

void
mokodesktop_items_prepend ( MokoDesktopItem **item_head,
			                      MokoDesktopItem  *item )
{
  MokoDesktopItem *item_tmp = NULL;

  item_tmp = *item_head;
  item->item_next_sibling = item_tmp;
  item_tmp->item_prev_sibling = item;

  *item_head = item;
}

MokoDesktopItem *
mokodesktop_item_get_next_sibling(MokoDesktopItem *item)
{
  return item->item_next_sibling;
}

MokoDesktopItem *
mokodesktop_item_get_prev_sibling(MokoDesktopItem *item)
{
  return item->item_prev_sibling;
}

MokoDesktopItem *
mokodesktop_item_get_parent(MokoDesktopItem *item)
{
  MokoDesktopItem *result = mokodesktop_item_get_first_sibling(item);

  if (result && result->item_parent)
    return result->item_parent;

  return NULL;
}

MokoDesktopItem *
mokodesktop_item_get_child(MokoDesktopItem *item)
{
  return item->item_child;
}


MokoDesktopItem *
mokodesktop_item_get_first_sibling(MokoDesktopItem *item)
{
  while (item->item_prev_sibling != NULL )
    item = item->item_prev_sibling;
  return item;
}

MokoDesktopItem *
mokodesktop_item_get_last_sibling(MokoDesktopItem *item)
{
  while (item->item_next_sibling != NULL )
    item = item->item_next_sibling;
  return item;
}



void
mokodesktop_item_set_name ( MokoDesktopItem *item,
			                      char            *name)
{
  if (item->name) free(item->name);
  item->name = strdup(name);
}

char *
mokodesktop_item_get_name (MokoDesktopItem *item)
{
  return item->name;
}

void
mokodesktop_item_set_comment ( MokoDesktopItem *item,
			                         char            *comment)
{
  if (item->comment) free(item->comment);
  item->comment = strdup(comment);
}

char *
mokodesktop_item_get_comment (MokoDesktopItem *item)
{
  return item->comment;
}


void
mokodesktop_item_set_extended_name ( MokoDesktopItem *item,
				                             char            *name)
{
  if (item->name_extended) free(item->name_extended);
  item->name_extended = strdup(name);
}

char *
mokodesktop_item_get_extended_name (MokoDesktopItem *item)
{
  return item->name_extended;
}

void
mokodesktop_item_set_user_data ( MokoDesktopItem *item,
			                         void          *data)
{
  if (item->data) free(item->data);
  item->data = data;
}

void *
mokodesktop_item_get_user_data (MokoDesktopItem *item)
{
  return item->data;
}

void
mokodesktop_item_set_activate_callback (MokoDesktopItem *item,
				                                MokoDesktopCB    activate_cb)
{
  item->activate_cb = activate_cb;
}

void
mokodesktop_item_set_type (MokoDesktopItem *item,
			                     int            type)
{
  item->subtype = type;
}

int
mokodesktop_item_get_type (MokoDesktopItem *item)
{
  return item->subtype;
}

void
mokodesktop_item_folder_activate_cb(void *data1, void *data2)
{
  MokoDesktopItem *top_head_item = (MokoDesktopItem *)data1;
  MokoDesktopItem *item = (MokoDesktopItem *)data2;
}

void
mbdesktop_item_folder_prev_activate_cb(void *data1, void *data2)
{
  MokoDesktopItem *top_head_item = (MokoDesktopItem *)data1;
  MokoDesktopItem *item = (MokoDesktopItem *)data2;
}
