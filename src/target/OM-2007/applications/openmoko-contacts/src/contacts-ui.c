#include <string.h>

#include "contacts-main.h"
#include "contacts-callbacks-ui.h"
#include "contacts-ui.h"
#include "contacts-utils.h"
#include "contacts-contact-pane.h"
#include "contacts-groups-editor.h"
#include "contacts-omoko.h"

#define CONTACTS_VIEW_PAGE 0
#define CONTACTS_EDIT_PAGE 1


void
contacts_remove_labels_from_focus_chain (GtkContainer *container)
{
	GList *chain, *l;
	
	gtk_container_get_focus_chain (container, &chain);
	
	for (l = chain; l; l = l->next) {
		if (GTK_IS_LABEL (l->data)) {
			gconstpointer data = l->data;
			l = l->prev;
			chain = g_list_remove (chain, data);
		}
	}
	
	gtk_container_set_focus_chain (container, chain);
	g_list_free (chain);
}


void
contacts_display_summary (EContact *contact, ContactsData *data)
{
  contacts_contact_pane_set_editable (CONTACTS_CONTACT_PANE (data->ui->contact_pane), FALSE);
  contacts_contact_pane_set_contact (CONTACTS_CONTACT_PANE (data->ui->contact_pane), contact);
}

/* Helper method to set edit/delete sensitive/insensitive */
void
contacts_set_available_options (ContactsData *data, gboolean new, gboolean open,
				gboolean delete)
{
	GtkWidget *widget;

	if ((widget = data->ui->new_menuitem))
		gtk_widget_set_sensitive (widget, new);
	if ((widget = data->ui->new_button))
		gtk_widget_set_sensitive (widget, new);

	if ((widget = data->ui->edit_menuitem))
		gtk_widget_set_sensitive (widget, open);
	if ((widget = data->ui->edit_button))
		gtk_widget_set_sensitive (widget, open);
	if ((widget = data->ui->contact_export))
		gtk_widget_set_sensitive (widget, open);

	if ((widget = data->ui->delete_menuitem))
		gtk_widget_set_sensitive (widget, delete);
	if ((widget = data->ui->delete_button))
		gtk_widget_set_sensitive (widget, delete);
}

void
contacts_setup_ui (ContactsData *data)
{


	/* Create model and groups/search filter for contacts list */
	data->contacts_liststore = gtk_list_store_new (3, G_TYPE_STRING,
							G_TYPE_STRING, G_TYPE_STRING);
	data->contacts_filter =
	    GTK_TREE_MODEL_FILTER (gtk_tree_model_filter_new
				   (GTK_TREE_MODEL (data->contacts_liststore), NULL));
	gtk_tree_model_filter_set_visible_func (data->contacts_filter,
						(GtkTreeModelFilterVisibleFunc)
						 contacts_is_row_visible_cb,
						data->contacts_table,
						NULL);


	/* Alphabetise the list */

	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (data->contacts_liststore),
						 contacts_sort_treeview_cb,
						 NULL, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (data->contacts_liststore),
				      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
				      GTK_SORT_ASCENDING);

	/* these are defined in the frontend header */
	/* create the ui */
	contacts_ui_create (data);

	/* Set transient parent for chooser */
	if (data->ui->chooser_dialog)
	{
		gtk_window_set_transient_for (
			GTK_WINDOW (data->ui->chooser_dialog),
			GTK_WINDOW (data->ui->main_window));
	}

}
