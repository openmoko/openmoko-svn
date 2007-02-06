/*
 * contacts-gtk.c
 * This file is part of Contacts
 *
 * Copyright (C) 2006 - OpenedHand Ltd
 *
 * Contacts is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Contacts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Contacts; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "contacts-callbacks-ui.h"
#include "contacts-main.h"
#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#define GCONF_PATH "/apps/contacts"
#define GCONF_KEY_SEARCH "/apps/contacts/search_type"

static GtkWidget *groups_combobox;

#ifdef HAVE_GCONF
static void
contacts_gconf_search_cb (GConfClient *client, guint cnxn_id, GConfEntry *entry,
	gpointer user_data)
{
	const gchar *search;
	GConfValue *value;
	GtkWidget *widget;
	ContactsData *contacts_data = user_data;
		
	value = gconf_entry_get_value (entry);
	search = gconf_value_get_string (value);
	if (strcmp (search, "entry") == 0) {
		widget = contacts_data->ui->search_entry_hbox;
		gtk_widget_show (widget);
		widget = contacts_data->ui->search_tab_hbox;
		gtk_widget_hide (widget);
	} else if (strcmp (search, "alphatab") == 0) {
		widget = contacts_data->ui->search_entry_hbox;
		gtk_widget_hide (widget);
		widget = contacts_data->ui->search_tab_hbox;
		gtk_widget_show (widget);
	} else {
		g_warning ("Unknown search UI type \"%s\"", search);
	}
}
#endif

static void
groups_combobox_changed_cb (GtkWidget *widget, ContactsData *data)
{
	GtkTreeIter iter;
	gchar *text = NULL;
	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (groups_combobox), &iter);
	gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (groups_combobox)), &iter, 0, &text, -1);
	g_free (data->selected_group);
	data->selected_group = text;
	contacts_update_treeview (data);
}

void
contacts_ui_update_groups_list (ContactsData *data)
{
	void add_to_list (gpointer data, gpointer list)
	{
		GtkTreeIter iter;
		gtk_list_store_insert_with_values (list, &iter, 1, 0, data, -1);
	}
	GtkListStore *list;

	list = (GtkListStore*) gtk_combo_box_get_model (GTK_COMBO_BOX(groups_combobox));
	if (!list) return;
	gtk_list_store_clear (list);
	gtk_list_store_insert_with_values (list, NULL, 0, 0, "All", -1);
	g_list_foreach (data->contacts_groups, add_to_list, list);

	/* Select 'All' in the groups combobox if nothing selected */
	if (gtk_combo_box_get_active (GTK_COMBO_BOX (groups_combobox)))
		gtk_combo_box_set_active (GTK_COMBO_BOX (groups_combobox), 0);
}

