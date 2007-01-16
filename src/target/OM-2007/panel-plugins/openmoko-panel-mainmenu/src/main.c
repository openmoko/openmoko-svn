/*  main.c
 *
 *  Authored By Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2007 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/12/21 18:03:04 $) [$Author: mickey $]
 */

#include "moko-panel-mainmenu.h"

#include <gtk/gtk.h>
#include <libmb/mb.h>

int
main (int argc, char **argv)
{  
    MokoPanelMainmenu *app;
  
    moko_panel_system_init (&argc, &argv);
    
    app = moko_panel_mainmenu_new();
    moko_panel_applet_set_icon (MOKO_PANEL_APPLET (app), PKGDATADIR"/btn_menu.png");

    gtk_main ();
    
    return;
}
