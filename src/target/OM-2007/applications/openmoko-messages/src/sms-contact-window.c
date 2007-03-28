/*
 *  sms-contact-window.c
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: alex $]
 */
#include "sms-contact-window.h"

G_DEFINE_TYPE (SmsContactWindow, sms_contact_window, MOKO_TYPE_WINDOW)

#define SMS_CONTACT_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SMS_TYPE_CONTACT_WINDOW, SmsContactWindowPrivate))

typedef struct
{
  /* private members define here */
  GtkWidget* vbox;
  GtkWidget* buttonbox;
  GtkWidget* contacts_view;
  SmsContactData* data;
}SmsContactWindowPrivate;

static gint sms_contact_signals[LAST_SIGNAL] = {0};

static void sms_contact_window_close (SmsContactWindow* self);
GtkWidget* create_contacts_list (SmsContactData* data);
static gboolean open_book(SmsContactData* data);
static void loadContacts (EBook* book, EBookStatus status, gpointer closure);
static void updateContactsView (EBook* book, EBookStatus status, 
                                GList* contacts, gpointer closure);
static void contacts_view_cursor_changed(GtkTreeSelection* selection, SmsContactData* data);
static void contact_select_done(void);
gboolean get_selected_contact (GtkTreeModel* model, GtkTreePath* path, 
                               GtkTreeIter* iter, gpointer data);

static void
sms_contact_window_dispose(GObject* object)
{
  if (G_OBJECT_CLASS (sms_contact_window_parent_class)->dispose)
    G_OBJECT_CLASS (sms_contact_window_parent_class)->dispose (object);
}

static void
sms_contact_window_finalize(GObject* object)
{
  G_OBJECT_CLASS (sms_contact_window_parent_class)->finalize (object);
}


static void
sms_contact_window_class_init (SmsContactWindowClass* klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private (klass, sizeof(SmsContactWindowPrivate));

  /* create a new signal */
  sms_contact_signals[CONTACT_SELECT_DONE_SIGNAL] = g_signal_new("contact_select_done",
                                                   SMS_TYPE_CONTACT_WINDOW,
						   G_SIGNAL_RUN_FIRST,
						   G_STRUCT_OFFSET(SmsContactWindowClass, contact_select_done),
						   NULL,NULL,
						   g_cclosure_marshal_VOID__VOID,
						   G_TYPE_NONE,0,NULL);

  object_class->dispose = sms_contact_window_dispose;
  object_class->finalize = sms_contact_window_finalize;

}

GtkWidget* sms_contact_new()
{
  return GTK_WIDGET(g_object_new(SMS_TYPE_CONTACT_WINDOW, NULL));
}

static void updateContactsView (EBook* book, EBookStatus status, 
                                GList* contacts, gpointer closure)
{
  GtkTreeIter iter;
  SmsContactData* data = (SmsContactData*)closure;
  GtkListStore *contacts_liststore = data->contacts_liststore;
  GList* c = contacts;
  data->contacts = contacts;
  g_debug ("list length %d", g_list_length(c));

  for (;c;c=c->next){
    EContact *contact = E_CONTACT (c->data);
    const gchar *name, *phoneNum;
    name = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
    phoneNum =  e_contact_get_const (contact, E_CONTACT_PHONE_BUSINESS);
    gtk_list_store_append(contacts_liststore, &iter);
    gtk_list_store_set (contacts_liststore, &iter, 
                        CONTACT_SEL_COL, FALSE,
                        CONTACT_NAME_COL, name,
			CONTACT_CELLPHONE_COL, phoneNum,
			-1);
  }
}

static void loadContacts(EBook* book, EBookStatus status, gpointer closure)
{
  EBookQuery* query;
  SmsContactData* data = (SmsContactData*)closure;

  if(status == E_BOOK_ERROR_OK){
    query = e_book_query_any_field_contains ("");
    e_book_async_get_contacts(data->book, query, updateContactsView, data);
    e_book_query_unref(query);
    g_debug("start loading to tree view");
  }
  else
    g_warning("Got error %d when opening book",status);
}

