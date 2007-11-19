/*
 * Philips PCF50605 / PCF50606 Power Management Unit chip.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This file is licensed under GNU GPL v2.
 */

#include "hw.h"
#include "qemu-timer.h"
#include "i2c.h"
#include "sysemu.h"
#include "console.h"

#define VERBOSE 1

struct pcf_s {
    i2c_slave i2c;
    qemu_irq irq;

    int firstbyte;
    int charging;
    uint8_t reg;
    uint8_t oocs;
    uint8_t oocc[2];
    uint8_t intr[3];
    uint8_t intm[3];
    uint8_t pssc;
    uint8_t pwrokm;
    uint8_t dcdc[4];
    uint8_t dcdec[2];
    uint8_t dcudc[2];
    uint8_t ioregc;
    uint8_t dregc[3];
    uint8_t lpregc[2];
    uint8_t mbcc[3];
    uint8_t bbcc;
    uint8_t adcc[2];
    uint8_t adcs[3];
    uint8_t acdc[1];
    uint8_t bvmc;
    uint8_t pwmc[1];
    uint8_t ledc[2];
    uint8_t gpoc[5];
    struct {
        int x;
        int y;
        int pendown;
    } ts;
    struct {
        QEMUTimer *hz;
        int64_t next;
        struct tm tm;
        uint32_t sec;
        uint8_t alm_sec;
        uint8_t alm_min;
        uint8_t alm_hour;
        uint8_t alm_wday;
        uint8_t alm_mday;
        uint8_t alm_mon;
        uint8_t alm_year;
    } rtc;
    qemu_irq gpo_handler[6];
    uint8_t gpo;
    QEMUTimer *onkeylow;
};

static inline void pcf_update(struct pcf_s *s)
{
    qemu_set_irq(s->irq,
                    !((s->intr[0] & ~s->intm[0]) ||
                      (s->intr[1] & ~s->intm[1]) ||
                      (s->intr[2] & ~s->intm[2])));
}

void pcf_reset(i2c_slave *i2c)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    time_t ti;
    s->charging = 1;
    s->reg = 0x00;
    s->oocs = 0x58;		/* Main and backup batteries present */
    s->oocc[0] = 0x64;
    s->oocc[1] = 0x05;
    s->intr[0] = 0x01;		/* ONKEY inactive high */
    s->intr[1] = 0x00 | s->charging | ((!s->charging) << 1);
    s->intr[2] = 0x00;
    s->intm[0] = 0x00;
    s->intm[1] = 0x00;
    s->intm[2] = 0x00;
    s->pssc = 0x00;
    s->pwrokm = 0x00;
    s->dcdc[0] = 0xd0;		/* 1.3V */
    s->dcdc[1] = 0xc8;		/* 1.1V */
    s->dcdc[2] = 0x00;
    s->dcdc[3] = 0x30;
    s->dcdec[0] = 0x88;		/* 3.3V */
    s->dcdec[1] = 0x00;
    s->dcudc[0] = 0x83;		/* 1.8V */
    s->dcudc[1] = 0x30;
    s->ioregc = 0xf8;		/* 3.3V */
    s->dregc[0] = 0xc2;		/* 1.1V */
    s->dregc[1] = 0xc4;		/* 1.3V */
    s->dregc[2] = 0x90;		/* 2.5V */
    s->lpregc[0] = 0xf8;	/* 3.3V */
    s->lpregc[1] = 0x01;	/* ECO */
    s->mbcc[0] = 0x1d;
    s->mbcc[1] = 0x14;
    s->mbcc[2] = 0x10;
    s->bbcc = 0x13;
    s->adcc[0] = 0x00;
    s->adcc[1] = 0x00;
    s->adcs[0] = 0x00;
    s->adcs[1] = 0x00;
    s->adcs[2] = 0x00;
    s->ts.pendown = 0;
    s->acdc[0] = 0x00;
    s->bvmc = 0x0e;
    time(&ti);
    s->rtc.sec = ti;
    s->rtc.alm_sec = 0x7f;
    s->rtc.alm_min = 0x7f;
    s->rtc.alm_hour = 0x3f;
    s->rtc.alm_wday = 0x07;
    s->rtc.alm_mday = 0x3f;
    s->rtc.alm_mon = 0x1f;
    s->rtc.alm_year = 0xff;
    s->pwmc[0] = 0x80;
    s->ledc[0] = 0x4d;
    s->ledc[1] = 0x4d;
    s->gpoc[0] = 0x00;
    s->gpoc[1] = 0x00;
    s->gpoc[2] = 0x00;
    s->gpoc[3] = 0x00;
    s->gpoc[4] = 0x00;
    s->gpo = 0x3c;
    pcf_update(s);
}

