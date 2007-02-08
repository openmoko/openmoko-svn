/**
 * @file dialergsm.h
 * @brief this file includes the definitions of the functions which is called by the GUI app to perform
 * phone operations such as hangup, callout, answer. and the functions then calls the APIs of libgsmd, so 
 * this file is a bridge from GUI to libgsmd.
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * @author Tony Guan (tonyguan@fic-sh.com.cn)
 * @date 2006-10-12
 */

#ifndef _DIALERGSM_H
#define _DIALERGSM_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/voicecall.h>
#include <libgsmd/misc.h>
//#include "pin.h"
#include "event.h"
#include <glib/gmain.h>
#include <glib/giochannel.h>
/*
int event_init(struct lgsm_handle *lh);

int shell_main(struct lgsm_handle *lgsmh);
	
int pin_init(struct lgsm_handle *lh, const char *pin_preset);
*/

//#include "lgsm_internals.h"

//#include "dialer.h"

#define STDIN_BUF_SIZE	1024
/**
 * @brief create the thread to monitor events from libgsmd
 *
 * 
 *
 * @param 
 * @return 
 * @retval -1 failed
 * @retval 0  success
 */
  int gsm_start_loop ();
/**
 * @brief take care the early initialization of libgsmd,this must be called before any physical phone operations such as dialing out.
 * @retval 1 failed, and the whole app will exit too.
 * @retval 0  success
 */
  int gsm_lgsm_start (GMainLoop * mainloop);
/**
 * @brief hangup an outgoing call or incoming call or talking call
 * @retval 0  success 
 * @retval other failed
 */
  int gsm_hangup ();

/**
 * @brief accept an incoming call
 * @retval 0  success 
 * @retval other failed
 */
  int gsm_answer ();
/**
 * @brief calls a number out
 * @param number the number to be called
 * @retval 0  success 
 * @retval other failed
 */
  int gsm_dial (const char *number);
/**
 * @brief monitor the connection with libgsmd, dispatch the event handler when needed.
 *
 * This function should be discarded once libgsmd evolves completely
 *
 * @param lgsmh struct lgsm_handle *,the handle of the libgsmd
 * @return 
 * @retval
 */

 void gsm_pin_require();

  void *gsm_monitor_thread (struct lgsm_handle *lgsmh);


  void gsm_watcher_install (GMainLoop * mainloop);

#ifdef __cplusplus
}
#endif

#endif                          /* _DIALERGSM_H */