void
create_main_window (ContactsData *data)
{
	GtkWidget *main_window;
	GtkWidget *vbox7;
	GtkWidget *main_menubar;
	GtkWidget *contacts_menu;
	GtkWidget *contacts_menu_menu;
	GtkWidget *new_menuitem;
	GtkWidget *edit_menuitem;
	GtkWidget *delete_menuitem;
	GtkWidget *contacts_import;
	GtkWidget *contacts_quit;
	GtkWidget *contact_menu;
	GtkWidget *contact_menu_menu;
	GtkWidget *contact_delete;
	GtkWidget *edit_groups;
	GtkWidget *contact_export;
	GtkWidget *contact_quit;
	GtkWidget *edit_menu;
	GtkWidget *menuitem5_menu;
	GtkWidget *cut;
	GtkWidget *copy;
	GtkWidget *paste;
	GtkWidget *help_menu;
	GtkWidget *menuitem7_menu;
	GtkWidget *about1;
	GtkWidget *main_notebook;
	GtkWidget *main_hpane;
	GtkWidget *contacts_vbox;
	GtkWidget *scrolledwindow2;
	GtkWidget *contacts_treeview;
	GtkWidget *search_hbox;
	GtkWidget *search_entry_hbox;
	GtkWidget *search_entry;
	GtkWidget *search_tab_hbox;
	GtkWidget *symbols_radiobutton;
	GSList *symbols_radiobutton_group = NULL;
	GtkWidget *atog_radiobutton;
	GtkWidget *hton_radiobutton;
	GtkWidget *otou_radiobutton;
	GtkWidget *vtoz_radiobutton;
	GtkWidget *vbox3;
	GtkWidget *summary_vbox;
	GtkWidget *preview_header_hbox;
	GtkWidget *summary_name_label;
	GtkWidget *photo_image;
	GtkWidget *scrolledwindow3;
	GtkWidget *viewport1;
	GtkWidget *summary_table;
	GtkWidget *summary_hbuttonbox;
	GtkWidget *new_button;
	GtkWidget *edit_button;
	GtkWidget *delete_button;
	GtkWidget *vbox4;
	GtkWidget *scrolledwindow4;
	GtkWidget *viewport2;
	GtkWidget *edit_table;
	GtkWidget *hbuttonbox2;
	GtkWidget *add_field_button;
	GtkWidget *remove_field_button;
	GtkWidget *edit_done_button;
	GtkWidget *widget;
	GtkAccelGroup *accel_group;
	ContactsUI *ui = data->ui;
	GtkSizeGroup *size_group;

	accel_group = gtk_accel_group_new ();

	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (main_window), _("Contacts"));
	gtk_window_set_default_size (GTK_WINDOW (main_window), -1, 300);
	gtk_window_set_icon_name (GTK_WINDOW (main_window), "stock_contact");

	vbox7 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_window), vbox7);

	main_menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (vbox7), main_menubar, FALSE, FALSE, 0);

	contacts_menu = gtk_menu_item_new_with_mnemonic (_("_Contacts"));
	gtk_container_add (GTK_CONTAINER (main_menubar), contacts_menu);

	contacts_menu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (contacts_menu), contacts_menu_menu);

	new_menuitem = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), new_menuitem);

	edit_menuitem = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), edit_menuitem);
	gtk_widget_set_sensitive (edit_menuitem, FALSE);

	delete_menuitem = gtk_image_menu_item_new_from_stock ("gtk-delete", accel_group);
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), delete_menuitem);
	gtk_widget_set_sensitive (delete_menuitem, FALSE);

	contacts_import = gtk_menu_item_new_with_mnemonic (_("_Import"));
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), contacts_import);

	widget = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), widget);
	gtk_widget_set_sensitive (widget, FALSE);

	contacts_quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), contacts_quit);

	contact_menu = gtk_menu_item_new_with_mnemonic (_("_Contact"));
	gtk_container_add (GTK_CONTAINER (main_menubar), contact_menu);

	contact_menu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (contact_menu), contact_menu_menu);

	contact_delete = gtk_image_menu_item_new_from_stock ("gtk-delete", accel_group);
	gtk_container_add (GTK_CONTAINER (contact_menu_menu), contact_delete);

	edit_groups = gtk_menu_item_new_with_mnemonic (_("_Groups"));
	gtk_container_add (GTK_CONTAINER (contact_menu_menu), edit_groups);

	contact_export = gtk_menu_item_new_with_mnemonic (_("_Export"));
	gtk_container_add (GTK_CONTAINER (contact_menu_menu), contact_export);

	widget = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (contact_menu_menu), widget);
	gtk_widget_set_sensitive (widget, FALSE);

	contact_quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
	gtk_container_add (GTK_CONTAINER (contact_menu_menu), contact_quit);

	edit_menu = gtk_menu_item_new_with_mnemonic (_("_Edit"));
	gtk_container_add (GTK_CONTAINER (main_menubar), edit_menu);

	menuitem5_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (edit_menu), menuitem5_menu);

	cut = gtk_image_menu_item_new_from_stock ("gtk-cut", accel_group);
	gtk_container_add (GTK_CONTAINER (menuitem5_menu), cut);

	copy = gtk_image_menu_item_new_from_stock ("gtk-copy", accel_group);
	gtk_container_add (GTK_CONTAINER (menuitem5_menu), copy);

	paste = gtk_image_menu_item_new_from_stock ("gtk-paste", accel_group);
	gtk_container_add (GTK_CONTAINER (menuitem5_menu), paste);

	help_menu = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_container_add (GTK_CONTAINER (main_menubar), help_menu);

	menuitem7_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu), menuitem7_menu);

	about1 = gtk_image_menu_item_new_from_stock ("gtk-about", accel_group);
	gtk_container_add (GTK_CONTAINER (menuitem7_menu), about1);

	main_notebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (vbox7), main_notebook, TRUE, TRUE, 0);
	GTK_WIDGET_UNSET_FLAGS (main_notebook, GTK_CAN_FOCUS);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (main_notebook), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (main_notebook), FALSE);

	main_hpane = gtk_hpaned_new ();
	gtk_container_add (GTK_CONTAINER (main_notebook), main_hpane);
	gtk_paned_set_position (GTK_PANED (main_hpane), 1);

	contacts_vbox = gtk_vbox_new (FALSE, 6);
	gtk_paned_pack1 (GTK_PANED (main_hpane), contacts_vbox, FALSE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (contacts_vbox), 6);

	groups_combobox = gtk_combo_box_new ();
	GtkListStore *ls = gtk_list_store_new (1, G_TYPE_STRING);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (groups_combobox), renderer, TRUE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (groups_combobox), renderer, "text", 0);
	gtk_combo_box_set_model (GTK_COMBO_BOX (groups_combobox), GTK_TREE_MODEL(ls));
	gtk_box_pack_start (GTK_BOX (contacts_vbox), groups_combobox, FALSE, TRUE, 0);
	gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (groups_combobox), FALSE);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (contacts_vbox), scrolledwindow2, TRUE, TRUE, 0);
	GTK_WIDGET_UNSET_FLAGS (scrolledwindow2, GTK_CAN_FOCUS);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);

	contacts_treeview = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), contacts_treeview);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (contacts_treeview), FALSE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (contacts_treeview), FALSE);

	search_hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (contacts_vbox), search_hbox, FALSE, TRUE, 0);

	search_entry_hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (search_hbox), search_entry_hbox, TRUE, TRUE, 0);

	search_entry = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (search_entry_hbox), search_entry, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (search_entry), TRUE);

	widget = gtk_label_new_with_mnemonic (_("_Search:"));
	gtk_box_pack_start (GTK_BOX (search_entry_hbox), widget, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (widget), 6, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), search_entry);

	search_tab_hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (search_hbox), search_tab_hbox, TRUE, TRUE, 0);

	symbols_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("0-9#"));
	gtk_box_pack_start (GTK_BOX (search_tab_hbox), symbols_radiobutton, TRUE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (symbols_radiobutton), symbols_radiobutton_group);
	symbols_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (symbols_radiobutton));

	atog_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("A-G"));
	gtk_box_pack_start (GTK_BOX (search_tab_hbox), atog_radiobutton, TRUE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (atog_radiobutton), symbols_radiobutton_group);
	symbols_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (atog_radiobutton));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (atog_radiobutton), TRUE);

	hton_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("H-N"));
	gtk_box_pack_start (GTK_BOX (search_tab_hbox), hton_radiobutton, TRUE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (hton_radiobutton), symbols_radiobutton_group);
	symbols_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (hton_radiobutton));

	otou_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("O-U"));
	gtk_box_pack_start (GTK_BOX (search_tab_hbox), otou_radiobutton, TRUE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (otou_radiobutton), symbols_radiobutton_group);
	symbols_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (otou_radiobutton));

	vtoz_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("V-Z"));
	gtk_box_pack_start (GTK_BOX (search_tab_hbox), vtoz_radiobutton, TRUE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (vtoz_radiobutton), symbols_radiobutton_group);
	symbols_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (vtoz_radiobutton));

	vbox3 = gtk_vbox_new (FALSE, 6);
	gtk_paned_pack2 (GTK_PANED (main_hpane), vbox3, TRUE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (vbox3), 6);

	summary_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox3), summary_vbox, TRUE, TRUE, 0);

	preview_header_hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (summary_vbox), preview_header_hbox, FALSE, TRUE, 0);

	summary_name_label = gtk_label_new (_("<span><big><b>Welcome to Contacts</b></big></span>"));
	gtk_box_pack_start (GTK_BOX (preview_header_hbox), summary_name_label, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (summary_name_label, GTK_CAN_FOCUS);
	gtk_label_set_use_markup (GTK_LABEL (summary_name_label), TRUE);
	gtk_label_set_selectable (GTK_LABEL (summary_name_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (summary_name_label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (summary_name_label), 6, 0);
	gtk_label_set_ellipsize (GTK_LABEL (summary_name_label), PANGO_ELLIPSIZE_END);

	photo_image = gtk_image_new_from_icon_name ("stock_contact", GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_end (GTK_BOX (preview_header_hbox), photo_image, FALSE, TRUE, 6);
	gtk_misc_set_padding (GTK_MISC (photo_image), 1, 0);

	scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (summary_vbox), scrolledwindow3, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	viewport1 = gtk_viewport_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrolledwindow3), viewport1);

	summary_table = gtk_table_new (1, 2, FALSE);
	gtk_container_add (GTK_CONTAINER (viewport1), summary_table);
	gtk_table_set_row_spacings (GTK_TABLE (summary_table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (summary_table), 6);

	summary_hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_end (GTK_BOX (vbox3), summary_hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (summary_hbuttonbox), GTK_BUTTONBOX_SPREAD);

	new_button = gtk_button_new_from_stock ("gtk-new");
	gtk_container_add (GTK_CONTAINER (summary_hbuttonbox), new_button);
	GTK_WIDGET_SET_FLAGS (new_button, GTK_CAN_DEFAULT);

	edit_button = gtk_button_new_from_stock ("gtk-edit");
	gtk_container_add (GTK_CONTAINER (summary_hbuttonbox), edit_button);
	gtk_widget_set_sensitive (edit_button, FALSE);
	GTK_WIDGET_SET_FLAGS (edit_button, GTK_CAN_DEFAULT);

	delete_button = gtk_button_new_from_stock ("gtk-delete");
	gtk_container_add (GTK_CONTAINER (summary_hbuttonbox), delete_button);
	gtk_widget_set_sensitive (delete_button, FALSE);
	GTK_WIDGET_SET_FLAGS (delete_button, GTK_CAN_DEFAULT);
	gtk_button_set_focus_on_click (GTK_BUTTON (delete_button), FALSE);

	vbox4 = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (main_notebook), vbox4);
	gtk_container_set_border_width (GTK_CONTAINER (vbox4), 6);

	scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox4), scrolledwindow4, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	viewport2 = gtk_viewport_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrolledwindow4), viewport2);

	edit_table = gtk_table_new (1, 2, FALSE);
	gtk_container_add (GTK_CONTAINER (viewport2), edit_table);
	gtk_container_set_border_width (GTK_CONTAINER (edit_table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (edit_table), 6);

	hbuttonbox2 = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox4), hbuttonbox2, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox2), GTK_BUTTONBOX_END);

	add_field_button = gtk_button_new_with_mnemonic (_("_Add Field"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), add_field_button);
	GTK_WIDGET_SET_FLAGS (add_field_button, GTK_CAN_DEFAULT);

	remove_field_button = gtk_button_new_with_mnemonic (_("_Remove Field"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), remove_field_button);
	GTK_WIDGET_SET_FLAGS (remove_field_button, GTK_CAN_DEFAULT);
	gtk_button_set_focus_on_click (GTK_BUTTON (remove_field_button), FALSE);

	edit_done_button = gtk_button_new_from_stock ("gtk-close");
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), edit_done_button);
	GTK_WIDGET_SET_FLAGS (edit_done_button, GTK_CAN_DEFAULT);


	gtk_widget_grab_focus (search_entry);
	gtk_widget_grab_default (edit_button);
	gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);

	/* Set up size group for bottom row of buttons and search */
	size_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
	gtk_size_group_add_widget (size_group, search_hbox);
	gtk_size_group_add_widget (size_group, summary_hbuttonbox);
	g_object_unref (size_group);



	/* connect signals */
	g_signal_connect ((gpointer) main_window, "destroy",
			G_CALLBACK (gtk_main_quit),
			NULL);
	g_signal_connect_swapped ((gpointer) main_window, "set_focus",
			G_CALLBACK (contacts_edit_set_focus_cb),
			GTK_OBJECT (remove_field_button));
	g_signal_connect ((gpointer) contacts_quit, "activate",
			G_CALLBACK (gtk_main_quit),
			NULL);
	g_signal_connect ((gpointer) contact_quit, "activate",
			G_CALLBACK (gtk_main_quit),
			NULL);
	g_signal_connect_swapped ((gpointer) cut, "activate",
			G_CALLBACK (contacts_cut_cb),
			GTK_OBJECT (main_window));
	g_signal_connect_swapped ((gpointer) copy, "activate",
			G_CALLBACK (contacts_copy_cb),
			GTK_OBJECT (main_window));
	g_signal_connect_swapped ((gpointer) paste, "activate",
			G_CALLBACK (contacts_paste_cb),
			GTK_OBJECT (main_window));
	g_signal_connect_swapped ((gpointer) about1, "activate",
			G_CALLBACK (contacts_about_cb),
			NULL);
	g_signal_connect_swapped ((gpointer) groups_combobox, "changed",
			G_CALLBACK (contacts_update_treeview),
			data);
	g_signal_connect_data ((gpointer) contacts_treeview, "key_press_event",
			G_CALLBACK (contacts_treeview_search_cb),
			GTK_OBJECT (search_entry),
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
	g_signal_connect ((gpointer) search_entry, "changed",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) symbols_radiobutton, "clicked",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) atog_radiobutton, "clicked",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) hton_radiobutton, "clicked",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) otou_radiobutton, "clicked",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) vtoz_radiobutton, "clicked",
			G_CALLBACK (contacts_search_changed_cb),
			data);
	g_signal_connect ((gpointer) remove_field_button, "clicked",
			G_CALLBACK (contacts_remove_field_cb),
			NULL);
	g_signal_connect (G_OBJECT (new_button), "clicked",
			  G_CALLBACK (contacts_new_cb), data);
	g_signal_connect (G_OBJECT (new_menuitem), "activate",
			  G_CALLBACK (contacts_new_cb), data);
	g_signal_connect (G_OBJECT (edit_button), "clicked",
			  G_CALLBACK (contacts_edit_cb), data);
	g_signal_connect (G_OBJECT (contacts_treeview), "row_activated",
			  G_CALLBACK (contacts_treeview_edit_cb), data);
	g_signal_connect (G_OBJECT (edit_menuitem), "activate",
			  G_CALLBACK (contacts_edit_cb), data);
	g_signal_connect (G_OBJECT (delete_button), "clicked",
			  G_CALLBACK (contacts_delete_cb), data);
	g_signal_connect (G_OBJECT (delete_menuitem), "activate",
			  G_CALLBACK (contacts_delete_cb), data);
	g_signal_connect (G_OBJECT (contacts_import), "activate",
			  G_CALLBACK (contacts_import_cb), data);
	g_signal_connect (G_OBJECT (edit_menu), "activate",
			  G_CALLBACK (contacts_edit_menu_activate_cb), data);
	g_signal_connect (G_OBJECT (groups_combobox), "changed",
			  G_CALLBACK (groups_combobox_changed_cb), data);

	ui->contact_delete = contact_delete;
	ui->contact_export = contact_export;
	ui->contact_menu = contact_menu;

	ui->contacts_import = contacts_import;
	ui->contacts_menu = contacts_menu;
	ui->contacts_treeview = contacts_treeview;

	ui->new_menuitem = new_menuitem;
	ui->copy_menuitem = copy;
	ui->cut_menuitem = cut;
	ui->delete_menuitem = delete_menuitem;
	ui->delete_button = delete_button;
	ui->edit_menuitem = edit_menuitem;
	ui->edit_button = edit_button;
	ui->edit_done_button = edit_done_button;
	ui->edit_groups = edit_groups;
	ui->edit_menu = edit_menu;
	ui->edit_table = edit_table;
	ui->main_menubar = main_menubar;
	ui->main_notebook = main_notebook;
	ui->main_window = main_window;
	ui->new_button = new_button;
	ui->paste_menuitem = paste;
	ui->photo_image = photo_image;
	ui->preview_header_hbox = preview_header_hbox;

	ui->add_field_button = add_field_button;
	ui->remove_field_button = remove_field_button;

	ui->search_entry = search_entry;
	ui->search_entry_hbox = search_entry_hbox;
	ui->search_hbox = search_hbox;
	ui->search_tab_hbox = search_tab_hbox;
	//ui->groups_combobox = groups_combobox;

	ui->summary_hbuttonbox = summary_hbuttonbox;
	ui->summary_name_label = summary_name_label;
	ui->summary_table = summary_table;
	ui->summary_vbox = summary_vbox;

	gtk_widget_show_all (main_window);
	gtk_widget_hide (search_tab_hbox);
	gtk_widget_hide (contact_menu);


