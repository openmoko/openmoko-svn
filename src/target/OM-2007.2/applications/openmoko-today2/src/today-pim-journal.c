
#include <glib/gi18n.h>
#include <moko-stock.h>
#include <libtaku/launcher-util.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libjana-gtk/jana-gtk.h>
#include "today-tasks-store.h"
#include "today-utils.h"
#include "today-pim-journal.h"

static void
today_pim_journal_update_messages (TodayData *data)
{
	gchar *message = NULL;
	gint n_tasks = 0;

	/* Missed calls */
	if (data->n_missed_calls > 0) {
		message = g_strdup_printf (_("%d missed call(s)"),
			data->n_missed_calls);
	}

	if (message) {
		gtk_label_set_text (GTK_LABEL (
			data->missed_calls_label), message);
		gtk_widget_show (data->missed_calls_box);
		g_free (message);
		message = NULL;
	} else
		gtk_widget_hide (data->missed_calls_box);

	/* Unread messages */
	if (data->n_unread_messages > 0) {
		message = g_strdup_printf (_("%d new message(s)"),
			data->n_unread_messages);
	}

	if (message) {
		gtk_label_set_text (GTK_LABEL (
			data->unread_messages_label), message);
		gtk_widget_show (data->unread_messages_box);
		g_free (message);
	} else {
		gtk_widget_hide (data->unread_messages_box);
	}
	
	/* Upcoming events */
	if (data->dates_model &&
	    gtk_tree_model_iter_n_children (data->dates_model, NULL) > 0) {
		GtkTreeIter iter;
		JanaTime *start;
		gchar *summary;
		
		/* Set the label from the first event in the model */
		gtk_tree_model_get_iter_first (data->dates_model, &iter);
		gtk_tree_model_get (data->dates_model, &iter,
			JANA_GTK_EVENT_STORE_COL_SUMMARY, &summary,
			JANA_GTK_EVENT_STORE_COL_START, &start, -1);
		
		message = g_strdup_printf ("(%02d:%02d) %s",
			jana_time_get_hours (start),
			jana_time_get_minutes (start),
			summary);
		gtk_label_set_text (GTK_LABEL (data->dates_label), message);
		gtk_widget_show (data->dates_box);

		g_free (summary);
		g_free (message);
		g_object_unref (start);
	} else
		gtk_widget_hide (data->dates_box);
	
	/* Uncompleted tasks */
	if (gtk_tree_model_iter_n_children (data->tasks_store, NULL) > 0) {
		GtkTreeIter iter;
		
		gtk_tree_model_get_iter_first (data->tasks_store, &iter);
		do {
			gboolean complete;
			gtk_tree_model_get (data->tasks_store, &iter,
				COLUMN_DONE, &complete, -1);
			if (!complete) n_tasks++;
		} while (gtk_tree_model_iter_next (data->tasks_store, &iter));
	}
	if (n_tasks > 0) {
		message = g_strdup_printf (_("%d incomplete task(s)"), n_tasks);
		gtk_label_set_text (GTK_LABEL (data->tasks_label), message);
		g_free (message);
		gtk_widget_show (data->tasks_box);
	} else
		gtk_widget_hide (data->tasks_box);
}

static void
today_pim_journal_entry_changed (MokoJournal *journal,
				 MokoJournalEntry *entry,
				 TodayData *data,
				 gint added)
{
	MessageDirection dir;
	
	switch (moko_journal_entry_get_entry_type (entry)) {
	    case SMS_JOURNAL_ENTRY :
	    case EMAIL_JOURNAL_ENTRY :
		data->n_unread_messages += added;
		today_pim_journal_update_messages (data);
		break;
	    case VOICE_JOURNAL_ENTRY :
		moko_journal_entry_get_direction (entry, &dir);
		if (dir == DIRECTION_IN) {
			if (moko_journal_voice_info_get_was_missed (entry)) {
				data->n_missed_calls += added;
				today_pim_journal_update_messages (data);
			}
		}
		break;
	    default :
		break;
	}
}

static void
today_pim_journal_entry_added_cb (MokoJournal *journal,
				  MokoJournalEntry *entry,
				  TodayData *data)
{
	today_pim_journal_entry_changed (journal, entry, data, 1);
}

static void
today_pim_journal_entry_removed_cb (MokoJournal *journal,
				    MokoJournalEntry *entry,
				    TodayData *data)
{
	today_pim_journal_entry_changed (journal, entry, data, -1);
}

static void
header_clicked_cb (GtkWidget *button, TodayData *data)
{
	if (data->clock_item) launcher_start (data->window, data->clock_item,
		(gchar *[]){ "openmoko-worldclock", NULL }, TRUE, TRUE);
}

static gboolean
missed_calls_button_press_cb (GtkWidget *widget, GdkEventButton *event,
			      TodayData *data)
{
	if (data->dialer_item) launcher_start (data->window, data->dialer_item,
		(gchar *[]){ "openmoko-dialer", "-m", NULL }, TRUE, TRUE);

	return FALSE;
}

static gboolean
unread_messages_button_press_cb (GtkWidget *widget, GdkEventButton *event,
				 TodayData *data)
{
	g_debug ("TODO: Launch messages app");

	return FALSE;
}

static gboolean
tasks_button_press_cb (GtkWidget *widget, GdkEventButton *event,
		       TodayData *data)
{
	if (data->tasks_item) launcher_start (data->window, data->tasks_item,
		(gchar *[]){ "tasks", NULL }, TRUE, TRUE);

	return FALSE;
}

