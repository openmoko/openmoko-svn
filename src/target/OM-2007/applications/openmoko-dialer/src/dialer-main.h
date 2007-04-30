/*  openmoko-dialer.h
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
#include "moko-dialer-includes.h"
#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokojournal/moko-journal.h>

typedef struct _dialer_data
{
  //the global data area begins here

  MokoGsmdConnection *connection;
  MokoJournal *journal;

  GtkListStore *history;

  char str_sim_pin[MOKO_DIALER_MAX_PIN_LEN + 1];
  gint int_sim_pin_end_point;
  MokoDialerTextview *moko_dialer_text_view;    ///<the textview for the dialer window 

  MokoDialerTextview *moko_dtmf_text_view;      ///<the textview for the dtmf window
  MokoDialerTextview *moko_pin_text_view;

  MokoDialerAutolist *moko_dialer_autolist;

  DIALER_CONTACTS_LIST_HEAD g_contactlist;      ///< the whole list of the contacts from the contact book.

  DIALER_CONTACT_PEER_INFO g_peer_info; ///<hold the peer's name, number, etc.

  GLOBAL_STATE g_state;         ///< the global states holder. we count on it a lot.


  TIMER_DATA g_timer_data;      ///< the data used by the timers

  MokoDialerStatus *status_outgoing;
  MokoDialerStatus *status_talking;
  MokoDialerStatus *status_incoming;

  GtkWidget *window_incoming;
  GtkWidget *window_outgoing;
  GtkWidget *window_talking;
  GtkWidget *window_history;
  GtkWidget *window_dialer;
  GtkWidget *window_pin;
  GtkWidget *window_present;

//buttons
  GtkWidget *buttonSpeaker;
  GtkWidget *buttonCancel;
  GtkWidget *buttonRedial;

  GtkWidget *imageTALK;
  GtkWidget *imageDTMF;

  GtkWidget *buttonTalk_DTMF;
  GtkWidget *content_talk;
  GtkWidget *content_dtmf;


  GtkWidget *wheel_talking;
  GtkWidget *toolbox_talking;


  GtkWidget *wheel_history;
  GtkWidget *toolbox_history;
  GtkWidget *label_filter_history;
  GtkWidget *label_counter_history;
  GtkWidget *treeview_history;
  GtkWidget *menu_history;

  gboolean dtmf_in_talking_window;
  gboolean history_need_to_update;

  GtkTreeModel *g_list_store_filter;    ///<the list store used by the gtktreeview, for displaying the history list dynamically.

  GdkPixbuf *g_iconReceived, *g_iconMissed, *g_iconDialed;      ///<the global pixbuf for the 3 icons displayed in the history window.}DIALER_APP_DATA;
} MokoDialerData;