static gboolean
open_book (SmsContactData* data)
{
  e_book_async_open(data->book,FALSE,loadContacts,data);
  return FALSE;
}

static void
sms_contact_window_init (SmsContactWindow* self)
{
  MokoWindow* parent = (MokoWindow*)moko_application_get_main_window( moko_application_get_instance() );
  if ( parent )
    {
      gtk_window_set_transient_for( GTK_WINDOW(self), GTK_WINDOW(parent) );
      gtk_window_set_modal( GTK_WINDOW(self), TRUE );
      gtk_window_set_destroy_with_parent( GTK_WINDOW(self), TRUE );
    }

  /* initialzation */
  SmsContactWindowPrivate* priv = SMS_CONTACT_WINDOW_GET_PRIVATE(self);
  priv->vbox = gtk_vbox_new(FALSE,0);
  SmsContactData* data = g_new0(SmsContactData,1);
  priv->data = data;
  data->book = e_book_new_system_addressbook(NULL);
  if (!data->book)
    g_critical ("Could not load system addressbook");

  /* Set the "select" window title */
  GtkWidget* titleLabel = gtk_label_new ("Select Contacts");
  gtk_widget_set_name( GTK_WIDGET(titleLabel), "mokodialogwindow-title-label" );
  GtkWidget* eventbox = gtk_event_box_new();
  gtk_container_add( GTK_CONTAINER(eventbox), GTK_WIDGET(titleLabel) );
  gtk_widget_set_name( eventbox, "mokodialogwindow-title-labelbox" );  
  gtk_box_pack_start(GTK_BOX(priv->vbox), eventbox, FALSE, TRUE, 0);

  /* create contact list */
  data->contacts_liststore = gtk_list_store_new (3, G_TYPE_BOOLEAN,
                                                 G_TYPE_STRING, G_TYPE_STRING);
  priv->contacts_view = create_contacts_list(data);
  gtk_box_pack_start(GTK_BOX(priv->vbox), priv->contacts_view, TRUE, TRUE, 0);
  g_object_unref (data->contacts_liststore);

  /* add close and back window button */
  GtkWidget* closebutton = moko_pixmap_button_new();
  moko_pixmap_button_set_action_btn_lower_label ( MOKO_PIXMAP_BUTTON(closebutton),"Select");
  g_signal_connect_swapped( G_OBJECT(closebutton), "clicked", G_CALLBACK(sms_contact_window_close), self );
  GtkWidget* backbutton = moko_pixmap_button_new();
  moko_pixmap_button_set_action_btn_lower_label ( MOKO_PIXMAP_BUTTON(backbutton),"Back");
  g_signal_connect_swapped( G_OBJECT(backbutton), "clicked", G_CALLBACK(sms_contact_window_close), self );
  priv->buttonbox = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start(GTK_BOX(priv->buttonbox), closebutton, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(priv->buttonbox), backbutton, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(priv->vbox), priv->buttonbox, FALSE, TRUE, 0);
  
  /* loading contacts */
  g_idle_add((GSourceFunc)open_book,data);
  //loadBook(data);

  gtk_widget_show_all(priv->vbox);
  gtk_container_add(GTK_CONTAINER(self), priv->vbox);

}

