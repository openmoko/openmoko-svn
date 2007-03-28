#ifndef _MOKO_PANEL_GSM_CONN_
#define _MOKO_PANEL_GSM_CONN_

#define STDIN_BUF_SIZE	1024
/*signal value: "0": best connection "5": connect error*/
enum{
  GSM_SIGNAL_LEVEL_5 = 0,
  GSM_SIGNAL_LEVEL_4,
  GSM_SIGNAL_LEVEL_3,
  GSM_SIGNAL_LEVEL_2,
  GSM_SIGNAL_LEVEL_1,
  GSM_SIGNAL_ERROR ,
  TOTAL_SIGNALS
};

void gsm_watcher_install (void);

int moko_panel_gsm_signal_quality(void);

#endif /*_MOKO_PANEL_GSM_CONN_*/