#define BATVOLT			88	/* Percentage */

#define BATT_MVOLTS_MIN		2800	/* Millivolts */
#define BATT_MVOLTS_MAX		4200	/* Millivolts */

#define BATT_MVOLTS_DIFF	(BATT_MVOLTS_MAX - BATT_MVOLTS_MIN)
#define MVOLTS(x)		(BATT_MVOLTS_MIN + (x) * BATT_MVOLTS_DIFF / 100)
#define MVOLTS2ADC(x)		((x * 1024) / 6000)

static void pcf_adc_convert(struct pcf_s *s)
{
    int cval;
#define PCF_STR_RES1(value)		\
    cval = (value);			\
    s->adcs[0] = cval >> 2;		\
    if (!(s->adcc[1] >> 7))		\
        s->adcs[1] |= cval & 3;
#define PCF_STR_RES2(value)		\
    cval = (value);			\
    s->adcs[2] = cval >> 2;		\
    if (!(s->adcc[1] >> 7))		\
        s->adcs[1] |= (cval & 3) << 2;
    s->adcs[0] = 0x00;
    s->adcs[1] = 0x80;
    s->adcs[2] = 0x00;
    switch ((s->adcc[1] >> 1) & 0xf) {	/* ADCMUX */
    case 0x0:	/* BATVOLT, resistive divider */
        PCF_STR_RES1(MVOLTS2ADC(MVOLTS(BATVOLT)));
        break;
    case 0x1:	/* BATVOLT, substractor */
        PCF_STR_RES1(MVOLTS2ADC(MVOLTS(BATVOLT)));
        break;
    case 0x2:	/* ADCIN1, resistive divider */
        PCF_STR_RES1(500);
        break;
    case 0x3:	/* ADCIN1, substractor */
        PCF_STR_RES1(500);
        break;
    case 0x4:	/* BATTEMP, ratiometric */
        PCF_STR_RES1(200);
        break;
    case 0x5:	/* ADCIN2 */
        PCF_STR_RES1(500);
        break;
    case 0x6:	/* ADCIN3 */
        PCF_STR_RES1(500);
        break;
    case 0x7:	/* ADCIN3, ratiometric */
        PCF_STR_RES1(500);
        break;
    case 0x8:	/* X position */
        PCF_STR_RES1(s->ts.x >> (15 - 10));
        break;
    case 0x9:	/* Y position */
        PCF_STR_RES1(s->ts.y >> (15 - 10));
        break;
    case 0xa:	/* P1 plate resistance */
        PCF_STR_RES1(500);
        break;
    case 0xb:	/* P2 plate resistance */
        PCF_STR_RES1(500);
        break;
    case 0xc:	/* BATVOLT + ADCIN1, substractor */
        PCF_STR_RES2(MVOLTS2ADC(MVOLTS(BATVOLT)));
        PCF_STR_RES1(500);
        break;
    case 0xe:	/* X + Y position */
        PCF_STR_RES2(s->ts.x >> (15 - 10));
        PCF_STR_RES1(s->ts.y >> (15 - 10));
        break;
    case 0xf:	/* P1 + P2 plate */
        PCF_STR_RES2(500);
        PCF_STR_RES1(500);
	break;
    default:
        return;
    }

    s->intr[3] |= (1 << 0);	/* set ADCRDY */
    pcf_update(s);
}

static void pcf_adc_event(void *opaque, int x, int y, int z, int buttons_state)
{
    struct pcf_s *s = (struct pcf_s *) opaque;
    s->ts.x = x;
    s->ts.y = y;
    s->ts.pendown = !!buttons_state;

    if (s->adcc[0] & 1) {	/* TSCMODACT */
        s->intr[3] |= (1 << 3);	/* set TSCPRES */
        pcf_update(s);
    }
}

static void pcf_rtc_hz(void *opaque)
{
    struct pcf_s *s = (struct pcf_s *) opaque;

    s->rtc.sec ++;
    s->rtc.next += 1000;
    qemu_mod_timer(s->rtc.hz, s->rtc.next);
    s->intr[0] |= 1 << 6;	/* set SECOND */
    pcf_update(s);
}

static void pcf_rtc_update(struct pcf_s *s)
{
    void *ret;
    time_t ti = s->rtc.sec;
    if (rtc_utc)
        ret = gmtime_r(&ti, &s->rtc.tm);
    else
        ret = localtime_r(&ti, &s->rtc.tm);
}

static inline uint8_t to_bcd(int val)
{
    return ((val / 10) << 4) | (val % 10);
}

static inline int from_bcd(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0f);
}

