/*
 *  Messenger -- An messenger application for OpenMoko Framework
 *
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 First International Company
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

#include "main.h"
#include "foldersdb.h"
#include "detail-area.h"
#include "callbacks.h"
#include "message.h"

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-tree-view.h>
#include <libmokoui/moko-details-window.h>

#include <gtk/gtk.h>

int main( int argc, char** argv )
{
    g_debug( "openmoko-messenger starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    MessengerData* d = g_new ( MessengerData, 1);
    d->foldersdb = foldersdb_new();
    d->app = MOKO_APPLICATION (moko_application_get_instance());
    d->currentfolder = g_strdup("Inbox");
    g_set_application_name( "Messenger" ); 
    
    /* ui */
    setup_ui(d);

		//disable mmitem 
		update_folder_sensitive (d, d->folderlist);
		
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(d->window) );
    gtk_main();

    return 0;

}

void update_folder_sensitive (MessengerData* d, GSList* folderlist)
{
	if (g_slist_length (folderlist) > 4){
			gtk_widget_set_sensitive (d->mmitem, TRUE);
			gtk_widget_set_sensitive (d->fnitem, TRUE);
		}
		else{
			gtk_widget_set_sensitive (d->mmitem, FALSE);
			gtk_widget_set_sensitive (d->fnitem, FALSE);
		}
}

GtkWidget* reload_filter_menu (MessengerData* d, GSList* folderlist)
{
    GSList *c;
    GtkMenu* filtmenu;
    filtmenu = GTK_MENU(gtk_menu_new());
    c = folderlist;
    for (; c; c = g_slist_next(c) ){
        gchar* folder = (gchar*) c->data;
	      g_debug( "adding folder '%s'", folder );
        gtk_menu_shell_append( GTK_MENU_SHELL( filtmenu ), gtk_menu_item_new_with_label( folder ) );
    }

    return GTK_WIDGET(filtmenu);
}

