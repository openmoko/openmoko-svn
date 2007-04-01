/*
 *  callbacks.c
 *
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */


#include "callbacks.h"
#include "detail-area.h"
#include "sms-contact-window.h"
#include <gtk/gtk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

void cb_addressBtn_clicked (GtkButton* button, gpointer* data);
void sms_contact_select_done (GtkWidget* widget, gpointer data);

void
send_signal_to_footer (DBusConnection* bus, gchar* message_str)
{
  DBusMessage *message;

  if(message_str == NULL)
    g_debug("Input string is null");
  else
    g_debug(message_str);

  message = dbus_message_new_signal ("/org/openmoko/footer",
                                     "org.openmoko.dbus.TaskManager",
                                     "push_statusbar_message");
  dbus_message_append_args (message,
                            DBUS_TYPE_STRING, &message_str,
                            DBUS_TYPE_INVALID);
  dbus_connection_send (bus, message, NULL);
  dbus_message_unref (message);
}

static gboolean
model_number_helper (GtkTreeModel* model,
                     GtkTreePath*  path,
                     GtkTreeIter*  iter,
                     gpointer      data)
{
  MessengerData* d = (MessengerData*)data;
  d->msg_num ++;
  return FALSE;
}

gint get_model_number (MessengerData* d)
{
  d->msg_num = 0;
  gtk_tree_model_foreach (d->filter,model_number_helper,d);
  return d->msg_num;
}

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, MessengerData* d)
{
  d->currentfolder = g_strdup(text);
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(d->filter));
  gchar* str = g_strdup_printf("folder %s has %d messages",text,get_model_number(d));
  send_signal_to_footer(d->bus,str);

  return FALSE;
}

void cb_new_sms (GtkMenuItem* item, MessengerData* d)
{
  SmsDialogWindow* sms_window = sms_dialog_window_new();
  sms_dialog_window_set_title (sms_window,"New SMS");
  gtk_window_set_decorated (GTK_WINDOW(sms_window), FALSE);
  gtk_widget_show_all (GTK_WIDGET(sms_window));

  g_signal_connect (G_OBJECT(sms_window->addressBtn),
                    "clicked",
                    G_CALLBACK(cb_addressBtn_clicked),
                    sms_window->toEntry );
}

void cb_new_mail (GtkMenuItem* item, MessengerData* d)
{
  SmsDialogWindow* mail_window = sms_dialog_window_new();
  mail_dialog_window_set_title (mail_window,"New Email");
  gtk_window_set_decorated (GTK_WINDOW(mail_window), FALSE);
  gtk_widget_show_all ( GTK_WIDGET(mail_window) );

  g_signal_connect (G_OBJECT(mail_window->addressBtn),
                    "clicked",
                    G_CALLBACK(cb_addressBtn_clicked),
                    mail_window->toEntry);
}

void cb_new_folder (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("new folder called");
  GtkWidget* hbox;
  GtkWidget* nfResetBtn;
  GtkWidget* nfConfirmBtn;

  if ((d->nfWin != NULL) && (d->nfWin->window != NULL))
    gtk_entry_set_text (GTK_ENTRY(d->nfEntry),"");
  else
    {
      d->nfWin = moko_dialog_window_new();
      GtkWidget* nfBox = gtk_vbox_new (FALSE,10);
      gtk_widget_set_size_request (nfBox, 480, -1);
      GtkWidget* nfAlign = gtk_alignment_new (0,0,1,1);
      gtk_alignment_set_padding (GTK_ALIGNMENT(nfAlign), 100, 0, 30, 10);
      moko_dialog_window_set_title ( MOKO_DIALOG_WINDOW(d->nfWin), "New Folder");

      GtkWidget* nfLabel = gtk_label_new ("Please input new folder name:");
      gtk_misc_set_alignment (GTK_MISC(nfLabel),0,0.5);
      gtk_box_pack_start (GTK_BOX(nfBox), nfLabel, FALSE, TRUE, 0);

      d->nfEntry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX(nfBox), d->nfEntry, FALSE, TRUE, 0);

      hbox = gtk_hbox_new (FALSE,20);
      nfConfirmBtn = gtk_button_new_with_label ("OK");
      nfResetBtn = gtk_button_new_with_label ("Reset");
      gtk_box_pack_start (GTK_BOX(hbox), nfConfirmBtn, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX(hbox), nfResetBtn, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX(nfBox), hbox, FALSE, TRUE, 0);

      gtk_container_add (GTK_CONTAINER(nfAlign),nfBox);

      moko_dialog_window_set_contents (MOKO_DIALOG_WINDOW(d->nfWin), nfAlign);
      g_signal_connect (G_OBJECT(nfConfirmBtn),
                        "clicked",
                        G_CALLBACK(cb_nfBtn_clicked),
                        d);
      g_signal_connect (G_OBJECT(nfResetBtn),
                        "clicked",
                        G_CALLBACK(cb_nfResetBtn_clicked),
                        d);
    }
  gtk_window_set_decorated (GTK_WINDOW(d->nfWin), FALSE);
  gtk_widget_show_all (d->nfWin);
}