static void pcf_gpo_set(struct pcf_s *s, int line, int state, int inv)
{
    state = (!!state) ^ inv;
    if (s->gpo_handler[line] && ((s->gpo & 1) ^ state))
        qemu_set_irq(s->gpo_handler[line], state);
    s->gpo &= ~(1 << line);
    s->gpo |= state << line;
}

#define PCF_ID		0x00	/* Identification register */
#define PCF_OOCS	0x01	/* Control and Status register */
#define PCF_INT1	0x02	/* Interrupt Control register */
#define PCF_INT2	0x03	/* Interrupt Control register */
#define PCF_INT3	0x04	/* Interrupt Control register */
#define PCF_INT1M	0x05	/* Interrupt Control register */
#define PCF_INT2M	0x06	/* Interrupt Control register */
#define PCF_INT3M	0x07	/* Interrupt Control register */
#define PCF_OOCC1	0x08	/* Control and Status register */
#define PCF_OOCC2	0x09	/* Control and Status register */
#define PCF_RTCSC	0x0a	/* RTC Seconds register */
#define PCF_RTCMN	0x0b	/* RTC Minutes register */
#define PCF_RTCHR	0x0c	/* RTC Hour register */
#define PCF_RTCWD	0x0d	/* RTC Week-day register */
#define PCF_RTCDT	0x0e	/* RTC Date register */
#define PCF_RTCMT	0x0f	/* RTC Month register */
#define PCF_RTCYR	0x10	/* RTC Year register */
#define PCF_RTCSCA	0x11	/* RTC Alarm Seconds register */
#define PCF_RTCMNA	0x12	/* RTC Alarm Minutes register */
#define PCF_RTCHRA	0x13	/* RTC Alarm Hour register */
#define PCF_RTCWDA	0x14	/* RTC Alarm Week-day register */
#define PCF_RTCDTA	0x15	/* RTC Alarm Date register */
#define PCF_RTCMTA	0x16	/* RTC Alarm Month register */
#define PCF_RTCYRA	0x17	/* RTC Alarm Year register */
#define PCF_PSSC	0x18	/* Power Sequencing register */
#define PCF_PWROKM	0x19	/* Power Ok Masking register */
#define PCF_PWROKS	0x1a	/* Power Ok Status register */
#define PCF_DCDC1	0x1b	/* Direct-Current Down Converter register */
#define PCF_DCDC2	0x1c	/* Direct-Current Down Converter register */
#define PCF_DCDC3	0x1d	/* Direct-Current Down Converter register */
#define PCF_DCDC4	0x1e	/* Direct-Current Down Converter register */
#define PCF_DCDEC1	0x1f	/* Direct-Current Down Converter register */
#define PCF_DCDEC2	0x20	/* Direct-Current Down Converter register */
#define PCF_DCUDC1	0x21	/* Direct-Current Down Converter register */
#define PCF_DCUDC2	0x22	/* Direct-Current Down Converter register */
#define PCF_IOREGC	0x23	/* IOREG Low Drop-out Linear Regulator */
#define PCF_D1REGC1	0x24	/* D1REG Low Drop-out Linear Regulator */
#define PCF_D2REGC1	0x25	/* D2REG Low Drop-out Linear Regulator */
#define PCF_D3REGC1	0x26	/* D3REG Low Drop-out Linear Regulator */
#define PCF_LPREGC1	0x27	/* LPREG Low Drop-out Linear Regulator */
#define PCF_LPREGC2	0x28	/* LPREG Low Drop-out Linear Regulator */
#define PCF_MBCC1	0x29	/* Charging Mode Control register */
#define PCF_MBCC2	0x2a	/* Charging Mode Control register */
#define PCF_MBCC3	0x2b	/* Charging Mode Control register */
#define PCF_MBCS1	0x2c	/* Charging Mode Status register */
#define PCF_BBCC	0x2d	/* Backup Battery Charging Control register */
#define PCF_ADCC1	0x2e	/* ADC Control register */
#define PCF_ADCC2	0x2f	/* ADC Control register */
#define PCF_ADCS1	0x30	/* ADC Status register */
#define PCF_ADCS2	0x31	/* ADC Status register */
#define PCF_ADCS3	0x32	/* ADC Status register */
#define PCF_ACDC1	0x33	/* Accessory Detection Control register */
#define PCF_BVMC	0x34	/* Battery Voltage Monitor Control register */
#define PCF_PWMC1	0x35	/* Pulse Width Modulation Control register */
#define PCF_LEDC1	0x36	/* LED Control and Status register */
#define PCF_LEDC2	0x37	/* LED Control and Status register */
#define PCF_GPOC1	0x38	/* General-purpose Output register */
#define PCF_GPOC2	0x39	/* General-purpose Output register */
#define PCF_GPOC3	0x3a	/* General-purpose Output register */
#define PCF_GPOC4	0x3b	/* General-purpose Output register */
#define PCF_GPOC5	0x3c	/* General-purpose Output register */