GtkWidget *
create_contacts_list (SmsContactData *data)
{
  MokoNavigationList* moko_navigation_list = (MokoNavigationList*)moko_navigation_list_new ();
  GtkWidget* treeview = GTK_WIDGET (moko_navigation_list_get_tree_view (moko_navigation_list));
  
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
                           GTK_TREE_MODEL (data->contacts_liststore));

  /* add columns to treeview */
  GtkCellRenderer* renderer;
  GtkTreeViewColumn* column;

  /* name column */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "Name");

  renderer = gtk_cell_renderer_toggle_new();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "active", CONTACT_SEL_COL, NULL);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "text", CONTACT_NAME_COL, NULL);
  gtk_tree_view_column_set_sort_column_id (column, CONTACT_NAME_COL);

  moko_navigation_list_append_column (moko_navigation_list, column);

  /* mobile column */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Cell Phone", renderer,
                                                     "text", CONTACT_CELLPHONE_COL, NULL);
  gtk_tree_view_column_set_sort_column_id (column, CONTACT_CELLPHONE_COL);
  moko_navigation_list_append_column (moko_navigation_list, column);

  GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
  g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(contacts_view_cursor_changed), data );

  return GTK_WIDGET (moko_navigation_list);
}

gboolean get_selected_contact (GtkTreeModel* model, GtkTreePath* path, 
                               GtkTreeIter* iter, gpointer data)
{
  SmsContactWindow* self = (SmsContactWindow*)data;
  SmsContactWindowPrivate* priv = SMS_CONTACT_WINDOW_GET_PRIVATE(self);
  SmsContactData* contactData = priv->data;
  g_debug ("select item contacts %d", g_list_length(contactData->contacts));

  gchar* name;
  gboolean selected;
  gtk_tree_model_get (model, iter,
                      CONTACT_SEL_COL, &selected,
		      CONTACT_NAME_COL,&name,
		      -1);
  if (selected)
    g_debug ("contact %s selected", name);
  else {
    g_debug ("contact %s not selected, remove from contacts list", name);
    GList* contactListItem = contactData->contacts;
    for ( ; contactListItem; contactListItem=contactListItem->next){
      EContact* contact = E_CONTACT (contactListItem->data);
      const gchar *contactName;
      contactName = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
      if (!g_strcasecmp(name, contactName)) 
        contactData->contacts = g_list_remove (contactData->contacts,
	                                       contactListItem->data);
    }
  }

  return FALSE;
}

static void sms_contact_window_close (SmsContactWindow* self)
{
  /* get selected items */
  SmsContactWindowPrivate* priv = SMS_CONTACT_WINDOW_GET_PRIVATE(self);
  GtkWidget* contactView = moko_navigation_list_get_tree_view(MOKO_NAVIGATION_LIST(priv->contacts_view));
  GtkTreeModel* contactModel = gtk_tree_view_get_model (GTK_TREE_VIEW(contactView));
  gtk_tree_model_foreach (contactModel, get_selected_contact, self);
  self->selectedContacts = priv->data->contacts;

  /* emit selection done signal */
  g_signal_emit (G_OBJECT(self),sms_contact_signals[CONTACT_SELECT_DONE_SIGNAL],0);

  /* Synthesize delete_event to close dialog. */

  GtkWidget *widget = GTK_WIDGET(self);
  GdkEvent *event;

  event = gdk_event_new( GDK_DELETE );

  event->any.window = g_object_ref(widget->window);
  event->any.send_event = TRUE;

  gtk_main_do_event( event );
  gdk_event_free( event );
  
}

static void contacts_view_cursor_changed(GtkTreeSelection* selection, SmsContactData* data)
{
  GtkTreeModel* model;
  GtkTreeIter iter;
  //GtkTreeView* view = gtk_tree_selection_get_tree_view( selection );

  if ( gtk_tree_selection_get_selected( selection, &model, &iter ) )
  {
     gchar* name;
     gchar* phoneNum;
     gboolean selected;
     GtkListStore* liststore = data->contacts_liststore;

     gtk_tree_model_get( model, &iter, 
                         CONTACT_SEL_COL, &selected,
			 CONTACT_NAME_COL, &name,
                         CONTACT_CELLPHONE_COL, &phoneNum,
                         -1);

    if (selected)
      gtk_list_store_set (liststore, &iter, CONTACT_SEL_COL, FALSE, -1);
    else
      gtk_list_store_set (liststore, &iter, CONTACT_SEL_COL, TRUE, -1);
  }

}

static void contact_select_done(void)
{
  g_debug("select ok");
}

