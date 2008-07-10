/*
 * Copyright (c) 2004-2005 Atheros Communications Inc.
 * All rights reserved.
 *
 * This file contains the definitions for wmiconfig utility
 *
 * $Id: //depot/sw/releases/olca2.0-GPL/host/tools/wmiconfig/wmiconfig.h#1 $
 */

#ifndef _WMI_CONFIG_H_
#define _WMI_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    WMI_GET_VERSION=501,     /* something that doesn't collide with ascii */
    WMI_SET_POWER_MODE,
    WMI_SET_IBSS_PM_CAPS,
    WMI_SET_PM_PARAMS,
    WMI_SET_SCAN_PARAMS,
    WMI_SET_LISTEN_INTERVAL,
    WMI_SET_BMISS_TIME,
    WMI_SET_BSS_FILTER,
    WMI_SET_RSSI_THRESHOLDS,
    WMI_SET_CHANNEL,
    WMI_SET_SSID,
    WMI_SET_BADAP,
    WMI_DELETE_BADAP,
    WMI_CREATE_QOS,
    WMI_DELETE_QOS,
    WMI_GET_QOS_QUEUE,
    WMI_GET_TARGET_STATS,
    WMI_SET_TARGET_ERROR_REPORTING_BITMASK,
    WMI_SET_AC_PARAMS,
    WMI_SET_ASSOC_IE,
    WMI_SET_DISC_TIMEOUT,
    WMI_SET_ADHOC_BSSID,
    WMI_SET_OPT_MODE,
    WMI_OPT_SEND_FRAME,
    WMI_SET_BEACON_INT,
    WMI_SET_VOICE_PKT_SIZE,
    WMI_SET_MAX_SP,
    WMI_GET_ROAM_TBL,
    WMI_SET_ROAM_CTRL,
    WMI_SET_POWERSAVE_TIMERS,
    WMI_SET_POWERSAVE_TIMERS_PSPOLLTIMEOUT,
    WMI_SET_POWERSAVE_TIMERS_TRIGGERTIMEOUT,
    WMI_GET_POWER_MODE,
    WMI_SET_WLAN_STATE,
    WMI_GET_ROAM_DATA,
    WMI_SET_BT_STATUS,
    WMI_SET_BT_PARAMS,
    WMI_SET_RETRYLIMITS,
    WMI_START_SCAN,
    WMI_SET_FIX_RATES,
    WMI_GET_FIX_RATES,
    WMI_SET_SNR_THRESHOLDS,
    WMI_CLR_RSSISNR,
    WMI_SET_LQ_THRESHOLDS,
    WMI_SET_AUTH_MODE,
    WMI_SET_REASSOC_MODE,
    WMI_SET_LPREAMBLE,
    WMI_SET_RTS,
    WMI_SET_WMM,
#ifdef USER_KEYS
    USER_SETKEYS,
#endif
    WMI_APSD_TIM_POLICY,
    WMI_SET_ERROR_DETECTION,
    WMI_GET_HB_CHALLENGE_RESP,
    WMI_SET_TXOP,
    DIAG_ADDR,
    DIAG_DATA,
    DIAG_READ,
    DIAG_WRITE,
    WMI_GET_RD,
    WMI_SET_KEEPALIVE,
    WMI_GET_KEEPALIVE,
    WMI_SET_APPIE,
    WMI_SET_MGMT_FRM_RX_FILTER,
    WMI_DBGLOG_CFG_MODULE,
    WMI_DBGLOG_GET_DEBUG_LOGS,
    WMI_SET_HOST_SLEEP_MODE,
    WMI_SET_WOW_MODE,
    WMI_GET_WOW_LIST,
    WMI_ADD_WOW_PATTERN,
    WMI_DEL_WOW_PATTERN,
    DIAG_DUMP_CHIP_MEM,
    WMI_SET_CONNECT_CTRL_FLAGS,
    DUMP_HTC_CREDITS,
    USER_SETKEYS_INITRSC,
    WMI_SCAN_DFSCH_ACT_TIME,
    WMI_SIMULATED_APSD_TIM_POLICY,
    WMI_SET_AKMP_INFO,
    WMI_AKMP_MULTI_PMKID,
    WMI_NUM_PMKID,
    WMI_PMKID_ENTRY,
    WMI_SET_PMKID_LIST,
    WMI_GET_PMKID_LIST,
    WMI_SET_IEMASK,
    WMI_SET_BSS_PMKID_INFO,
    WMI_BSS_PMKID_ENTRY,
    WMI_BSSID,
};

/*
***************************************************************************
**  How to form a regcode from CountryCode, Regulatory domain or WWR code:
**
**  WWR code is nothing but a special case of Regulatory domain.
**
**      code is of type U_INT32
**
**      Bit-31  Bit-30      Bit11-0
**        0       0          xxxx    -> Bit11-0 is a Regulatory Domain
**        0       1          xxxx    -> Bit11-0 is a WWR code
**        1       X          xxxx    -> Bit11-0 is a Country code
**  
***************************************************************************
*/
#define REGCODE_IS_CC_BITSET(x)     ((x) & 0x80000000)
#define REGCODE_GET_CODE(x)         ((x) & 0xFFF)
#define REGCODE_IS_WWR_BITSET(x)    ((x) & 0x40000000)


typedef struct {
    A_UINT32        numPMKIDUser;          /* PMKIDs user wants to enter */
    WMI_SET_PMKID_LIST_CMD  *pmkidInfo;
} pmkidUserInfo_t;

#ifdef __cplusplus
}
#endif

#endif /* _WMI_CONFIG_H_ */