static uint8_t pcf_read(void *opaque, uint8_t addr)
{
    struct pcf_s *s = (struct pcf_s *) opaque;
    int reg = 0;
    uint8_t ret;

    switch (addr) {
    case PCF_ID:
        return 0x4d;

    case PCF_OOCS:
        return s->oocs | (s->charging << 5);

    case PCF_OOCC2: reg ++;
    case PCF_OOCC1:
        return s->oocc[reg];

    case PCF_INT1:
        ret = s->intr[0];
        s->intr[0] = 0x00;
        pcf_update(s);
        return ret;

    case PCF_INT2:
        ret = s->intr[1];
        s->intr[1] = 0x00;
        pcf_update(s);
        return ret;

    case PCF_INT3:
        ret = s->intr[2];
        s->intr[2] = 0x00;
        pcf_update(s);
        return ret;

    case PCF_INT1M:
        return s->intm[0];

    case PCF_INT2M:
        ret = s->intm[1];
        s->intm[1] &= 0xfc;
        pcf_update(s);
        return ret;

    case PCF_INT3M:
        return s->intm[2];

    case PCF_PSSC:
        return s->pssc;

    case PCF_PWROKM:
        return s->pwrokm;

    case PCF_PWROKS:
        return 0xff;	/* Present a very optimistic PWR OK Status value.  */

    case PCF_DCDC4: reg ++;
    case PCF_DCDC3: reg ++;
    case PCF_DCDC2: reg ++;
    case PCF_DCDC1:
        return s->dcdc[reg];

    case PCF_DCDEC2: reg ++;
    case PCF_DCDEC1:
        return s->dcdec[reg];

    case PCF_DCUDC2: reg ++;
    case PCF_DCUDC1:
        return s->dcudc[reg];

    case PCF_IOREGC:
        return s->ioregc;

    case PCF_D3REGC1: reg ++;
    case PCF_D2REGC1: reg ++;
    case PCF_D1REGC1:
        return s->dregc[reg];

    case PCF_LPREGC1: reg ++;
    case PCF_LPREGC2:
        return s->lpregc[reg];

    case PCF_MBCC3: reg ++;
    case PCF_MBCC2: reg ++;
    case PCF_MBCC1:
        return s->mbcc[reg];

    case PCF_MBCS1:
        return 0x55;	/* All within limits.  */

    case PCF_BBCC:
        return s->bbcc;

    case PCF_ADCC1:
        return s->adcc[0] | (s->ts.pendown << 7);
    case PCF_ADCC2:
        return s->adcc[1];

    case PCF_ADCS3: reg ++;
    case PCF_ADCS2: reg ++;
    case PCF_ADCS1:
        return s->adcs[reg];

    case PCF_ACDC1:
        return s->acdc[reg];

    case PCF_BVMC:
        return s->bvmc;

    case PCF_RTCSC:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_sec);
    case PCF_RTCMN:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_min);
    case PCF_RTCHR:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_hour);
    case PCF_RTCWD:
        pcf_rtc_update(s);
        return s->rtc.tm.tm_wday;
    case PCF_RTCDT:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_mday);
    case PCF_RTCMT:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_mon + 1);
    case PCF_RTCYR:
        pcf_rtc_update(s);
        return to_bcd(s->rtc.tm.tm_year % 100);

    case PCF_RTCSCA:
        return s->rtc.alm_sec;
    case PCF_RTCMNA:
        return s->rtc.alm_min;
    case PCF_RTCHRA:
        return s->rtc.alm_hour;
    case PCF_RTCWDA:
        return s->rtc.alm_wday;
    case PCF_RTCDTA:
        return s->rtc.alm_mday;
    case PCF_RTCMTA:
        return s->rtc.alm_mon;
    case PCF_RTCYRA:
        return s->rtc.alm_year;

    case PCF_PWMC1:
        return s->pwmc[reg];

    case PCF_LEDC2: reg ++;
    case PCF_LEDC1:
        return s->ledc[reg];

    case PCF_GPOC5: reg ++;
    case PCF_GPOC4: reg ++;
    case PCF_GPOC3: reg ++;
    case PCF_GPOC2: reg ++;
    case PCF_GPOC1:
        return s->gpoc[reg];

    default:
#ifdef VERBOSE
        printf("%s: unknown register %02x\n", __FUNCTION__, addr);
#endif
        break;
    }
    return 0;
}

