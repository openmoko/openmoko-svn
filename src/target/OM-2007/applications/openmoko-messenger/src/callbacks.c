#include "callbacks.h"
#include "detail-area.h"
#include <gtk/gtk.h>


gboolean cb_filter_changed(GtkWidget* widget, gchar* text, MessengerData* d)
{
    g_debug("changed to %s folder",text);
    d->currentfolder = g_strdup(text);
    gtk_tree_model_filter_refilter (d->filter);
    
    return FALSE;
}

void cb_new_sms (GtkMenuItem* item, MessengerData* d)
{
    SmsDialogWindow* sms_window = sms_dialog_window_new();
    sms_dialog_window_set_title (sms_window,"New SMS");
    gtk_widget_show_all ( GTK_WIDGET(sms_window) );
}

void cb_new_mail (GtkMenuItem* item, MessengerData* d)
{
    SmsDialogWindow* sms_window = sms_dialog_window_new();
    mail_dialog_window_set_title (sms_window,"New Email");
    gtk_widget_show_all ( GTK_WIDGET(sms_window) );
}

void cb_new_folder (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("new folder called");
    MokoDialogWindow* nfWin = moko_dialog_window_new();
    GtkWidget* nfBox = gtk_vbox_new (FALSE,10);
    gtk_widget_set_size_request (nfBox, 480, -1);
    GtkWidget* nfAlign = gtk_alignment_new (0,0,1,1);
    gtk_alignment_set_padding (GTK_ALIGNMENT(nfAlign),100,NULL,30,10);
    moko_dialog_window_set_title (nfWin, "New Folder");
    
    GtkWidget* nfLabel = gtk_label_new ("Please input new folder name:");
    gtk_misc_set_alignment (GTK_MISC(nfLabel),0,0.5);
    gtk_box_pack_start (GTK_BOX(nfBox), nfLabel, FALSE, TRUE, 0);

    GtkWidget* hbox = gtk_hbox_new (FALSE,20);
    d->nfEntry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX(hbox), d->nfEntry, FALSE, TRUE, 0);
    GtkWidget* nfResetBtn = gtk_button_new_with_label ("Reset");
    gtk_box_pack_start (GTK_BOX(hbox), nfResetBtn, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(nfBox), hbox, FALSE, TRUE, 0);

    hbox = gtk_hbox_new (FALSE,0);
    GtkWidget* nfConfirmBtn = gtk_button_new_with_label ("OK");
    gtk_box_pack_start (GTK_BOX(hbox), nfConfirmBtn, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(nfBox), hbox, FALSE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER(nfAlign),nfBox);

    moko_dialog_window_set_contents (nfWin, nfAlign);
    g_signal_connect (G_OBJECT(nfConfirmBtn), 
                      "clicked",
		      G_CALLBACK(cb_nfBtn_clicked),
		      d);
    g_signal_connect (G_OBJECT(nfResetBtn),
                      "clicked",
		      G_CALLBACK(cb_nfResetBtn_clicked),
		      d);
    gtk_widget_show_all ( GTK_WIDGET(nfWin) );
    
}

void cb_mode_read (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("mode read");
    message* msg;
    GtkTreeSelection* selection = gtk_tree_view_get_selection( d->view );
    GtkTreeView* view = gtk_tree_selection_get_tree_view( selection );
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

    gchar* name = NULL;
    gchar* folder = NULL;
    if ( has_selection )
    	{
    		msg = g_malloc(sizeof(message));
        gtk_tree_model_get( model, &iter, COLUMN_FROM, &msg->name, -1 );
        gtk_tree_model_get( model, &iter, COLUMN_SUBJECT, &msg->subject, -1 );
        gtk_tree_model_get( model, &iter, COLUMN_FOLDER, &msg->folder, -1 );
      }
    else msg = NULL;
    detail_read_message (d->details,msg);
}

void cb_mode_reply (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("mode reply");
    message* msg;
    GtkTreeSelection* selection = gtk_tree_view_get_selection( d->view );
    GtkTreeView* view = gtk_tree_selection_get_tree_view( selection );
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

    gchar* name = NULL;
    gchar* folder = NULL;
    if ( has_selection )
    	{
    		msg = g_malloc(sizeof(message));
        gtk_tree_model_get( model, &iter, COLUMN_FROM, &msg->name, -1 );
        gtk_tree_model_get( model, &iter, COLUMN_SUBJECT, &msg->subject, -1 );
        gtk_tree_model_get( model, &iter, COLUMN_FOLDER, &msg->folder, -1 );
      }
    else msg = NULL;
    
}

void cb_mode_forward (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("mode forward");
    message* msg;
    GtkTreeSelection* selection = gtk_tree_view_get_selection( d->view );
    GtkTreeView* view = gtk_tree_selection_get_tree_view( selection );
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

    gchar* name = NULL;
    gchar* folder = NULL;
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
    detail_forward_message (d->details,msg);
}

