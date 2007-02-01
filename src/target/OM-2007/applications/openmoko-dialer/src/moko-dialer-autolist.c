/* moko-dialer-autolist .c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
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
#include "common.h"
G_DEFINE_TYPE (MokoDialerAutolist, moko_dialer_autolist, GTK_TYPE_HBOX)
     enum
     {
       SELECTED_SIGNAL,
       CONFIRMED_SIGNAL,
       NOMATCH_SIGNAL,
       LAST_SIGNAL
     };

//forward definition

     gboolean on_tip_press_event (MokoDialerTip * tip, GdkEventButton * event,
                                  gpointer user_data);

     static gint moko_dialer_autolist_signals[LAST_SIGNAL] = { 0 };

static void
moko_dialer_autolist_class_init (MokoDialerAutolistClass * class)
{
/*
  GtkVBoxClass* vbox_class;

  vbox_class= (GtkVBoxClass*) class;
*/

  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) class;

  g_print ("moko_dialer_autolist:start signal register\n");

  moko_dialer_autolist_signals[SELECTED_SIGNAL] =
    g_signal_new ("user_selected",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   moko_dialer_autolist_selected), NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                  g_type_from_name ("gpointer"));





  moko_dialer_autolist_signals[CONFIRMED_SIGNAL] =
    g_signal_new ("user_confirmed",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   moko_dialer_autolist_confirmed), NULL,
                  NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                  g_type_from_name ("gpointer"));

//moko_dialer_autolist_nomatch
  moko_dialer_autolist_signals[NOMATCH_SIGNAL] =
    g_signal_new ("autolist_nomatch",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   moko_dialer_autolist_nomatch), NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);



}


static void
moko_dialer_autolist_init (MokoDialerAutolist * moko_dialer_autolist)
{
//DBG_ENTER();
  int i;
  for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    moko_dialer_autolist->tips[i] = 0;
  moko_dialer_autolist->tipscreated = FALSE;
  moko_dialer_autolist->head = 0;
  moko_dialer_autolist->g_alternatecount = 0;
  moko_dialer_autolist->imagePerson = 0;
  gtk_widget_set_size_request (GTK_WIDGET (moko_dialer_autolist), 480, 40);

}

/**
 * @brief please call this function first before the autolist start to work.
*/
gboolean
moko_dialer_autolist_set_data (MokoDialerAutolist * moko_dialer_autolist,
                               DIALER_CONTACTS_LIST_HEAD * head)
{
/*
if(moko_dialer_autolist->head)
	contact_release_contact_list(moko_dialer_autolist->head);
	*/

  moko_dialer_autolist->head = head;

//contact_print_contact_list(moko_dialer_autolist->head);

  return TRUE;
}

gint
moko_dialer_autolist_hide_all_tips (MokoDialerAutolist * moko_dialer_autolist)
{

  if (moko_dialer_autolist->tipscreated)
  {
    moko_dialer_autolist->selected = FALSE;
    //no alternative, hide all 3 labels.
    gint i;
    for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], FALSE);
      gtk_widget_hide (moko_dialer_autolist->tips[i]);
    }
    //hide the imagePerson
    gtk_widget_hide (moko_dialer_autolist->imagePerson);
  }
  return 1;

}

/**
 * @brief initiate the font for widget of textviewCodes 
 *
 *
 *
 * @param text_view GtkWidget*, any widget which can help to lookup for labelcontactN
 * @param count gint, the count of the alternative, max set to MAXDISPNAMENUM.
 * @param selectdefault if selectdefault, then we will automatically emit the message to fill the sensed string,
 *  else, we only refresh our tip list.
 * @retval 
 */