static void pcf_write(void *opaque, uint8_t addr, uint8_t value)
{
    struct pcf_s *s = (struct pcf_s *) opaque;
    int diff;
    int reg = 0;
    const char *chgmod[8] = {
        "Qualification Mode",
        "Pre-charge Mode",
        "Trickle-charge Mode",
        "Fast-charge Mode (CCCV)",
        "Fast-charge Mode (no CC)",
        "Fast-charge Mode (no CV)",
        "Switch Mode",
        "Idle Mode",
    };
    const int vchgcon[16] = {
        0x400, 0x402, 0x404, 0x406, 0x408, 0x410, 0x412, 0x414,
        0x416, 0x418, 0x420, 0x422, 0x424, 0x426, 0x428, 0x500,
    };

    switch (addr) {
    case PCF_OOCS:
        s->oocs &= ~4;
        s->oocs |= value & 4;
        break;

    case PCF_OOCC1:
        if (value & (1 << 3))
            printf("%s: Watchdog timer enable attempt.\n", __FUNCTION__);
        if (value & (1 << 0)) {
            printf("%s: Power-off requested.\n", __FUNCTION__);
            qemu_system_shutdown_request();
        }
        s->oocc[0] = value & 0xfc;
        break;

    case PCF_OOCC2:
        s->oocc[1] = value & 0x0f;
        break;

    case PCF_INT1M:
        s->intm[0] = value & 0xdf;
        pcf_update(s);
        break;

    case PCF_INT2M:
        s->intm[1] = value & 0xff;
        pcf_update(s);
        break;

    case PCF_INT3M:
        s->intm[2] = value & 0xcf;
        pcf_update(s);
        break;

    case PCF_PSSC:
        s->pssc = value;
        break;

    case PCF_PWROKM:
        s->pwrokm = value;
        break;

    case PCF_DCDC4: reg ++;
    case PCF_DCDC3: reg ++;
    case PCF_DCDC2: reg ++;
    case PCF_DCDC1:
        s->dcdc[reg] = value;
        break;

    case PCF_DCDEC2: reg ++;
    case PCF_DCDEC1:
        s->dcdec[reg] = value;
        break;

    case PCF_DCUDC2: reg ++;
    case PCF_DCUDC1:
        s->dcudc[reg] = value;
        break;

    case PCF_IOREGC:
        s->ioregc = value;
        break;

    case PCF_D3REGC1: reg ++;
    case PCF_D2REGC1: reg ++;
    case PCF_D1REGC1:
        s->dregc[reg] = value;
        break;

    case PCF_LPREGC2: reg ++;
    case PCF_LPREGC1:
        s->lpregc[reg] = value;
        break;

    case PCF_MBCC1:
        if ((((s->mbcc[0] ^ value) & (7 << 2)) && (value & 1)) ||
                        ((s->mbcc[0] ^ value) & 1)) {
            if (value & 1)
                printf("%s: charging in %s.\n", __FUNCTION__,
                                chgmod[(value >> 2) & 7]);
            else
                printf("%s: charging disabled.\n", __FUNCTION__);
        }
        if ((s->mbcc[0] ^ value) & 2)
            printf("%s: automatic Fast-charge %sabled.\n", __FUNCTION__,
                            (value & 2) ? "en" : "dis");
        s->mbcc[0] = value & 0x3f;
        break;

    case PCF_MBCC2:
        s->mbcc[1] = value & 0x1f;
        break;

    case PCF_MBCC3:
        if ((s->mbcc[2] ^ value) & 0xf)
            printf("%s: charge voltage %i.%02xV.\n", __FUNCTION__,
                            (vchgcon[value & 0xf] >> 8) & 0xf,
                            vchgcon[value & 0xf] & 0xff);
        s->mbcc[2] = value & 0x3f;
        break;

    case PCF_BBCC:
        if ((s->bbcc ^ value) & 1)
            printf("%s: backup battery charger %s.\n", __FUNCTION__,
                            (value & 1) ? "On" : "Off");
        s->bbcc = value & 0x3f;
        break;

    case PCF_ADCC1:
        s->adcc[0] = value & 0x2f;
        break;

    case PCF_ADCC2:
        if (value & 1)
            pcf_adc_convert(s);
        s->adcc[1] = value;
        break;

    case PCF_ACDC1:
        s->acdc[reg] = value & 0x9e;
        break;

    case PCF_BVMC:
        s->bvmc = value & 0x1e;
        break;

    /* XXX This is not very exact time setting */
    case PCF_RTCSC:
        pcf_rtc_update(s);
        diff = from_bcd(value) - s->rtc.tm.tm_sec;
        s->rtc.sec += diff * 1;
        break;
    case PCF_RTCMN:
        pcf_rtc_update(s);
        diff = from_bcd(value) - s->rtc.tm.tm_min;
        s->rtc.sec += diff * 60;
        break;
    case PCF_RTCHR:
        pcf_rtc_update(s);
        diff = from_bcd(value) - s->rtc.tm.tm_hour;
        s->rtc.sec += diff * 60 * 60;
        break;
    case PCF_RTCWD:
        pcf_rtc_update(s);
        diff = (value & 7) - s->rtc.tm.tm_wday;
        s->rtc.sec += diff * 60 * 60 * 24;
        break;
    case PCF_RTCDT:
        pcf_rtc_update(s);
        diff = from_bcd(value) - s->rtc.tm.tm_mday;
        s->rtc.sec += diff * 60 * 60 * 24;
        break;
    case PCF_RTCMT:
        pcf_rtc_update(s);
        diff = from_bcd(value) - s->rtc.tm.tm_mon - 1;
        s->rtc.sec += diff * 60 * 60 * 24 * 30;
        break;
    case PCF_RTCYR:
        pcf_rtc_update(s);
        diff = from_bcd(value) - (s->rtc.tm.tm_year % 100);
        s->rtc.sec += diff * 60 * 60 * 24 * 365;
        break;

    case PCF_RTCSCA:
        s->rtc.alm_sec = value & 0x7f;
        break;
    case PCF_RTCMNA:
        s->rtc.alm_min = value & 0x7f;
        break;
    case PCF_RTCHRA:
        s->rtc.alm_hour = value & 0x3f;
        break;
    case PCF_RTCWDA:
        s->rtc.alm_wday = value & 0x07;
        break;
    case PCF_RTCDTA:
        s->rtc.alm_mday = value & 0x3f;
        break;
    case PCF_RTCMTA:
        s->rtc.alm_mon = value & 0x1f;
        break;
    case PCF_RTCYRA:
        s->rtc.alm_year = value & 0x7f;
        break;

    case PCF_PWMC1:
        s->pwmc[reg] = value;
        break;

    case PCF_LEDC2: reg ++;
    case PCF_LEDC1:
        if ((s->ledc[reg] ^ value) & (1 << 7))
            printf("%s: LED %i %s.\n", __FUNCTION__,
                            reg + 1, (value >> 7) ? "On" : "Off");
        s->ledc[reg] = value;
        break;

    case PCF_GPOC1:
        pcf_gpo_set(s, 0, (value >> 0) & 7, (value >> 3) & 1);
        pcf_gpo_set(s, 1, (value >> 4) & 7, (value >> 7) & 1);
        s->gpoc[0] = value & 0x7f;
        break;

    case PCF_GPOC2:
        pcf_gpo_set(s, 2, (value >> 0) & 7, (~value >> 6) & 1);
        s->gpoc[1] = value & 0x3f;
        break;

    case PCF_GPOC3:
        pcf_gpo_set(s, 3, (value >> 0) & 7, (~value >> 6) & 1);
        s->gpoc[2] = value & 0x3f;
        break;

    case PCF_GPOC4:
        pcf_gpo_set(s, 4, (value >> 0) & 7, (~value >> 6) & 1);
        s->gpoc[3] = value & 0x3f;
        break;

    case PCF_GPOC5:
        pcf_gpo_set(s, 5, (value >> 0) & 7, (~value >> 6) & 1);
        s->gpoc[4] = value & 0x3f;
        break;

    default:
#ifdef VERBOSE
        printf("%s: unknown register %02x\n", __FUNCTION__, addr);
#endif
    }
}

