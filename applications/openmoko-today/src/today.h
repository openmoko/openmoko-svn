
#ifndef TODAY_H
#define TODAY_H

#include <gtk/gtk.h>

typedef struct {
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
} TodayData;
#endif

