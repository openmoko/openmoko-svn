
#ifndef TODAY_H
#define TODAY_H

#include <gtk/gtk.h>
#include <libtaku/taku-launcher-tile.h>
#include <moko-journal.h>
#include <libjana/jana.h>

#define GCONF_POKY_INTERFACE_PREFIX "/desktop/poky/interface"
#define GCONF_POKY_WALLPAPER "/wallpaper"
#define GCONF_POKY_DIGITAL "/digital_clock"

typedef struct {
	/* Home */
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *home_toolbar;
	GtkWidget *clock;
	GtkWidget *message_box;
	GtkWidget *summary_box;
	GtkToolItem *dial_button;
	GtkToolItem *contacts_button;
	GtkToolItem *messages_button;
	GtkToolItem *dates_button;
	GtkWidget *bg_ebox;
	GdkPixmap *wallpaper;
	gchar *location;
	
	GtkWidget *date_button;
	GtkWidget *missed_calls_box;
	GtkWidget *missed_calls_label;
	gint n_missed_calls;
	GtkWidget *unread_messages_box;
	GtkWidget *unread_messages_label;
	gint n_unread_messages;
	JanaStoreView *dates_view;
	GtkTreeModel *dates_model;
	GtkWidget *dates_box;
	GtkWidget *dates_label;
	GtkTreeModel *tasks_store;
	GtkWidget *tasks_box;
	GtkWidget *tasks_label;

	/* App launcher */
	GList *categories;
	GtkWidget *launcher_table;
	GtkWidget *launcher_viewport;
	GtkWidget *search_bar;
	GtkWidget *filter_combo;
	TakuLauncherCategory *search_cat;
	
	TakuMenuItem *dialer_item;
	TakuMenuItem *contacts_item;
	TakuMenuItem *tasks_item;
	TakuMenuItem *dates_item;
	TakuMenuItem *clock_item;
	
	/* App manager */
	GdkWindow *root_window;
	GtkWidget *tasks_table;
} TodayData;
#endif