static void pcf_event(i2c_slave *i2c, enum i2c_event event)
{
    struct pcf_s *s = (struct pcf_s *) i2c;

    if (event == I2C_START_SEND)
        s->firstbyte = 1;
}

static int pcf_tx(i2c_slave *i2c, uint8_t data)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    /* Interpret register address byte */
    if (s->firstbyte) {
        s->reg = data;
        s->firstbyte = 0;
    } else
        pcf_write(s, s->reg ++, data);

    return 0;
}

static int pcf_rx(i2c_slave *i2c)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    return pcf_read(s, s->reg ++);
}

static void pcf_onkey1s(void *opaque)
{
    struct pcf_s *s = (struct pcf_s *) opaque;

    s->intr[0] |= 1 << 2;	/* set ONKEY1S */
    pcf_update(s);
    /* Does the timer start when INTR0[ONKEY1S] is cleared instead? */
    qemu_mod_timer(s->onkeylow, qemu_get_clock(vm_clock) + ticks_per_sec);
}

static void pcf_save(QEMUFile *f, void *opaque)
{
    struct pcf_s *s = (struct pcf_s *) opaque;
    qemu_put_be32(f, s->firstbyte);
    qemu_put_be32(f, s->charging);
    qemu_put_8s(f, &s->reg);
    qemu_put_8s(f, &s->oocs);
    qemu_put_8s(f, &s->oocc[0]);
    qemu_put_8s(f, &s->oocc[1]);
    qemu_put_8s(f, &s->intr[0]);
    qemu_put_8s(f, &s->intr[1]);
    qemu_put_8s(f, &s->intr[2]);
    qemu_put_8s(f, &s->intm[0]);
    qemu_put_8s(f, &s->intm[1]);
    qemu_put_8s(f, &s->intm[2]);
    qemu_put_8s(f, &s->pssc);
    qemu_put_8s(f, &s->pwrokm);
    qemu_put_8s(f, &s->dcdc[0]);
    qemu_put_8s(f, &s->dcdc[1]);
    qemu_put_8s(f, &s->dcdc[2]);
    qemu_put_8s(f, &s->dcdc[3]);
    qemu_put_8s(f, &s->dcdec[0]);
    qemu_put_8s(f, &s->dcdec[1]);
    qemu_put_8s(f, &s->dcudc[0]);
    qemu_put_8s(f, &s->dcudc[1]);
    qemu_put_8s(f, &s->ioregc);
    qemu_put_8s(f, &s->dregc[0]);
    qemu_put_8s(f, &s->dregc[1]);
    qemu_put_8s(f, &s->dregc[2]);
    qemu_put_8s(f, &s->lpregc[0]);
    qemu_put_8s(f, &s->lpregc[1]);
    qemu_put_8s(f, &s->mbcc[0]);
    qemu_put_8s(f, &s->mbcc[1]);
    qemu_put_8s(f, &s->mbcc[2]);
    qemu_put_8s(f, &s->bbcc);
    qemu_put_8s(f, &s->adcc[0]);
    qemu_put_8s(f, &s->adcc[1]);
    qemu_put_8s(f, &s->adcs[0]);
    qemu_put_8s(f, &s->adcs[1]);
    qemu_put_8s(f, &s->adcs[2]);
    qemu_put_8s(f, &s->acdc[0]);
    qemu_put_8s(f, &s->bvmc);
    qemu_put_8s(f, &s->pwmc[0]);
    qemu_put_8s(f, &s->ledc[0]);
    qemu_put_8s(f, &s->ledc[1]);
    qemu_put_8s(f, &s->gpoc[0]);
    qemu_put_8s(f, &s->gpoc[1]);
    qemu_put_8s(f, &s->gpoc[2]);
    qemu_put_8s(f, &s->gpoc[3]);
    qemu_put_8s(f, &s->gpoc[4]);
    qemu_put_be32(f, s->ts.x);
    qemu_put_be32(f, s->ts.y);
    qemu_put_be32s(f, &s->rtc.sec);
    qemu_put_8s(f, &s->rtc.alm_sec);
    qemu_put_8s(f, &s->rtc.alm_min);
    qemu_put_8s(f, &s->rtc.alm_hour);
    qemu_put_8s(f, &s->rtc.alm_wday);
    qemu_put_8s(f, &s->rtc.alm_mday);
    qemu_put_8s(f, &s->rtc.alm_mon);
    qemu_put_8s(f, &s->rtc.alm_year);
    qemu_put_8s(f, &s->gpo);
    i2c_slave_save(f, &s->i2c);
}

