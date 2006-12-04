#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "folder.h"
#include "message.h"

gchar *mode[] = {"New SMS", "New Email", "New Folder", "Mode Read",
				"Mode Reply", "Mode Forward", "Delete Message", "Delete Folder"};

void
on_toolbar_search_btn_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *toolbar = lookup_widget (GTK_WIDGET(button), "toolbar");
    gtk_notebook_set_current_page( GTK_NOTEBOOK(toolbar), SEARCH_PAGE);
	
    GtkWidget* filter_menu_item = lookup_widget (GTK_WIDGET(window1),"filter_menu_item");

    if (GTK_BIN(filter_menu_item)->child)
    {
        GtkWidget *child = GTK_BIN(filter_menu_item)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_set_text (GTK_LABEL (child), "Search Result");
    }
	
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter));
}

void
on_search_entry_changed                (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gchar *s_key;
	GtkWidget *search_entry;
	
	search_entry = GTK_WIDGET(editable);
	s_key = g_strdup(gtk_entry_get_text (GTK_ENTRY(search_entry)));
  	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter));
  	printf("%s \n",s_key);
}

void
on_searchbar_quit_btn_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *toolbar = lookup_widget (GTK_WIDGET(button), "toolbar");
	GtkWidget *search_entry = lookup_widget (GTK_WIDGET(button), "search_entry");
    gtk_notebook_set_current_page( GTK_NOTEBOOK(toolbar), TOOLBAR_PAGE);
	gtk_entry_set_text (GTK_ENTRY(search_entry),"");
}

void
on_sms_btnsend_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *sms_to_entry;
  GtkWidget *sms_txtview;
  GtkTextBuffer * buffer;
  GtkTextIter start, end;
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkWidget *message_treeview;
  GdkPixbuf    *icon;
  GtkTreeIter  selected_row;
  GtkTreeIter  real_selected;
  gint         msg_id;


  sms_to_entry = lookup_widget(GTK_WIDGET(button),"sms_to_entry");
  sms_txtview = lookup_widget(GTK_WIDGET(button),"sms_txtview");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sms_txtview));
  message_treeview = lookup_widget(GTK_WIDGET(window1),"message_treeview");
  
  /*
   * set the start and end point
   * then get the characters
   */
  gtk_text_buffer_get_iter_at_offset(buffer,&start,0);
  gtk_text_buffer_get_iter_at_offset(buffer,&end,160); /*FIXME: The total number of characters should get by parsing the label */

  printf("Message from: %s\n",gtk_entry_get_text(GTK_ENTRY(sms_to_entry)));
  printf("Message content: %s\n",gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start,&end,FALSE));
	
  /* if there is a selection, load the message id, set the icon */
  sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(message_treeview));
  model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter));

  if(gtk_tree_selection_get_selected(sel, &model, &selected_row))
  {
  	gtk_tree_model_get(model,&selected_row,COL_MSG_ID,&msg_id,-1);

  	if(current_status == ST_REPLIED)
	  icon = create_pixbuf("mode_reply.png");
  	else if(current_status == ST_FORWARD)
	  icon = create_pixbuf("mode_forward.png");

	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(filter),&real_selected,&selected_row);
  	gtk_list_store_set (liststore, &real_selected,
					  	COL_ICON, icon, 
			  		  	COL_STATUS, current_status,
			  		  	-1);
	
  	set_msg_status (msg_id,current_status);
  }
  
  printf("Button send clicked\n");
}

void
on_btn_address_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Button address clicked\n");
}

gboolean
on_sms_txtview_key_release_event       (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
  GtkTextBuffer * buffer;
  GtkWidget * sms_input_label;
  GtkWidget * sms_txtview;
  gint n,m;

  sms_input_label = lookup_widget(GTK_WIDGET(widget),"sms_input_label");
  if(sms_input_label!=NULL)
    {
      sms_txtview = lookup_widget(GTK_WIDGET(widget),"sms_txtview");
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sms_txtview));
      n = 160 - gtk_text_buffer_get_char_count(buffer)%160;
      m = gtk_text_buffer_get_char_count(buffer)/160 + 1;
      gtk_label_set_text(GTK_LABEL(sms_input_label),g_strdup_printf("%d(%d)",n,m));
    }
	
  return FALSE;
}

void
on_bar_separate_btn_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *detail_area;
	
	detail_area = lookup_widget (GTK_WIDGET(button), "detail_area");
	
	if(GTK_WIDGET_VISIBLE(detail_area))
		gtk_widget_hide(detail_area);
	else
		gtk_widget_show(detail_area);

}

