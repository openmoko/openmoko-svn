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

#define neo_printf(format, ...)	\
    fprintf(stderr, "%s: " format, __FUNCTION__, ##__VA_ARGS__)

#define GTA01Bv4		1

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
    CharDriverState *modem;
    QEMUTimer *modem_timer;
    qemu_irq *kbd_pic;
    const char *kernel;
    struct sd_card_s *mmc;
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
    neo_printf("GPS powered %s.\n", level ? "up" : "down");
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

    sd_set_cb(s->mmc, 0, qemu_irq_invert(s3c_gpio_in_get(
                                    s->cpu->io)[GTA01_IRQ_nSD_DETECT]));
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

/* I2C bus */
#define NEO_PMU_ADDR	0x08
#define NEO_WM_ADDR	0x1a
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

static void neo_reset(void *opaque)
{
    struct neo_board_s *s = (struct neo_board_s *) opaque;
#if 0
    s->cpu->env->regs[15] = S3C_SRAM_BASE;
#else
    load_image("u-boot.bin", phys_ram_base + 0x03f80000);
    load_image(s->kernel, phys_ram_base + 0x00800000);
    s->cpu->env->regs[15] = S3C_RAM_BASE | 0x03f80000;
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
static void gta01_init(int ram_size, int vga_ram_size, const char *boot_device,
                DisplayState *ds, const char *kernel_filename,
                const char *kernel_cmdline, const char *initrd_filename,
                const char *cpu_model)
{
    struct neo_board_s *s = (struct neo_board_s *)
            qemu_mallocz(sizeof(struct neo_board_s));
    int sd_idx = drive_get_index(IF_SD, 0, 0);

    s->ram = 0x08000000;
    s->kernel = kernel_filename;
    if (sd_idx >= 0)
        s->mmc = sd_init(drives_table[sd_idx].bdrv, 0);

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
}

QEMUMachine gta01_machine = {
    "gta01",
    "FIC Neo1973 rev GTA01 aka OpenMoko phone (S3C2410A)",
    gta01_init,
};
