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
#include "mokodesktop.h"
#include "mokodesktop_item.h"

#include "stylusmenu.h"
#include "callbacks.h"

struct _MokoStylusMenuPrivate{

};
enum {
    MENU_SIGNAL = 0,
    LAST_SIGNAL
};

static void moko_stylus_menu_class_init(MokoStylusMenuClass *klass);

static void moko_stylus_menu_init(MokoStylusMenu *self);

static guint stylus_menu_signals[LAST_SIGNAL] = { 0 };

GType 
moko_stylus_menu_get_type (void) /* Typechecking */
{
    static GType menu_type = 0;

    if (!menu_type)
    {
        static const GTypeInfo menu_info =
        {
            sizeof (MokoStylusMenuClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_stylus_menu_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoStylusMenu),
            0,
            (GInstanceInitFunc) moko_stylus_menu_init,
            NULL
        };
        menu_type = g_type_register_static (GTK_TYPE_MENU_SHELL, "MokoStylusMenu", &menu_info, 0);
    }

    return menu_type;
}

void
moko_stylus_menu_class_init (MokoStylusMenuClass * klass)
{

}

void 
moko_stylus_menu_init (MokoStylusMenu * self)
{
}

MokoStylusMenu *
moko_stylus_menu_new ()
{
    return STYLUSMENU(g_object_new(moko_stylus_menu_get_type, NULL));
}

static GtkImageMenuItem *moko_build_new_menu_item (const char *icon_name, const char *icon_path);

void
moko_stylus_menu_build (GtkMenu *menu, MokoDesktopItem *item)
{
  //GtkMenu *sub_menu;
  GtkImageMenuItem *menu_item;
    
  MokoDesktopItem *item_new;

  mokodesktop_items_enumerate_siblings(item->item_child, item_new)
  { 
     
     if (access (item_new->icon_name, 0) == 0)
     {
         g_debug ("item patch %s", item_new->icon_name);
         menu_item = moko_build_new_menu_item (item_new->name, item_new->icon_name);
     }
     else 
     {
       char path[512];
       snprintf (path, 512, "%s/%s", PIXMAP_PATH, item_new->icon_name);
       if (access (path, 0) == 0)
           menu_item = moko_build_new_menu_item (item_new->name, path);
       else
         {
	     snprintf (path, 512, "%s/%s", PKGDATADIR, "default-app-icon.xpm");
	     menu_item = moko_build_new_menu_item (item_new->name, path);
	     //moko_fill_model(self->list_store, path, item_new->name, item_new);
         }
      }
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), GTK_WIDGET(menu_item));
    gtk_widget_show (GTK_WIDGET(menu_item));

    if (item_new->type == ITEM_TYPE_FOLDER)
    {
      MokoDesktopItem *tmp_item;
      GtkWidget *sub_menu;
      sub_menu = gtk_menu_new();
      gtk_menu_item_set_submenu (GTK_MENU_ITEM(menu_item), GTK_WIDGET(sub_menu));
      mokodesktop_items_enumerate_siblings(item_new->item_child, tmp_item)
      {
        if (access (tmp_item->icon_name, 0) == 0)
        {
          menu_item = moko_build_new_menu_item (tmp_item->name, tmp_item->icon_name);
        }
        else 
        {
          char path[512];
          snprintf (path, 512, "%s/%s", PIXMAP_PATH, tmp_item->icon_name);
          if (access (path, 0) == 0)
            menu_item = moko_build_new_menu_item (tmp_item->name, path);
          else
          {
	        snprintf (path, 512, "%s/%s", PKGDATADIR, "default-app-icon.xpm");
	        menu_item = moko_build_new_menu_item (tmp_item->name, path);
	     //moko_fill_model(self->list_store, path, item_new->name, item_new);
           }
         }
         
		 if (tmp_item->type == ITEM_TYPE_DOTDESKTOP_ITEM ||tmp_item->type == ITEM_TYPE_APP )
           g_signal_connect (menu_item, "activate" ,G_CALLBACK(moko_stylus_menu_activate_item), tmp_item->data);
         gtk_menu_shell_append( GTK_MENU_SHELL(sub_menu), GTK_WIDGET(menu_item) );
         gtk_widget_show (GTK_WIDGET(menu_item));
      }
	}
  }

  return ;
}

static GtkImageMenuItem *
moko_build_new_menu_item(const char *name, const char *path)
{
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    GtkWidget *item;

    pixbuf = gdk_pixbuf_new_from_file_at_size (path, 32, 32, NULL);
    if(!pixbuf) g_debug ("Can't get pixbuf");
    image = gtk_image_new_from_pixbuf (pixbuf);

    item = gtk_image_menu_item_new_with_label (name);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(item), image);
    gtk_widget_show (GTK_WIDGET(item));
    
    return GTK_IMAGE_MENU_ITEM(item);
}

void
moko_menu_position_cb (GtkMenu *menu, int *x, int *y, gboolean *push_in, GtkWidget  *button)
{
    GtkAllocation* allocation = &GTK_WIDGET(button)->allocation;
    //GtkRequisition req;
    //GtkRequisition menu_req;
    //GtkOrientation orientation;
    //GtkTextDirection direction;

    gdk_window_get_origin(GTK_BUTTON(button)->event_window, x, y);

    *y += allocation->height;

    *push_in = TRUE;
}

