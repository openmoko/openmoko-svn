
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <libtaku/taku-table.h>
#include <libtaku/taku-launcher-tile.h>
#include <gtk/gtk.h>

typedef struct {
	GtkWidget *window;
	GtkWidget *notebook;

	/* For task manager */
	GtkWidget *root_window;
	GtkWidget *tasks_table;
	GtkToolItem *kill_button;
	GtkToolItem *killall_button;
	GtkToolItem *switch_button;
} LauncherData;

#endif