void setup_ui( MessengerData* d )
{
     /* main window */
     d->window = MOKO_PANED_WINDOW(moko_paned_window_new());
		 d->mmWin = NULL;
		 
     /* application menu */
     d->menu = GTK_MENU(gtk_menu_new()); 
     d->mmitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Message Membership" ));/* message membershp item */
     d->fnitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Folder Rename" ));/* folder rename item */
     GtkMenuItem* accountitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Account" ));/* account item */
     GtkMenuItem* helpitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Help" ));/* message membershp item */
     GtkWidget* sepitem = gtk_separator_menu_item_new(); 
     GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
     g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(main_quit), d );
     g_signal_connect( G_OBJECT(d->mmitem), "activate", G_CALLBACK(cb_mmitem_activate), d );
     g_signal_connect( G_OBJECT(d->fnitem), "activate", G_CALLBACK(cb_fnitem_activate), d );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(d->mmitem) );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(d->fnitem) );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(accountitem) );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(helpitem) );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(sepitem) );
     gtk_menu_shell_append( GTK_MENU_SHELL(d->menu), GTK_WIDGET(closeitem) );
     moko_paned_window_set_application_menu( d->window, GTK_MENU(d->menu) );

     /* filter menu */
     d->filtmenu = GTK_MENU(gtk_menu_new());

     d->folderlist = foldersdb_get_folders( d->foldersdb );
     d->filtmenu = GTK_MENU(reload_filter_menu (d, d->folderlist));

    moko_paned_window_set_filter_menu( d->window, d->filtmenu );
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );

     /* connect close event */
     g_signal_connect( G_OBJECT(d->window), "delete_event", G_CALLBACK( main_quit ), d );

    /* set navagation area */
    populate_navigation_area( d );

    /* set toolbox */
    MokoPixmapButton* newButton;
    MokoPixmapButton* modeButton;
    MokoPixmapButton* getmailButton;
    MokoPixmapButton* deleteButton;

    GtkMenu* newMenu;
    GtkMenu* modeMenu;
    GtkMenu* deleteMenu;
    
    GtkWidget* image;

    /* set tool bar */
    d->toolbox = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    //gtk_widget_grab_focus( GTK_WIDGET(d->toolbox) );
    GtkWidget* searchEntry = moko_tool_box_get_entry (d->toolbox);
    g_signal_connect( G_OBJECT(searchEntry), "changed", G_CALLBACK(cb_search_entry_changed), d ); 
    g_signal_connect_swapped ( G_OBJECT(d->toolbox), "searchbox_visible", G_CALLBACK(cb_search_on), d ); 
    g_signal_connect_swapped( G_OBJECT(d->toolbox), "searchbox_invisible", G_CALLBACK(cb_search_off), d ); 

    /* set action buttons*/
    deleteMenu = GTK_MENU( gtk_menu_new() );
    GtkImageMenuItem* delMsgItem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label( "Delete Message" ));
    GtkImageMenuItem* delFolderItem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label( "Delete Folder" ));
    g_signal_connect( G_OBJECT(delFolderItem), "activate", G_CALLBACK(cb_delete_folder), d ); 
    g_signal_connect( G_OBJECT(delMsgItem), "activate", G_CALLBACK(cb_delete_message), d ); 

    gtk_menu_shell_append( GTK_MENU_SHELL(deleteMenu), GTK_WIDGET(delMsgItem) );
    gtk_menu_shell_append( GTK_MENU_SHELL(deleteMenu), GTK_WIDGET(delFolderItem) );
    gtk_widget_show_all (GTK_WIDGET(deleteMenu));
    deleteButton = moko_tool_box_add_action_button( MOKO_TOOL_BOX(d->toolbox) );
    image = gtk_image_new_from_file (PKGDATADIR "/Delete_Message.png");
    moko_pixmap_button_set_center_image (MOKO_PIXMAP_BUTTON(deleteButton),image);
    moko_pixmap_button_set_menu (MOKO_PIXMAP_BUTTON(deleteButton), deleteMenu);

    getmailButton = moko_tool_box_add_action_button( MOKO_TOOL_BOX(d->toolbox) );
    image = gtk_image_new_from_file (PKGDATADIR "/GetMail.png");
    moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(getmailButton),image);

    modeMenu = GTK_MENU( gtk_menu_new() );
    GtkImageMenuItem* modeReadItem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label( "Mode Read" ));
    GtkImageMenuItem* modeReplyItem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label( "Mode Reply" ));
    GtkImageMenuItem* modeFwdItem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label( "Mode Forward" ));

    gtk_menu_shell_append( GTK_MENU_SHELL(modeMenu), GTK_WIDGET(modeReadItem));
    gtk_menu_shell_append( GTK_MENU_SHELL(modeMenu), GTK_WIDGET(modeReplyItem));
    gtk_menu_shell_append( GTK_MENU_SHELL(modeMenu), GTK_WIDGET(modeFwdItem));
    g_signal_connect( G_OBJECT(modeReadItem), "activate", G_CALLBACK(cb_mode_read), d );
    g_signal_connect( G_OBJECT(modeReplyItem), "activate", G_CALLBACK(cb_mode_reply), d );
    g_signal_connect( G_OBJECT(modeFwdItem), "activate", G_CALLBACK(cb_mode_forward), d );
    gtk_widget_show_all (GTK_WIDGET(modeMenu));
    modeButton = moko_tool_box_add_action_button( MOKO_TOOL_BOX(d->toolbox) );
    image = gtk_image_new_from_file (PKGDATADIR "/Mode_Read.png");
    moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(modeButton),image);
    moko_pixmap_button_set_menu ( MOKO_PIXMAP_BUTTON(modeButton),GTK_MENU(modeMenu) );

    newMenu = GTK_MENU( gtk_menu_new() );
    GtkImageMenuItem* newMsgItem = GTK_IMAGE_MENU_ITEM ( gtk_image_menu_item_new_with_label( "New SMS") );
    GtkImageMenuItem* newFolderItem = GTK_IMAGE_MENU_ITEM ( gtk_image_menu_item_new_with_label( "New Folder") );
    GtkImageMenuItem* newMailItem = GTK_IMAGE_MENU_ITEM ( gtk_image_menu_item_new_with_label( "New Mail") );

    gtk_menu_shell_append( GTK_MENU_SHELL(newMenu), GTK_WIDGET(newMsgItem));
    gtk_menu_shell_append( GTK_MENU_SHELL(newMenu), GTK_WIDGET(newMailItem));
    gtk_menu_shell_append( GTK_MENU_SHELL(newMenu), GTK_WIDGET(newFolderItem));
    gtk_widget_show_all ( GTK_WIDGET(newMenu) );
    g_signal_connect( G_OBJECT(newMsgItem), "activate", G_CALLBACK(cb_new_sms), d );
    g_signal_connect( G_OBJECT(newMailItem), "activate", G_CALLBACK(cb_new_mail), d );
    g_signal_connect( G_OBJECT(newFolderItem), "activate", G_CALLBACK(cb_new_folder), d );
    newButton = moko_tool_box_add_action_button( MOKO_TOOL_BOX(d->toolbox) );
    image = gtk_image_new_from_file (PKGDATADIR "/New_Mail.png");
    moko_pixmap_button_set_center_image (MOKO_PIXMAP_BUTTON(newButton),image);
    moko_pixmap_button_set_menu ( MOKO_PIXMAP_BUTTON(newButton),GTK_MENU(newMenu) );

    moko_paned_window_add_toolbox( d->window, d->toolbox );

    /* detail area */
    populate_detail_area (d);
    
		//select the first column
		gint index = 0;
		GtkTreeSelection* tree_selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(d->view) );
		GtkTreePath *path = gtk_tree_path_new_from_indices( index, -1 );
		gtk_tree_selection_select_path( tree_selection, path );
		gtk_tree_view_set_cursor( GTK_TREE_VIEW(d->view), path, NULL, FALSE ); 
    gtk_widget_grab_focus (d->view);
}

