/*
 * Neo1973 mobile telephone platforms emulation.
 * Detailed information at openmoko.org.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licensed under the GNU GPL v2.
 */

#include "hw.h"
#include "s3c.h"
#include "arm-misc.h"
#include "sysemu.h"
#include "i2c.h"
#include "qemu-timer.h"
#include "devices.h"
#include "audio/audio.h"
#include "boards.h"
#include "console.h"
#include "usb.h"
#include "net.h"
#include "sd.h"

#define neo_printf(format, ...)	\
    fprintf(stderr, "%s: " format, __FUNCTION__, ##__VA_ARGS__)

#define GTA01Bv4		1

enum {
    NEO1973_GTA01	= 1182,
    NEO1973_GTA02	= 1304,
    NEO1973_GTA02F	= 1555,
};

/* Wiring common to all revisions */
#define GTA01_GPIO_BACKLIGHT	S3C_GPB(0)
#define GTA01_GPIO_GPS_PWRON	S3C_GPB(1)
#define GTA01_GPIO_MODEM_RST	S3C_GPB(6)
#define GTA01_GPIO_MODEM_ON	S3C_GPB(7)
#define GTA01_GPIO_VIBRATOR_ON	S3C_GPB(10)
#define GTA01_GPIO_LCD_RESET	S3C_GPC(6)
#define GTA01_GPIO_JACK_INSERT	S3C_GPF(4)
#define GTA01_GPIO_nSD_DETECT	S3C_GPF(5)
#define GTA01_GPIO_911_KEY	S3C_GPF(6)
#define GTA01_GPIO_HOLD_KEY	S3C_GPF(7)
#define GTA01_GPIO_VIBRATOR_ON2	S3C_GPG(11)

#define GTA01_IRQ_JACK_INSERT	S3C_EINT(4)
#define GTA01_IRQ_nSD_DETECT	S3C_EINT(5)
#define GTA01_IRQ_911_KEY	S3C_EINT(6)
#define GTA01_IRQ_HOLD_KEY	S3C_EINT(7)
#define GTA01_IRQ_PCF50606	S3C_EINT(16)

/* GTA01v3 */
#define GTA01v3_GPIO_nGSM_EN	S3C_GPG(9)

/* GTA01v4 */
#define GTA01_GPIO_MODEM_DNLOAD	S3C_GPG(0)

/* GTA01Bv2 */
#define GTA01Bv2_GPIO_nGSM_EN	S3C_GPF(2)

/* GTA01Bv3 */
#define GTA01_GPIO_GPS_EN_3V3	S3C_GPG(9)

#define GTA01_GPIO_SDMMC_ON	S3C_GPB(2)
#define GTA01_GPIO_BT_EN	S3C_GPB(5)
#define GTA01_GPIO_AB_DETECT	S3C_GPB(8)
#define GTA01_GPIO_USB_PULLUP	S3C_GPB(9)
#define GTA01_GPIO_USB_ATTACH	S3C_GPB(10)

#define GTA01_GPIO_GPS_EN_2V8	S3C_GPG(9)
#define GTA01_GPIO_GPS_EN_3V	S3C_GPG(10)
#define GTA01_GPIO_GPS_RESET	S3C_GPC(0)

/* GTA01Bv4 */
#define GTA01Bv4_GPIO_nNAND_WP	S3C_GPA(16)
#define GTA01Bv4_GPIO_VIBRA_ON	S3C_GPB(3)
#define GTA01Bv4_GPIO_PMU_IRQ	S3C_GPG(1)

#define GTA01Bv4_IRQ_PCF50606	S3C_EINT(9)

struct neo_board_s {
    struct s3c_state_s *cpu;
    unsigned int ram;
    i2c_slave *pmu;
    i2c_slave *wm;
    i2c_slave *lcm;
    i2c_slave *accel[2];
    CharDriverState *modem;
    CharDriverState *gps;
    QEMUTimer *modem_timer;
    qemu_irq *kbd_pic;
    const char *kernel;
    struct sd_card_s *mmc;
    uint32_t id;
};

/* Handlers for output ports */
static void neo_bl_switch(void *opaque, int line, int level)
{
    neo_printf("LCD Backlight now %s.\n", level ? "on" : "off");
}

static void neo_bl_intensity(int line, int level, void *opaque)
{
#if 0
    if (((s->io->bank[1].con >> (line * 2)) & 3) == 2)		/* TOUT0 */
        neo_printf("LCD Backlight now at %i/%i.\n",
                        s->timers->compareb[line], s->timers->countb[line]);
#else
    neo_printf("LCD Backlight now at %i/64.\n", level >> 8);	/* XXX */
#endif
}

static void neo_gpspwr_switch(void *opaque, int line, int level)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;

    neo_printf("GPS powered %s.\n", level ? "up" : "down");

    if (s->gps)
        gps_enable(s->gps, level);
}