int
moko_dialer_autolist_fill_alternative (MokoDialerAutolist *
                                       moko_dialer_autolist, gint count,
                                       gboolean selectdefault)
{
  gint i;
//DBG_ENTER();
  moko_dialer_autolist->selected = FALSE;

  moko_dialer_autolist->g_alternatecount = count;

  if (count > 0)
  {
    //init the labels.
    for (i = 0; i < count && i < MOKO_DIALER_MAX_TIPS; i++)
    {
      moko_dialer_tip_set_label (moko_dialer_autolist->tips[i],
                                 moko_dialer_autolist->readycontacts[i].
                                 p_contact->name);
      moko_dialer_tip_set_index (moko_dialer_autolist->tips[i], i);
      moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], FALSE);
      gtk_widget_show (moko_dialer_autolist->tips[i]);
    }

    for (; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      moko_dialer_tip_set_index (moko_dialer_autolist->tips[i], -1);
      moko_dialer_tip_set_label (moko_dialer_autolist->tips[i], "");
      gtk_widget_hide (moko_dialer_autolist->tips[i]);
      moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], FALSE);
    }

    if (selectdefault)
    {
      //we set the first one as defaultly selected
      moko_dialer_autolist_set_select (moko_dialer_autolist, 0);
    }
  }
  else
  {
    moko_dialer_autolist_hide_all_tips (moko_dialer_autolist);
    //notify the client that no match has been foudn
//              autolist_nomatch
    g_signal_emit (moko_dialer_autolist,
                   moko_dialer_autolist_signals[NOMATCH_SIGNAL], 0, 0);
  }
  return 1;
}

//if selectdefault, then we will automatically emit the message to fill the sensed string
//else, we only refresh our tip list.
gint
moko_dialer_autolist_refresh_by_string (MokoDialerAutolist *
                                        moko_dialer_autolist, gchar * string,
                                        gboolean selectdefault)
{
//first, we fill the ready list

  DIALER_CONTACT *contacts;     //=moko_dialer_autolist->head->contacts;

  DIALER_CONTACT_ENTRY *entry;

  gint inserted = 0;


  gint len;

  if (string)
    len = strlen (string);
  else
    len = 0;
//  DBG_TRACE();

//insert the tips here to avoid the _show_all show it from the start.
  GtkWidget *imagePerson;
  GtkWidget *tip;

  if (!moko_dialer_autolist->tipscreated)
  {
    gchar filepath[MOKO_DIALER_MAX_PATH_LEN + 1];
    if (file_create_data_path_for_the_file
        (MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH, filepath))
    {
      imagePerson = gtk_image_new_from_file (filepath);
    }
    else
    {
      imagePerson = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_DND);
    }
    gtk_widget_hide (imagePerson);
    gtk_widget_set_size_request (imagePerson, 40, 40);
//  gtk_box_pack_start (GTK_CONTAINER(moko_dialer_autolist), imagePerson, TRUE, TRUE, 0);
//gtk_box_pack_start (GTK_CONTAINER(moko_dialer_autolist), imagePerson, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_CONTAINER (moko_dialer_autolist), imagePerson,
                        FALSE, FALSE, 0);

    moko_dialer_autolist->imagePerson = imagePerson;
    gint i;
    for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      tip = moko_dialer_tip_new ();

      moko_dialer_tip_set_index (tip, i);

      moko_dialer_tip_set_label (tip, "tony guan");

      moko_dialer_tip_set_selected (tip, FALSE);

      gtk_box_pack_start (GTK_CONTAINER (moko_dialer_autolist), tip, TRUE,
                          TRUE, 0);
//       gtk_box_pack_start(GTK_CONTAINER(moko_dialer_autolist), tip, FALSE,FALSE, 0);                  
//       gtk_box_pack_start(GTK_CONTAINER(moko_dialer_autolist), tip, FALSE,TRUE, 0);
//                       gtk_box_pack_start(GTK_CONTAINER(moko_dialer_autolist), tip, TRUE, FALSE,0);

      g_signal_connect ((gpointer) tip, "button_press_event",
                        G_CALLBACK (on_tip_press_event),
                        moko_dialer_autolist);

//      gtk_widget_set_size_request (tip, 20, 20);      

      gtk_widget_hide (tip);

      moko_dialer_autolist->tips[i] = tip;
    }
    moko_dialer_autolist->tipscreated = TRUE;
  }

  contacts = moko_dialer_autolist->head->contacts;

//  DBG_MESSAGE("CONTACTS:%d,list@0x%x,first@0x%x",moko_dialer_autolist->head->length,moko_dialer_autolist->head,moko_dialer_autolist->head->contacts);

//        DBG_TRACE();  
  while (contacts != NULL && inserted < MOKO_DIALER_MAX_TIPS)
  {
    //  DBG_TRACE();
    entry = contacts->entry;
    //  DBG_TRACE();
    while (entry != NULL && inserted < MOKO_DIALER_MAX_TIPS)
    {
//       DBG_TRACE();
      //judge if the entry includes the string
      if (contact_string_has_sensentive (entry->content, string))
      {
        //if the person not inserted, then insert first
        moko_dialer_autolist->readycontacts[inserted].p_contact = contacts;
        moko_dialer_autolist->readycontacts[inserted].p_entry = entry;
        inserted++;
        //break;
      }
      entry = entry->next;
    }

    contacts = contacts->next;

  }

