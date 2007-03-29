#ifndef _MOKO_PANEL_GSM_CONN_
#define _MOKO_PANEL_GSM_CONN_

#include <glib-object.h>

#define STDIN_BUF_SIZE	1024
/*signal value: "0": best connection "5": connect error*/
G_BEGIN_DECLS

typedef enum{
  GSM_SIGNAL_LEVEL_1 = 0,
  GSM_SIGNAL_LEVEL_2,
  GSM_SIGNAL_LEVEL_3,
  GSM_SIGNAL_LEVEL_4,
  GSM_SIGNAL_LEVEL_5,
  GSM_SIGNAL_ERROR,
  TOTAL_GSM_SIGNALS
}GsmSignalQuality;

typedef enum{
  GPRS_SIGNAL_LEVEL_1 = 0,
  GPRS_SIGNAL_LEVEL_2,
  GPRS_SIGNAL_LEVEL_3,
  GPRS_SIGNAL_LEVEL_4,
  GPRS_SIGNAL_LEVEL_5,
  TOTAL_GPRS_SIGNALS,
  GPRS_CLOSE
}GprsSignalQuality;

void gsm_watcher_install (void);

GsmSignalQuality moko_panel_gsm_signal_quality(void);

GprsSignalQuality moko_panel_gprs_signal_quality(void);

G_END_DECLS

#endif /*_MOKO_PANEL_GSM_CONN_*/