static void neo_modem_rst_switch(void *opaque, int line, int level)
{
    if (level)
        neo_printf("Modem reset.\n");
}

#ifdef INTERNAL_MODEM
static void neo_modem_switch_tick(void *opaque)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;
    modem_enable(s->modem, 1);
}
#endif

static void neo_modem_switch(void *opaque, int line, int level)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;

    if (s->modem) {
        /* The GSM modem seems to take a little while to power up and
         * start talking to the serial.  This turns out to be critical because
         * before gsmd runs and disables Local Echo for the UART, everything
         * that the modem outputs is looped back and confuses the parser.
         */
        if (level)
            qemu_mod_timer(s->modem_timer, qemu_get_clock(vm_clock) +
                            (ticks_per_sec >> 4));
        else {
            qemu_del_timer(s->modem_timer);
            modem_enable(s->modem, 0);
        }
    }

    neo_printf("Modem powered %s.\n", level ? "up" : "down");
}

static void neo_lcd_rst_switch(void *opaque, int line, int level)
{
    if (level)
        neo_printf("LCD reset.\n");
}

static void neo_vib_switch(void *opaque, int line, int level)
{
    neo_printf("%s.\n", level ? "Buzz, buzz" : "Vibrator stopped");
}

static void neo_gsm_switch(void *opaque, int line, int level)
{
    neo_printf("GSM %sabled.\n", level ? "dis" : "en");
}

static void neo_bt_switch(void *opaque, int line, int level)
{
    neo_printf("Bluetooth transciever %sabled.\n", level ? "en" : "dis");
}

static void neo_gps_switch(void *opaque, int line, int level)
{
    neo_printf("GPS %sV supply is now %s.\n",
#if defined(GTA01Bv3) || defined (GTA01Bv4)
                    line == GTA01_GPIO_GPS_EN_3V3 ? "3.3" : "3.0",
#else
                    line == GTA01_GPIO_GPS_EN_2V8 ? "2.8" : "3.0",
#endif
                    level ? "on" : "off");
}

static void neo_gps_rst_switch(void *opaque, int line, int level)
{
    if (level)
        neo_printf("GPS reset.\n");
}

/* Handlers for input ports */
static void neo_nand_wp_switch(void *opaque, int line, int level)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;
    s3c_nand_setwp(s->cpu, level);
}

/* Hardware keys */
static void neo_kbd_handler(void *opaque, int keycode)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;
    switch (keycode & 0x7f) {
    case 0x1c:	/* Return */
        qemu_set_irq(s->kbd_pic[GTA01_IRQ_911_KEY], !(keycode & 0x80));
        break;
    case 0x39:	/* Space */
        qemu_set_irq(s->kbd_pic[GTA01_IRQ_HOLD_KEY], !(keycode & 0x80));
        pcf_onkey_set(s->pmu, !(keycode & 0x80));	/* Active LOW */
        break;
    }
}

static void neo_kbd_init(struct neo_board_s *s)
{
    s->kbd_pic = s3c_gpio_in_get(s->cpu->io);
    qemu_add_kbd_event_handler(neo_kbd_handler, s);
}

