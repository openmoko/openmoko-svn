/*
 * Fake GSM modem.  Tries to be Texas Instruments "Calypso" compatible.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

#include "misc.h"
#include "gnokii.h"
#include "compat.h"
#include "at-emulator.h"
#include "datapump.h"

struct modem_s {
    int enable;
    CharDriverState chr;
#define FIFO_LEN	4096
    int out_start;
    int out_len;
    char outfifo[FIFO_LEN];
    QEMUTimer *out_tm;
    int64_t baud_delay;

    struct gn_statemachine state;
    struct gsmmodem_info_s info;
};

#define TICALYPSO_MANF	"<manufacturer>"
#define TICALYPSO_MODEL	"<model>"
#define TICALYPSO_REV	"<revision>"
#define TICALYPSO_IMEI	"<serial number>"

static gn_error modem_ops(gn_operation op, gn_data *data,
                struct gn_statemachine *sm)
{
    switch (op) {
    case GN_OP_MakeCall:
        fprintf(stderr, "%s: calling %s: busy.\n", __FUNCTION__,
                        data->call_info->number);
        return GN_ERR_LINEBUSY;

    case GN_OP_CancelCall:
        fprintf(stderr, "%s: hangup.\n", __FUNCTION__);
        break;

    case GN_OP_GetSMS:
        fprintf(stderr, "%s: SMS number %i requested\n",
                        __FUNCTION__, data->sms->number);
        return GN_ERR_EMPTYLOCATION;

    case GN_OP_DeleteSMS:
        fprintf(stderr, "%s: deleting SMS number %i\n", __FUNCTION__,
                        data->sms->number);
        return GN_ERR_EMPTYLOCATION;

    case GN_OP_SendSMS:
        fprintf(stderr, "%s: SMS type 0x%02x sent\n",
                        __FUNCTION__, data->sms->type);
        return GN_ERR_NOTSUPPORTED;

    case GN_OP_GetRFLevel:
        *data->rf_level = 32.0f;	/* Some -50 dBm */
        break;

    case GN_OP_GetImei:
        strcpy(data->imei, TICALYPSO_IMEI);
        break;

    case GN_OP_GetRevision:
        strcpy(data->revision, TICALYPSO_REV);
        break;

    case GN_OP_GetModel:
        strcpy(data->revision, TICALYPSO_MODEL);
        break;

    case GN_OP_Identify:
        strcpy(data->model, TICALYPSO_MODEL);
        strcpy(data->revision, TICALYPSO_REV);
        strcpy(data->imei, TICALYPSO_IMEI);
        strcpy(data->manufacturer, TICALYPSO_MANF);
        break;

    default:
        return GN_ERR_NOTSUPPORTED;
    }
    return GN_ERR_NONE;
}

static void modem_reset(struct modem_s *s)
{
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

static void modem_resp(void *opaque, const char *fmt, ...)
        __attribute__ ((__format__ (__printf__, 2, 3)));
static void modem_resp(void *opaque, const char *fmt, ...)
{
    struct modem_s *s = (struct modem_s *) opaque;
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

static int modem_write(struct CharDriverState *chr,
                const uint8_t *buf, int len)
{
    struct modem_s *s = (struct modem_s *) chr->opaque;
    if (!s->enable)
        return 0;

    gn_atem_incoming_data_handle(buf, len);
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
            modem_resp(s, "AT-Command Interpreter Ready\r\nOK\r\n");
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

    s->state.info = &s->info;
    s->info.write = modem_resp;
    s->info.gn_sm_functions = modem_ops;
    s->info.opaque = s;

    if (!gn_atem_initialise(&s->state))
        goto fail;

    if (!dp_Initialise())
        goto fail;

    return &s->chr;
fail:
    fprintf(stderr, "%s: GSM modem initialisation failed\n", __FUNCTION__);
    exit(-1);
}