//DBG_MESSAGE("inserted=%d",inserted);
  moko_dialer_autolist_fill_alternative (moko_dialer_autolist, inserted,
                                         selectdefault);

//DBG_LEAVE();
  return inserted;
}

gboolean
on_tip_press_event (MokoDialerTip * tip, GdkEventButton * event,
                    gpointer user_data)
{

//DBG_ENTER();
  MokoDialerAutolist *moko_dialer_autolist;
  moko_dialer_autolist = (MokoDialerAutolist *) user_data;

  gint selected = moko_dialer_tip_get_index (tip);


  if (selected != -1 && selected < MOKO_DIALER_MAX_TIPS
      && moko_dialer_autolist->g_alternatecount)
  {

    return moko_dialer_autolist_set_select (moko_dialer_autolist, selected);

  }
  else
  {
//we notify the client that no match found!
    DBG_WARN ("the selected index is out of range!");
    return FALSE;
  }


}

gboolean
moko_dialer_autolist_has_selected (MokoDialerAutolist * moko_dialer_autolist)
{
  return moko_dialer_autolist->selected;
}

// selected ==-1 means there are no selected tips
gboolean
moko_dialer_autolist_set_select (MokoDialerAutolist * moko_dialer_autolist,
                                 gint selected)
{
  gint i;
  if (selected == -1)
  {

    //set the selected status to be false
    for (i = 0; i < moko_dialer_autolist->g_alternatecount; i++)
    {
      moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], FALSE);
    }
    //set
    gtk_widget_hide (moko_dialer_autolist->imagePerson);
    moko_dialer_autolist->selected = FALSE;
    return TRUE;
  }


  if (selected < MOKO_DIALER_MAX_TIPS
      && moko_dialer_autolist->g_alternatecount)
  {
    //first of all, determin if this tip is already selected previously.
    if (moko_dialer_tip_is_selected (moko_dialer_autolist->tips[selected]))
    {

      //hide the others;
      for (i = 0; i < moko_dialer_autolist->g_alternatecount; i++)
      {
        if (i != selected)
        {                       //hide the others
          gtk_widget_hide (moko_dialer_autolist->tips[i]);
        }
      }
      moko_dialer_autolist->selected = FALSE;
      //emit confirm message;
//              DBG_MESSAGE("we confirm %s is right.",moko_dialer_autolist->readycontacts[selected].p_contact->name);                   
      g_signal_emit (moko_dialer_autolist,
                     moko_dialer_autolist_signals[CONFIRMED_SIGNAL], 0,
                     &(moko_dialer_autolist->readycontacts[selected]));

    }
    else
    {

      //refresh the imagePerson widget
//      file_load_person_image_from_relative_path(moko_dialer_autolist->imagePerson,moko_dialer_autolist->readycontacts[selected].p_contact->picpath);
      file_load_person_image_scalable_from_relative_path
        (moko_dialer_autolist->imagePerson,
         moko_dialer_autolist->readycontacts[selected].p_contact->picpath);
      gtk_widget_show (moko_dialer_autolist->imagePerson);
      //just change the selected attribute of the tips
      for (i = 0; i < moko_dialer_autolist->g_alternatecount; i++)
      {
        if (i != selected)
        {                       //set selected to false;
          moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], FALSE);
        }
        else
          moko_dialer_tip_set_selected (moko_dialer_autolist->tips[i], TRUE);
      }
      moko_dialer_autolist->selected = TRUE;
      //emit selected message
//              DBG_MESSAGE(" %s is selectd.",moko_dialer_autolist->readycontacts[selected].p_contact->name);
      g_signal_emit (moko_dialer_autolist,
                     moko_dialer_autolist_signals[SELECTED_SIGNAL], 0,
                     &(moko_dialer_autolist->readycontacts[selected]));
    }
    return TRUE;
  }
  else
  {
    DBG_WARN ("the selected index is out of range!");
    return FALSE;
  }

}

GtkWidget *
moko_dialer_autolist_new ()
{
  DBG_ENTER ();

  MokoDialerAutolist *dp;

  dp = (MokoDialerAutolist *) g_object_new (MOKO_TYPE_DIALER_AUTOLIST, NULL);
  return GTK_WIDGET (dp);

}
