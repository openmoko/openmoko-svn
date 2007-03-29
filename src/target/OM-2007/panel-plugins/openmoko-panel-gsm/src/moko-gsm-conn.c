#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#include <glib/gmain.h>
#include <glib/giochannel.h>

#include "moko-gsm-conn.h"

#undef FALSE
#define FALSE 	0
#undef TRUE
#define TRUE 	1

static struct lgsm_handle *lgsmh = NULL;
static GPollFD GPfd;
static gint gsm_q = -99;
static gint gprs_q = -99;

static gboolean
gsm_watcher_prepare (GSource * source, gint * timeout)
{
  *timeout = -1;

  return FALSE;
}

static gboolean
gsm_watcher_check (GSource * source)
{
 if (GPfd.revents & (G_IO_IN | G_IO_PRI))
  {
    return TRUE;
  }
  else
  {
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
    return FALSE;
  }
  else
  {
    rc = lgsm_handle_packet (lgsmh, buf, rc);
  }
  
  return TRUE;
}

static int 
sigq_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Signal Quality: %u\n", aux->u.signal.sigq.rssi);
	gsm_q = aux->u.signal.sigq.rssi;
	//FIXME: Call panel applet image change function here, instead of use g timeout function to check singal value
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
gsm_connect_init()
{
  lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);	
    
  if (!lgsmh) 
  { 
    printf("Can't connect to gsmd\n");
    return FALSE;
  }
  else 
  {
    lgsm_evt_handler_register(lgsmh, GSMD_EVT_NETREG, &netreg_handler);
    lgsm_evt_handler_register(lgsmh, GSMD_EVT_SIGNAL, &sigq_handler);
    return TRUE;
  }
}

GsmSignalQuality
moko_panel_gsm_signal_quality()
{
  switch (gsm_q)
  {
    case 4 :
      return GSM_SIGNAL_LEVEL_1 ;
    case 3 :
      return GSM_SIGNAL_LEVEL_2 ;
    case 2 :
      return GSM_SIGNAL_LEVEL_3 ;
    case 1 :
      return GSM_SIGNAL_LEVEL_4 ;
    case 0 :
      return GSM_SIGNAL_LEVEL_5 ;
    default :
      return GSM_SIGNAL_ERROR;
  }
}
/*for a rainning day*/
GprsSignalQuality
moko_panel_gprs_signal_quality()
{

if(0)
{  switch (gprs_q)
  {
		  
  }
}
  return GPRS_CLOSE;
}

void
gsm_watcher_install (void)
{
  static GSourceFuncs gsm_watcher_funcs = {
    gsm_watcher_prepare,
    gsm_watcher_check,
    gsm_watcher_dispatch,
    NULL
  };
  /* FIXME: we never unref the watcher. */
  GSource *gsm_watcher = g_source_new (&gsm_watcher_funcs, sizeof (GSource));

  gsm_connect_init();
    
  if (!lgsmh)
    return ;

  GPfd.fd = lgsm_fd (lgsmh);
  GPfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI;
  GPfd.revents = 0;

  g_source_add_poll (gsm_watcher, &GPfd);
  g_source_attach (gsm_watcher, NULL);

  return;
}