void cb_mode_read (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("mode read");
  message* msg;
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW( d->view ));
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

  if ( has_selection )
    {
      msg = g_malloc(sizeof(message));
      gtk_tree_model_get( model, &iter, COLUMN_FROM, &msg->name, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_SUBJECT, &msg->subject, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_FOLDER, &msg->folder, -1 );
    }
  else msg = NULL;
  detail_read_message (DETAIL_AREA(d->details),msg);
}

void cb_mode_reply (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("mode reply");
  message* msg;
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW( d->view ));
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

  if ( has_selection )
    {
      msg = g_malloc(sizeof(message));
      gtk_tree_model_get( model, &iter, COLUMN_FROM, &msg->name, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_SUBJECT, &msg->subject, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_FOLDER, &msg->folder, -1 );
    }
  else msg = NULL;

  SmsDialogWindow* sms_window = sms_dialog_window_new();
  if (msg != NULL)
    {
      sms_dialog_window_set_title (sms_window,"Reply SMS");
      sms_dialog_reply_message (sms_window,msg);
    }
  else sms_dialog_window_set_title (sms_window,"New SMS");
  gtk_widget_show_all ( GTK_WIDGET(sms_window) );
}

void cb_mode_forward (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("mode forward");
  message* msg;
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW( d->view ));
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

  if ( has_selection )
    {
      msg = g_malloc(sizeof(message));
      gtk_tree_model_get( model, &iter,
                          COLUMN_FROM, &msg->name,
                          COLUMN_SUBJECT, &msg->subject,
                          COLUMN_FOLDER, &msg->folder,
                          COLUMN_CONTENT, &msg->content,
                          -1 );
    }
  else msg = NULL;

  SmsDialogWindow* sms_window = sms_dialog_window_new();
  sms_dialog_window_set_title (sms_window,"Forward SMS");
  sms_dialog_forward_message (sms_window, msg);
  gtk_widget_show_all ( GTK_WIDGET(sms_window) );
}

void cb_delete_folder (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("delete folder called");
  GtkWidget* msgDialog;

  GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
  GtkWidget* menulabel = GTK_BIN(menuitem)->child;
  gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
  if (!g_strcasecmp(oldName,"Inbox")  ||
      !g_strcasecmp(oldName,"Outbox") ||
      !g_strcasecmp(oldName,"Draft")  ||
      !g_strcasecmp(oldName,"Sent")   ||
      !g_strcasecmp(oldName,"Trash"))
    {
      msgDialog = gtk_message_dialog_new( GTK_WINDOW(moko_application_get_main_window(d->app)),
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_MESSAGE_WARNING,
                                          GTK_BUTTONS_CLOSE,
                                          g_strdup_printf("Current folder '%s'\nis not a custom folder\nCan't delete",oldName) );
      gtk_dialog_run (GTK_DIALOG (msgDialog));
      gtk_widget_destroy (msgDialog);
    }
  else
    {
      GtkWidget* dialog = gtk_message_dialog_new( GTK_WINDOW(moko_application_get_main_window(d->app)),
                          GTK_DIALOG_DESTROY_WITH_PARENT,
                          GTK_MESSAGE_WARNING,
                          GTK_BUTTONS_OK_CANCEL,
                          g_strdup_printf("Are you sure to delete folder: %s",d->currentfolder));
      gint result = gtk_dialog_run (GTK_DIALOG (dialog));
      switch (result)
        {
        case GTK_RESPONSE_OK:
          g_debug ("clicked ok");
          delete_folder(d,oldName);
          break;
        case GTK_RESPONSE_CANCEL:
          g_debug ("clicked cancel");
          break;
        default:
          g_debug ("clicked default");
          break;
        }
      gtk_widget_destroy (dialog);
    }
}

