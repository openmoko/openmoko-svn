/* moko-dialer-autolist .c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */

 #include "moko-dialer-autolist.h"
  #include "error.h"
 
G_DEFINE_TYPE (MokoDialerAutolist, moko_dialer_autolist, GTK_TYPE_HBOX)

enum {
  SELECTED_SIGNAL,
  CONFIRMED_SIGNAL,
  LAST_SIGNAL
};

//forward definition

gboolean on_tip_press_event (MokoDialerTip       *tip,GdkEventButton  *event,gpointer        user_data);

static gint moko_dialer_autolist_signals[LAST_SIGNAL] = { 0 };

static void
moko_dialer_autolist_class_init (MokoDialerAutolistClass *class)
{
/*
  GtkVBoxClass* vbox_class;

  vbox_class= (GtkVBoxClass*) class;
*/

  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;

 g_print("moko_dialer_autolist:start signal register\n");
  
  moko_dialer_autolist_signals[SELECTED_SIGNAL] = 
  		g_signal_new ("user_selected",
               G_OBJECT_CLASS_TYPE (object_class),
		G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET (MokoDialerAutolistClass, moko_dialer_autolist_selected),
               NULL,NULL,
               g_cclosure_marshal_VOID__CHAR,
		G_TYPE_NONE, 1,g_type_from_name("gchar"));

  //                G_TYPE_NONE, 0);

g_print("moko_dialer_autolist:signal register end,got the id :%d\n", moko_dialer_autolist_signals[SELECTED_SIGNAL]);

  moko_dialer_autolist_signals[CONFIRMED_SIGNAL] = 
  		g_signal_new ("user_confirmed",
               G_OBJECT_CLASS_TYPE (object_class),
		G_SIGNAL_RUN_FIRST,
		 G_STRUCT_OFFSET (MokoDialerAutolistClass, moko_dialer_autolist_confirmed),
               NULL,NULL,
               g_cclosure_marshal_VOID__CHAR,
		G_TYPE_NONE, 1,g_type_from_name("gchar"));

  //                G_TYPE_NONE, 0);

g_print("moko_dialer_autolist:signal register end,got the id :%d\n", moko_dialer_autolist_signals[CONFIRMED_SIGNAL]);

}


static void
moko_dialer_autolist_init (MokoDialerAutolist *moko_dialer_autolist)
{
DBG_ENTER();

 
  GtkWidget * imagePerson;
  GtkWidget * tip;
  imagePerson = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (imagePerson);
  gtk_box_pack_start (GTK_CONTAINER(moko_dialer_autolist), imagePerson, TRUE, TRUE, 0);

//insert the tips

  for(int i=0;i<MOKO_DIALER_MAX_TIPS;i++)
  	{
  	tip=moko_dialer_tip_new();
  	moko_dialer_tip_set_index(tip, i);
  	moko_dialer_tip_set_label(tip,"tony guan");
  	moko_dialer_autolist->tips[i]=tip;

	 gtk_box_pack_start(GTK_CONTAINER(moko_dialer_autolist), tip, TRUE, TRUE, 0);  	
	 
	  g_signal_connect ((gpointer) tip, "button_press_event",
                    G_CALLBACK (on_tip_press_event),moko_dialer_autolist);
	gtk_widget_set_size_request (tip, 20, 20);	
	
  	}
moko_dialer_autolist->head=0;
//here we begin to initialize the contacts data.
	  

    
}

gboolean moko_dialer_autolist_set_data (MokoDialerAutolist *moko_dialer_autolist,DIALER_CONTACTS_LIST_HEAD* head)
{
if(moko_dialer_autolist->head)
	contact_release_contact_list(moko_dialer_autolist->head);
moko_dialer_autolist->head=head;
return TRUE;
}
/**
 * @brief initiate the font for widget of textviewCodes 
 *
 *
 *
 * @param text_view GtkWidget*, any widget which can help to lookup for labelcontactN
 * @param count gint, the count of the alternative, max set to MAXDISPNAMENUM.
 * @retval 
 */

