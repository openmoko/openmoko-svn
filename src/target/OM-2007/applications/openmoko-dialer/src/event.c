#include <stdio.h>
#include <string.h>

#include <gdk/gdk.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#include "moko-dialer-includes.h"

static int IncomingSignaled;    ///<to keep communication with GUI
static int ClipSignaled;        ///<to keep communication with GUI
static int KeepCalling;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

int
event_get_incoming_signaled ()
{
  return IncomingSignaled;
}

int
event_set_incoming_signaled ()
{
  IncomingSignaled = 1;
  return IncomingSignaled;
}

int
event_reset_incoming_signaled ()
{
  IncomingSignaled = 0;
  return 1;
}

int
event_get_clip_signaled ()
{
  return ClipSignaled;

}

int
event_set_clip_signaled ()
{
  ClipSignaled = 1;
  return 1;

}

int
event_reset_clip_signaled ()
{
  ClipSignaled = 0;
  return 1;

}

int
event_get_keep_calling ()
{
  return KeepCalling;
}

int
event_set_keep_calling ()
{
  KeepCalling = 1;
  return 1;
}

int
event_reset_keep_calling ()
{
  KeepCalling = 0;
  return 1;
}

static int
incall_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  printf ("EVENT: Incoming call type = %u\n", aux->u.call.type);

  if (event_get_incoming_signaled ())
  {
    printf ("already signaled, just set keep_calling");
    event_set_keep_calling ();
  }
  else
  {
    printf ("set incoming signaled");
    event_set_incoming_signaled ();
    //PhoneIncoming(0);     
  }
  return 0;
}


static int
clip_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  printf ("EVENT: Incoming call clip = %s\n", aux->u.clip.addr.number);
  if (event_get_clip_signaled ())
  {
    printf ("already signaled, just set keep_calling");
    event_set_keep_calling ();
  }
  else
  {
    event_set_clip_signaled ();
    printf ("set clip signaled and call phoneincoming");
    gdk_threads_enter ();
    //here!
    gsm_incoming_call (aux->u.clip.addr.number);
//              PhoneIncomingClip(aux->u.clip.addr.number);
    gdk_threads_leave ();
  }
  return 0;
}

static int
netreg_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  printf ("EVENT: Netreg ");

  switch (aux->u.netreg.state)
  {
  case GSMD_NETREG_NONE:
    printf ("not searching for network ");
    break;
  case 1:
    printf ("registered (home network) ");
    break;
    /* 
     * FIXME: these are not defined in gsmd/event.h
     *
     case 2:
     printf ("searching for network ");
     break;
     case 3:
     printf ("registration denied ");
     break;
     case 5:
     printf ("registered (roaming) ");
     break;
     *
     *
     */
  }

  if (aux->u.netreg.lac)
    printf ("LocationAreaCode = 0x%04X ", aux->u.netreg.lac);
  if (aux->u.netreg.ci)
    printf ("CellID = 0x%04X ", aux->u.netreg.ci);

  printf ("\n");

  return 0;
}

static int
sigq_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  printf ("EVENT: Signal Quality: %u\n", aux->u.signal.sigq.rssi);
  return 0;
}
static const char *cprog_names[] = {
  [GSMD_CALLPROG_SETUP] = "SETUP",
  [GSMD_CALLPROG_DISCONNECT] = "DISCONNECT",
  [GSMD_CALLPROG_ALERT] = "ALERT",
  [GSMD_CALLPROG_CALL_PROCEED] = "PROCEED",
  [GSMD_CALLPROG_SYNC] = "SYNC",
  [GSMD_CALLPROG_PROGRESS] = "PROGRESS",
  [GSMD_CALLPROG_CONNECTED] = "CONNECTED",
  [GSMD_CALLPROG_RELEASE] = "RELEASE",
  [GSMD_CALLPROG_REJECT] = "REJECT",
  [GSMD_CALLPROG_UNKNOWN] = "UNKNOWN",
};

static const char *cdir_names[] = {
  [GSMD_CALL_DIR_MO] = "Outgoing",
  [GSMD_CALL_DIR_MT] = "Incoming",
  [GSMD_CALL_DIR_CCBS] = "CCBS",
  [GSMD_CALL_DIR_MO_REDIAL] = "Outgoing Redial",
};

static int
cprog_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  const char *name, *dir;

  if (aux->u.call_status.prog >= ARRAY_SIZE (cprog_names))
    name = "UNDEFINED";
  else
    name = cprog_names[aux->u.call_status.prog];

  if (aux->u.call_status.dir >= ARRAY_SIZE (cdir_names))
    dir = "";
  else
    dir = cdir_names[aux->u.call_status.dir];

  printf ("EVENT: %s Call Progress: %s\n", dir, name);

  if (aux->u.call_status.prog == GSMD_CALLPROG_CONNECTED)
  {

    if (aux->u.call_status.dir == GSMD_CALL_DIR_MO)
      gsm_peer_accept ();
  }

  if (aux->u.call_status.prog == GSMD_CALLPROG_REJECT)
  {
    if (aux->u.call_status.dir == GSMD_CALL_DIR_MO)
      gsm_peer_refuse ();
  }

  if (aux->u.call_status.prog == GSMD_CALLPROG_DISCONNECT)
  {
//    if (aux->u.call_status.dir == GSMD_CALL_DIR_MO)
    gsm_peer_disconnect ();

  }


  return 0;
}

static int
colp_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  printf ("EVENT: Outgoing call colp = %s\n", aux->u.colp.addr.number);

  return 0;
}


int
event_init (struct lgsm_handle *lh)
{
  int rc;

  rc = lgsm_evt_handler_register (lh, GSMD_EVT_IN_CALL, &incall_handler);
  rc |= lgsm_evt_handler_register (lh, GSMD_EVT_IN_CLIP, &clip_handler);
  rc |= lgsm_evt_handler_register (lh, GSMD_EVT_OUT_COLP, &colp_handler);
  rc |= lgsm_evt_handler_register (lh, GSMD_EVT_NETREG, &netreg_handler);
  rc |= lgsm_evt_handler_register (lh, GSMD_EVT_SIGNAL, &sigq_handler);
  rc |= lgsm_evt_handler_register (lh, GSMD_EVT_OUT_STATUS, &cprog_handler);

  return rc;
}
