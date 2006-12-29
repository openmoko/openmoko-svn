/*
 *  openmoko-mainmenu
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 First International Computer Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
 #ifndef _MOKO_MAIN_MENU_CLOSE_PAGE_H_
#define _MOKO_MAIN_MENU_CLOSE_PAGE_H_

#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <libmokoui/moko-pixmap-button.h>


#define MOKO_CLOSE_PAGE_TYPE				(moko_close_page_get_type())
#define MOKO_CLOSE_PAGE(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_CLOSE_PAGE_TYPE, MokoClosePage))
#define MOKO_CLOSE_PAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_CLOSE_PAGE_TYPE, MokoClosePageClass))
#define IS_MOKO_CLOSE_PAGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_CLOSE_PAGE_TYPE))
#define IS_MOKO_CLOSE_PAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_CLOSE_PAGE_TYPE))

typedef struct _MokoClosePage MokoClosePage;
typedef struct _MokoClosePageClass MokoClosePageClass;

struct _MokoClosePage {
	GtkVBox vbox;

	//GtkButton *close_btn;
	MokoPixmapButton *close_btn;
	GtkLabel *info[2];
};

struct _MokoClosePageClass {
	GtkVBoxClass parent_class;
};

GType
moko_close_page_get_type(void);

GtkWidget *
moko_close_page_new ();

#endif /*close-page.h*/
