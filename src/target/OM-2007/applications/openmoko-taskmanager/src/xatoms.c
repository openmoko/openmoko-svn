/**
 * @file xatoms.c
 * @brief xatoms.c based on X11/Xlib.
 * @author Sun Zhiyong
 * @date 2006-10
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
 */

 #include "xatoms.h"

 gboolean g_Atom_initialized = FALSE;

 char* atom_names[] = {
 	"WM_PROTOCOLS",
     	"_NET_WM_PING",
  	"WM_DELETE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_CLIENT_LIST_STACKING",
   	"_NET_ACTIVE_WINDOW",
   	"WM_STATE",
    	"_NET_WM_WINDOW_TYPE",
    	"_NET_WM_WINDOW_TYPE_DESKTOP",
    	"_NET_WM_WINDOW_TYPE_DOCK",
    	"_NET_WM_WINDOW_TYPE_NORMAL",
    	"_NET_WM_WINDOW_TYPE_TOPLEVEL",
    	"_NET_WM_NAME",
    	"_NET_WM_ICON",
    	"_NET_WM_ID",
    	"_NET_STARTUP_ID",
    	"_NET_CLIENT_ID",
    	"_MB_CURRENT_APP_WINDOW",
    	"_MB_COMMAND",
    	"UTF8_STRING",
};

/**
*@brief initialize openmoko footer dbus connection.
*@param dpy		Display *
*@return Bool
*/
gboolean 
om_initialize_X_atoms(const Display* dpy) {
    if (g_Atom_initialized == TRUE)
        return TRUE;
    if (dpy ==NULL)
    	 return FALSE;
    
    if (XInternAtoms (dpy, atom_names, (sizeof (atom_names) / sizeof (atom_names[0])), False, atoms)
		!= Success)
        return FALSE;
    
    g_Atom_initialized = TRUE;
    return TRUE;
    }

