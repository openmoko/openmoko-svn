/*
 * gui.c - Editor GUI core
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <gtk/gtk.h>

#include "inst.h"
#include "obj.h"
#include "dump.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_status.h"
#include "gui_canvas.h"
#include "gui_tools.h"
#include "gui_frame.h"
#include "gui.h"


GtkWidget *root;

static GtkWidget *frames_box;


/* ----- menu bar ---------------------------------------------------------- */


static void menu_save(GtkWidget *widget, gpointer user)
{
	dump(stdout);
}


static void make_menu_bar(GtkWidget *vbox)
{
	GtkWidget *bar;
	GtkWidget *file_menu, *file, *quit, *save;

	bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

	file_menu = gtk_menu_new();

	file = gtk_menu_item_new_with_label("File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(bar), file);

	save = gtk_menu_item_new_with_label("Save");
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save);
	g_signal_connect(G_OBJECT(save), "activate",
	    G_CALLBACK(menu_save), NULL);

	quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit);
	g_signal_connect(G_OBJECT(quit), "activate",
	    G_CALLBACK(gtk_main_quit), NULL);
}


/* ----- central screen area ----------------------------------------------- */


static void make_center_area(GtkWidget *vbox)
{
	GtkWidget *hbox, *frames_area, *paned;
	GtkWidget *tools;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	paned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(hbox), paned, TRUE, TRUE, 0);
	
	/* Frames */

	frames_area = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add1(GTK_PANED(paned), frames_area);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frames_area),
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(frames_area, 250, 100);

	frames_box = gtk_vbox_new(FALSE, 0);
	build_frames(frames_box);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(frames_area),
	    frames_box);

	/* Canvas */

	gtk_paned_add2(GTK_PANED(paned), make_canvas());

	/* Icon bar */

	tools = gui_setup_tools(root->window);
	gtk_box_pack_end(GTK_BOX(hbox), tools, FALSE, FALSE, 0);
}


/* ----- GUI construction -------------------------------------------------- */


void change_world(void)
{
	inst_deselect();
	status_begin_reporting();
	instantiate();
	label_in_box_bg(active_frame->label, COLOR_FRAME_SELECTED);
	build_frames(frames_box);
	redraw();
}


static void make_screen(GtkWidget *window)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	make_menu_bar(vbox);
	make_center_area(vbox);
	make_status_area(vbox);
}


int gui_init(int *argc, char ***argv)
{
	gtk_init(argc, argv);
	return 0;
}


int gui_main(void)
{
	root = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(root), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(root), 620, 460);
	gtk_window_set_title(GTK_WINDOW(root), "fped");

	/* get root->window */
	gtk_widget_show_all(root);

	g_signal_connect_swapped(G_OBJECT(root), "destroy",
	    G_CALLBACK(gtk_main_quit), NULL);

	make_screen(root);

	gtk_widget_show_all(root);

	gui_setup_style(root->window);
	init_canvas();
	edit_nothing();
	select_frame(root_frame);
	make_popups();

	gtk_main();

	return 0;
}
