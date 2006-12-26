#ifndef _HAVE_MOKODESKTOP_ITEM_H
#define _HAVE_MOKODESKTOP_ITEM_H

#include "mokodesktop.h"


#define mokodesktop_items_enumerate_siblings(item_head, item)  \
     for ( (item) = (item_head);                             \
	   (item) != NULL;                                   \
	   (item) = (item)->item_next_sibling )              \

/**
 * Constructs a new blank mbpixbuf image without an alpha channel.
 *
 * @returns a MBPixbufImage object
 */
MokoDesktopItem *mokodesktop_item_new();

/**
 * Constructs a new blank mbpixbuf image without an alpha channel.
 *
 * @param mbdesktop
 * @param item
 */
void
mokodesktop_item_free(MokoDesktopItem *item);

Bool
mokodesktop_item_folder_has_contents(MokoDesktopItem *folder);

void
mokodesktop_item_folder_contents_free( MokoDesktopItem *top_head_item,
                                       MokoDesktopItem *item);


MokoDesktopItem *
mokodesktop_item_new_with_params (  const char    *name, 
			                              const char    *icon_name,
			                              void          *data,
			                              int            type);

void
mokodesktop_items_append ( MokoDesktopItem *item_head,
			                     MokoDesktopItem *item );

void
mokodesktop_items_insert_after ( MokoDesktopItem *suffix_item,
		  	                         MokoDesktopItem *item );

void
mokodesktop_items_append_to_folder ( MokoDesktopItem  *item_folder,
				                             MokoDesktopItem  *item );

void
mokodesktop_items_append_to_top_level ( MokoDesktopItem  *top_head_item,
                                        MokoDesktopItem  *item );

void
mokodesktop_items_prepend ( MokoDesktopItem **item_head,
			                      MokoDesktopItem  *item );

MokoDesktopItem *
mokodesktop_item_get_next_sibling(MokoDesktopItem *item) ;

MokoDesktopItem *
mokodesktop_item_get_prev_sibling(MokoDesktopItem *item);

MokoDesktopItem *
mokodesktop_item_get_parent(MokoDesktopItem *item);

MokoDesktopItem *
mokodesktop_item_get_child(MokoDesktopItem *item);

MokoDesktopItem *
mokodesktop_item_get_first_sibling(MokoDesktopItem *item);

MokoDesktopItem *
mokodesktop_item_get_last_sibling(MokoDesktopItem *item);

void
mokodesktop_item_set_name ( MokoDesktopItem *item, 
			                      char            *name);

char *
mokodesktop_item_get_name (MokoDesktopItem *item);

void
mokodesktop_item_set_comment ( MokoDesktopItem *item, 
			                         char            *comment);

char *
mokodesktop_item_get_comment (MokoDesktopItem *item);

void
mokodesktop_item_set_extended_name ( MokoDesktopItem *item, 
				                             char            *name);

char *
mokodesktop_item_get_extended_name (MokoDesktopItem *item);


void
mokodesktop_item_set_user_data ( MokoDesktopItem *item, 
			                         void          *data);
void *
mokodesktop_item_get_user_data (MokoDesktopItem *item);

void
mokodesktop_item_set_type (MokoDesktopItem *item,
			                     int            type);

int
mokodesktop_item_get_type (MokoDesktopItem *item);

void
mokodesktop_item_set_activate_callback (MokoDesktopItem *item, 
				                                MokoDesktopCB    activate_cb);

void
mokodesktop_item_folder_activate_cb(void *data1, void *data2);

void
mbdesktop_item_folder_prev_activate_cb(void *data1, void *data2);
#endif