int in_string(char *str, char *key)
{
  int length, key_length;
  int m,n,i;

  length = strlen(str);
  key_length = strlen(key);

  n=0;

  for(m=0;m<length;m++)
    {
      if(str[m] == key[n])
			{
			  for(i=0;i<key_length;i++)
			    if(str[m+i]!= key[i+n])
			      break;
		
			  if(i == key_length)
			    return 1;
			  else
			    {
			      m = m+i+1;
			      n=0;
			    }
			}
    }
  return 0;
}

gboolean filter_visible_function (GtkTreeModel* model, GtkTreeIter* iter, MessengerData* d)
{
	  gchar* folder;
	  gchar* from;
	  gchar* subject;
	  gtk_tree_model_get (model, iter, COLUMN_FOLDER, &folder, -1);
	  gtk_tree_model_get (model, iter, COLUMN_FROM, &from, -1);
	  gtk_tree_model_get (model, iter, COLUMN_SUBJECT, &subject, -1);
	  
	  if (d->searchOn){
	  	if ((strlen(d->s_key) > 0) && !in_string(from, d->s_key) && !in_string(subject, d->s_key))
	  		return FALSE;
	  }
	  else {
	  	gtk_menu_set_active (d->filtmenu,0);
	  	if(g_strcasecmp(folder,d->currentfolder))
	  				return FALSE;
		}

		return TRUE;
}

void
cell_data_func (GtkTreeViewColumn *col,
				GtkCellRenderer   *renderer,
				GtkTreeModel      *model,
				GtkTreeIter  	    *iter,
				gpointer          user_data)
{
	gint status;
	
  gtk_tree_model_get(model, iter, COLUMN_STATUS, &status, -1);

  if (status == UNREAD)
      g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
  else
      g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, "weight-set", FALSE, NULL);
}

void populate_navigation_area( MessengerData* d )
{
		guint         i;
    GdkPixbuf* icon;
    GError*   error = NULL;
    GtkTreeIter  iter;
    
    d->liststore = gtk_list_store_new( NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING );
    d->filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (d->liststore),NULL);
    d->view = MOKO_TREE_VIEW( moko_tree_view_new_with_model( GTK_TREE_MODEL (d->filter)) ); 
    
    for (i = 0;  i < G_N_ELEMENTS(names);  ++i)
	  {
		  gtk_list_store_append(d->liststore, &iter);
			switch(states[i])
			{
				case UNREAD	:	icon = gdk_pixbuf_new_from_file (PKGDATADIR "/Unread.png", &error);break;
				case READ	:	icon = gdk_pixbuf_new_from_file (PKGDATADIR "/Mode_Read.png", &error);break;
				case REPLIED	:	icon = gdk_pixbuf_new_from_file (PKGDATADIR "/Mode_Reply.png", &error);break;
				case FORWARD	:	icon = gdk_pixbuf_new_from_file (PKGDATADIR "/Mode_Forward.png", &error);break;
			}
		  gtk_list_store_set(d->liststore, &iter,
		  									 COLUMN_ICON, icon,
		                     COLUMN_FROM, names[i],
		                     COLUMN_SUBJECT, subjects[i],
		                     COLUMN_CONTENT, contents[i],
		                     COLUMN_STATUS, states[i],
		                     COLUMN_FOLDER, folders[i],
		                     -1);
	  }
	  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (d->filter),
    																				filter_visible_function,
    																				d,
    																				NULL);
    																				
    GtkCellRenderer* ren;
    GtkTreeViewColumn* column = gtk_tree_view_column_new();
	  gtk_tree_view_column_set_title(column, "From");
	  
    /* Add status picture */
    ren = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, ren, FALSE);
	  gtk_tree_view_column_set_attributes(column, ren,
										  "pixbuf", COLUMN_ICON,
										  NULL);
		/* add message from name */
    ren = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, ren, TRUE);
		gtk_tree_view_column_set_attributes(column, ren,
						  "text", COLUMN_FROM,
						  NULL);
		/* Bold if UNREAD */
		gtk_tree_view_column_set_cell_data_func (column, ren, cell_data_func, d->liststore, NULL);
    moko_tree_view_append_column( d->view, column );

    ren = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column, "Subject");
		gtk_tree_view_column_pack_start(column, ren, TRUE);
		gtk_tree_view_column_set_attributes(column, ren,
						  "text", COLUMN_SUBJECT,
						  NULL);
    gtk_tree_view_column_set_cell_data_func (column, ren, cell_data_func, d->liststore, NULL);
    moko_tree_view_append_column( d->view, column );

    GtkTreeSelection* selection = gtk_tree_view_get_selection( d->view );
    g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(cb_cursor_changed), d );
    
    moko_paned_window_set_upper_pane( d->window, GTK_WIDGET(moko_tree_view_put_into_scrolled_window(d->view)) );
    gtk_widget_hide(d->mmWin);
}

void populate_detail_area( MessengerData* d )
{
   d->details = detail_area_new();
   gtk_widget_show (d->details);
   moko_paned_window_set_lower_pane( d->window, GTK_WIDGET(moko_details_window_put_in_box(d->details)));
}

void main_quit(GtkWidget* widget, GdkEvent* event, MessengerData* d)
{
    foldersdb_update (d->folderlist);
    gtk_main_quit();
}

