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

#include "moko-dialer-tip.h"
#include "error.h"
#include "common.h"

#include <string.h>

G_DEFINE_TYPE (MokoDialerAutolist, moko_dialer_autolist, GTK_TYPE_HBOX)

#define MOKO_DIALER_AUTOLIST_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	MOKO_TYPE_DIALER_AUTOLIST, MokoDialerAutolistPrivate))

struct _MokoDialerAutolistPrivate 
{
  GtkHBox hbox;
  
  /* An sorted list of AutolistEntries */
  GList *numbers;
  
  /* The list of entries for the previous search */
  GList *last;
  
  /* Previous search string */
  gchar *last_string;
  
  /* The current list to search */
  GThread *thread;
  GList *search_list;
  gboolean selectdefault;
  
  /* old method of finding data */
  DIALER_CONTACTS_LIST_HEAD *head;
  DIALER_READY_CONTACT readycontacts[MOKO_DIALER_MAX_TIPS];
  
  gint g_alternatecount_last_time; 

  gchar g_last_string[MOKO_DIALER_MAX_NUMBER_LEN]; 

  gboolean selected; 

  gint g_alternatecount;

  gboolean tipscreated;
  
  GtkWidget *tips[MOKO_DIALER_MAX_TIPS];
  GtkWidget *imagePerson;
  
};

enum
{
  SELECTED_SIGNAL,
  CONFIRMED_SIGNAL,
  NOMATCH_SIGNAL,
  LAST_SIGNAL
};

static gint moko_dialer_autolist_signals[LAST_SIGNAL] = { 0 };

static GtkHBoxClass *parent_class = NULL;

/* Forward declerations */
gboolean        on_tip_press_event (MokoDialerTip * tip, GdkEventButton * event,
                                    MokoDialerAutolist *moko_dialer_autolist);