void cb_delete_folder (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("delete folder called");
    GtkWidget* msgDialog;
    
    GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
		GtkWidget* menulabel = GTK_BIN(menuitem)->child;
		gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
    if (!g_strcasecmp(oldName,"Inbox") ||
			  !g_strcasecmp(oldName,"Outbox") ||
			  !g_strcasecmp(oldName,"Draft") ||
			  !g_strcasecmp(oldName,"Sent")){
			  	
			  msgDialog = gtk_message_dialog_new( moko_application_get_main_window(d->app),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_WARNING,
                                  GTK_BUTTONS_CLOSE,
                                  g_strdup_printf("Current folder '%s'\nis not a custom folder\nCan't delete",oldName) );
		    gtk_dialog_run (GTK_DIALOG (msgDialog));
		    gtk_widget_destroy (msgDialog);
		}else{
		    GtkWidget* dialog = gtk_message_dialog_new( moko_application_get_main_window(d->app),
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
	  
	  selection = gtk_tree_view_get_selection (d->view);
	  gboolean has_selection = gtk_tree_selection_get_selected (selection, &model,&iter);
	  if (has_selection){
	  	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(d->filter),&childiter,&iter);
	  	gtk_list_store_remove (d->liststore, &childiter);
	  }
	  else {
	  	GtkWidget* dialog = gtk_message_dialog_new( moko_application_get_main_window(d->app),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_WARNING,
                                  GTK_BUTTONS_OK,
                                  "No message selected");
		    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
			  gtk_widget_destroy (dialog);
	  }
}

void cb_mmitem_activate (GtkMenuItem* item, MessengerData* d)
{
    g_debug ("message membership");
    if (d->mmWin == NULL){
    	d->mmWin = sms_membership_window_new();
    	sms_membership_window_set_menubox (SMS_MEMBERSHIP_WINDOW(d->mmWin), d->folderlist);
    	sms_membership_window_set_messages (SMS_MEMBERSHIP_WINDOW(d->mmWin), d->liststore);
    }
    sms_membership_window_show ( d->mmWin );
}

void cb_frBtn_clicked (GtkButton* button, MessengerData* d)
{
		GSList *c;
    GtkMenu* filtmenu;
    gchar* folder;
    
		GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
		GtkWidget* menulabel = GTK_BIN(menuitem)->child;
		gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
    gchar* newName = g_strdup (gtk_entry_get_text(GTK_ENTRY(d->frEntry)));
		gtk_label_set_text (GTK_LABEL(menulabel),newName);
		gtk_tree_model_filter_refilter (d->filter);
		
    c = d->folderlist;
    for (; c; c = g_slist_next(c) ){
        folder = (gchar*) c->data;
	      if (!g_strcasecmp(folder,oldName)){
	      	g_debug ("old %s, new %s", oldName, newName);
	      	c->data = g_strdup(newName); 
	      }
    }
    d->filtmenu = reload_filter_menu (d,d->folderlist);
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
    moko_menu_box_set_filter_menu(menubox,d->filtmenu);
    gtk_widget_show_all (menubox);
}

void cb_frResetBtn_clicked (GtkButton* button, GtkWidget* entry)
{
		gtk_entry_set_text(GTK_ENTRY(entry),"");
}

void cb_fnitem_activate (GtkMenuItem* item, MessengerData* d)
{
		g_debug ("folder rename called");
		GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
		GtkWidget* menulabel = GTK_BIN(menuitem)->child;
		gchar* oldName = g_strdup (gtk_label_get_text (GTK_LABEL(menulabel)));
		if (!g_strcasecmp(oldName,"Inbox") ||
			  !g_strcasecmp(oldName,"Outbox") ||
			  !g_strcasecmp(oldName,"Draft") ||
			  !g_strcasecmp(oldName,"Sent")){
			  	
			  GtkWidget* msgDialog = gtk_message_dialog_new( moko_application_get_main_window(d->app),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_WARNING,
                                  GTK_BUTTONS_CLOSE,
                                  g_strdup_printf("Current folder '%s'\nis not a custom folder\nCan't rename",oldName) );
		    gtk_dialog_run (GTK_DIALOG (msgDialog));
		    gtk_widget_destroy (msgDialog);
		}else {
		    MokoDialogWindow* frWin = moko_dialog_window_new();
		    GtkWidget* frBox = gtk_vbox_new (FALSE,10);
		    gtk_widget_set_size_request (frBox, 480, -1);
		    GtkWidget* frAlign = gtk_alignment_new (0,0,1,1);
		    gtk_alignment_set_padding (GTK_ALIGNMENT(frAlign),100,NULL,30,10);
		    moko_dialog_window_set_title (frWin, "Folder Rename");
		    
		    GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
				GtkWidget* menulabel = GTK_BIN(menuitem)->child;
		    GtkWidget* frLabel = gtk_label_new (g_strdup_printf("Please input new folder name for %s:", 
		    					gtk_label_get_text (GTK_LABEL(menulabel))));
		    gtk_misc_set_alignment (GTK_MISC(frLabel),0,0.5);
		    gtk_box_pack_start (GTK_BOX(frBox), frLabel, FALSE, TRUE, 0);
		
		    GtkWidget* hbox = gtk_hbox_new (FALSE,20);
		    d->frEntry = gtk_entry_new ();
		    gtk_box_pack_start (GTK_BOX(hbox), d->frEntry, FALSE, TRUE, 0);
		    GtkWidget* frResetBtn = gtk_button_new_with_label ("Reset");
		    gtk_box_pack_start (GTK_BOX(hbox), frResetBtn, FALSE, TRUE, 0);
		    gtk_box_pack_start (GTK_BOX(frBox), hbox, FALSE, TRUE, 0);
		
		    hbox = gtk_hbox_new (FALSE,0);
		    GtkWidget* frConfirmBtn = gtk_button_new_with_label ("OK");
		    gtk_box_pack_start (GTK_BOX(hbox), frConfirmBtn, FALSE, TRUE, 0);
		    gtk_box_pack_start (GTK_BOX(frBox), hbox, FALSE, TRUE, 0);
		    gtk_container_add (GTK_CONTAINER(frAlign),frBox);
		
		    moko_dialog_window_set_contents (frWin, frAlign);
		    g_signal_connect (G_OBJECT(frConfirmBtn), 
		                      "clicked",
				      G_CALLBACK(cb_frBtn_clicked),
				      d);
		    g_signal_connect (G_OBJECT(frResetBtn),
		                      "clicked",
				      G_CALLBACK(cb_frResetBtn_clicked),
				      d->frEntry);
		    gtk_widget_show_all ( GTK_WIDGET(frWin) );
   }
}

void cb_nfBtn_clicked (GtkButton* button, MessengerData* d)
{
    gchar* folder = g_strdup(gtk_entry_get_text(GTK_ENTRY(d->nfEntry)));
    g_debug ("new folder %s",folder);
    d->folderlist = g_slist_append (d->folderlist,folder);
    d->filtmenu = reload_filter_menu (d,d->folderlist);
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
    moko_menu_box_set_filter_menu(menubox,d->filtmenu);
    gtk_widget_show_all (menubox);
    /*foldersdb_update (d->folderlist);*/
    update_folder_sensitive(d, d->folderlist);
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
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
    moko_menu_box_set_filter_menu(menubox,d->filtmenu);

    /*set the default filter item to "Inbox" */
    gchar* str = g_strdup("Inbox");
    moko_menu_box_set_active_filter (menubox,str);
    gtk_widget_show_all (menubox);
    update_folder_sensitive (d, d->folderlist);
    
    /*result inform */
    GtkWidget *dialog = gtk_message_dialog_new ((gtk_widget_get_parent_window(button)),
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
    MokoMenuBox* menubox = moko_paned_window_get_menubox( d->window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), d );
    moko_menu_box_set_filter_menu(menubox,d->filtmenu);

    /*set the default filter item to "Inbox" */
    gchar* str = g_strdup("Inbox");
    moko_menu_box_set_active_filter (menubox,str);
    gtk_widget_show_all (menubox);
    update_folder_sensitive (d, d->folderlist);
}

void on_mmode_rdo_btn_clicked (gchar* folder)
{
    g_debug ("switch to %s folder", folder);
}

void cb_cursor_changed(GtkTreeSelection* selection, MessengerData* d)
{
		gchar* name = NULL;
    gchar* folder = NULL;
    GtkTreeModel* model;
    GtkTreeIter iter;
    GtkTreeIter childiter;
    GtkTreeView* view;
    message* msg;
    GdkPixbuf* icon;
    GError*   error = NULL;
    
	  g_debug( "openmoko-messenger: selection changed" );
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
        detail_read_message (d->details,msg);
      }
}