int moko_dialer_autolist_fill_alternative(MokoDialerAutolist *moko_dialer_autolist,gint count)
{
gint i;
DBG_ENTER();


if(count>0)
{
	//init the labels.
	for(i=0;i<count&&i<MOKO_DIALER_MAX_TIPS;i++)
	{
	moko_dialer_tip_set_label(moko_dialer_autolist->tips[i],moko_dialer_autolist->readycontacts[i].name);
	moko_dialer_tip_set_index(moko_dialer_autolist->tips[i],i);
	gtk_widget_show(moko_dialer_autolist->tips[i]);
	}
	for(;i<MOKO_DIALER_MAX_TIPS;i++)
	{
	moko_dialer_tip_set_index(moko_dialer_autolist->tips[i],-1);
	moko_dialer_tip_set_label(moko_dialer_autolist->tips[i],"");
	gtk_widget_hide(moko_dialer_autolist->tips[i]);
	}
 //    i=strlen(codestring);
 //   settextviewCodesSensentiveString(text_view,readycontacts[0].number+i);

}	
else
	{
			//no alternative, hide all 3 labels.
		for(i=0;i<MOKO_DIALER_MAX_TIPS;i++)
		{
		gtk_widget_hide(moko_dialer_autolist->tips[i]);
		}
	}
	return 1;
}


gint  moko_dialer_autolist_refresh_by_string (MokoDialerAutolist *moko_dialer_autolist,gchar * string)
{
//first, we fill the ready list
  DIALER_CONTACT* contacts=moko_dialer_autolist->head->contacts;

  DIALER_CONTACT_ENTRY* entry;

  int inserted=0;
  
  int len;
  
  if(string)
	  len=strlen(string);
  else
	  len=0;
  //DBG_TRACE();
  
   while(contacts!= NULL&&inserted<MOKO_DIALER_MAX_DISP_NAME_LEN)
   {
	//  DBG_TRACE();
      entry=contacts->entry;
	//  DBG_TRACE();
	 while(entry!=NULL&&inserted<MOKO_DIALER_MAX_DISP_NAME_LEN)
	 {
	// DBG_TRACE();
	 //judge if the entry includes the string
	 if(contact_string_has_sensentive(entry->content,string))
	 {	
		//if the person not inserted, then insert first

	 //DBG_MESSAGE("%dth contacts:%s,%s,%s",inserted,contacts->name,entry->desc,entry->content);

		 if(strlen(contacts->name)<=MOKO_DIALER_MAX_DISP_NAME_LEN)
		strcpy(moko_dialer_autolist->readycontacts[inserted].name,contacts->name);
	if(strlen(contacts->picpath)<=MOKO_DIALER_MAX_PATH_LEN)
		strcpy(moko_dialer_autolist->readycontacts[inserted].picpath,contacts->picpath);
	if(strlen(entry->content)<=MOKO_DIALER_MAX_NUMBER_LEN)
		strcpy(moko_dialer_autolist->readycontacts[inserted].number,entry->content);
	
/*
	strncpy(readycontacts[inserted].name,contacts->name,MAXDISPNAMELEN);
	strncpy(readycontacts[inserted].picpath,contacts->picpath,MAXPATHLEN);
    strncpy(readycontacts[inserted].number,entry->content,MAXDIALNUMBERLEN);
*/
	 DBG_MESSAGE("insert=%d,%s,%s,%s",inserted,moko_dialer_autolist->readycontacts[inserted].name,moko_dialer_autolist->readycontacts[inserted].picpath,moko_dialer_autolist->readycontacts[inserted].number);	
	
	 inserted++;
		 //break;
	 }
     entry=entry->next;
	 }
	 
	 contacts= contacts->next;
	
  }
  //DBG_LEAVE();
  
moko_dialer_autolist_fill_alternative(moko_dialer_autolist,inserted);
/*
if(ret>0&&autofill!=0)
	setselected(text_view,1);
else
	setselected(text_view,0);
*/
  return inserted;
}

gboolean on_tip_press_event (MokoDialerTip       *tip,GdkEventButton  *event,gpointer         user_data)
{

DBG_ENTER();
MokoDialerAutolist *moko_dialer_autolist;
moko_dialer_autolist=(MokoDialerAutolist *)user_data;

DBG_MESSAGE("selected:%d",moko_dialer_tip_get_index(tip));

 return FALSE;
}


GtkWidget*      moko_dialer_autolist_new()
{
DBG_ENTER();

MokoDialerAutolist * dp;

dp=(MokoDialerAutolist * )g_object_new (MOKO_TYPE_DIALER_AUTOLIST, NULL);
return GTK_WIDGET(dp);

}

 

