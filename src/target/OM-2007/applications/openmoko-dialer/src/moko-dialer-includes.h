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
#include "moko-dialer-status.h"
#include <libmokoui/moko-dialog-window.h>
#include <time.h>
#include <gtk/gtkliststore.h>
//


typedef enum _state{
  STATE_NON= 0,
  STATE_CALLING,
  STATE_INCOMING,
  STATE_TALKING,
  STATE_FINISHED,
  STATE_FAILED,
  STATE_REJECTED,
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
	CONNECTION_STATE callstate;
	WINDOW_STATE talkingstate;
	SPEAKER_STATE speakerstate;
	HISTORY_TYPE historytype;
	char starttime[24];
	char startdate[24];
	char lastnumber[MOKO_DIALER_MAX_NUMBER_LEN+1];
}GLOBAL_STATE;

typedef int (*TimeExpireCallback)();

typedef struct _timerdata
{
	gint ptimer;
	gint stopsec; ///<indicates when the ticks reaches stopsec, then this timer has to be stopped.
	gint timeout; ///<indicates wether this timer has timeout to stopsec.

	gint ticks; //seconds together
	gint sec;   
	gint min;
	gint hour;
	char timestring[MOKO_DIALER_MAX_TIME_STATUS_LEN+1] ;
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
void gsm_incoming_call(gchar * number);

#endif
