#include <stdio.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
//#include "../include/dialer.h"
static int IncomingSignaled; ///<to keep communication with GUI
static int ClipSignaled;///<to keep communication with GUI
static int KeepCalling;


int event_get_incoming_signaled()
{
	return IncomingSignaled;
}

int event_set_incoming_signaled()
{
	IncomingSignaled=1;
	return IncomingSignaled;
}

int event_reset_incoming_signaled()
{
	IncomingSignaled=0;
	return 1;
}
int event_get_clip_signaled()
{
	return ClipSignaled;
	
}
int event_set_clip_signaled()
{
	ClipSignaled=1;
	return 1;
	
}
int event_reset_clip_signaled()
{
	ClipSignaled=0;
	return 1;
	
}
int event_get_keep_calling()
{
	return KeepCalling;
}
int event_set_keep_calling()
{
	KeepCalling=1;
	return 1;
}
int event_reset_keep_calling()
{
	KeepCalling=0;
	return 1;
}

static int incall_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming call type = %u\n", aux->u.call.type);
	
	if(event_get_incoming_signaled())
	{
		printf("already signaled, just set keep_calling");
		event_set_keep_calling();
	}
	else
	{
		printf("set incoming signaled");
		event_set_incoming_signaled();
		//PhoneIncoming(0);	
	}
	return 0;
}


static int clip_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming call clip = %s\n", aux->u.clip.addr.number);
	if(event_get_clip_signaled())
	{
		printf("already signaled, just set keep_calling");
		event_set_keep_calling();
	}
	else
	{
		event_set_clip_signaled();
		printf("set clip signaled and call phoneincoming");
		gdk_threads_enter();
		//here!
		gsm_incoming_call(aux->u.clip.addr.number);
//		PhoneIncomingClip(aux->u.clip.addr.number);
		gdk_threads_leave();
	}
	return 0;
}

static int netreg_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Netreg ");

	switch (aux->u.netreg.state) {
	case 0:
		printf("not searching for network ");
		break;
	case 1:
		printf("registered (home network) ");
		break;
	case 2:
		printf("searching for network ");
		break;
	case 3:
		printf("registration denied ");
		break;
	case 5:
		printf("registered (roaming) ");
		break;
	}

	if (aux->u.netreg.lac)
		printf("LocationAreaCode = 0x%04X ", aux->u.netreg.lac);
	if (aux->u.netreg.ci)
		printf("CellID = 0x%04X ", aux->u.netreg.ci);
	
	printf("\n");

	return 0;
}

static int sigq_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Signal Quality: %u\n", aux->u.signal.sigq.rssi);
	return 0;
}
static int out_status_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: OUT GOING CALL status\n");
	return 0;
}


int event_init(struct lgsm_handle *lh)
{
	int rc;

	rc  = lgsm_evt_handler_register(lh, GSMD_EVT_IN_CALL, &incall_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_IN_CLIP, &clip_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_NETREG, &netreg_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_SIGNAL, &sigq_handler);
	//rc|=lgsm_evt_handler_register(lh, GSMD_EVT_OUT_STATUS, &out_status_handler);
	return rc;
}