void cb_delete_message (GtkMenuItem* item, MessengerData* d)
{
  GtkTreeModel* model;
  GtkTreeIter iter;
  GtkTreeIter childiter;
  GtkTreeSelection* selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(d->view));
  gboolean has_selection = gtk_tree_selection_get_selected (selection, &model,&iter);
  if (has_selection)
    {
      gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(d->filter),&childiter,&iter);
      gtk_list_store_remove (d->liststore, &childiter);
    }
  else
    {
      GtkWidget* dialog = gtk_message_dialog_new( GTK_WINDOW(moko_application_get_main_window(d->app)),
                          GTK_DIALOG_DESTROY_WITH_PARENT,
                          GTK_MESSAGE_WARNING,
                          GTK_BUTTONS_OK,
                          "No message selected");
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
}

void cb_mmitem_activate (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("message membership");
  d->mmWin = sms_membership_window_new();
  gtk_window_set_decorated (GTK_WINDOW(d->mmWin), FALSE);
  sms_membership_window_set_messages (SMS_MEMBERSHIP_WINDOW(d->mmWin), d->liststore);
  sms_membership_window_set_menubox (SMS_MEMBERSHIP_WINDOW(d->mmWin), d->folderlist);
  sms_membership_window_show ( SMS_MEMBERSHIP_WINDOW(d->mmWin) );
}

void cb_frBtn_clicked (GtkButton* button, MessengerData* d)
{
  GSList *c;
  gchar* folder;

  GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
  GtkWidget* menulabel = GTK_BIN(menuitem)->child;
  gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
  gchar* newName = g_strdup (gtk_entry_get_text(GTK_ENTRY(d->frEntry)));
  gtk_label_set_text (GTK_LABEL(menulabel),newName);
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(d->filter));

  c = d->folderlist;
  for (; c; c = g_slist_next(c) )
    {
      folder = (gchar*) c->data;
      if (!g_strcasecmp(folder,oldName))
        {
          g_debug ("old %s, new %s", oldName, newName);
          c->data = g_strdup(newName);
        }
    }
  d->filtmenu = reload_filter_menu (d,d->folderlist);
  MokoMenuBox* menubox = (MokoMenuBox*)moko_paned_window_get_menubox( MOKO_PANED_WINDOW(d->window) );
  g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
  moko_menu_box_set_filter_menu(menubox, GTK_MENU(d->filtmenu));
  gtk_widget_show_all (GTK_WIDGET(menubox));
  gtk_widget_hide (d->frWin);
}

void cb_frResetBtn_clicked (GtkButton* button, GtkWidget* entry)
{
  gtk_entry_set_text(GTK_ENTRY(entry),"");
}

void cb_fnitem_activate (GtkMenuItem* item, MessengerData* d)
{
  g_debug ("folder rename called");
  GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
  GtkWidget* menulabel = GTK_BIN(menuitem)->child;

  gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
  if (!g_strcasecmp(oldName,"Inbox") ||
      !g_strcasecmp(oldName,"Outbox") ||
      !g_strcasecmp(oldName,"Draft") ||
      !g_strcasecmp(oldName,"Sent") ||
      !g_strcasecmp(oldName,"Sent"))
    {
      GtkWidget* msgDialog = gtk_message_dialog_new( GTK_WINDOW(moko_application_get_main_window(d->app)),
                             GTK_DIALOG_DESTROY_WITH_PARENT,
                             GTK_MESSAGE_WARNING,
                             GTK_BUTTONS_CLOSE,
                             g_strdup_printf("Current folder '%s'\nis not a custom folder\nCan't rename",oldName) );
      gtk_dialog_run (GTK_DIALOG (msgDialog));
      gtk_widget_destroy (msgDialog);
    }
  else
    {
      GtkWidget* hbox;
      GtkWidget* frResetBtn;
      GtkWidget* frConfirmBtn;

      if ((d->frWin != NULL) && (d->frWin->window != NULL))
        gtk_entry_set_text (GTK_ENTRY(d->frEntry),"");
      else
        {
          d->frWin = moko_dialog_window_new();
          GtkWidget* frBox = gtk_vbox_new (FALSE,10);
          gtk_widget_set_size_request (frBox, 480, -1);
          GtkWidget* frAlign = gtk_alignment_new (0,0,1,1);
          gtk_alignment_set_padding (GTK_ALIGNMENT(frAlign), 100, 0, 30, 10);
          moko_dialog_window_set_title (MOKO_DIALOG_WINDOW(d->frWin), "Folder Rename");

          GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
          GtkWidget* menulabel = GTK_BIN(menuitem)->child;
          GtkWidget* frLabel = gtk_label_new (g_strdup_printf("Please input new folder name for %s:",	gtk_label_get_text (GTK_LABEL(menulabel))));
          gtk_misc_set_alignment (GTK_MISC(frLabel),0,0.5);
          gtk_box_pack_start (GTK_BOX(frBox), frLabel, FALSE, TRUE, 0);

          d->frEntry = gtk_entry_new ();
          gtk_box_pack_start (GTK_BOX(frBox), d->frEntry, FALSE, TRUE, 0);

          hbox = gtk_hbox_new (FALSE,20);
          frConfirmBtn = gtk_button_new_with_label ("OK");
          frResetBtn = gtk_button_new_with_label ("Reset");
          gtk_box_pack_start (GTK_BOX(hbox), frConfirmBtn, FALSE, TRUE, 0);
          gtk_box_pack_start (GTK_BOX(hbox), frResetBtn, FALSE, TRUE, 0);
          gtk_box_pack_start (GTK_BOX(frBox), hbox, FALSE, TRUE, 0);
          gtk_container_add (GTK_CONTAINER(frAlign),frBox);

          moko_dialog_window_set_contents (MOKO_DIALOG_WINDOW(d->frWin), frAlign);
          g_signal_connect (G_OBJECT(frConfirmBtn),
                            "clicked",
                            G_CALLBACK(cb_frBtn_clicked),
                            d);
          g_signal_connect (G_OBJECT(frResetBtn),
                            "clicked",
                            G_CALLBACK(cb_frResetBtn_clicked),
                            d->frEntry);
        }
      gtk_widget_show_all ( GTK_WIDGET(d->frWin) );
    }
}

