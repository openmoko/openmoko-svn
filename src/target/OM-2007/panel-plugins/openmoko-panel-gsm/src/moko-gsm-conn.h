#ifndef _MOKO_PANEL_GSM_CONN_
#define _MOKO_PANEL_GSM_CONN_

#include <glib-object.h>

#define STDIN_BUF_SIZE	1024
/*signal value: "0": best connection "5": connect error*/
G_BEGIN_DECLS

enum{
  GSM,
  GPRS,
  SIN_NO
};

typedef enum{
  LEVEL_1 = 0,
  LEVEL_2,
  LEVEL_3,
  LEVEL_4,
  LEVEL_5,
  UN_CONN,
  TOTAL_STATUS
}SignalStatus;

void gsm_watcher_install (void);

SignalStatus moko_panel_gsm_signal_quality(void);

SignalStatus moko_panel_gprs_signal_quality(void);

G_END_DECLS

#endif /*_MOKO_PANEL_GSM_CONN_*/
