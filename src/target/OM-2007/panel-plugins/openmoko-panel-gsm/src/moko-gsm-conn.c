#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#include "moko-gsm-conn.h"

#undef FALSE
#define FALSE 	0
#undef TRUE
#define TRUE 	1

static struct lgsm_handle *lgsmh = NULL;
static int signal_value = 0;
static int updated = FALSE;
static int gsm_conn_init = FALSE;

static int 
incall_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming call type = %u\n", aux->u.call.type);
	return 0;
}

static int 
sigq_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Signal Quality: %u\n", aux->u.signal.sigq.rssi);
	//signal_value = aux->u.signal.sigq.rssi;
	return 0;
}

static int 
netreg_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Netreg ");

	switch (aux->u.netreg.state)
	{
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

static int 
event_init(struct lgsm_handle *lh)
{
	int rc;

	rc  = lgsm_evt_handler_register(lh, GSMD_EVT_IN_CALL, &incall_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_NETREG, &netreg_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_SIGNAL, &sigq_handler);

	return rc;
}

static void
gsm_connect_init()
{
    lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);	
    
    if (!lgsmh) 
    { 
       gsm_conn_init = FALSE;
	//fprintf(stderr, "openmoko-panel-gsm:Can't connect to gsmd\n");
	printf("Can't connect to gsmd\n");
	return FALSE;
    }
    else 
    {
	event_init(lgsmh);
	return TRUE;
    }
}

int
update_gsm_signal_qualite()
{
    printf("update_gsm_signal_qualite\n");
    fd_set readset;
    int rc;
    char buf[STDIN_BUF_SIZE+1];
    struct timeval t;
    t.tv_sec=0;
    t.tv_usec=0;

    if (!gsm_conn_init)
    	gsm_connect_init();
    
    if (!lgsmh){
    	gsm_conn_init = FALSE;
    	return FALSE;
    	}
    
    int gsm_fd = lgsm_fd (lgsmh);

    FD_SET(gsm_fd, &readset);
    printf("select>\n");
    rc = select(gsm_fd+1, &readset, NULL, NULL, &t);
    printf("select<\n");
    
    if (FD_ISSET(gsm_fd, &readset)) 
    {
	printf("read>\n");
	rc = read(gsm_fd, buf, sizeof(buf));
	printf("read<\n");
	if (rc <= 0) 
	{
	    printf("ERROR reding from gsm_fd\n");
	    return FALSE;
	}
	else
	{
	    printf("data from gsm_fd\n");
	    rc = lgsm_handle_packet (lgsmh, buf, rc);
	    updated = TRUE;
	    return TRUE;
	}
    }
}

int
moko_panel_gsm_quality(int *quality)
{
    update_gsm_signal_qualite();



    if (updated)
	{
		/*switch (signal_value) //needs debug board to test signal value range.
		{
		}
		*/
	

    	updated = FALSE;
    	return TRUE;
    }
    else
	{
		printf ("This is a test resualt without libgsmd support\n");
    	static int test = 0;

    	*quality = test;

		if ( ++test >= TOTAL_SIGNALS )
			test = 0;

    	return FALSE;
	}
}
