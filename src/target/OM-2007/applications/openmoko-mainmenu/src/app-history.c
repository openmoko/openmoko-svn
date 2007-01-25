/**
 *  @file app-history.c
 *  @brief The Main Menu in the Openmoko
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
 *
 */

#include "app-history.h"

static int current = 0;

static void 
pointer_check()
{
    if (current < MAX_RECORD_APP )
    	return;
    else
    	current = 0;
}

void
moko_hisory_app_fill(MokoPixmapButton **btn, const char *path)
{
   GtkWidget *image;
   image = gtk_image_new_from_file (path);

   if (!path)
   	return;
   pointer_check();
   moko_pixmap_button_set_finger_toolbox_btn_center_image(btn[current], image);
   current++;
}