static void neo_gpio_setup(struct neo_board_s *s)
{
    qemu_irq vib_output = *qemu_allocate_irqs(neo_vib_switch, s, 1);

    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_BACKLIGHT,
                    *qemu_allocate_irqs(neo_bl_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_GPS_PWRON,
                    *qemu_allocate_irqs(neo_gpspwr_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_MODEM_RST,
                    *qemu_allocate_irqs(neo_modem_rst_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_MODEM_ON,
                    *qemu_allocate_irqs(neo_modem_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_LCD_RESET,
                    *qemu_allocate_irqs(neo_lcd_rst_switch, s, 1));
#ifdef GTA01Bv4
    s3c_gpio_out_set(s->cpu->io, GTA01Bv4_GPIO_VIBRA_ON,
                    vib_output);
#else
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_VIBRATOR_ON,
                    vib_output);
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_VIBRATOR_ON2,
                    vib_output);
#endif
    s3c_gpio_out_set(s->cpu->io, GTA01v3_GPIO_nGSM_EN,
                    *qemu_allocate_irqs(neo_gsm_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01Bv2_GPIO_nGSM_EN,
                    *qemu_allocate_irqs(neo_gsm_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_BT_EN,
                    *qemu_allocate_irqs(neo_bt_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_GPS_EN_2V8,
                    *qemu_allocate_irqs(neo_gps_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_GPS_EN_3V,
                    *qemu_allocate_irqs(neo_gps_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_GPS_EN_3V3,
                    *qemu_allocate_irqs(neo_gps_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01_GPIO_GPS_RESET,
                    *qemu_allocate_irqs(neo_gps_rst_switch, s, 1));
    s3c_gpio_out_set(s->cpu->io, GTA01Bv4_GPIO_nNAND_WP,
                    *qemu_allocate_irqs(neo_nand_wp_switch, s, 1));

    s3c_timers_cmp_handler_set(s->cpu->timers, 0, neo_bl_intensity, s);

    if (s->id == NEO1973_GTA01)
        sd_set_cb(s->mmc, 0,
                        s3c_gpio_in_get(s->cpu->io)[GTA01_IRQ_nSD_DETECT]);
}

/* PMB 2520 Hammerhead A-GPS chip */
#define PMB_UART_IRQ_LO	0x7b
#define PMB_UART_IRQ_HI	0x7a
#define PMB_UART_SYNC	0x80
#define PMB_UART_INTACK	0xfa
#define PMB_UART_WAKE	0xfb

/* National Semiconductor LM4857 Boomer audio amplifier */
struct lm4857_s {
    i2c_slave i2c;
    uint8_t regs[4];
};

void lm_reset(i2c_slave *i2c)
{
    struct lm4857_s *s = (struct lm4857_s *) i2c;
    memset(s->regs, 0, sizeof(s->regs));
}

static void lm_event(i2c_slave *i2c, enum i2c_event event)
{
}

static int lm_rx(i2c_slave *i2c)
{
    return 0x00;
}

static int lm_tx(i2c_slave *i2c, uint8_t data)
{
    struct lm4857_s *s = (struct lm4857_s *) i2c;
    int reg, value;

    reg = data >> 6;
    value = data & 0x3f;

    if ((reg == 1 || reg == 2) && ((s->regs[reg] ^ value) & (1 << 5)))
        printf("%s: 3D enhance %s.\n", __FUNCTION__,
                        (value & (1 << 5)) ? "On" : "Off");
    s->regs[reg] = value;
    return 0;
}

static void lm_save(QEMUFile *f, void *opaque)
{
    struct lm4857_s *s = (struct lm4857_s *) opaque;
    qemu_put_buffer(f, s->regs, sizeof(s->regs));
    i2c_slave_save(f, &s->i2c);
}

static int lm_load(QEMUFile *f, void *opaque, int version_id)
{
    struct lm4857_s *s = (struct lm4857_s *) opaque;
    qemu_get_buffer(f, s->regs, sizeof(s->regs));
    i2c_slave_load(f, &s->i2c);
    return 0;
}

i2c_slave *lm4857_init(i2c_bus *bus)
{
    struct lm4857_s *s = (struct lm4857_s *)
            i2c_slave_init(bus, 0, sizeof(struct lm4857_s));
    s->i2c.event = lm_event;
    s->i2c.send = lm_tx;
    s->i2c.recv = lm_rx;

    lm_reset(&s->i2c);
    register_savevm("lm4857", 0, 0, lm_save, lm_load, s);

    return &s->i2c;
}

/* LIS302DL "piccolo" motion sensor/accelerometer */
struct piccolo_s {
    i2c_slave i2c;
    int firstbyte;

    qemu_irq intr[2];

    uint8_t regs[0x40];
    int reg;

    int enable;
    uint64_t interval;

    QEMUTimer *sample;
};

void piccolo_reset(i2c_slave *i2c)
{
    struct piccolo_s *s = (struct piccolo_s *) i2c;

    memset(s->regs, 0, sizeof(s->regs));
    s->regs[0x0f] = 0x3b;	/* Who_Am_I */
    s->regs[0x20] = 0x07;	/* Ctrl_Reg1 */
}

static void piccolo_data_update(struct piccolo_s *s, int mask)
{
    /* TODO: poll the Qemu SDL window position to find 2d acceleration */

    /* 1G */
    int x = 0x00, y = 0x00, z = 0x10;

    if (mask & (1 << 0))
        s->regs[0x29] = x;
    if (mask & (1 << 1))
        s->regs[0x2b] = y;
    if (mask & (1 << 2))
        s->regs[0x2d] = z;
}

static void piccolo_intr_update(struct piccolo_s *s)
{
    int inv = (s->regs[0x22] >> 7) & 1;
    int level;
    int i;

    for (i = 0; i < 6; i += 3) {
        switch ((s->regs[0x22] >> i) & 7) {
        case 0:
        default:
            level = 0;
            break;
        case 1:
            level = (s->regs[0x31] >> 6) & 1;
            break;
        case 2:
            level = (s->regs[0x35] >> 6) & 1;
            break;
        case 3:
            level = ((s->regs[0x31] | s->regs[0x35]) >> 6) & 1;
            break;
        case 4:
            level = !!s->regs[0x27];
            break;
        }

        qemu_set_irq(s->intr[!!i], level ^ inv);
    }
}

static void piccolo_ff_wu_update(struct piccolo_s *s)
{
    /* TODO: update high and low interrupts */
    /* TODO: also update the counter and check treshold */
}

static void piccolo_sample_sched(struct piccolo_s *s)
{
    qemu_mod_timer(s->sample, qemu_get_clock(vm_clock) + s->interval);
}

static void piccolo_sample_tick(void *opaque)
{
    struct piccolo_s *s = (struct piccolo_s *) opaque;
    int nu_data = s->regs[0x20] & 7;

    if (nu_data == 7)
        nu_data |= 8;
    s->regs[0x27] |= nu_data | ((s->regs[0x27] & nu_data) << 4);

    piccolo_data_update(s, nu_data & 7);
    piccolo_ff_wu_update(s);
    piccolo_intr_update(s);
    piccolo_sample_sched(s);
}

static void piccolo_sample_update(struct piccolo_s *s)
{
    int rate = (s->regs[0x20] & (1 << 7)) ? 400 : 100;	/* DR */

    s->enable = (s->regs[0x20] >> 6) & 1;		/* PD */
    if (!s->enable) {
        qemu_del_timer(s->sample);
        return;
    }

    s->interval = ticks_per_sec / rate;
    if (!(s->regs[0x20] & 7))				/* Xen | Yen | Zen */
        qemu_del_timer(s->sample);
    else if (!qemu_timer_pending(s->sample))
        piccolo_sample_sched(s);
}

static void piccolo_event(i2c_slave *i2c, enum i2c_event event)
{
    struct piccolo_s *s = (struct piccolo_s *) i2c;

    if (event == I2C_START_SEND)
        s->firstbyte = 1;
}

static int piccolo_rx(i2c_slave *i2c)
{
    struct piccolo_s *s = (struct piccolo_s *) i2c;
    int reg = s->reg;
    int ret;

    if (reg >= 0x80) {
        reg -= 0x80;
        s->reg ++;
    } else
        s->reg = -1;

    switch (reg) {
    case 0x00 ... 0x0e:
    case 0x10 ... 0x1f:
    case 0x2e ... 0x2f:
    case 0x0f:	/* Who_Am_I */
    case 0x20:	/* Ctrl_Reg1 */
    case 0x21:	/* Ctrl_Reg2 */
    case 0x22:	/* Ctrl_Reg3 */
    case 0x23:	/* HP_filter_reset */
    case 0x27:	/* Status_Reg */
    case 0x29:	/* OutX */
    case 0x2b:	/* OutY */
    case 0x2d:	/* OutZ */
    case 0x30:	/* FF_WU_CFG_1 */
    case 0x32:	/* FF_WU_THS_1 */
    case 0x33:	/* FF_WU_DURATION_1 */
    case 0x34:	/* FF_WU_CFG_2 */
    case 0x36:	/* FF_WU_THS_2 */
    case 0x37:	/* FF_WU_DURATION_2 */
        break;

    case 0x31:	/* FF_WU_SRC_1 */
    case 0x35:	/* FF_WU_SRC_2 */
        ret = s->regs[reg];
        if ((s->regs[reg - 1] >> 6) & 1) {			/* LIR */
            s->regs[reg] = 0;
            piccolo_intr_update(s);
        }
        return ret;

    case 0x38:	/* CLICK_CFG */
    case 0x39:	/* CLICK_SRC */
    case 0x3b:	/* CLICK_THSY_X */
    case 0x3c:	/* CLICK_THSZ */
    case 0x3d:	/* CLICK_timelimit */
    case 0x3e:	/* CLICK_latency */
    case 0x3f:	/* CLICK_window */
    default:
        fprintf(stderr, "%s: unknown register %02x\n", __FUNCTION__, reg);
        return 0x00;
    }

    return s->regs[reg];
}

static int piccolo_tx(i2c_slave *i2c, uint8_t data)
{
    struct piccolo_s *s = (struct piccolo_s *) i2c;
    int reg = s->reg;

    if (s->firstbyte) {
        s->firstbyte = 0;
        s->reg = data;
        return 0;
    }

    if (reg >= 0x80) {
        reg -= 0x80;
        s->reg ++;
    }

    switch (reg) {
    case 0x00 ... 0x0e:
    case 0x10 ... 0x1f:
    case 0x2e ... 0x2f:
        fprintf(stderr, "%s: write to a \"do not modify\" register %02x\n",
                        __FUNCTION__, s->reg);
        fprintf(stderr, "%s: may cause permanent damage!\n", __FUNCTION__);
        break;

    case 0x20:	/* Ctrl_Reg1 */
        s->regs[reg] = data;
        if (data & (3 << 3))
            fprintf(stderr, "%s: Self-Test Enable attempt\n", __FUNCTION__);
        piccolo_sample_update(s);
        break;

    case 0x21:	/* Ctrl_Reg2 */
        if (data & (1 << 6)) {
            /* Memory reboot */
            data &= ~(1 << 6);
        }
        break;

    case 0x22:	/* Ctrl_Reg3 */
        piccolo_intr_update(s);
        break;

    case 0x30:	/* FF_WU_CFG_1 */
    case 0x32:	/* FF_WU_THS_1 */
    case 0x33:	/* FF_WU_DURATION_1 */
    case 0x34:	/* FF_WU_CFG_2 */
    case 0x36:	/* FF_WU_THS_2 */
    case 0x37:	/* FF_WU_DURATION_2 */
        break;

    case 0x38:	/* CLICK_CFG */
    case 0x39:	/* CLICK_SRC */
    case 0x3b:	/* CLICK_THSY_X */
    case 0x3c:	/* CLICK_THSZ */
    case 0x3d:	/* CLICK_timelimit */
    case 0x3e:	/* CLICK_latency */
    case 0x3f:	/* CLICK_window */
    default:
        fprintf(stderr, "%s: unknown register %02x\n", __FUNCTION__, reg);
        return 1;
    }

    s->regs[reg] = data;
    return 0;
}

static void piccolo_save(QEMUFile *f, void *opaque)
{
    struct piccolo_s *s = (struct piccolo_s *) opaque;

    qemu_put_buffer(f, s->regs, sizeof(s->regs));
    i2c_slave_save(f, &s->i2c);
}

static int piccolo_load(QEMUFile *f, void *opaque, int version_id)
{
    struct piccolo_s *s = (struct piccolo_s *) opaque;

    qemu_get_buffer(f, s->regs, sizeof(s->regs));
    i2c_slave_load(f, &s->i2c);

    piccolo_sample_update(s);
    return 0;
}

i2c_slave *lis302dl_init(i2c_bus *bus, qemu_irq int1, qemu_irq int2)
{
    struct piccolo_s *s = (struct piccolo_s *)
            i2c_slave_init(bus, 0, sizeof(struct piccolo_s));
    s->i2c.event = piccolo_event;
    s->i2c.send = piccolo_tx;
    s->i2c.recv = piccolo_rx;

    s->intr[0] = int1;
    s->intr[1] = int2;
    s->sample = qemu_new_timer(vm_clock, piccolo_sample_tick, s);

    piccolo_reset(&s->i2c);
    register_savevm("lis302dl", 0, 0, piccolo_save, piccolo_load, s);

    return &s->i2c;
}

/* I2C bus */
#define NEO_PMU_ADDR	0x08
#define NEO_WM_ADDR	0x1a
#define NEO_ACCEL_ADDR0	0x1c
#define NEO_ACCEL_ADDR1	0x1d
#define NEO_AMP_ADDR	0x7c	/* ADR wired to low */

static void neo_i2c_setup(struct neo_board_s *s)
{
    /* Attach the CPU on one end of our I2C bus.  */
    i2c_bus *bus = s3c_i2c_bus(s->cpu->i2c);
#ifdef HAS_AUDIO
    AudioState *audio;
#endif

    /* Attach a PCF50606 to the bus */
    s->pmu = pcf5060x_init(bus,
#ifdef GTA01Bv4
                    s3c_gpio_in_get(s->cpu->io)[GTA01Bv4_IRQ_PCF50606], 0);
#else
                    s3c_gpio_in_get(s->cpu->io)[GTA01_IRQ_PCF50606], 0);
#endif
    i2c_set_slave_address(s->pmu, NEO_PMU_ADDR);

    /* Attach a LM4857 to the bus */
    s->lcm = lm4857_init(bus);
    i2c_set_slave_address(s->lcm, NEO_AMP_ADDR);

    /* Attach two LIS302DL slaves to the bus */
    if (s->id == NEO1973_GTA02 || s->id == NEO1973_GTA02F) {
        s->accel[0] = lis302dl_init(bus, 0, 0);	/* TODO: connect IRQs */
        s->accel[1] = lis302dl_init(bus, 0, 0);	/* TODO: connect IRQs */
        i2c_set_slave_address(s->accel[0], NEO_ACCEL_ADDR0);
        i2c_set_slave_address(s->accel[1], NEO_ACCEL_ADDR1);
    }

#ifdef HAS_AUDIO
    audio = AUD_init();
    if (!audio)
        return;

    /* Attach a WM8750 to the bus */
    s->wm = wm8753_init(bus, audio);
    i2c_set_slave_address(s->wm, NEO_WM_ADDR);

    /* .. and to the sound interface.  */
    s->cpu->i2s->opaque = s->wm;
    s->cpu->i2s->codec_out = wm8753_dac_dat;
    s->cpu->i2s->codec_in = wm8753_adc_dat;
    wm8753_data_req_set(s->wm, s->cpu->i2s->data_req, s->cpu->i2s);
#endif
}

static void neo_spi_setup(struct neo_board_s *s)
{
    void *jbt6k74 = jbt6k74_init();

    s3c_spi_attach(s->cpu->spi, 0, 0, 0, 0);
    s3c_spi_attach(s->cpu->spi, 1, jbt6k74_txrx, jbt6k74_btxrx, jbt6k74);
}

static void neo_gsm_setup(struct neo_board_s *s)
{
#ifdef INTERNAL_MODEM
    s->modem = modem_init();
    s->modem_timer = qemu_new_timer(vm_clock, neo_modem_switch_tick, s);

    s3c_uart_attach(s->cpu->uart[0], s->modem);
#endif
}

static void neo_gps_setup(struct neo_board_s *s)
{
    s->gps = gps_antaris_serial_init();

    s3c_uart_attach(s->cpu->uart[1], s->gps);
}

static void neo_reset(void *opaque)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;
#if 0
    s->cpu->env->regs[15] = S3C_SRAM_BASE;
#else
    load_image("u-boot.bin", phys_ram_base + 0x03f80000);
    load_image(s->kernel, phys_ram_base + 0x00100000);

    s->cpu->env->regs[15] = S3C_RAM_BASE | 0x03f80000;

    if (strstr(s->kernel, "u-boot")) {	/* FIXME */
        /* Exploit preboot-override to set up an initial environment */
        stl_raw(phys_ram_base + 0x03f80040, S3C_RAM_BASE | 0x000fff00);
        strcpy(phys_ram_base + 0x000fff00,
                        "setenv stdin serial; "
                        "setenv stdout serial; "
                        "setenv stderr serial; ");
        /* Disable ENV pre-load */
        stl_raw(phys_ram_base + 0x03f80044, 0x00000000);
    }
#endif

    /* Imitate ONKEY wakeup */
    pcf_onkey_set(s->pmu, 0);
    pcf_onkey_set(s->pmu, 1);
    /* Connect the charger */
    pcf_exton_set(s->pmu, 1);
}

/* Typical touchscreen calibration values */
static const int gta01_ts_scale[6] = {
    0, (90 - 960) * 256 / 1021, -90 * 256 * 32,
    (940 - 75) * 256 / 1021, 0, 75 * 256 * 32,
};

/* Board init.  */
static struct neo_board_s *neo1973_init_common(int ram_size, DisplayState *ds,
                const char *kernel_filename, const char *cpu_model,
                struct sd_card_s *mmc, uint32_t id)
{
    struct neo_board_s *s = (struct neo_board_s *)
            qemu_mallocz(sizeof(struct neo_board_s));

    s->ram = 0x08000000;
    s->kernel = kernel_filename;
    s->mmc = mmc;
    s->id = id;

    /* Setup CPU & memory */
    if (ram_size < s->ram + S3C_SRAM_SIZE) {
        fprintf(stderr, "This platform requires %i bytes of memory\n",
                        s->ram + S3C_SRAM_SIZE);
        exit(1);
    }
    if (cpu_model && strcmp(cpu_model, "arm920t")) {
        fprintf(stderr, "This platform requires an ARM920T core\n");
        exit(2);
    }
    s->cpu = s3c2410_init(s->ram, ds, s->mmc);

    s3c_nand_register(s->cpu, nand_init(NAND_MFR_SAMSUNG, 0x76));

    /* Setup peripherals */
    neo_gpio_setup(s);

    neo_i2c_setup(s);

    neo_kbd_init(s);

    neo_spi_setup(s);

    neo_gsm_setup(s);

    if (usb_enabled)
        usb_device_attach(usb_bt_init(local_piconet));

    s3c_adc_setscale(s->cpu->adc, gta01_ts_scale);

    /* Setup initial (reset) machine state */
    qemu_register_reset(neo_reset, s);
#if 0
    arm_load_kernel(s->ram, kernel_filename, kernel_cmdline,
                    initrd_filename, 0x49e, S3C_RAM_BASE);
#endif
    neo_reset(s);

    dpy_resize(ds, 480, 640);

    return s;
}

static uint32_t neo_machid_read(void *opaque, target_phys_addr_t addr)
{
    return 'Q';
}

static void neo_machid_write(void *opaque,
                target_phys_addr_t addr, uint32_t val)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;

    if ((val >> 4) == 'Q')
        s->cpu->env->regs[val & 0xf] = s->id;
}

static void neo_machid_init(struct neo_board_s *s)
{
    static CPUReadMemoryFunc *readfn[] = {
        neo_machid_read, neo_machid_read, neo_machid_read,
    };
    static CPUWriteMemoryFunc *writefn[] = {
        neo_machid_write, neo_machid_write, neo_machid_write,
    };
    int iomemtype = cpu_register_io_memory(0, readfn, writefn, s);

    /* SROM chipselect 5 base */
    cpu_register_physical_memory(0x28000000, 1, iomemtype);
}

static void gta01_init(int ram_size, int vga_ram_size,
                const char *boot_device, DisplayState *ds,
                const char *kernel_filename, const char *kernel_cmdline,
                const char *initrd_filename, const char *cpu_model)
{
    int sd_idx = drive_get_index(IF_SD, 0, 0);
    struct sd_card_s *sd = 0;

    if (sd_idx >= 0)
        sd = sd_init(drives_table[sd_idx].bdrv, 0);

    neo1973_init_common(ram_size, ds,
                    kernel_filename, cpu_model, sd, NEO1973_GTA01);
}

static void gta02f_init(int ram_size, int vga_ram_size,
                const char *boot_device, DisplayState *ds,
                const char *kernel_filename, const char *kernel_cmdline,
                const char *initrd_filename, const char *cpu_model)
{
    struct neo_board_s *neo;
    struct sd_card_s *sd = ar6k_init(&nd_table[0]);

    neo = neo1973_init_common(ram_size, ds,
                    kernel_filename, cpu_model, sd, NEO1973_GTA02F);

    neo_gps_setup(neo);
    neo_machid_init(neo);
}

QEMUMachine gta01_machine = {
    "gta01",
    "FIC Neo1973 rev GTA01 aka OpenMoko phone (S3C2410A)",
    gta01_init,
};

QEMUMachine gta02f_machine = {
    "gta02fake",
    "Paravirtual FIC Neo1973 rev GTA02 (S3C2410A)",
    gta02f_init,
};
