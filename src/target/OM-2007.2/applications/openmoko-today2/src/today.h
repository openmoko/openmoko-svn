
#ifndef TODAY_H
#define TODAY_H

#include <gtk/gtk.h>
#include <libtaku/taku-launcher-tile.h>

typedef struct {
	/* UI vars */
	
	/* Main */
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *home_toolbar;
	GtkWidget *message_box;
	GtkWidget *summary_box;
	GtkWidget *filter_combo;
	GtkWidget *search_entry;
	GtkToolItem *dial_button;
	GtkToolItem *contacts_button;
	GtkToolItem *messages_button;
	GtkToolItem *dates_button;
	
	/* Misc. vars */
	
	/* App launcher */
	GList *categories;
	GtkWidget *launcher_table;
	
	/* App manager */
	GdkWindow *root_window;
	GtkWidget *tasks_table;
} TodayData;
#endif

