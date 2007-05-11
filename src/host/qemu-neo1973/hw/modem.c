/*
 * AT commands interpreter. (Fake GSM modem)
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

struct modem_s {
    int enable;
    CharDriverState chr;
    int cmd_len;
    char cmd[1024];
#define FIFO_LEN	4096
    int out_start;
    int out_len;
    char outfifo[FIFO_LEN];
    QEMUTimer *out_tm;
    int64_t baud_delay;
};

static void modem_reset(struct modem_s *s)
{
    s->cmd_len = 0;
    s->out_len = 0;
    s->baud_delay = ticks_per_sec;
}

static inline void modem_fifo_wake(struct modem_s *s)
{
    if (!s->enable || !s->out_len)
        return;

    if (s->chr.chr_can_read && s->chr.chr_can_read(s->chr.handler_opaque) &&
                    s->chr.chr_read) {
        s->chr.chr_read(s->chr.handler_opaque,
                        s->outfifo + s->out_start ++, 1);
        s->out_len --;
        s->out_start &= FIFO_LEN - 1;
    }

    if (s->out_len)
        qemu_mod_timer(s->out_tm, qemu_get_clock(vm_clock) + s->baud_delay);
}

static void modem_resp(struct modem_s *s, const char *fmt, ...)
        __attribute__ ((__format__ (__printf__, 2, 3)));
static void modem_resp(struct modem_s *s, const char *fmt, ...)
{
    static char buf[FIFO_LEN];
    int len, off;
    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (len + s->out_len > FIFO_LEN) {
        s->out_len = 0;
        return;
    }
    off = (s->out_start + s->out_len) & (FIFO_LEN - 1);
    if (off + len > FIFO_LEN) {
        memcpy(s->outfifo + off, buf, FIFO_LEN - off);
        memcpy(s->outfifo, buf + (FIFO_LEN - off), off + len - FIFO_LEN);
    } else
        memcpy(s->outfifo + off, buf, len);
    s->out_len += len;
    modem_fifo_wake(s);
}

static struct modem_cmd_s {
    const char *cmd;
    enum { modem_ok, modem_error, modem_call, modem_str } action;
    int (*call)(struct modem_s *s, const char *param);
    const char *resp;
} modem_cmds[] = {
    { "AT+CACM", modem_error },
    { "AT+CAMM", modem_error },
    { "AT+CAOC", modem_error },
    { "AT+CBC", modem_error },
    { "AT+CBST", modem_error },
    { "AT+CCFC", modem_error },
    { "AT+CCUG", modem_error },
    { "AT+CCWA", modem_error },
    { "AT+CCWE", modem_error },
    { "AT+CEER", modem_error },
    { "AT+CFUN", modem_error },
    { "AT+CGACT", modem_error },
    { "AT+CGANS", modem_error },
    { "AT+CGATT", modem_error },
    { "AT+CGAUTO", modem_error },
    { "AT+CGCLASS", modem_error },
    { "AT+CGDATA", modem_error },
    { "AT+CGDCONT", modem_error },
    { "AT+CGEREP", modem_error },
    { "AT+CGMI", modem_str, 0, "<manufacturer>\n\n" },
    { "AT+CGMM", modem_str, 0, "<model>\n\n" },
    { "AT+CGMR", modem_str, 0, "<revision>\n\n" },
    { "AT+CGPADDR", modem_error },
    { "AT+CGQMIN", modem_error },
    { "AT+CGQREQ", modem_error },
    { "AT+CGREG", modem_error },
    { "AT+CGSMS", modem_error },
    { "AT+CGSN", modem_str, 0, "<serial number>\n\n" },
    { "AT+CHLD", modem_error },
    { "AT+CHUP", modem_error },
    { "AT+CIMI", modem_error },
    { "AT+CLAC", modem_error },
    { "AT+CLAE", modem_error },
    { "AT+CLAN", modem_error },
    { "AT+CLCC", modem_error },
    { "AT+CLCK", modem_error },
    { "AT+CLIP", modem_error },
    { "AT+CDIP", modem_error },
    { "AT+CLIR", modem_error },
    { "AT+CLVL", modem_error },
    { "AT+CMEE", modem_error },
    { "AT+CMGC", modem_error },
    { "AT+CMGD", modem_error },
    { "AT+CMGF", modem_error },
    { "AT+CMGL", modem_error },
    { "AT+CMGR", modem_error },
    { "AT+CMGS", modem_error },
    { "AT+CMGW", modem_error },
    { "AT+CMOD", modem_error },
    { "AT+CMSS", modem_error },
    { "AT+CMMS", modem_error },
    { "AT+CMUT", modem_error },
    { "AT+CMUX", modem_error },
    { "AT+CNMA", modem_error },
    { "AT+CNMI", modem_error },
    { "AT+CNUM", modem_error },
    { "AT+COLP", modem_error },
    { "AT+COPN", modem_error },
    { "AT+COPS", modem_ok },
    { "AT+CPAS", modem_error },
    { "AT+CPBF", modem_error },
    { "AT+CPBR", modem_error },
    { "AT+CPBS", modem_error },
    { "AT+CPBW", modem_error },
    { "AT+CPIN", modem_ok },
    { "AT+CPMS", modem_error },
    { "AT+CPOL", modem_error },
    { "AT+CPUC", modem_error },
    { "AT+CPWD", modem_error },
    { "AT+CR", modem_error },
    { "AT+CRC", modem_error },
    { "AT+CREG", modem_error },
    { "AT+CRES", modem_error },
    { "AT+CRLP", modem_error },
    { "AT+CRSL", modem_error },
    { "AT+CRSM", modem_error },
    { "AT+CSAS", modem_error },
    { "AT+CSCA", modem_error },
    { "AT+CSCB", modem_error },
    { "AT+CSCS", modem_error },
    { "AT+CSDH", modem_error },
    { "AT+CSIM", modem_error },
    { "AT+CSMP", modem_error },
    { "AT+CSMS", modem_error },
    { "AT+CSNS", modem_error },
    { "AT+CSQ", modem_error },
    { "AT%CSQ", modem_error },
    { "AT+CSSN", modem_error },
    { "AT+CSTA", modem_error },
    { "AT+CSVM", modem_error },
    { "AT+CTFR", modem_error },
    { "AT+CUSD", modem_error },
    { "AT+DR", modem_error },
    { "AT+FAP", modem_error },
    { "AT+FBO", modem_error },
    { "AT+FBS", modem_error },
    { "AT+FBU", modem_error },
    { "AT+FCC", modem_error },
    { "AT+FCLASS", modem_error },
    { "AT+FCQ", modem_error },
    { "AT+FCR", modem_error },
    { "AT+FCS", modem_error },
    { "AT+FCT", modem_error },
    { "AT+FDR", modem_error },
    { "AT+FDT", modem_error },
    { "AT+FEA", modem_error },
    { "AT+FFC", modem_error },
    { "AT+FHS", modem_error },
    { "AT+FIE", modem_error },
    { "AT+FIP", modem_error },
    { "AT+FIS", modem_error },
    { "AT+FIT", modem_error },
    { "AT+FKS", modem_error },
    { "AT+FLI", modem_error },
    { "AT+FLO", modem_error },
    { "AT+FLP", modem_error },
    { "AT+FMI", modem_error },
    { "AT+FMM", modem_error },
    { "AT+FMR", modem_error },
    { "AT+FMS", modem_error },
    { "AT+FND", modem_error },
    { "AT+FNR", modem_error },
    { "AT+FNS", modem_error },
    { "AT+FPA", modem_error },
    { "AT+FPI", modem_error },
    { "AT+FPS", modem_error },
    { "AT+FPW", modem_error },
    { "AT+FRQ", modem_error },
    { "AT+FSA", modem_error },
    { "AT+FSP", modem_error },
    { "AT+GCAP", modem_error },
    { "AT+GCI", modem_error },
    { "AT+GMI", modem_error },
    { "AT+GMM", modem_error },
    { "AT+GMR", modem_error },
    { "AT+GSN", modem_error },
    { "AT+ICF", modem_error },
    { "AT+IFC", modem_error },
    { "AT+ILRR", modem_error },
    { "AT+IPR", modem_error },
    { "AT+VTS", modem_error },
    { "AT+WS46", modem_error },
    { "AT%ALS", modem_error },
    { "AT%ATR", modem_error },
    { "AT%BAND", modem_error },
    { "AT%CACM", modem_error },
    { "AT%CAOC", modem_error },
    { "AT%CCBS", modem_error },
    { "AT%STDR", modem_error },
    { "AT%CGAATT", modem_error },
    { "AT%CGMM", modem_error },
    { "AT%CGREG", modem_error },
    { "AT%CNAP", modem_error },
    { "AT%CPI", modem_error },
    { "AT%COLR", modem_error },
    { "AT%CPRIM", modem_error },
    { "AT%CTV", modem_error },
    { "AT%CUNS", modem_error },
    { "AT%NRG", modem_error },
    { "AT%SATC", modem_error },
    { "AT%SATE", modem_error },
    { "AT%SATR", modem_error },
    { "AT%SATT", modem_error },
    { "AT%SNCNT", modem_error },
    { "AT%VER", modem_error },
    { "AT%CGCLASS", modem_error },
    { "AT%CGPCO", modem_error },
    { "AT%CGPPP", modem_error },
    { "AT%EM", modem_error },
    { "AT%EMET", modem_error },
    { "AT%EMETS", modem_error },
    { "AT%CBHZ", modem_error },
    { "AT%CPHS", modem_error },
    { "AT%CPNUMS", modem_error },
    { "AT%CPALS", modem_error },
    { "AT%CPVWI", modem_error },
    { "AT%CPOPN", modem_error },
    { "AT%CPCFU", modem_error },
    { "AT%CPINF", modem_error },
    { "AT%CPMB", modem_error },
    { "AT%CPRI", modem_error },
    { "AT%DATA", modem_error },
    { "AT%DINF", modem_error },
    { "AT%CLCC", modem_error },
    { "AT%DBGINFO", modem_error },
    { "AT%VTS", modem_error },
    { "AT%CHPL", modem_error },
    { "AT%CREG", modem_error },
    { "AT+CTZR", modem_error },
    { "AT+CTZU", modem_error },
    { "AT%CTZV", modem_error },
    { "AT%CNIV", modem_error },
    { "AT%PVRF", modem_error },
    { "AT%CWUP", modem_error },
    { "AT%DAR", modem_error },
    { "AT+CIND", modem_error },
    { "AT+CMER", modem_error },
    { "AT%CSCN", modem_error },
    { "AT%RDL", modem_error },
    { "AT%RDLB", modem_error },
    { "AT%CSTAT", modem_error },
    { "AT%CPRSM", modem_error },
    { "AT%CHLD", modem_error },
    { "AT%SIMIND", modem_error },
    { "AT%SECP", modem_error },
    { "AT%SECS", modem_error },
    { "AT%CSSN", modem_error },
    { "AT+CCLK", modem_error },
    { "AT%CSSD", modem_error },
    { "AT%COPS", modem_error },
    { "AT%CPMBW", modem_error },
    { "AT%CUST", modem_error },
    { "AT%SATCC", modem_error },
    { "AT%COPN", modem_error },
    { "AT%CGEREP", modem_error },
    { "AT%CUSCFG", modem_error },
    { "AT%CUSDR", modem_error },
    { "AT%CPBS", modem_error },
    { "AT%PBCF", modem_error },
    { "AT%SIMEF", modem_error },
    { "AT%EFRSLT", modem_error },
    { "AT%CMGMDU", modem_error },
    { "AT%CMGL", modem_error },
    { "AT%CMGR", modem_error },
    { "ATA", modem_ok },
    { "ATB", modem_error },
    { "AT&C", modem_error },
    { "ATD", modem_ok },
    { "AT&D", modem_error },
    { "ATE", modem_ok },
    { "ATF", modem_error },
    { "AT&F", modem_error },
    { "ATH", modem_ok },
    { "ATI", modem_error },
    { "AT&K", modem_error },
    { "ATL", modem_error },
    { "ATM", modem_error },
    { "ATO", modem_error },
    { "ATP", modem_error },
    { "ATQ", modem_error },
    { "ATS", modem_error },
    { "ATT", modem_error },
    { "ATV", modem_error },
    { "ATW", modem_error },
    { "AT&W", modem_error },
    { "ATX", modem_error },
    { "ATZ", modem_error },
    { 0 }
};

static void modem_cmd(struct modem_s *s, const char *cmd)
{
    struct modem_cmd_s *entry;
    const char *eq, *param;
    int ok;
    eq = strchr(cmd, '=');
    if (eq)
        param = eq + 1;
    else
        param = 0;
    ok = 0;
    for (entry = modem_cmds; entry->cmd; entry ++)
        if (!strncmp(cmd, entry->cmd, strlen(entry->cmd))) {
            switch (entry->action) {
            case modem_ok:
                ok = 1;
                break;
            case modem_error:
                ok = 0;
                break;
            case modem_call:
                ok = entry->call(s, param);
                break;
            case modem_str:
                modem_resp(s, entry->resp);
                ok = 1;
                break;
            }
            return;
        }
    if (ok)
        modem_resp(s, "OK\n");
    else
        modem_resp(s, "ERROR\n");
}

static int modem_write(struct CharDriverState *chr,
                const uint8_t *buf, int len)
{
    struct modem_s *s = (struct modem_s *) chr->opaque;
    char *eol;
    if (!s->enable)
        return 0;

    len = MIN(len, 1023 - s->cmd_len);
    memcpy(s->cmd + s->cmd_len, buf, len);
    s->cmd_len += len;
    s->cmd[s->cmd_len] = 0;

    eol = strchr(s->cmd, '\n');
    if (eol) {
        s->cmd_len -= eol - s->cmd;
        *eol = 0;
        modem_cmd(s, s->cmd);
        memcpy(s->cmd, eol + 1, s->cmd_len --);
    } else if (s->cmd_len >= 1023)
        s->cmd_len = 0;
    return len;
}

static int modem_ioctl(struct CharDriverState *chr, int cmd, void *arg)
{
    QEMUSerialSetParams *ssp;
    struct modem_s *s = (struct modem_s *) chr->opaque;
    switch (cmd) {
    case CHR_IOCTL_SERIAL_SET_PARAMS:
        ssp = (QEMUSerialSetParams *) arg;
        s->baud_delay = ticks_per_sec / ssp->speed;
        break;

    case CHR_IOCTL_MODEM_HANDSHAKE:
        if (!s->enable)
            return -ENOTSUP;
        if (*(int *) arg)
            modem_resp(s, "AT-Command Interpreter Ready\nOK\n");
        break;

    default:
        return -ENOTSUP;
    }
    return 0;
}

void modem_enable(CharDriverState *chr, int enable)
{
    struct modem_s *s = (struct modem_s *) chr->opaque;
    if (enable)
        modem_reset(s);
    s->enable = enable;
}

static void modem_out_tick(void *opaque)
{
    modem_fifo_wake((struct modem_s *) opaque);
}

CharDriverState *modem_init()
{
    struct modem_s *s = (struct modem_s *)
            qemu_mallocz(sizeof(struct modem_s));
    s->chr.opaque = s;
    s->chr.chr_write = modem_write;
    s->chr.chr_ioctl = modem_ioctl;
    s->out_tm = qemu_new_timer(vm_clock, modem_out_tick, s);

    return &s->chr;
}