void
on_btnsend_clicked                 (GtkButton       *button,
                                    gpointer         user_data)
{
	g_printf("Button send clicked\n");
}

void
on_btn_address_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	g_printf("Button address clicked\n");
}

void cb_search_entry_changed (GtkEditable* editable, MessengerData* d)
{
	GtkWidget* search_entry = GTK_WIDGET(editable);
	d->s_key = g_strdup (gtk_entry_get_text(search_entry));
	gtk_tree_model_filter_refilter (d->filter);
	g_debug ("search %s",d->s_key);
}

void cb_search_on (MessengerData* d)
{
	g_debug ("search on",d->s_key);
	GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
	GtkWidget* menulabel = GTK_BIN(menuitem)->child;
	gtk_label_set_text (GTK_LABEL(menulabel),"Search Result");
	d->searchOn = TRUE;
}

void cb_search_off (MessengerData* d)
{
	g_debug ("search off",d->s_key);
	GtkWidget* menuitem = gtk_menu_get_attach_widget (d->filtmenu);
	GtkWidget* menulabel = GTK_BIN(menuitem)->child;
	gtk_label_set_text (GTK_LABEL(menulabel),"Inbox");
	d->searchOn = FALSE; 
	GtkWidget* search_entry = moko_tool_box_get_entry (d->toolbox);
	gtk_entry_set_text (search_entry, "");
	d->s_key = "";
	gtk_tree_model_filter_refilter (d->filter);
}
