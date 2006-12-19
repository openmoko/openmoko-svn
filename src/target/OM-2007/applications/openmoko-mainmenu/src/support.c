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

#include "support.h"

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <glib-object.h>

 
 /**
*@brief fill model
*@param store		GtkListSrore*
*@param icon_path	const char*
*@param icon_name	const char*
*@return Bool
*/
gboolean
moko_fill_model(GtkListStore *store, const char* icon_path, const char* icon_name) {
    if (!icon_path || !icon_name)
        return FALSE;

    GtkTreeIter iter;
    GdkPixbuf *pixbuf;

    gtk_list_store_append (store, &iter);
    pixbuf = gdk_pixbuf_new_from_file_at_size (icon_path, ICON_SIZE_W, ICON_SIZE_H, NULL);// ADD Gerro handle later

    gtk_list_store_set (store, &iter, PIXBUF_COLUMN, pixbuf, TEXT_COLUMN, icon_name, -1);
    g_object_unref (pixbuf);

    return TRUE;
}

void 
moko_sample_model_fill(GtkListStore *store) {
    moko_fill_model(store,"/usr/share/pixmaps/abiword.png","abiword");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/anjuta.xpm","anjuta");
    moko_fill_model(store,"/usr/share/pixmaps/battstat.png","battstat");
    moko_fill_model(store,"/usr/share/pixmaps/gdm.png","gdm");
    moko_fill_model(store,"/usr/share/pixmaps/gdm-setup.png","gdm-setup");
    moko_fill_model(store,"/usr/share/pixmaps/gnome-eyes.png","gnome-eyes");
    moko_fill_model(store,"/usr/share/pixmaps/gnome-geg12.png","gnome-geg12");
    moko_fill_model(store,"/usr/share/pixmaps/gdm.xpm","gdm");
}

void
moko_set_label_content(GtkLabel *label, const char *content) {
    if (label)
    	  gtk_label_set_text (label, content);
    }

/* Use this function to set the directory containing installed pixmaps. */
/*void
add_pixmap_directory(const gchar *directory){
    pixmaps_directories = g_list_prepend (pixmaps_directories,
    						g_strdup (directory));
}
*/