static gboolean
dates_button_press_cb (GtkWidget *widget, GdkEventButton *event,
		       TodayData *data)
{
	if (data->dates_item) launcher_start (data->window, data->dates_item,
		(gchar *[]){ "openmoko-dates", NULL }, TRUE, TRUE);

	return FALSE;
}

static void
store_opened_cb (JanaStore *store, TodayData *data)
{
	JanaTime *start, *end;
	
	start = jana_ecal_utils_time_now (data->location);
	end = jana_time_duplicate (start);
	jana_time_set_day (end, jana_time_get_day (end) + 1);
	jana_time_set_isdate (end, TRUE);
	
	data->dates_view = jana_store_get_view (store);
	jana_store_view_set_range (data->dates_view, start, end);
	
	data->dates_model = GTK_TREE_MODEL (
		jana_gtk_event_store_new_full (data->dates_view,
		jana_time_get_offset (start)));
	
	g_object_unref (start);
	g_object_unref (end);
	
	g_signal_connect_swapped (data->dates_model, "row-inserted",
		G_CALLBACK (today_pim_journal_update_messages), data);
	g_signal_connect_swapped (data->dates_model, "row-changed",
		G_CALLBACK (today_pim_journal_update_messages), data);
	g_signal_connect_swapped (data->dates_model, "row-deleted",
		G_CALLBACK (today_pim_journal_update_messages), data);
	
	jana_store_view_start (data->dates_view);
}

GtkWidget *
today_pim_journal_box_new (TodayData *data)
{
	JanaStore *store;
	MokoJournal *journal;
	GtkWidget *vbox, *hbox, *image;
	
	data->date_button = gtk_button_new_with_label ("");

	/* Missed calls box */
	data->missed_calls_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (
		data->missed_calls_box), FALSE);
	hbox = gtk_hbox_new (FALSE, 6);
	image = gtk_image_new_from_stock (
		MOKO_STOCK_CALL_MISSED, GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->missed_calls_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (data->missed_calls_label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (hbox),
		data->missed_calls_label, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data->missed_calls_box), hbox);
	gtk_widget_add_events (data->missed_calls_box, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (data->missed_calls_box, "button-press-event",
		G_CALLBACK (missed_calls_button_press_cb), data);

	/* Unread messages box */
	data->unread_messages_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (
		data->unread_messages_box), FALSE);
	hbox = gtk_hbox_new (FALSE, 6);
	image = gtk_image_new_from_icon_name (
		"openmoko-messages", GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->unread_messages_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (data->unread_messages_label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (hbox),
		data->unread_messages_label, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data->unread_messages_box), hbox);
	gtk_widget_add_events (data->unread_messages_box,
		GDK_BUTTON_PRESS_MASK);
	g_signal_connect (data->missed_calls_box, "button-press-event",
		G_CALLBACK (unread_messages_button_press_cb), data);

	/* Events box */
	data->dates_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (
		data->dates_box), FALSE);
	hbox = gtk_hbox_new (FALSE, 6);
	image = gtk_image_new_from_icon_name (
		"dates", GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->dates_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (data->dates_label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (hbox),
		data->dates_label, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data->dates_box), hbox);
	gtk_widget_add_events (data->dates_box,
		GDK_BUTTON_PRESS_MASK);
	g_signal_connect (data->dates_box, "button-press-event",
		G_CALLBACK (dates_button_press_cb), data);

	/* Tasks box */
	data->tasks_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (
		data->tasks_box), FALSE);
	hbox = gtk_hbox_new (FALSE, 6);
	image = gtk_image_new_from_icon_name (
		"tasks", GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->tasks_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (data->tasks_label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (hbox),
		data->tasks_label, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data->tasks_box), hbox);
	gtk_widget_add_events (data->tasks_box,
		GDK_BUTTON_PRESS_MASK);
	g_signal_connect (data->tasks_box, "button-press-event",
		G_CALLBACK (tasks_button_press_cb), data);
	
	g_signal_connect (data->date_button, "clicked",
		G_CALLBACK (header_clicked_cb), data);
	
	/* Pack widgets */
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox), data->date_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
		data->missed_calls_box, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
		data->unread_messages_box, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
		data->dates_box, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
		data->tasks_box, FALSE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Open up journal and connect to signals to find out about missed
	 * calls and new messages.
	 */
	data->n_missed_calls = 0;
	data->n_unread_messages = 0;
	data->dates_view = NULL;
	data->dates_model = NULL;
	journal = moko_journal_open_default ();

	if (journal) {
		g_signal_connect (G_OBJECT (journal), "entry_added",
			G_CALLBACK (today_pim_journal_entry_added_cb), data);
		g_signal_connect (G_OBJECT (journal), "entry_removed",
			G_CALLBACK (today_pim_journal_entry_removed_cb), data);
	} else {
		g_warning ("Could not load journal");
	}
	
	/* Open up calendar store */
	store = jana_ecal_store_new (JANA_COMPONENT_EVENT);
	g_signal_connect (store, "opened",
		G_CALLBACK (store_opened_cb), data);
	jana_store_open (store);
	
	/* Create tasks store */
	data->tasks_store = GTK_TREE_MODEL (today_tasks_store_new ());
	g_signal_connect_swapped (data->tasks_store, "row-inserted",
		G_CALLBACK (today_pim_journal_update_messages), data);
	g_signal_connect_swapped (data->tasks_store, "row-changed",
		G_CALLBACK (today_pim_journal_update_messages), data);
	g_signal_connect_swapped (data->tasks_store, "row-deleted",
		G_CALLBACK (today_pim_journal_update_messages), data);

	/* Start up journal */
	if (journal) moko_journal_load_from_storage (journal);
	
	/* Update labels */
	today_pim_journal_update_messages (data);
	
	return vbox;
}