void cb_nfBtn_clicked (GtkButton* button, MessengerData* d)
{
  gchar* folder = g_strdup(gtk_entry_get_text(GTK_ENTRY(d->nfEntry)));
  g_debug ("new folder %s",folder);
  d->folderlist = g_slist_append (d->folderlist,folder);
  d->filtmenu = reload_filter_menu (d,d->folderlist);
  MokoMenuBox* menubox = (MokoMenuBox*)moko_paned_window_get_menubox( d->window );
  g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
  moko_menu_box_set_filter_menu(menubox,GTK_MENU(d->filtmenu));
  gtk_widget_show_all (GTK_WIDGET(menubox));
  foldersdb_update (d->folderlist);
  update_folder_sensitive(d, d->folderlist);
  gtk_widget_hide (d->nfWin);
}

void cb_nfResetBtn_clicked (GtkButton* button, MessengerData* d)
{
  gtk_entry_set_text(GTK_ENTRY(d->nfEntry),"");
  g_debug ("reset entry");
}

void cb_dfBtn_clicked (GtkButton* button, MessengerData* d)
{
  GSList* c;

  for( c =d->folderlist; c; c=g_slist_next(c))
    {
      if(!g_strcasecmp((gchar*)c->data, d->currentfolder))
        {
          d->folderlist = g_slist_remove (d->folderlist, c->data);
          break;
        }
    }

  d->filtmenu = reload_filter_menu (d,d->folderlist);
  MokoMenuBox* menubox = (MokoMenuBox*)moko_paned_window_get_menubox( MOKO_PANED_WINDOW(d->window) );
  g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
  moko_menu_box_set_filter_menu(menubox,GTK_MENU(d->filtmenu));

  /*set the default filter item to "Inbox" */
  gchar* str = g_strdup("Inbox");
  moko_menu_box_set_active_filter (menubox,str);
  gtk_widget_show_all (GTK_WIDGET(menubox));
  update_folder_sensitive (d, d->folderlist);

  /*result inform */
  GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(gtk_widget_get_parent_window(GTK_WIDGET(button))),
                      GTK_DIALOG_DESTROY_WITH_PARENT,
                      GTK_MESSAGE_INFO,
                      GTK_BUTTONS_CLOSE,
                      "Delete successful");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

void delete_folder (MessengerData* d, gchar* oldName)
{
  GSList* c;

  for( c =d->folderlist; c; c=g_slist_next(c))
    {
      if(!g_strcasecmp((gchar*)c->data, oldName))
        {
          d->folderlist = g_slist_remove (d->folderlist, c->data);
          break;
        }
    }

  d->filtmenu = reload_filter_menu (d,d->folderlist);
  MokoMenuBox* menubox = (MokoMenuBox*)moko_paned_window_get_menubox( MOKO_PANED_WINDOW(d->window) );
  g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
  moko_menu_box_set_filter_menu(menubox,GTK_MENU(d->filtmenu));

  /*set the default filter item to "Inbox" */
  gchar* str = g_strdup("Inbox");
  moko_menu_box_set_active_filter (menubox,str);
  gtk_widget_show_all (GTK_WIDGET(menubox));
  update_folder_sensitive (d, d->folderlist);
}

