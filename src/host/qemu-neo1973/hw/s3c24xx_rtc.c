/*
 * Samsung S3C24xx series Real-Time Clock.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

struct s3c_rtc_state_s {
    target_phys_addr_t base;
    qemu_irq irq;
    int enable;
    QEMUTimer *timer;
    QEMUTimer *hz;
    int64_t next;
    struct tm tm;

    uint8_t control;
    uint8_t tick;
    uint8_t alarm;
    uint8_t almsec;
    uint8_t almmin;
    uint8_t almday;
    uint8_t almhour;
    uint8_t almmon;
    uint8_t almyear;
    uint8_t reset;
    uint32_t sec;
};

void s3c_rtc_reset(struct s3c_rtc_state_s *s)
{
    time_t ti;
    s->control = 0x00;
    s->enable = 0;
    s->tick = 0x00;
    s->alarm = 0x00;
    s->almsec = 0x00;
    s->almmin = 0x00;
    s->almhour = 0x00;
    s->almday = 0x01;
    s->almmon = 0x01;
    s->almyear = 0x00;
    s->reset = 0;
    time(&ti);
    s->sec = ti;
}

static void s3c_rtc_tick_mod(struct s3c_rtc_state_s *s)
{
    qemu_mod_timer(s->timer,
                    qemu_get_clock(vm_clock) + muldiv64((s->tick & 0x7f) + 1,
                            ticks_per_sec, S3C_XTAL_FREQ));
}

static void s3c_rtc_tick(void *opaque)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;
    if (!(s->tick & (1 << 7)))
        return;
    qemu_irq_raise(s->irq);

    s3c_rtc_tick_mod(s);
}

static void s3c_rtc_hz(void *opaque)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;

    s->sec ++;
    s->next += 1000;
    qemu_mod_timer(s->hz, s->next);
}

static void s3c_rtc_update(struct s3c_rtc_state_s *s)
{
    void *ret;
    time_t ti = s->sec;
    if (rtc_utc)
        ret = gmtime_r(&ti, &s->tm);
    else
        ret = localtime_r(&ti, &s->tm);
}

static inline uint32_t to_bcd(int val)
{
    return ((val / 10) << 4) | (val % 10);
}

static inline int from_bcd(uint32_t val)
{
    return ((val >> 4) * 10) + (val & 0x0f);
}

#define S3C_RTC_CON	0x40	/* RTC Control register */
#define S3C_RTC_TICNT	0x44	/* Tick Time Count register */
#define S3C_RTC_ALM	0x50	/* RTC Alarm Control register */
#define S3C_RTC_ALMSEC	0x54	/* Alarm Second Data register */
#define S3C_RTC_ALMMIN	0x58	/* Alarm Minute Data register */
#define S3C_RTC_ALMHOUR	0x5c	/* Alarm Hour Data register */
#define S3C_RTC_ALMDATE	0x60	/* Alarm Date Data register */
#define S3C_RTC_ALMMON	0x64	/* Alarm Month Data register */
#define S3C_RTC_ALMYEAR	0x68	/* Alarm Year Data register */
#define S3C_RTC_RST	0x6c	/* RTC Round Reset register */
#define S3C_RTC_BCDSEC	0x70	/* RTC BCD Second register */
#define S3C_RTC_BCDMIN	0x74	/* RTC BCD Minute register */
#define S3C_RTC_BCDHOUR	0x78	/* RTC BCD Hour register */
#define S3C_RTC_BCDDATE	0x7c	/* RTC BCD Day register */
#define S3C_RTC_BCDDAY	0x80	/* RTC BCD Day register */
#define S3C_RTC_BCDMON	0x84	/* RTC BCD Month register */
#define S3C_RTC_BCDYEAR	0x88	/* RTC BCD Year register */