void
on_email_btnsend_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *email_to_entry;
  GtkWidget *email_cc_entry;
  GtkWidget *email_bcc_entry;
  GtkWidget *email_subject_entry;
  GtkWidget *email_txtview;
  GtkTextBuffer * buffer;
  GtkTextIter start, end;

  email_to_entry = lookup_widget(GTK_WIDGET(button),"email_to_entry");
  email_cc_entry = lookup_widget(GTK_WIDGET(button),"email_cc_entry");
  email_bcc_entry = lookup_widget(GTK_WIDGET(button),"email_bcc_entry");
  email_subject_entry = lookup_widget(GTK_WIDGET(button),"email_subject_entry");
  email_txtview = lookup_widget(GTK_WIDGET(button),"email_txtview");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (email_txtview));
  
  /*
   * set the start and end point
   * then get the characters
   */
  gtk_text_buffer_get_iter_at_offset(buffer,&start,0);
  gtk_text_buffer_get_iter_at_offset(buffer,&end,160);/*FIXME: The total number of characters should get by parsing the label */
  

  printf("Mail from: %s\n",gtk_entry_get_text(GTK_ENTRY(email_to_entry)));
  printf("Mail cc: %s\n",gtk_entry_get_text(GTK_ENTRY(email_cc_entry)));
  printf("Mail bcc: %s\n",gtk_entry_get_text(GTK_ENTRY(email_bcc_entry)));
  printf("Mail subject: %s\n",gtk_entry_get_text(GTK_ENTRY(email_subject_entry)));
  printf("Message content: %s\n",gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start,&end,FALSE));

  printf("Button send clicked\n");
}

void
on_email_btnattach_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Email button attach clicked\n");
}

void reload_messages (gint sel_id)
{
	GtkTreeIter    iter;
	GdkPixbuf     *icon;
	GError        *error = NULL;
	GtkTreeModel *model;
	
	FILE *fp;
  	MESSAGE msg;
  	char next;
	
	/* add to the list*/
	fp = fopen("message.dat","r");
  
	fseek(fp,0,SEEK_SET);
  	while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);

      if(msg.id == sel_id) /*is found */
		{
			g_debug ("message %d get found\n",sel_id);
	 	 	/* read the status ,set the icon */
	  	 	if(msg.status == ST_UNREAD)
				icon = create_pixbuf ("Email.png");
	  		else
				icon = create_pixbuf ("mode_read.png");
	  		if (error)
	    	{
	     		g_warning ("Could not load icon: %s\n", error->message);
	      		g_error_free(error);
	     		error = NULL;
	    	}
			model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER(filter));
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						     COL_ICON, icon,
			 		         COL_FROM, msg.from,
			     			 COL_CONTENT, msg.content,
			     			 COL_TIME, msg.time,
			     			 COL_STATUS, msg.status,
							 COL_FOLDER, msg.folder,
							 COL_MSG_ID, msg.id,
							 -1);
		}
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
		break;
    }
	fclose(fp);
	
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter));
}

void on_mmode_rdo_btn_clicked (gchar *fname)
{
	GtkTreeSelection *sel;
	GtkWidget         *treeview;
	GtkTreeModel     *model;
	GtkTreeModel     *childmodel;
	GtkTreeIter       real_selected;
	GtkTreeIter       selected_row;
	int sel_id;
	
	/* get the tree view and the selection */
	treeview = lookup_widget (window1,"message_treeview");
	g_assert(treeview != NULL);
	
	/* get selected message */
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	g_assert(gtk_tree_selection_get_mode(sel) == GTK_SELECTION_SINGLE);

	/* if there is selected, change the folder in message.dat*/
	if (gtk_tree_selection_get_selected(sel, &model, &selected_row))
    {
      /* get the selected id */
      gtk_tree_model_get(model,&selected_row,COL_MSG_ID,&sel_id,-1);
      childmodel = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER(model));
      gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(filter),&real_selected,&selected_row);

	  g_debug("move %d message to %s folder\n",sel_id,fname);
	  set_msg_folder(sel_id,fname);
		
		
      /* remove from the navagation list */
      gtk_list_store_remove(GTK_LIST_STORE(childmodel), &real_selected);
	  
	  /* then we reload it from the message.dat */
	  reload_messages(sel_id);
    }
  else
    {
      /* If no row is selected, the button should
       *  not be clickable in the first place 
       * popup the information dialog to alert */
    	GtkWidget *info_win;
		
		info_win = create_infor_dialog();
		set_information_dialog(info_win,"You haven't select a message yet!");
		gtk_widget_show (info_win);
    }
}

