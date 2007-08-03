
#ifndef TODAY_H
#define TODAY_H

#include <gtk/gtk.h>
#include <libtaku/taku-launcher-tile.h>
#include <moko-journal.h>

typedef struct {
	/* Home */
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *home_toolbar;
	GtkWidget *message_box;
	GtkWidget *summary_box;
	GtkToolItem *dial_button;
	GtkToolItem *contacts_button;
	GtkToolItem *messages_button;
	GtkToolItem *dates_button;
	
	GtkTreeModel *events_model;
	GtkTreeModel *tasks_model;
	GtkTreeIter notice;
	gboolean notice_visible;
	int rows;
	
	MokoJournal *journal;
	GtkListStore *journal_model;
	GtkTreeIter missed_calls;
	gint n_missed_calls;
	GtkTreeIter unread_messages;
	gint n_unread_messages;

	/* App launcher */
	GList *categories;
	GtkWidget *launcher_table;
	GtkWidget *filter_combo;
	GtkWidget *search_entry;
	TakuLauncherCategory *search_cat;
	
	/* App manager */
	GdkWindow *root_window;
	GtkWidget *tasks_table;
} TodayData;
#endif

