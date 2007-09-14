
#include <glib/gi18n.h>
#include <moko-stock.h>
#include <libtaku/launcher-util.h>
#include "today-utils.h"
#include "today-pim-journal.h"

enum {
	COLUMN_ICON,
	COLUMN_TEXT,
	COLUMN_LAST
};

static void
today_pim_journal_update_messages (TodayData *data)
{
	gchar *message;

	if (data->n_unread_messages > 0)
		message = g_strdup_printf (_("%d new messages"),
			data->n_unread_messages);
	else
		message = g_strdup_printf (_("No new messages"));

	gtk_list_store_set (GTK_LIST_STORE (data->journal_model),
		&data->unread_messages, COLUMN_TEXT,
		message, -1);
	g_free (message);

	if (data->n_missed_calls > 0)
		message = g_strdup_printf (_("%d missed calls"),
			data->n_missed_calls);
	else
		message = g_strdup_printf (_("No missed calls"));
	gtk_list_store_set (GTK_LIST_STORE (data->journal_model),
		&data->missed_calls, COLUMN_TEXT,
		message, -1);
	g_free (message);
}

static void
today_pim_journal_entry_added_cb (MokoJournal *journal,
				  MokoJournalEntry *entry,
				  TodayData *data)
{
	switch (moko_journal_entry_get_entry_type (entry)) {
	    case SMS_JOURNAL_ENTRY :
	    case EMAIL_JOURNAL_ENTRY :
		data->n_unread_messages ++;
		today_pim_journal_update_messages (data);
		break;
	    case VOICE_JOURNAL_ENTRY :
		data->n_missed_calls ++;
		today_pim_journal_update_messages (data);
		break;
	    default :
		break;
	}
}

static void
today_pim_journal_entry_removed_cb (MokoJournal *journal,
				    MokoJournalEntry *entry,
				    TodayData *data)
{
	switch (moko_journal_entry_get_entry_type (entry)) {
	    case SMS_JOURNAL_ENTRY :
	    case EMAIL_JOURNAL_ENTRY :
		data->n_unread_messages --;
		today_pim_journal_update_messages (data);
		break;
	    case VOICE_JOURNAL_ENTRY :
		data->n_missed_calls --;
		today_pim_journal_update_messages (data);
		break;
	    default :
		break;
	}
}

static void
today_pim_journal_header_clicked_cb (GtkTreeViewColumn *column, TodayData *data)
{
	/* TODO: Maybe just launch dialer normally here? */
	launcher_start (data->window, today_get_launcher ((const gchar *[])
		{ "openmoko-dialer", "-m", NULL }, TRUE, TRUE));
}

static void
today_pim_journal_selection_changed_cb (GtkTreeSelection *selection,
					TodayData *data)
{
	if (gtk_tree_selection_count_selected_rows (selection)) {
		gtk_tree_selection_unselect_all (selection);
		launcher_start (data->window, today_get_launcher (
			(const gchar *[]){ "openmoko-dialer", "-m", NULL },
			TRUE, TRUE));
	}
}

GtkWidget *
today_pim_journal_box_new (TodayData *data)
{
	GtkWidget *treeview;
	GtkCellRenderer *text_renderer, *pixbuf_renderer;
	GtkTreeViewColumn *column;	
	
	data->journal_model = gtk_list_store_new (COLUMN_LAST,
		G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_list_store_insert_with_values (data->journal_model,
		&data->missed_calls,
		0, COLUMN_ICON, MOKO_STOCK_CALL_MISSED,
		COLUMN_TEXT, _("No missed calls"), -1);

	gtk_list_store_insert_with_values (data->journal_model,
		&data->unread_messages,
		1, COLUMN_ICON, "openmoko-messages",
		COLUMN_TEXT, _("No new messages"), -1);
	
	treeview = gtk_tree_view_new_with_model (
		GTK_TREE_MODEL (data->journal_model));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), FALSE);

	text_renderer = gtk_cell_renderer_text_new ();
	pixbuf_renderer = gtk_cell_renderer_pixbuf_new ();

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "OpenMoko");

	gtk_tree_view_column_pack_start (column,
		pixbuf_renderer, FALSE);
	gtk_tree_view_column_pack_end (column,
		text_renderer, TRUE);

	gtk_tree_view_column_add_attribute (column, pixbuf_renderer,
		"icon-name", COLUMN_ICON);
	gtk_tree_view_column_add_attribute (column, text_renderer,
		"text", COLUMN_TEXT);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (treeview), TRUE);
	
	g_signal_connect (G_OBJECT (column), "clicked",
		G_CALLBACK (today_pim_journal_header_clicked_cb), data);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
		"changed", G_CALLBACK (today_pim_journal_selection_changed_cb),
		data);
	
	/* Open up journal and connect to signals to find out about missed
	 * calls and new messages.
	 * TODO: Revise this when libmokojournal has support for 'new' missed
	 *       calls and changes.
	 */
	data->n_missed_calls = 0;
	data->n_unread_messages = 0;
	data->journal = moko_journal_open_default ();

  if (data->journal)
  {
  	g_signal_connect (G_OBJECT (data->journal), "entry_added",
  		G_CALLBACK (today_pim_journal_entry_added_cb), data);
  	g_signal_connect (G_OBJECT (data->journal), "entry_removed",
  		G_CALLBACK (today_pim_journal_entry_removed_cb), data);
  	moko_journal_load_from_storage (data->journal);
  }
  else
  {
    g_warning ("Could not load journal");
	}

	return treeview;
}