static void
moko_dialer_autolist_finalize (GObject *obj)
{
  MokoDialerAutolistPrivate *priv;
  GList *n;
  AutolistEntry *entry;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (obj);
  
  /* Go through priv->numbers and free the entry strings */
  for (n = priv->numbers; n != NULL; n = n->next)
  {
    entry = (AutolistEntry*)n->data;
    g_free (entry->number);
    g_free (entry);
    n->data = NULL;
  }
  g_list_free (priv->numbers);
  g_list_free (priv->last);
  
  if (G_OBJECT_CLASS(parent_class)->finalize)
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
moko_dialer_autolist_class_init (MokoDialerAutolistClass * klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  parent_class = GTK_HBOX_CLASS (klass);

  object_class->finalize = moko_dialer_autolist_finalize;
  
  g_type_class_add_private (object_class, sizeof (MokoDialerAutolistPrivate));
  
  /* Setup signals */
  moko_dialer_autolist_signals[SELECTED_SIGNAL] =
    g_signal_new ("user_selected",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   _autolist_selected), NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                  g_type_from_name ("gpointer"));
 
  moko_dialer_autolist_signals[CONFIRMED_SIGNAL] =
    g_signal_new ("user_confirmed",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   _autolist_confirmed), NULL,
                  NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                  g_type_from_name ("gpointer"));

  moko_dialer_autolist_signals[NOMATCH_SIGNAL] =
    g_signal_new ("autolist_nomatch",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (MokoDialerAutolistClass,
                                   _autolist_nomatch), NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
moko_dialer_autolist_init (MokoDialerAutolist * moko_dialer_autolist)
{
  MokoDialerAutolistPrivate *priv;
  int i;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  /* Set all the tips pointers to NULL */
  for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    priv->tips[i] = NULL;
    
  priv->tipscreated = FALSE;
  priv->head = 0;
  priv->g_alternatecount = 0;
  priv->imagePerson = 0;
  priv->g_alternatecount_last_time = 0;
  priv->g_last_string[0] = 0;  
  
  for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    priv->tips[i] = NULL;
  
  priv->tipscreated = FALSE;
  priv->head = 0;
  priv->g_alternatecount = 0;
  priv->imagePerson = 0;
  priv->g_alternatecount_last_time = 0;
  priv->g_last_string[0] = 0;

  gtk_widget_set_size_request (GTK_WIDGET (moko_dialer_autolist), 480, 40);
}

/*
 * Takes a standard number as entered by the user, and removes all the 'fancy'
 * chars, such as spaces and dashs. This way, we can match "01234567890" even
 * when "01234 567 890" was in the 'content' field
 */
static gchar*
_normalize (const gchar *string)
{
  gint len = strlen (string);
  gchar buf[len];
  gint i;
  gint j = 0;
  
  for (i = 0; i < len; i++)
  {
    char c = string[i];
    if (c != ' ' && c != '-')
    {
      buf[j] = c;
      j++;
    }
  }
  return g_strdup (buf);
}

/*
 * Takes two AutolistEntrys, and strcmp's the numbers
 */
static gint
_entry_compare (AutolistEntry *e1, AutolistEntry *e2)
{
  return strcmp (e1->number, e2->number);
}

/**
 * @brief please call this function first before the autolist start to work.
*/
gboolean
moko_dialer_autolist_set_data (MokoDialerAutolist * moko_dialer_autolist,
                               DIALER_CONTACTS_LIST_HEAD * head)
{
  MokoDialerAutolistPrivate *priv;
  DIALER_CONTACT *contact = head->contacts;
  DIALER_CONTACT_ENTRY *entry;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  /* We go through each of the contacts and their numbers, creating an 
   * optimised list of entries which are ordered, and therefore can be searched
   * through quickly
   */ 
  while (contact != NULL)
  {
    entry = contact->entry;
    while (entry != NULL)
    {
      AutolistEntry *aentry = g_new0 (AutolistEntry, 1);
      aentry->contact = contact;
      aentry->entry = entry;
      aentry->number = _normalize (entry->content);
      
      priv->numbers = g_list_insert_sorted (priv->numbers, 
                                            (gpointer)aentry,
                                            (GCompareFunc)_entry_compare);
      entry = entry->next;
    }
    contact = contact->next;
  }
  
  priv->head = head;
  return TRUE;
}

gint
moko_dialer_autolist_hide_all_tips (MokoDialerAutolist *moko_dialer_autolist)
{
  MokoDialerAutolistPrivate *priv;
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  if (priv->tipscreated)
  {
    priv->selected = FALSE;
    //no alternative, hide all 3 labels.
    gint i;
    for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
      gtk_widget_hide (priv->tips[i]);
    }
    //hide the imagePerson
    gtk_widget_hide (priv->imagePerson);
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
#if 0
int
moko_dialer_autolist_fill_alternative (MokoDialerAutolist *moko_dialer_autolist,
                                       gint count,
                                       gboolean selectdefault)
{
  MokoDialerAutolistPrivate *priv;
  gint i;
  AutolistEntry *entry = NULL;
  
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  priv->selected = FALSE;
  if (count > 0)
  {
    //init the labels.
    for (i = 0; i < count && i < MOKO_DIALER_MAX_TIPS; i++)
    {
      entry = (AutolistEntry*)g_list_nth_data (priv->last, i);
      gdk_threads_enter ();
      moko_dialer_tip_set_label (priv->tips[i], entry->contact->name);
      moko_dialer_tip_set_index (priv->tips[i], i);
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
      gtk_widget_show (priv->tips[i]);
      gdk_threads_leave ();
    }
    /* Invalidate the remaining tips */
    for (; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      gdk_threads_enter ();
      moko_dialer_tip_set_index (priv->tips[i], -1);
      moko_dialer_tip_set_label (priv->tips[i], "");
      gtk_widget_hide (priv->tips[i]);
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
      gdk_threads_leave ();
    }
    if (selectdefault)
    {
      //we set the first one as defaultly selected
      gdk_threads_enter ();
      moko_dialer_autolist_set_select (moko_dialer_autolist, 0);
      gdk_threads_leave ();
    }
  }
  else
  {
    gdk_threads_enter ();    
    moko_dialer_autolist_hide_all_tips (moko_dialer_autolist);
    
    //notify the client that no match has been found
    g_signal_emit (moko_dialer_autolist,
                   moko_dialer_autolist_signals[NOMATCH_SIGNAL], 0, 0);
    gdk_threads_leave ();    
  }
  return 1;
}
#endif
static void
moko_dialer_autolist_create_tips (MokoDialerAutolist *moko_dialer_autolist)
{
  MokoDialerAutolistPrivate *priv;
  int i;
  GtkWidget *imagePerson;
  GtkWidget *tip;

  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  gchar *filepath;
  if ((filepath =
       file_create_data_path_for_the_file
       (MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH)))
  {
    imagePerson = gtk_image_new_from_file (filepath);
  }
  else
  {
    imagePerson = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_DND);
  }
  gtk_widget_hide (imagePerson);
  gtk_widget_set_size_request (imagePerson, 40, 40);

  gtk_box_pack_start (GTK_BOX (moko_dialer_autolist), imagePerson,
                      FALSE, FALSE, 0);

  priv->imagePerson = imagePerson;
  
  for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
  {
    tip = moko_dialer_tip_new ();
    moko_dialer_tip_set_index (tip, i);
    moko_dialer_tip_set_label (tip, "tony guan");
    moko_dialer_tip_set_selected (tip, FALSE);
    gtk_box_pack_start (GTK_BOX (moko_dialer_autolist), tip, TRUE, TRUE, 0);
    g_signal_connect ((gpointer) tip, "button_press_event",
                      G_CALLBACK (on_tip_press_event),
                      moko_dialer_autolist);
    gtk_widget_hide (tip);

    priv->tips[i] = tip;
  }
  priv->tipscreated = TRUE;
}

/* 
 * Go through the list looking for matches, we know the list is optimized, so
 * we can make some assuptions and our search is therefore faster
 */
static GList*
_autolist_find_number_in_entry_list (GList *numbers, const char *string)
{
  GList *matches = NULL;
  GList *n;
  AutolistEntry *entry = NULL;
  gint i = 0;
  gint len = strlen (string);
  gboolean found_one = FALSE;
  gboolean prev_matched = FALSE;
  
  for (n = numbers; n != NULL; n = n->next)
  {
    entry = (AutolistEntry*)n->data;
    if (g_strstr_len (entry->number, len, string))
    {
      matches = g_list_append (matches, (gpointer)entry);
      found_one = prev_matched = TRUE;
      i++;
    }
    else
    {
      /* 
       * If we have already found at least one, but the previous entry didn't
       * match, we don't bother continuing.
       */
      if (found_one && !prev_matched)
        break;
      prev_matched = FALSE;
    }
  }
  
  return matches;
}

gpointer
moko_dialer_autolist_worker (MokoDialerAutolist *autolist)
{
  MokoDialerAutolistPrivate *priv;
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (autolist);
  GList *numbers = priv->search_list;
  const gchar *string = priv->last_string;
  GList *matches = NULL;
  gint count = 0;
  GList *n;
  AutolistEntry *entry = NULL;
  gint i = 0;
  gint len = strlen (string);
  gboolean found_one = FALSE;
  gboolean prev_matched = FALSE;
  gboolean selectdefault = priv->selectdefault;
  
  for (n = numbers; n != NULL; n = n->next)
  {
    entry = (AutolistEntry*)n->data;
    if (g_strstr_len (entry->number, len, string))
    {
      matches = g_list_append (matches, (gpointer)entry);
      found_one = prev_matched = TRUE;
      i++;
    }
    else
    {
      /* 
       * If we have already found at least one, but the previous entry didn't
       * match, we don't bother continuing.
       */
      if (found_one && !prev_matched)
        break;
      prev_matched = FALSE;
    }
  }
  priv->last = matches;
  
  count = g_list_length (matches);
  priv->selected = FALSE;
  if (count > 0)
  {
    //init the labels.
    for (i = 0; i < count && i < MOKO_DIALER_MAX_TIPS; i++)
    {
      entry = (AutolistEntry*)g_list_nth_data (priv->last, i);
      gdk_threads_enter ();
      moko_dialer_tip_set_label (priv->tips[i], entry->contact->name);
      moko_dialer_tip_set_index (priv->tips[i], i);
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
      gtk_widget_show (priv->tips[i]);
      gdk_threads_leave ();
    }
    /* Invalidate the remaining tips */
    for (; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      gdk_threads_enter ();
      moko_dialer_tip_set_index (priv->tips[i], -1);
      moko_dialer_tip_set_label (priv->tips[i], "");
      gtk_widget_hide (priv->tips[i]);
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
      gdk_threads_leave ();
    }
    if (selectdefault)
    {
      //we set the first one as defaultly selected
      gdk_threads_enter ();
      moko_dialer_autolist_set_select (autolist, 0);
      gdk_threads_leave ();
    }
  }
  else
  {
    gdk_threads_enter ();    
    moko_dialer_autolist_hide_all_tips (autolist);
    
    //notify the client that no match has been found
    g_signal_emit (autolist,
                   moko_dialer_autolist_signals[NOMATCH_SIGNAL], 0, 0);
    gdk_threads_leave ();    
  }
  
  return 0;
}

gint
moko_dialer_autolist_refresh_by_string (MokoDialerAutolist *autolist, 
                                        gchar * string,
                                        gboolean selectdefault)
{
  MokoDialerAutolistPrivate *priv;
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (autolist);
  if (priv->thread)
    g_thread_join (priv->thread);
  priv->thread = NULL;
  GList *matches = NULL;
  gint n_matches = 0;
  
  
  
  if (!priv->tipscreated)
    moko_dialer_autolist_create_tips (autolist);

  /* First we check if the new string is just the last one with on more number*/
  if (priv->last && priv->last_string) {
    
    if (strlen (string) < strlen (priv->last_string))
    {
      /* We have 'deleted' so we can't use our previous results */
      ;
    }
    else if (strstr (string, priv->last_string))
    {
      matches = priv->last;
    }
  }
  
  if (matches == NULL)
  {
    if (priv->last)
      g_list_free (priv->last);
    priv->last = NULL;
    
    /* We need to look though the whole list */
    matches = priv->numbers;
  }
  

  /* We reset the last & last_string variables */
  priv->search_list = matches;
  if (priv->last_string)
    g_free (priv->last_string);
  priv->last_string = g_strdup (string);
  priv->selectdefault = selectdefault;
  priv->thread = g_thread_create ((GThreadFunc)moko_dialer_autolist_worker,
                                  (gpointer)autolist,
                                  TRUE,
                                  NULL);
  return n_matches;
}


gboolean
on_tip_press_event (MokoDialerTip * tip, GdkEventButton * event,
                    MokoDialerAutolist *moko_dialer_autolist)
{

  MokoDialerAutolistPrivate *priv;
  gint selected;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  selected = moko_dialer_tip_get_index (tip);
  
  if (selected != -1 && selected < MOKO_DIALER_MAX_TIPS
      && g_list_length (priv->last))
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
  MokoDialerAutolistPrivate *priv;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);
  
  return priv->selected;
}

