/**
 * @file dialergsm.c
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

#include "dialergsm.h"
#include "moko-dialer-includes.h"

#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

//pthread_t thread;///<the gsm_monitor_thread thread handler

static struct lgsm_handle *lgsmh;       ///< the handle of the libgsmd

static GPollFD GPfd;


/**
 * @brief monitor the connection with libgsmd, dispatch the event handler when needed.
 *
 * This function should be discarded once libgsmd evolves completely
 *
 * @param lgsmh struct lgsm_handle *,the handle of the libgsmd
 * @return 
 * @retval
 */
 /*

    void *gsm_monitor_thread(struct lgsm_handle *lgsmh)
    {
    int rc;
    char buf[STDIN_BUF_SIZE+1];
    //char rbuf[STDIN_BUF_SIZE+1];
    //int rlen = sizeof(rbuf);
    fd_set readset;
    //  lgsm_register_handler(lgsmh, GSMD_MSG_PASSTHROUGH, &pt_msghandler);

    FD_ZERO(&readset);

    while (1) {
    int gsm_fd = lgsm_fd(lgsmh);
    //          FD_SET(0, &readset);
    FD_SET(gsm_fd, &readset);

    rc = select(gsm_fd+1, &readset, NULL, NULL, NULL);
    if (rc <= 0)        
    break;
    / we've received something on the gsmd socket, pass it on to the library 
    rc = read(gsm_fd, buf, sizeof(buf));
    if (rc <= 0) {
    printf("ERROR reding from gsm_fd\n");
    break;
    }
    rc = lgsm_handle_packet(lgsmh, buf, rc);
    }


    printf("you know what? i quit!");
    pthread_exit(NULL);
    }

  */
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
 /*
    int gsm_start_loop()
    {

    //pthread_mutex_init(&mut,NULL);
    memset(&thread, 0, sizeof(thread));          //comment1

    if(pthread_create(&thread, NULL, gsm_monitor_thread, lgsmh) != 0)       //comment2
    {  printf("failed to create libgsmd monitor thread\n");
    return -1;
    }

    return 0;
    }

  */

/**
 * @brief this is the handler for receiving passthrough responses 
 *
 * 
 *
 * @param lh struct lgsm_handle *, the libgsm handle
 * @param gmh struct gsmd_msg_hdr *, the data to passthrough
 * @return 
 * @retval -1 failed
 * @retval 0  success
 */

static int
pt_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  char *payload = (char *) gmh + sizeof (*gmh);
  printf ("RSTR=`%s'\n", payload);
  return 0;
}



/**
 * @brief take care the early initialization of libgsmd,this must be called before any physical phone operations such as dialing out.
 * @retval 1 failed, and the whole app will exit too.
 * @retval 0  success
 */

int
gsm_lgsm_start (GMainLoop * mainloop)
{

  char *pin = NULL;
  lgsmh = lgsm_init (LGSMD_DEVICE_GSMD);
  if (!lgsmh)
  {
    fprintf (stderr, "Can't connect to gsmd\n");
    return -1;
  }
  gsm_pin_init (lgsmh);
  event_init (lgsmh);
  lgsm_register_handler (lgsmh, GSMD_MSG_PASSTHROUGH, &pt_msghandler);
  lgsm_netreg_register (lgsmh, 0);

  gsm_watcher_install (mainloop);
  //gsm_start_loop();
  return 0;

}



/**
 * @brief calls a number out
 * @param number the number to be called
 * @retval 0  success 
 * @retval other failed
 */
int
gsm_dial (const char *number)
{
  struct lgsm_addr addr;
  addr.type = 129;
  strncpy (addr.addr, number, strlen (number));
  addr.addr[strlen (number)] = '\0';
  return lgsm_voice_out_init (lgsmh, &addr);

}

/**
 * @brief accept an incoming call
 * @retval 0  success 
 * @retval other failed
 */
int
gsm_answer ()
{
  return lgsm_voice_in_accept (lgsmh);

}

/**
 * @brief hangup an outgoing call or incoming call or talking call
 * @retval 0  success 
 * @retval other failed
 */
int
gsm_hangup ()
{

  return lgsm_voice_hangup (lgsmh);

}


static gboolean
gsm_watcher_prepare (GSource * source, gint * timeout)
{
  //DBG_ENTER();
  *timeout = -1;

  return FALSE;
}

static gboolean
gsm_watcher_check (GSource * source)
{

  //DBG_ENTER();
  //|G_IO_IN|G_IO_HUP|G_IO_ERR|G_IO_PRI;
  if (GPfd.revents & (G_IO_IN | G_IO_PRI))
  {

    //GPfd.revents=0;
    return TRUE;
  }
  else
  {
    //DBG_MESSAGE("FALSE");
    return FALSE;
  }

}
static gboolean
gsm_watcher_dispatch (GSource * source,
                      GSourceFunc callback, gpointer user_data)
{


  int rc;
  char buf[STDIN_BUF_SIZE + 1];
  int gsm_fd = lgsm_fd (lgsmh);
  /* we've received something on the gsmd socket, pass it
   * on to the library */

  rc = read (gsm_fd, buf, sizeof (buf));
  if (rc <= 0)
  {
    DBG_MESSAGE ("ERROR reding from gsm_fd");
    return FALSE;
  }
  else
  {
    rc = lgsm_handle_packet (lgsmh, buf, rc);
  }
  return TRUE;
}

void
gsm_watcher_install (GMainLoop * mainloop)
{

  static GSourceFuncs gsm_watcher_funcs = {
    gsm_watcher_prepare,
    gsm_watcher_check,
    gsm_watcher_dispatch,
    NULL
  };
  /* FIXME: we never unref the watcher. */
  GSource *gsm_watcher = g_source_new (&gsm_watcher_funcs, sizeof (GSource));
  GPfd.fd = lgsm_fd (lgsmh);
  GPfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI;
  GPfd.revents = 0;

  g_source_add_poll (gsm_watcher, &GPfd);

//  DBG_MESSAGE ("ATACH");
  g_source_attach (gsm_watcher, NULL);
//  DBG_MESSAGE ("ATACH OUT");

  return;

}

void
gsm_dtmf_send (char dtmf)
{
  DBG_MESSAGE ("lgsm_voice_dtmf");
  lgsm_voice_dtmf (lgsmh, dtmf);
}
#define PIN_SIZE 32

static int
gsm_pin_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
 
  int rc;

  printf ("EVENT: PIN request (type=%u) ", aux->u.pin.type);

  /* FIXME: read pin from STDIN and send it back via lgsm_pin */

    gsm_pin_require();


  return 0;
}

int
gsm_pin_init (struct lgsm_handle *lh)
{
  return lgsm_evt_handler_register (lh, GSMD_EVT_PIN, &gsm_pin_handler);
}
