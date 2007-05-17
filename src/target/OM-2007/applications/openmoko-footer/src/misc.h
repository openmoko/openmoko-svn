#ifndef _MISC_H_
#define _MISC_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MB_CMD_SET_THEME     1
#define MB_CMD_EXIT          2
#define MB_CMD_DESKTOP       3
#define MB_CMD_NEXT          4
#define MB_CMD_PREV          5
#define MB_CMD_SHOW_EXT_MENU 6
#define MB_CMD_MISC          7
#define MB_CMD_COMPOSITE     8
#define MB_CMB_KEYS_RELOAD   9

#define MB_CMD_PANEL_TOGGLE_VISIBILITY 1
#define MB_CMD_PANEL_SIZE              2
#define MB_CMD_PANEL_ORIENTATION       3

#define MB_PANEL_ORIENTATION_NORTH     1
#define MB_PANEL_ORIENTATION_EAST      2
#define MB_PANEL_ORIENTATION_SOUTH     3
#define MB_PANEL_ORIENTATION_WEST      4

void
mbcommand(Display *dpy, int cmd_id, char *data);

#endif