void
open_mail_account                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	g_debug("open email account called!\n");
}

void
help                                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	g_debug("get help called!\n");
}

void set_information_dialog (GtkWidget *infor_dialog, gchar *information)
{
	GtkWidget *lbl_warning;
	GtkWidget *btn_infor_ok;
	
	btn_infor_ok = lookup_widget (infor_dialog,"btn_infor_ok");
	lbl_warning = lookup_widget (infor_dialog,"lbl_warning");
	
	gtk_label_set_text (GTK_LABEL(lbl_warning),information);
	handler_id = g_signal_connect_swapped ((gpointer) btn_infor_ok, "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (infor_dialog));
}

void
on_btn_rf_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *rf_entry;
	
	rf_entry = lookup_widget(GTK_WIDGET(button),"rf_entry");
	gtk_entry_set_text(GTK_ENTRY(rf_entry),"");
}

void
on_btn_rf_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *rf_window ;
	GtkWidget *rf_entry ;
	GtkWidget *filter_menu_item;
	GtkWidget *child;
	GtkWidget *lbl_fname;
	gchar *oldname, *fname;
	
	rf_window = lookup_widget (GTK_WIDGET(button),"rf_window");
	filter_menu_item = lookup_widget (GTK_WIDGET(window1),"filter_menu_item");
	lbl_fname = lookup_widget (GTK_WIDGET(button),"lbl_fname");
	rf_window = lookup_widget (GTK_WIDGET(button),"rf_window");
	rf_entry = lookup_widget (GTK_WIDGET(button),"rf_entry");
	
	/* check the new name entry , warning if it's empty*/
	child = GTK_BIN(filter_menu_item)->child;
	oldname = g_strdup(gtk_label_get_text(GTK_LABEL(child)));
	fname = g_strdup(gtk_entry_get_text (GTK_ENTRY(rf_entry)));
	
	if(strlen(fname) == 0)
	{
		GtkWidget * infor_dialog;
		infor_dialog = create_infor_dialog();
		set_information_dialog(infor_dialog,"Folder name can't be empty!");
	}
	else
	{
		GtkWidget *menu;
		GtkWidget *activate_item;
		GtkWidget *mitem_label;
		
		/* rename the folder , then reload the menu */
		rename_folder(oldname,fname);
		gtk_label_set(GTK_LABEL (child), fname);
		gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(filter));
		
		menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(filter_menu_item));
		activate_item = gtk_menu_get_active (GTK_MENU(menu));
		mitem_label = GTK_BIN(activate_item)->child;
		gtk_label_set_text (GTK_LABEL(mitem_label),fname);
		
		/* destroy the window */
		gtk_widget_destroy (rf_window);
	}
}

void
cb_open_mm_mode                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *detail_area;
	GSList *rdo_btn_group;
	GtkWidget *rdo_btn;
	GtkWidget *mmode_folder_box;
	GtkWidget *box;
	GtkWidget *viewport;
	GtkWidget *label;
	
	label = gtk_label_new ("Message Membership");
	gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	//gtk_label_set_angle (GTK_LABEL(label),123.0);
	gtk_misc_set_alignment (GTK_MISC(label),0.5,0.5);
	gtk_widget_set_size_request(label,480,30);
	gtk_widget_show (label);
	
	mmode_folder_box = gtk_vbox_new (FALSE, 0);
	detail_area = lookup_widget (GTK_WIDGET(menuitem),"detail_area");
	viewport = lookup_widget (GTK_WIDGET(menuitem),"mmode_viewport");
	
	box = GTK_BIN(viewport)->child;
	if (box != NULL)
		gtk_container_remove (GTK_CONTAINER(viewport),box);
	gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_MESSAGE_MEMBERSHIP);
	gtk_box_pack_start (GTK_BOX(mmode_folder_box),label,FALSE,TRUE,0);
	
	FILE *fp;
	FOLDER folder;
	char next;
	
	/* read from folder_list.dat       */
	/* add each radio button to box  */
	fp = fopen("folder_list.dat","r");
	
	fseek(fp,0,SEEK_SET);
	while(1) 
	{
		fread(&folder,sizeof(folder),1,fp);
		if(g_strcasecmp(folder.fname,"deleted") && (next!=EOF))
		{
			if(!g_strcasecmp(folder.fname,"Inbox"))
			{
				rdo_btn = gtk_radio_button_new_with_label (NULL, folder.fname);
				rdo_btn_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rdo_btn));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rdo_btn), TRUE);
			}
			else
				rdo_btn = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rdo_btn), folder.fname);
	
			gtk_box_pack_start (GTK_BOX (mmode_folder_box), rdo_btn, FALSE, TRUE, 0);
			/* can only use "released" signal */
			g_signal_connect_swapped (G_OBJECT(rdo_btn), "released",
									  G_CALLBACK (on_mmode_rdo_btn_clicked),
									  (gpointer) g_strdup (folder.fname));
			gtk_widget_show (rdo_btn);
		}
	
		next = fgetc(fp);
		if(next == EOF)
			break;
		fseek(fp,-1,SEEK_CUR);
	}
  
	fclose(fp);
	
	gtk_container_add (GTK_CONTAINER(viewport),mmode_folder_box);
	gtk_widget_show(mmode_folder_box);

	g_debug("mm mode called!\n");
}