void on_mmode_rdo_btn_clicked (gchar* folder)
{
  g_debug ("switch to %s folder", folder);
}

void cb_cursor_changed(GtkTreeSelection* selection, MessengerData* d)
{
  GtkTreeModel* model;
  GtkTreeIter iter;
  GtkTreeIter childiter;
  GtkTreeView* view;
  message* msg;
  GdkPixbuf* icon;
  GError*   error = NULL;

  view = gtk_tree_selection_get_tree_view( selection );
  if ( gtk_tree_selection_get_selected( selection, &model, &iter ) )
    {
      msg = g_malloc(sizeof(message));
      gtk_tree_model_get( model, &iter, COLUMN_FROM, &msg->name, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_SUBJECT, &msg->subject, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_FOLDER, &msg->folder, -1 );
      gtk_tree_model_get( model, &iter, COLUMN_STATUS, &msg->status, -1);

      if (msg->status == UNREAD)
        {
          icon = gdk_pixbuf_new_from_file (PKGDATADIR "/Mode_Read.png", &error);
          msg->status = READ;
          gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(d->filter),&childiter,&iter);
          gtk_list_store_set (d->liststore, &childiter,
                              COLUMN_ICON, icon,
                              COLUMN_STATUS, msg->status,
                              -1);
        }
      detail_read_message (DETAIL_AREA(d->details),msg);
    }
}

void
on_btnsend_clicked                 (GtkButton       *button,
                                    gpointer         user_data)
{
  g_debug("Button send clicked\n");
}

void
on_btn_address_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  g_debug("Button address clicked\n");
}

void cb_search_entry_changed (GtkEditable* editable, MessengerData* d)
{
  GtkWidget* search_entry = GTK_WIDGET(editable);
  d->s_key = g_strdup (gtk_entry_get_text(GTK_ENTRY(search_entry)));
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(d->filter));
  g_debug ("search %s, result has %d messages",d->s_key,get_model_number(d));
  gchar* str = g_strdup_printf("search %s, result has %d messages",d->s_key,get_model_number(d));
  send_signal_to_footer(d->bus,str);
}

void cb_search_on (MessengerData* d)
{
  g_debug ("search on");
  GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
  GtkWidget* menulabel = GTK_BIN(menuitem)->child;
  gtk_label_set_text (GTK_LABEL(menulabel),"Search Result");
  d->searchOn = TRUE;
}

void cb_search_off (MessengerData* d)
{
  g_debug ("search off ");
  GtkWidget* menuitem = gtk_menu_get_attach_widget (GTK_MENU(d->filtmenu));
  GtkWidget* menulabel = GTK_BIN(menuitem)->child;
  gtk_label_set_text (GTK_LABEL(menulabel),"Inbox");
  d->searchOn = FALSE;
  GtkWidget* search_entry = GTK_WIDGET(moko_tool_box_get_entry (MOKO_TOOL_BOX(d->toolbox)));
  gtk_entry_set_text (GTK_ENTRY(search_entry), "");
  d->s_key = "";
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(d->filter));
}

void cb_addressBtn_clicked (GtkButton* button, gpointer* data)
{
  GtkWidget* contactWin = sms_contact_new();
  gtk_window_set_decorated (GTK_WINDOW(contactWin),FALSE);
  GtkWidget* toEntry = (GtkWidget*)data;
  g_signal_connect (G_OBJECT(contactWin),"contact_select_done",
                    G_CALLBACK(sms_contact_select_done), toEntry);
  gtk_widget_show(contactWin);
}

void sms_contact_select_done (GtkWidget* widget, gpointer data)
{
  g_debug("sms select contact done");
  GtkWidget* toEntry = (GtkWidget*)data;
  GList* contacts = sms_get_selected_contacts(SMS_CONTACT_WINDOW(widget));
  g_debug ("start to add %d contacts to entry", g_list_length(contacts));
  gchar* nameList = NULL;
  gchar* name;
  GList* nextContext = contacts;
  EContact *contact;
  for ( ; nextContext != NULL; nextContext = nextContext->next){
    contact =  E_CONTACT (nextContext->data);
    name = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
    if (nameList == NULL)
      nameList = g_strdup (name);
    else
      nameList = g_strconcat (nameList,",",name,NULL);
    g_debug(nameList);
  }
  if (strlen(gtk_entry_get_text (GTK_ENTRY(toEntry))) > 0)
    gtk_entry_append_text (GTK_ENTRY(toEntry),g_strdup_printf(",%s",nameList));
  else
    gtk_entry_set_text (GTK_ENTRY(toEntry),nameList);
  g_free(nameList);
  g_free(name);
}

