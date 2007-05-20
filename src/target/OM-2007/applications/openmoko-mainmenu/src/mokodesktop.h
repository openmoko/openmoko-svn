#ifndef _MOKODESKTOP_H
#define _MOKODESKTOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#include <libmb/mb.h>

#define VFOLDERDIR		"/usr/share/matchbox"
#define DD_DIR			"/usr/share/applications/"

#define PIXMAP_PATH		"/usr/share/pixmaps"

#ifdef DEBUG
#define DBG(txt, args... ) fprintf(stderr, "DT-DEBUG: " txt , ##args )
#else
#define DBG(txt, args... ) /* nothing */
#endif


enum {
  ITEM_TYPE_UNKNOWN = 0,
  ITEM_TYPE_ROOT,

  ITEM_TYPE_DOTDESKTOP_FOLDER,
  ITEM_TYPE_DOTDESKTOP_ITEM,
  ITEM_TYPE_MODULE_ITEM,
  ITEM_TYPE_MODULE_WINDOW,
  ITEM_TYPE_APP,
  ITEM_TYPE_FOLDER,  /* Same as 'official' Directory */
  ITEM_TYPE_LINK,    /* URL  */

  ITEM_TYPE_FSDEVICE,
  ITEM_TYPE_MIMETYPE,
  ITEM_TYPE_DIRECTORY,
  ITEM_TYPE_SERVICE,
  ITEM_TYPE_SERVICETYPE ,

  ITEM_TYPE_TASK_FOLDER,		/* Not official */
  ITEM_TYPE_PREVIOUS,
  ITEM_TYPE_CNT,
};

typedef void (*MokoDesktopCB)( void *data1, void *data2 ) ;

typedef struct _mokodesktop_item {

  int type;
  int subtype; /* user defined type */

  char *name;
  char *name_extended;
  char *comment;
  char *icon_name;
  void *data;

  MokoDesktopCB activate_cb;

  struct _mokodesktop_item *item_next_sibling;
  struct _mokodesktop_item *item_prev_sibling;
  struct _mokodesktop_item *item_child;
  struct _mokodesktop_item *item_parent;

} MokoDesktopItem;

#endif