#ifdef HAVE_GCONF
	client = gconf_client_get_default ();
	search = gconf_client_get_string (client, GCONF_KEY_SEARCH, NULL);
	if (!search) {
		gconf_client_set_string (
			client, GCONF_KEY_SEARCH, "entry", NULL);
	} else if (strcmp (search, "alphatab") == 0) {
		gtk_widget_hide (search_entry_hbox);
		gtk_widget_show (search_tab_hbox);
	}
	gconf_client_add_dir (client, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE,
		NULL);
	gconf_client_notify_add (client, GCONF_KEY_SEARCH,
		contacts_gconf_search_cb, xml, NULL, NULL);
#endif

}

void
create_chooser_dialog (ContactsData *data)
{
	GtkWidget *chooser_dialog;
	GtkWidget *dialog_vbox1;
	GtkWidget *vbox6;
	GtkWidget *chooser_label;
	GtkWidget *chooser_add_hbox;
	GtkWidget *chooser_entry;
	GtkWidget *add_type_button;
	GtkWidget *scrolledwindow5;
	GtkWidget *chooser_treeview;
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;

	chooser_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (chooser_dialog), _("Edit Types"));
	gtk_window_set_modal (GTK_WINDOW (chooser_dialog), TRUE);
	gtk_window_set_default_size (GTK_WINDOW (chooser_dialog), -1, 280);
	gtk_window_set_type_hint (GTK_WINDOW (chooser_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator (GTK_DIALOG (chooser_dialog), FALSE);

	dialog_vbox1 = GTK_DIALOG (chooser_dialog)->vbox;

	vbox6 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox6, TRUE, TRUE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox6), 6);

	chooser_label = gtk_label_new (_("<span><b>Make a choice:</b></span>"));
	gtk_box_pack_start (GTK_BOX (vbox6), chooser_label, FALSE, FALSE, 6);
	gtk_label_set_use_markup (GTK_LABEL (chooser_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (chooser_label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (chooser_label), 6, 0);

	chooser_add_hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox6), chooser_add_hbox, FALSE, TRUE, 6);

	chooser_entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (chooser_add_hbox), chooser_entry, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (chooser_entry), TRUE);

	add_type_button = gtk_button_new_from_stock ("gtk-add");
	gtk_box_pack_start (GTK_BOX (chooser_add_hbox), add_type_button, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (add_type_button, GTK_CAN_DEFAULT);
	gtk_button_set_focus_on_click (GTK_BUTTON (add_type_button), FALSE);

	scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox6), scrolledwindow5, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_SHADOW_IN);

	chooser_treeview = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (scrolledwindow5), chooser_treeview);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (chooser_treeview), FALSE);

	widget = GTK_DIALOG (chooser_dialog)->action_area;
	gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);

	widget = gtk_button_new_from_stock ("gtk-cancel");
	gtk_dialog_add_action_widget (GTK_DIALOG (chooser_dialog), widget, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);

	widget = gtk_button_new_from_stock ("gtk-ok");
	gtk_dialog_add_action_widget (GTK_DIALOG (chooser_dialog), widget, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);

	gtk_widget_grab_focus (chooser_entry);
	gtk_widget_grab_default (add_type_button);

	/* connect signals */
	g_signal_connect ((gpointer) add_type_button, "clicked",
			G_CALLBACK (contacts_chooser_add_cb),
			data);


	/* Create model/view for groups/type chooser list */
	model = (GtkTreeModel *) gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (renderer, "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled",
			  (GCallback) contacts_chooser_toggle_cb, model);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW
						     (chooser_treeview), -1,
						     NULL, renderer, "active",
						     CHOOSER_TICK_COL, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW
						     (chooser_treeview), -1,
						     NULL, renderer, "text",
						     CHOOSER_NAME_COL, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW (chooser_treeview),
				 GTK_TREE_MODEL (model));
	g_object_unref (model);

	data->ui->chooser_add_hbox = chooser_add_hbox;
	data->ui->chooser_dialog = chooser_dialog;
	data->ui->chooser_entry = chooser_entry;
	data->ui->chooser_label = chooser_label;
	data->ui->chooser_treeview = chooser_treeview;

	gtk_widget_show_all (dialog_vbox1);
}

void
contacts_create_ui (ContactsData *data)
{
	create_main_window (data);
	create_chooser_dialog (data);
}
