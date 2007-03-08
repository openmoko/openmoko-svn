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
#include "callbacks.h"

static gint current = 0;

static void 
pointer_check()
{
    if (current < MAX_RECORD_APP )
    	return;
    else
    	current = 0;
}

MokoAppHistory *
moko_app_history_init (MokoFingerToolBox *toolbox)
{
	gint i = 0;
	MokoAppHistory *self;

    if (!toolbox)
		return NULL;

	self = g_malloc0 (sizeof (MokoAppHistory));

	if (!self)
		return NULL;

	for (i; i<MAX_RECORD_APP; i++)
	{
    	self->btn[i] = moko_finger_tool_box_add_button_without_label (toolbox);
        g_signal_connect( G_OBJECT(self->btn[i]), "clicked", G_CALLBACK(moko_tool_box_btn_clicked_cb), self);
        gtk_widget_show (self->btn[i]);
		self->item[i] = NULL;
    }

	return self;
}

gboolean
moko_app_history_set (MokoAppHistory *self, GdkPixbuf *pixbuf, MokoDesktopItem *item)
{
    if (!self || !pixbuf)
		return FALSE;

	pointer_check ();

   moko_pixmap_button_set_finger_toolbox_btn_center_image_pixbuf (self->btn[current], pixbuf);

	if (item)
		self->item[current] = item;
	else
		self->item[current] = NULL;

	current ++;

	return TRUE;
}

void
moko_app_history_free (MokoAppHistory *self)
{
    if (self)
		g_free (self);
	return;
}
