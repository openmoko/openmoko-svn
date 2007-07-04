
#ifndef TODAY_H
#define TODAY_H

#include <gtk/gtk.h>

typedef struct {
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *toolbar;
	GtkWidget *notebook;
	GtkWidget *message_box;
	GtkWidget *events_box;
	GtkToolItem *dial_button;
	GtkToolItem *contacts_button;
	GtkToolItem *messages_button;
	GtkToolItem *dates_button;	
} TodayData;
#endif

