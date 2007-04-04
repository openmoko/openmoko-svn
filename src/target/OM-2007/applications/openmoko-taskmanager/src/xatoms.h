/**
 *  xatoms.h
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef _TASK_MANAGER_X_ATOMS_H
#define _TASK_MANAGER_X_ATOMS_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>

enum {
  WM_PROTOCOLS,
  _NET_WM_PING,
  WM_DELETE_WINDOW,
  _NET_CLIENT_LIST,
  _NET_CLIENT_LIST_STACKING,
  _NET_ACTIVE_WINDOW,
  _NET_CLOSE_WINDOW,
  _NET_SHOW_DESKTOP,
  WM_STATE,
  _NET_WM_WINDOW_TYPE,
  _NET_WM_WINDOW_TYPE_DESKTOP,
  _NET_WM_WINDOW_TYPE_DOCK,
  _NET_WM_WINDOW_TYPE_NORMAL,
  _NET_WM_WINDOW_TYPE_TOPLEVEL,
  _NET_WM_NAME,
  _NET_WM_ICON,
  _NET_WM_ID,
  WM_CLIENT_LEADER,
  _NET_CLIENT_ID,
  _MB_CURRENT_APP_WINDOW,
  _MB_COMMAND,
  UTF8_STRING,
  MAX_ATOM_NO
};

Atom atoms[MAX_ATOM_NO];

gboolean moko_initialize_X_atoms(const Display* dpy);

#endif /*_TASK_MANAGER_X_ATOMS_H*/