void
cb_folder_rename                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	g_debug("rename folder called!\n");
	gchar *current_folder;
	GtkWidget *filter_menu_item;
	GtkWidget *lbl_fname;
	
	filter_menu_item = lookup_widget (GTK_WIDGET(menuitem),"filter_menu_item");
	
	if (GTK_BIN(filter_menu_item)->child)
    {
        GtkWidget *child = GTK_BIN(filter_menu_item)->child;
        g_assert( GTK_IS_LABEL(child));
        current_folder = g_strdup(gtk_label_get_text(GTK_LABEL(child)));
    }
	
	if(is_custom_folder(current_folder))
	{
		GtkWidget *rf_win;
		
		rf_win = create_rf_window ();
		lbl_fname = lookup_widget (GTK_WIDGET(rf_win),"lbl_fname");
		gtk_label_set_text (GTK_LABEL(lbl_fname),current_folder);
		gtk_widget_show (rf_win);
	}
	else
	{
		GtkWidget *info_win;
		
		info_win = create_infor_dialog();
		set_information_dialog(info_win,"It's a default folder.\nYou can't rename it");
		gtk_widget_show (info_win);
	}
}

void
on_btn_nf_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *nf_entry;
	
	nf_entry = lookup_widget(GTK_WIDGET(button),"nf_entry");
	gtk_entry_set_text (GTK_ENTRY(nf_entry),"");
}

void cb_clean_mode()
{
	GtkWidget *to_entry;
	GtkWidget *txtview;
	GtkTextBuffer *buffer;
	
	to_entry = lookup_widget (GTK_WIDGET(window1),"sms_to_entry");
	gtk_entry_set_text (GTK_ENTRY(to_entry),"");
	txtview = lookup_widget (GTK_WIDGET(window1),"sms_txtview");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(txtview));
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER(buffer),"", -1);
}

void on_mitem_new_folder()
{
	GtkWidget *nf_window = create_nf_window();
	GtkWidget *btn_nf_ok = lookup_widget (nf_window,"btn_nf_ok");
	
	g_signal_connect (G_OBJECT(btn_nf_ok),
						"clicked",
						G_CALLBACK(cb_new_folder),
						(gpointer)nf_window);
	
	gtk_widget_show (nf_window);
}

void on_mitem_mode_reply ()
{
	GtkWidget *detail_area;
	GtkWidget *message_treeview;
	GtkWidget *to_entry;
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	gchar *str_from;
	gint id;
	
	detail_area = lookup_widget (GTK_WIDGET(window1),"detail_area");
	message_treeview = lookup_widget (GTK_WIDGET(window1),"message_treeview");
	
	/* clean before take action */
	cb_clean_mode();
	
	/* first get the selection */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(message_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		{
	  		/* get the selected messages
	   		* load the params*/
	  		gtk_tree_model_get (model, &iter,
			      				COL_FROM, &str_from,
								COL_MSG_ID, &id,
			      				-1);

	  		/* set the "To:" entry, update the status */
	  		to_entry = lookup_widget (GTK_WIDGET(window1),"sms_to_entry");
	  		gtk_entry_set_text (GTK_ENTRY(to_entry),str_from);
			current_status = ST_REPLIED;
			
			g_free(str_from);
		}
	
	/* turn to reply mode */
	gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_SMS_REPLY);
}

