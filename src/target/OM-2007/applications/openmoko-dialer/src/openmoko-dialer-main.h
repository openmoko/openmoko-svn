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
/*

typedef struct _window_outgoing
{

MokoDialerStatus * status_outgoing;

GtkWidget* window_outgoing;

GtkWidget* buttonSpeaker;
GtkWidget* buttonCancel;
GtkWidget* buttonRedial;
}WindowOutgoing;
*/
 typedef struct _dialer_data
{
 //the global data area begins here


MokoDialerTextview *moko_dialer_text_view;

MokoDialerAutolist *moko_dialer_autolist;

DIALER_CONTACTS_LIST_HEAD       g_contactlist; ///< the whole list of the contacts from the contact book.

DIALER_CONTACT_PEER_INFO g_peer_info; ///<hold the peer's name, number, etc.

HISTORY_LIST_HEAD g_historylist; ///< the whole list of the talk history

GLOBAL_STATE g_state; ///< the global states holder. we count on it a lot.

//gint g_ptimeout; ///< the timer hanle

TIMER_DATA g_timer_data;///< the data used by the timers

MokoDialerStatus * status_outgoing;
GtkWidget* window_outgoing;

MokoDialerStatus * status_talking;
GtkWidget* window_talking;

GtkWidget * window_dialer;

//buttons
GtkWidget* buttonSpeaker;
GtkWidget* buttonCancel;
GtkWidget* buttonRedial;

//WindowOutgoing window_outgoing_data;
// GtkListStore  *g_list_store_filter;///<the list store used by the gtktreeview, for displaying the history list dynamically.

// HISTORY_TYPE g_historyfiltertype;///<indicates the current history filter type, the gtktreeview will be filtered on the value.

// GdkPixbuf * g_iconReceived,*g_iconMissed,*g_iconDialed;///<the global pixbuf for the 3 icons displayed in the history window.}DIALER_APP_DATA;
}MOKO_DIALER_APP_DATA;