static uint32_t s3c_rtc_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_RTC_CON:
        return s->control;

    case S3C_RTC_TICNT:
        return s->tick;

    case S3C_RTC_ALM:
        return s->alarm;
    case S3C_RTC_ALMSEC:
        return s->almsec;
    case S3C_RTC_ALMMIN:
        return s->almmin;
    case S3C_RTC_ALMHOUR:
        return s->almhour;
    case S3C_RTC_ALMDATE:
        return s->almday;
    case S3C_RTC_ALMMON:
        return s->almmon;
    case S3C_RTC_ALMYEAR:
        return s->almyear;

    case S3C_RTC_RST:
        return s->reset;

    case S3C_RTC_BCDSEC:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_sec);
    case S3C_RTC_BCDMIN:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_min);
    case S3C_RTC_BCDHOUR:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_hour);
    case S3C_RTC_BCDDATE:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_mday);
    case S3C_RTC_BCDDAY:
        s3c_rtc_update(s);
        return s->tm.tm_wday;
    case S3C_RTC_BCDMON:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_mon + 1);
    case S3C_RTC_BCDYEAR:
        s3c_rtc_update(s);
        return to_bcd(s->tm.tm_year % 100);
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_rtc_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;
    int diff;
    addr -= s->base;

    switch (addr) {
    case S3C_RTC_CON:
        s->control = value & 0xf;
        s->enable = (s->control == 0x1);
        break;

    case S3C_RTC_TICNT:
        s->tick = value;
        if (s->tick & (1 << 7))
            s3c_rtc_tick_mod(s);
        break;

    case S3C_RTC_ALM:
        s->alarm = value;
        break;
    case S3C_RTC_ALMSEC:
        s->almsec = value;
        break;
    case S3C_RTC_ALMMIN:
        s->almmin = value;
        break;
    case S3C_RTC_ALMHOUR:
        s->almhour = value;
        break;
    case S3C_RTC_ALMDATE:
        s->almday = value;
        break;
    case S3C_RTC_ALMMON:
        s->almmon = value;
        break;
    case S3C_RTC_ALMYEAR:
        s->almyear = value;
        break;

    case S3C_RTC_RST:
        s->reset = value & 0xf;
        break;

    /* XXX This is not very exact time setting */
    case S3C_RTC_BCDSEC:
        s3c_rtc_update(s);
        diff = from_bcd(value) - s->tm.tm_sec;
        s->sec += diff * 1;
        break;
    case S3C_RTC_BCDMIN:
        s3c_rtc_update(s);
        diff = from_bcd(value) - s->tm.tm_min;
        s->sec += diff * 60;
        break;
    case S3C_RTC_BCDHOUR:
        s3c_rtc_update(s);
        diff = from_bcd(value) - s->tm.tm_hour;
        s->sec += diff * 60 * 60;
        break;
    case S3C_RTC_BCDDATE:
        s3c_rtc_update(s);
        diff = from_bcd(value) - s->tm.tm_mday;
        s->sec += diff * 60 * 60 * 24;
        break;
    case S3C_RTC_BCDDAY:
        s3c_rtc_update(s);
        diff = (value & 7) - s->tm.tm_wday;
        s->sec += diff * 60 * 60 * 24;
        break;
    case S3C_RTC_BCDMON:
        s3c_rtc_update(s);
        diff = from_bcd(value) - s->tm.tm_mon - 1;
        s->sec += diff * 60 * 60 * 24 * 30;
        break;
    case S3C_RTC_BCDYEAR:
        s3c_rtc_update(s);
        diff = from_bcd(value) - (s->tm.tm_year % 100);
        s->sec += diff * 60 * 60 * 24 * 365;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_rtc_readfn[] = {
    s3c_rtc_read,
    s3c_rtc_read,
    s3c_rtc_read,
};

static CPUWriteMemoryFunc *s3c_rtc_writefn[] = {
    s3c_rtc_write,
    s3c_rtc_write,
    s3c_rtc_write,
};

static void s3c_rtc_save(QEMUFile *f, void *opaque)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;
    qemu_put_be64s(f, &s->next);
    qemu_put_8s(f, &s->control);
    qemu_put_8s(f, &s->tick);
    qemu_put_8s(f, &s->alarm);
    qemu_put_8s(f, &s->almsec);
    qemu_put_8s(f, &s->almmin);
    qemu_put_8s(f, &s->almday);
    qemu_put_8s(f, &s->almhour);
    qemu_put_8s(f, &s->almmon);
    qemu_put_8s(f, &s->almyear);
    qemu_put_8s(f, &s->reset);
    qemu_put_be32s(f, &s->sec);
}

static int s3c_rtc_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *) opaque;
    qemu_get_be64s(f, &s->next);
    qemu_get_8s(f, &s->control);
    qemu_get_8s(f, &s->tick);
    qemu_get_8s(f, &s->alarm);
    qemu_get_8s(f, &s->almsec);
    qemu_get_8s(f, &s->almmin);
    qemu_get_8s(f, &s->almday);
    qemu_get_8s(f, &s->almhour);
    qemu_get_8s(f, &s->almmon);
    qemu_get_8s(f, &s->almyear);
    qemu_get_8s(f, &s->reset);
    qemu_get_be32s(f, &s->sec);

    s->enable = (s->control == 0x1);
    s3c_rtc_tick_mod(s);

    return 0;
}

struct s3c_rtc_state_s *s3c_rtc_init(target_phys_addr_t base, qemu_irq irq)
{
    int iomemtype;
    struct s3c_rtc_state_s *s = (struct s3c_rtc_state_s *)
            qemu_mallocz(sizeof(struct s3c_rtc_state_s));

    s->base = base;
    s->irq = irq;
    s->timer = qemu_new_timer(vm_clock, s3c_rtc_tick, s);
    s->hz = qemu_new_timer(rt_clock, s3c_rtc_hz, s);

    s3c_rtc_reset(s);
    s->next = qemu_get_clock(rt_clock) + 1000;
    qemu_mod_timer(s->hz, s->next);

    iomemtype = cpu_register_io_memory(0, s3c_rtc_readfn,
                    s3c_rtc_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_rtc", 0, 0, s3c_rtc_save, s3c_rtc_load, s);

    return s;
}