void on_mitem_mode_forward ()
{
	GtkWidget *detail_area;
	GtkWidget *message_treeview;
	GtkTextBuffer *buffer;
    GtkWidget *txtview;
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	gchar *str_content;
	
	detail_area = lookup_widget (GTK_WIDGET(window1),"detail_area");
	message_treeview = lookup_widget (GTK_WIDGET(window1),"message_treeview");
	
	/* clean before take action */
	cb_clean_mode();
	
	/* first get the selection */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(message_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		{
	  		/* get the selected messages
	   		* load the params*/
	  		gtk_tree_model_get (model, &iter,
			      				COL_CONTENT, &str_content,
			      				-1);

	  		/* set the textview */
			txtview = lookup_widget (GTK_WIDGET(window1),"sms_txtview");
			buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(txtview));
	  		gtk_text_buffer_set_text (GTK_TEXT_BUFFER(buffer),str_content, -1);
			current_status = ST_FORWARD;
			
			g_free(str_content);
		}
	
	/* turn to reply mode */
	gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_SMS_FORWARD);
}

void cb_delete_message (gpointer user_data)
{
	GtkTreeSelection *sel;
	GtkTreeModel     *model;
	GtkTreeModel     *childmodel;
	GtkTreeIter       real_selected;
	GtkTreeIter       selected_row;
	GtkWidget         *treeview;
	GtkWidget         *button;
	GtkWidget         *infor_dialog;
	int sel_id;
	
	button = (GtkWidget *)user_data;
	infor_dialog = lookup_widget (GTK_WIDGET(button),"infor_dialog");
	
	/* get the tree view and the selection */
	treeview = lookup_widget (window1,"message_treeview");
	g_assert(treeview != NULL);

	/* get selection */
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	g_assert(gtk_tree_selection_get_mode(sel) == GTK_SELECTION_SINGLE);
	gtk_tree_selection_get_selected(sel, &model, &selected_row);
	
	/* get the selected id */
    gtk_tree_model_get(model,&selected_row,COL_MSG_ID,&sel_id,-1);
    childmodel = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER(model));
    gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(filter),&real_selected,&selected_row);
	
	/* remove from the navagation list */
    gtk_list_store_remove(GTK_LIST_STORE(childmodel), &real_selected);

    /* remove from message.dat */
	/* later we should remove from the db */
    delete_msg(sel_id);
	
	gtk_widget_destroy (infor_dialog);
}

/* delete selection message */
void on_mitem_delete_selected_message ()
{
	GtkTreeSelection *sel;
	GtkTreeModel     *model;
	GtkTreeIter       selected_row;
	GtkWidget         *treeview;
	
	/* get the tree view and the selection */
	treeview = lookup_widget (window1,"message_treeview");
	g_assert(treeview != NULL);

	/* get selection */
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	g_assert(gtk_tree_selection_get_mode(sel) == GTK_SELECTION_SINGLE);

	/* if there is selected, remove from the list and message.dat*/
	if (gtk_tree_selection_get_selected(sel, &model, &selected_row))
    {
      GtkWidget *infor_dialog = create_infor_dialog();
	  GtkWidget *btn_infor_ok = lookup_widget (GTK_WIDGET(infor_dialog),"btn_infor_ok");
	
	  set_information_dialog(infor_dialog,"Are you sure to delete this message ?");
	  g_signal_handler_disconnect (G_OBJECT(btn_infor_ok),handler_id);
	  g_signal_connect (G_OBJECT(btn_infor_ok),
						"clicked",
						G_CALLBACK(cb_delete_message),
						(gpointer)infor_dialog);
		
	  gtk_widget_show (infor_dialog);
    }
  else
    {
      /* If no row is selected, the button should
       *  not be clickable in the first place 
       * popup the information dialog to alert */
	  GtkWidget *infor_dialog = create_infor_dialog();
	  
      set_information_dialog(infor_dialog,"You haven't select a message yet!");
	  gtk_widget_show (infor_dialog);
    }
}