// selected ==-1 means there are no selected tips
gboolean
moko_dialer_autolist_set_select (MokoDialerAutolist * moko_dialer_autolist,
                                 gint selected)
{
  MokoDialerAutolistPrivate *priv;
  AutolistEntry *entry = NULL;
  gint len;
  
  priv = MOKO_DIALER_AUTOLIST_GET_PRIVATE (moko_dialer_autolist);

  len = g_list_length (priv->last);
  gint i;
  if (selected == -1)
  {
    //set the selected status to be false
    for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
    {
      moko_dialer_tip_set_selected (priv->tips[i], FALSE);
    }
    gtk_widget_hide (priv->imagePerson);
    priv->selected = FALSE;
    return TRUE;
  }
  
  if (selected < len)
  {
    entry = (AutolistEntry*)g_list_nth_data (priv->last, selected);
    //first of all, determine if this tip is already selected previously.
    if (moko_dialer_tip_is_selected (priv->tips[selected]))
    {
      //hide the others;
      for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
      {
        if (i != selected)
        {                       //hide the others
          gtk_widget_hide (priv->tips[i]);
        }
      }
      priv->selected = FALSE;
      
      
      g_signal_emit (moko_dialer_autolist,
                     moko_dialer_autolist_signals[CONFIRMED_SIGNAL], 0, entry);
      
    }
    else
    {
      contact_load_contact_photo (GTK_IMAGE (priv->imagePerson), 
                                  entry->contact->ID);
      gtk_widget_show (priv->imagePerson);
      
      //just change the selected attribute of the tips
      for (i = 0; i < MOKO_DIALER_MAX_TIPS; i++)
      {
        if (i != selected)
        {                       //set unselected to false;
          moko_dialer_tip_set_selected (priv->tips[i], FALSE);
        }
        else
          moko_dialer_tip_set_selected (priv->tips[i], TRUE);
      }
      priv->selected = TRUE;
      
      g_signal_emit (moko_dialer_autolist,
                     moko_dialer_autolist_signals[SELECTED_SIGNAL], 0, entry);
      
    }
    return TRUE;
  }
  else
  {
    DBG_WARN ("the selected index is out of range!");
    return FALSE;
  }

  return FALSE;
}

GtkWidget *
moko_dialer_autolist_new ()
{
  DBG_ENTER ();

  MokoDialerAutolist *dp;

  dp = (MokoDialerAutolist *) g_object_new (MOKO_TYPE_DIALER_AUTOLIST, NULL);
  return GTK_WIDGET (dp);

}