static int pcf_load(QEMUFile *f, void *opaque, int version_id)
{
    struct pcf_s *s = (struct pcf_s *) opaque;
    s->firstbyte = qemu_get_be32(f);
    s->charging = qemu_get_be32(f);
    qemu_get_8s(f, &s->reg);
    qemu_get_8s(f, &s->oocs);
    qemu_get_8s(f, &s->oocc[0]);
    qemu_get_8s(f, &s->oocc[1]);
    qemu_get_8s(f, &s->intr[0]);
    qemu_get_8s(f, &s->intr[1]);
    qemu_get_8s(f, &s->intr[2]);
    qemu_get_8s(f, &s->intm[0]);
    qemu_get_8s(f, &s->intm[1]);
    qemu_get_8s(f, &s->intm[2]);
    qemu_get_8s(f, &s->pssc);
    qemu_get_8s(f, &s->pwrokm);
    qemu_get_8s(f, &s->dcdc[0]);
    qemu_get_8s(f, &s->dcdc[1]);
    qemu_get_8s(f, &s->dcdc[2]);
    qemu_get_8s(f, &s->dcdc[3]);
    qemu_get_8s(f, &s->dcdec[0]);
    qemu_get_8s(f, &s->dcdec[1]);
    qemu_get_8s(f, &s->dcudc[0]);
    qemu_get_8s(f, &s->dcudc[1]);
    qemu_get_8s(f, &s->ioregc);
    qemu_get_8s(f, &s->dregc[0]);
    qemu_get_8s(f, &s->dregc[1]);
    qemu_get_8s(f, &s->dregc[2]);
    qemu_get_8s(f, &s->lpregc[0]);
    qemu_get_8s(f, &s->lpregc[1]);
    qemu_get_8s(f, &s->mbcc[0]);
    qemu_get_8s(f, &s->mbcc[1]);
    qemu_get_8s(f, &s->mbcc[2]);
    qemu_get_8s(f, &s->bbcc);
    qemu_get_8s(f, &s->adcc[0]);
    qemu_get_8s(f, &s->adcc[1]);
    qemu_get_8s(f, &s->adcs[0]);
    qemu_get_8s(f, &s->adcs[1]);
    qemu_get_8s(f, &s->adcs[2]);
    qemu_get_8s(f, &s->acdc[0]);
    qemu_get_8s(f, &s->bvmc);
    qemu_get_8s(f, &s->pwmc[0]);
    qemu_get_8s(f, &s->ledc[0]);
    qemu_get_8s(f, &s->ledc[1]);
    qemu_get_8s(f, &s->gpoc[0]);
    qemu_get_8s(f, &s->gpoc[1]);
    qemu_get_8s(f, &s->gpoc[2]);
    qemu_get_8s(f, &s->gpoc[3]);
    qemu_get_8s(f, &s->gpoc[4]);
    s->ts.x = qemu_get_be32(f);
    s->ts.y = qemu_get_be32(f);
    qemu_get_be32s(f, &s->rtc.sec);
    qemu_get_8s(f, &s->rtc.alm_sec);
    qemu_get_8s(f, &s->rtc.alm_min);
    qemu_get_8s(f, &s->rtc.alm_hour);
    qemu_get_8s(f, &s->rtc.alm_wday);
    qemu_get_8s(f, &s->rtc.alm_mday);
    qemu_get_8s(f, &s->rtc.alm_mon);
    qemu_get_8s(f, &s->rtc.alm_year);
    qemu_get_8s(f, &s->gpo);
    i2c_slave_load(f, &s->i2c);
    return 0;
}