void cb_new_folder(gpointer user_data)
{
	GtkWidget *nf_window = lookup_widget (GTK_WIDGET(user_data),"nf_window");
	GtkWidget *entry = lookup_widget (GTK_WIDGET(user_data),"nf_entry");
	const gchar *entry_text;
 
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
	if(strlen(entry_text) != 0)/* FIXME: check in the folder list add */
	{
		/* add folder in the filter menu
		 * connect the signal */
		GtkWidget *filter_menu_item = lookup_widget (GTK_WIDGET(window1),"filter_menu_item");
		GtkWidget *filter_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM(filter_menu_item));
		GtkWidget *fi_mitem;
		
		fi_mitem = gtk_menu_item_new_with_mnemonic (entry_text);
		gtk_menu_shell_append (GTK_MENU_SHELL (GTK_MENU(filter_menu)), fi_mitem);
		gtk_widget_show (fi_mitem);
		
		/* add folder in the folder list */
 		add_folder(g_strdup(entry_text));
		gtk_widget_destroy(nf_window);
	}
	else
	{
		GtkWidget *infor_dialog = create_infor_dialog();
	  
     	set_information_dialog(infor_dialog,"New folder name empty!");
		gtk_widget_show (infor_dialog);
	}
}


void cb_delete_folder (gpointer user_data)
{
	GtkWidget *info_win;
	GtkWidget *old_menu;
	GtkWidget *filter_menu_item;
	gchar *fname;
	GtkWidget *child;
	
	info_win = lookup_widget(GTK_WIDGET(user_data),"infor_dialog");
	filter_menu_item = lookup_widget (GTK_WIDGET(window1),"filter_menu_item");
	
	if (GTK_BIN(filter_menu_item)->child)
    {
        child = GTK_BIN(filter_menu_item)->child;
        g_assert( GTK_IS_LABEL(child) );
        fname = g_strdup(gtk_label_get_text(GTK_LABEL(child)));
    }
	
	g_assert (info_win != NULL);
	g_debug("delete %s folder\n",fname);
	
	old_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM(filter_menu_item));
	gtk_menu_detach (GTK_MENU(old_menu));
	
	/* delete folder from the list */
	delete_folder(fname);
	
	/* reload the filter menu */
	set_filter_menu(window1);
	//g_free(old_menu);
	
	g_assert( GTK_IS_LABEL(child) );
    gtk_label_set(GTK_LABEL (child), "Inbox");
	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(filter));
	
	/* close the window */
	gtk_widget_destroy (info_win);
}

void on_mitem_delete_current_folder()
{
	GtkWidget *filter_menu_item;
	gchar *current_folder;
	
	filter_menu_item = lookup_widget (GTK_WIDGET(window1),"filter_menu_item");
	if (GTK_BIN(filter_menu_item)->child)
    {
        GtkWidget *child = GTK_BIN(filter_menu_item)->child;
        g_assert( GTK_IS_LABEL(child) );
        current_folder = g_strdup(gtk_label_get_text(GTK_LABEL(child)));
    }
	
	if(is_custom_folder(current_folder))
	{
		GtkWidget *info_win;
		GtkWidget *btn_infor_ok;
		
		info_win = create_infor_dialog();
		btn_infor_ok = lookup_widget (GTK_WIDGET(info_win),"btn_infor_ok");
		set_information_dialog(info_win,"Are you sure to delete this folder?");
		g_signal_handler_disconnect (G_OBJECT(btn_infor_ok),handler_id);
		g_signal_connect  (G_OBJECT(btn_infor_ok),
							"clicked",
							G_CALLBACK(cb_delete_folder),
							(gpointer)info_win);
		gtk_widget_show (info_win);
	}
	else
	{
		GtkWidget *info_win;
		
		info_win = create_infor_dialog();
		set_information_dialog(info_win,"It's a default folder.\nYou can't delete it");
		gtk_widget_show (info_win);
	}
}

void
cb_action_mitem_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gchar* text;
	GtkWidget *detail_area = lookup_widget (GTK_WIDGET(window1),"detail_area");
	gint i;
    
	if (GTK_BIN(menuitem)->child)
    {
        GtkWidget *child = GTK_BIN(menuitem)->child;
        g_assert( GTK_IS_LABEL(child) );
        gtk_label_get(GTK_LABEL (child), &text);
    }
    
	g_debug("%s get selected\n",text);
	
	for (i=0;i<8;i++)
	{
		if(!g_strcasecmp(mode[i],text))
			break;
	}
	
	switch(i)
	{
		case 0:	cb_clean_mode();
				gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_SMS_NEW);
				break;
		case 1: gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_MAIL_NEW);break;
		case 2:	on_mitem_new_folder();break;
		case 3:	gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_SMS_READ);break;
		case 4:	on_mitem_mode_reply (); break;
		case 5: on_mitem_mode_forward ();break;
		case 6: on_mitem_delete_selected_message();break;
		case 7: on_mitem_delete_current_folder();break;
		default: gtk_notebook_set_current_page (GTK_NOTEBOOK(detail_area),MODE_SMS_READ);break;
	}
}
