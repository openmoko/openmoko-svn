/* moko-dialer-includes.h
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
 #ifndef _MOKO_DIALER_INCLUDES_H
#define _MOKO_DIALER_INCLUDES_H


#include "moko-dialer-declares.h"
#include "error.h"
#include "contacts.h"
#include "history.h"
#include "moko-dialer-autolist.h"
#include "moko-dialer-panel.h"
#include "moko-dialer-textview.h"
#include "moko-digit-button.h"



typedef enum _state{
  STATE_NON= 0,
  STATE_CALLING,
  STATE_INCOMING,
  STATE_TALKING,
  STATE_FINISHED,
  STATE_FAILED,
  STATE_TIMEOUT,
  STATE_IGNORED,
  STATE_MISSED
}CONNECTION_STATE;
typedef enum _windowstate{
	WINDOWS_TALKING=0,
	WINDOWS_DTMF
}WINDOW_STATE;	
typedef enum _speakerstate{
	SPEAKER_ON=0,
	SPEAKER_OFF
}SPEAKER_STATE;
typedef enum _incomingstate{
	NO_INCOMING_EVENT=0,
	HAS_INCOMING_EVENT
}INCOMINGSTATE;
typedef enum _clipstate{
	NO_CLIP_EVENT=0,
	HAS_CLIP_EVENT
}CLIPSTATE;

typedef struct _globalstate
{
	DIALER_READY_CONTACT contactinfo;
	CONNECTION_STATE callstate;
	WINDOW_STATE talkingstate;
	SPEAKER_STATE speakerstate;
	HISTORY_TYPE historytype;
	char starttime[9];
	char startdate[11];
	char lastnumber[MOKO_DIALER_MAX_NUMBER_LEN];
}GLOBAL_STATE;

typedef int (*TimeExpireCallback)();

typedef struct _timerdata
{
	gint ptimer;
	gint stopsec; ///<indicates when the ticks reaches stopsec, then this timer has to be stopped.
	gint timeout; ///<indicates wether this timer has timeout to stopsec.
	GtkWidget* label;
	gint ticks; //seconds together
	gint sec;   
	gint min;
	gint hour;
	char timestring[9] ;
	gint updatewidget;
	TimeExpireCallback expirecallback;
}TIMER_DATA;
enum {
	COLUMN_TYPE,
	COLUMN_TYPEICON,
	COLUMN_SEPRATE,
	COLUMN_NAME_NUMBER,
	COLUMN_TIME,
	COLUMN_DURATION,
	COLUMN_ENTRYPOINTER,
	COLUMN_HASNAME,
	N_COLUMN
};

typedef struct _dialer_data
{
 //the global data area begins here

HISTORY_LIST_HEAD g_historylist; ///< the whole list of the talk history

DIALER_CONTACTS_LIST_HEAD       g_contactlist; ///< the whole list of the contacts from the contact book.

MokoDialerTextview *moko_dialer_text_view;

MokoDialerAutolist *moko_dialer_autolist;

GLOBAL_STATE g_state; ///< the global states holder. we count on it a lot.

gint g_ptimeout; ///< the timer hanle

TIMER_DATA g_timer_data;///< the data used by the timers

// GtkListStore  *g_list_store_filter;///<the list store used by the gtktreeview, for displaying the history list dynamically.

// HISTORY_TYPE g_historyfiltertype;///<indicates the current history filter type, the gtktreeview will be filtered on the value.

// GdkPixbuf * g_iconReceived,*g_iconMissed,*g_iconDialed;///<the global pixbuf for the 3 icons displayed in the history window.}DIALER_APP_DATA;
}MOKO_DIALER_APP_DATA;

#endif
