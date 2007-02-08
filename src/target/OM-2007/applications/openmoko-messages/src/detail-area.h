/*  detail-area.h
 *  
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: alex $]
 */

#ifndef _DETAIL_AREA_H
#define _DETAIL_AREA_H

#include <gtk/gtk.h>
#include <glib-object.h>
#include <libmokoui/moko-details-window.h>
#include <libmokoui/moko-pixmap-button.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-fixed.h>
//#include "main.h"
#include "message.h"

G_BEGIN_DECLS

#define TYPE_DETAIL_AREA detail_area_get_type()
#define DETAIL_AREA(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_DETAIL_AREA, DetailArea))
#define DETAIL_AREA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DETAIL_AREA, DetailAreaClass))
#define IS_DETAIL_AREA(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_DETAIL_AREA))
#define IS_DETAIL_AREA_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DETAIL_AREA))
#define DETAIL_AREA_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DETAIL_AREA, DetailAreaClass))

typedef struct _Read_Attributes{
    GtkWidget* from_label;
    GtkWidget* date_label;
    GtkWidget* details;
}ReadAttributes;

typedef struct _Edit_Attributes{
    GtkWidget* sendBtn;
    GtkWidget* addrBtn;
    GtkWidget* toEntry;
    GtkWidget* txtView;
}EditAttributes;

typedef struct _DetailArea{
    MokoDetailsWindow parent;
    GtkNotebook* notebook;
    GtkVBox* detailbox;
    GtkHBox* toolbox;
    MokoFixed* entryarea;
    GSList* folderlist;
    guint* page;
    ReadAttributes* readAttributes;
    EditAttributes* editAttributes;
}DetailArea;

typedef struct _DetailAreaClass{
    MokoDetailsWindowClass parent_class;
}DetailAreaClass;

GType detail_area_get_type();
GtkWidget* detail_area_new(void);

void detail_new_sms (DetailArea* self);
void detail_read_message (DetailArea* self, message* msg);
void detail_reply_message (DetailArea* self, message* msg);
void detail_forward_message (DetailArea* self, message* msg);

G_END_DECLS

#endif

