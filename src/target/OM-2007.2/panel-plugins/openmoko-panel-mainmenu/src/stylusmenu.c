/*
 *  openmoko-mainmenu
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2.0 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#include "mokodesktop_item.h"
#include "stylusmenu.h"
//#include "callbacks.h"

#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn-launcher.h>
#include <gdk/gdkx.h>

static GtkImageMenuItem *moko_build_new_menu_item(const char *icon_name,
						  const char *icon_path);
static void moko_stylus_menu_activate_item(GtkWidget * widget, void *user_data);

void moko_stylus_menu_build(GtkMenu * menu, MokoDesktopItem * item)
{
	//GtkMenu *sub_menu;
	GtkImageMenuItem *menu_item;

	MokoDesktopItem *item_new;
	g_debug("menu build-------------------------V");
	mokodesktop_items_enumerate_siblings(item->item_child, item_new) {

		if (access(item_new->icon_name, 0) == 0) {
			g_debug("item patch %s", item_new->icon_name);
			menu_item =
			    moko_build_new_menu_item(item_new->name,
						     item_new->icon_name);
		} else {
			char path[PATH_MAX];
			snprintf(path, PATH_MAX, "%s/%s", PIXMAP_PATH,
				 item_new->icon_name);
			if (access(path, 0) == 0)
				menu_item =
				    moko_build_new_menu_item(item_new->name,
							     path);
			else {
				snprintf(path, PATH_MAX, "%s/%s", PKGDATADIR,
					 "default-app-icon.xpm");
				menu_item =
				    moko_build_new_menu_item(item_new->name,
							     path);
				//moko_fill_model(self->list_store, path, item_new->name, item_new);
			}
		}
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),
				      GTK_WIDGET(menu_item));
		gtk_widget_show(GTK_WIDGET(menu_item));
		switch (item_new->type) {
		case ITEM_TYPE_FOLDER:
			{
				MokoDesktopItem *tmp_item;
				GtkWidget *sub_menu;
				sub_menu = gtk_menu_new();
				gtk_menu_item_set_submenu(GTK_MENU_ITEM
							  (menu_item),
							  GTK_WIDGET(sub_menu));
				mokodesktop_items_enumerate_siblings(item_new->
								     item_child,
								     tmp_item) {
					if (access(tmp_item->icon_name, 0) == 0) {
						menu_item =
						    moko_build_new_menu_item
						    (tmp_item->name,
						     tmp_item->icon_name);
					} else {
						char path[PATH_MAX];
						snprintf(path, PATH_MAX,
							 "%s/%s", PIXMAP_PATH,
							 tmp_item->icon_name);
						if (access(path, 0) == 0)
							menu_item =
							    moko_build_new_menu_item
							    (tmp_item->name,
							     path);
						else {
							snprintf(path, PATH_MAX,
								 "%s/%s",
								 PKGDATADIR,
								 "default-app-icon.xpm");
							menu_item =
							    moko_build_new_menu_item
							    (tmp_item->name,
							     path);
							//moko_fill_model(self->list_store, path, item_new->name, item_new);
						}
					}

					if (tmp_item->type ==
					    ITEM_TYPE_DOTDESKTOP_ITEM
					    || tmp_item->type == ITEM_TYPE_APP)
						g_signal_connect(menu_item,
								 "activate",
								 G_CALLBACK
								 (moko_stylus_menu_activate_item),
								 tmp_item);
					gtk_menu_shell_append(GTK_MENU_SHELL
							      (sub_menu),
							      GTK_WIDGET
							      (menu_item));
					gtk_widget_show(GTK_WIDGET(menu_item));
				}
			}
			break;
		case ITEM_TYPE_DOTDESKTOP_ITEM:
		case ITEM_TYPE_APP:
			g_signal_connect(menu_item, "activate",
					 G_CALLBACK
					 (moko_stylus_menu_activate_item),
					 item_new);
			break;
		}		/* case */

	}			/* enumerate */

	return;
}

static GtkImageMenuItem *moko_build_new_menu_item(const char *name,
						  const char *path)
{
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	GtkWidget *item;

	pixbuf = gdk_pixbuf_new_from_file_at_size(path, 32, 32, NULL);
	if (!pixbuf)
		g_debug("Can't get pixbuf");
	image = gtk_image_new_from_pixbuf(pixbuf);

	item = gtk_image_menu_item_new_with_label(name);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	gtk_widget_show(GTK_WIDGET(item));

	return GTK_IMAGE_MENU_ITEM(item);
}

static void moko_stylus_menu_activate_item(GtkWidget * widget, void *user_data)
{
	MokoDesktopItem *ditem = user_data;
	char* child = (char *)ditem->data;
	SnLauncherContext* sn_context;
	SnDisplay* sn_dpy;
	Display* display;
	int screen;
	pid_t pid = 0;

	g_debug("item activated: %s", ditem->data);
    
	display = gdk_x11_display_get_xdisplay(gtk_widget_get_display (widget));
	sn_dpy = sn_display_new(display, NULL, NULL);

	screen = gdk_screen_get_number(gtk_widget_get_screen (widget));
	sn_context = sn_launcher_context_new(sn_dpy, screen);
	sn_display_unref(sn_dpy);

    sn_launcher_context_set_binary_name(sn_context, child);
	sn_launcher_context_initiate(sn_context, "openmoko-panel-mainmenu", child, CurrentTime);
	
	switch ((pid =fork())) {
	case 0:
	    sn_launcher_context_setup_child_process(sn_context);
		mb_exec(child);
	    g_warning("Failed to exec %s", child);
		exit(1);
	case -1:
	    g_warning("Failed to fork %s", child);
		break;
	}

	sn_launcher_context_unref(sn_context);
}

void
moko_menu_position_cb(GtkMenu * menu, int *x, int *y, gboolean * push_in,
		      GtkWidget * data)
{
	GtkAllocation *allocation = &GTK_WIDGET(data)->allocation;

	*x = allocation->x;
	*y = allocation->y + allocation->height;

	*push_in = TRUE;
}