static int pcf_iid = 0;

i2c_slave *pcf5060x_init(i2c_bus *bus, qemu_irq irq, int tsc)
{
    struct pcf_s *s = (struct pcf_s *)
            i2c_slave_init(bus, 0, sizeof(struct pcf_s));
    s->i2c.event = pcf_event;
    s->i2c.recv = pcf_rx;
    s->i2c.send = pcf_tx;

    s->irq = irq;
    s->rtc.hz = qemu_new_timer(rt_clock, pcf_rtc_hz, s);
    s->onkeylow = qemu_new_timer(vm_clock, pcf_onkey1s, s);

    pcf_reset(&s->i2c);
    s->rtc.next = qemu_get_clock(rt_clock) + 1000;
    qemu_mod_timer(s->rtc.hz, s->rtc.next);

    if (tsc) {
        /* We want absolute coordinates */
        qemu_add_mouse_event_handler(pcf_adc_event, s, 1,
                        "QEMU PCF50606-driven Touchscreen");
    }

    register_savevm("pcf5060x", pcf_iid ++, 0, pcf_save, pcf_load, s);

    return &s->i2c;
}

void pcf_gpo_handler_set(i2c_slave *i2c, int line, qemu_irq handler)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    if (line >= 6 || line < 0) {
        fprintf(stderr, "%s: No GPO line %i\n", __FUNCTION__, line);
        exit(-1);
    }
    s->gpo_handler[line] = handler;
}

void pcf_onkey_set(i2c_slave *i2c, int level)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    if (level) {
        s->oocs |= 1 << 0;		/* set ONKEY */
        s->intr[0] |= 1 << 0;		/* set ONKEYR */
        qemu_del_timer(s->onkeylow);
    } else {
        s->oocs &= ~(1 << 0);		/* clr ONKEY */
        s->intr[0] |= 1 << 1;		/* set ONKEYF */
        qemu_mod_timer(s->onkeylow, qemu_get_clock(vm_clock) + ticks_per_sec);
    }
    pcf_update(s);
}

void pcf_exton_set(i2c_slave *i2c, int level)
{
    struct pcf_s *s = (struct pcf_s *) i2c;
    if (level) {
        s->oocs |= 1 << 1;		/* set EXTON */
        s->intr[0] |= 1 << 3;		/* set EXTONR */
    } else {
        s->oocs &= ~(1 << 1);		/* clr EXTON */
        s->intr[0] |= 1 << 4;		/* set EXTONF */
    }
    pcf_update(s);
}
