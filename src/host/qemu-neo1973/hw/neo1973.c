/*
 * Neo1973/GTA01 mobile telephone platform emulation.
 * Detailed information at openmoko.org.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licensed under the GNU GPL v2.
 */

#include "vl.h"

#define neo_printf(format, ...)	\
    fprintf(stderr, "%s: " format, __FUNCTION__, ##__VA_ARGS__)

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
#define GTA01_IRQ_PCF50606      S3C_EINT(16)

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

/* Handlers for output ports */
static void neo_bl_switch(int line, int level, void *opaque)
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

static void neo_gpspwr_switch(int line, int level, void *opaque)
{
    neo_printf("GPS powered %s.\n", level ? "up" : "down");
}

static void neo_modem_rst_switch(int line, int level, void *opaque)
{
    if (level)
        neo_printf("Modem reset.\n");
}

static void neo_modem_switch(int line, int level, void *opaque)
{
    neo_printf("Modem powered %s.\n", level ? "up" : "down");
}

static void neo_lcd_rst_switch(int line, int level, void *opaque)
{
    if (level)
        neo_printf("LCD reset.\n");
}

static void neo_vib_switch(int line, int level, void *opaque)
{
    neo_printf("%s.\n", level ? "Buzz, buzz" : "Vibrator stopped");
}

static void neo_gsm_switch(int line, int level, void *opaque)
{
    neo_printf("GSM %sabled.\n", level ? "dis" : "en");
}

static void neo_bt_switch(int line, int level, void *opaque)
{
    neo_printf("Bluetooth transciever %sabled.\n", level ? "en" : "dis");
}

static void neo_gps_switch(int line, int level, void *opaque)
{
    neo_printf("GPS %sV supply is now %s.\n",
#ifdef GTA01Bv3
                    line == GTA01_GPIO_GPS_EN_3V3 ? "3.3" : "3.0",
#else
                    line == GTA01_GPIO_GPS_EN_2V8 ? "2.8" : "3.0",
#endif
                    level ? "on" : "off");
}

static void neo_gps_rst_switch(int line, int level, void *opaque)
{
    if (level)
        neo_printf("GPS reset.\n");
}

/* Handlers for input ports */
static void neo_mmc_cover_switch(void *pic, int in)
{
    pic_set_irq_new(pic, GTA01_IRQ_nSD_DETECT, !in);
}

static void neo_mmc_writeprotect_switch(void *pic, int wp)
{
}

struct neo_kbd_s {
    void *pic;
    struct i2c_slave_s *pmu;
};

/* Hardware keys */
static void neo_kbd_handler(void *opaque, int keycode)
{
    struct neo_kbd_s *kbd = (struct neo_kbd_s *) opaque;
    switch (keycode & 0x7f) {
    case 0x1c:	/* Return */
        pic_set_irq_new(kbd->pic, GTA01_IRQ_911_KEY, !(keycode & 0x80));
        break;
    case 0x39:	/* Space */
        pic_set_irq_new(kbd->pic, GTA01_IRQ_HOLD_KEY, !(keycode & 0x80));
        pcf_onkey_set(kbd->pmu, !(keycode & 80));	/* Active LOW */
        break;
    }
}

static void neo_kbd_init(struct s3c_state_s *cpu, struct i2c_slave_s *pmu)
{
    struct neo_kbd_s *kbd = (struct neo_kbd_s *)
            qemu_mallocz(sizeof(struct neo_kbd_s));
    kbd->pic = cpu->io;
    kbd->pmu = pmu;
    qemu_add_kbd_event_handler(neo_kbd_handler, kbd);
}

static void neo_gpio_setup(struct s3c_state_s *cpu)
{
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_BACKLIGHT,
                    neo_bl_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_GPS_PWRON,
                    neo_gpspwr_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_MODEM_RST,
                    neo_modem_rst_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_MODEM_ON,
                    neo_modem_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_LCD_RESET,
                    neo_lcd_rst_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_VIBRATOR_ON,
                    neo_vib_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_VIBRATOR_ON2,
                    neo_vib_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01v3_GPIO_nGSM_EN,
                    neo_gsm_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01Bv2_GPIO_nGSM_EN,
                    neo_gsm_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_BT_EN,
                    neo_bt_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_GPS_EN_2V8,
                    neo_gps_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_GPS_EN_3V,
                    neo_gps_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_GPS_EN_3V3,
                    neo_gps_switch, cpu);
    s3c_gpio_handler_set(cpu->io, GTA01_GPIO_GPS_RESET,
                    neo_gps_rst_switch, cpu);

    s3c_timers_cmp_handler_set(cpu->timers, 0, neo_bl_intensity, cpu);

    /* MMC/SD host */
    s3c_mmci_handlers(cpu->mmci, cpu->io, neo_mmc_writeprotect_switch,
                    neo_mmc_cover_switch);
}

/* PMB 2520 Hammerhead A-GPS chip */
#define PMB_UART_IRQ_LO	0x7b
#define PMB_UART_IRQ_HI	0x7a
#define PMB_UART_SYNC	0x80
#define PMB_UART_INTACK	0xfa
#define PMB_UART_WAKE	0xfb

/* National Semiconductor LM4857 Boomer audio amplifier */
struct lm4857_s {
    int i2c_dir;
    struct i2c_slave_s i2c;
    uint8_t regs[4];
};

void lm_reset(struct i2c_slave_s *i2c)
{
    struct lm4857_s *s = (struct lm4857_s *) i2c->opaque;
    memset(s->regs, 0, sizeof(s->regs));
}

static void lm_start(void *opaque, int dir)
{
    struct lm4857_s *s = (struct lm4857_s *) opaque;
    s->i2c_dir = dir;
}

static int lm_tx(void *opaque, uint8_t *data, int len)
{
    struct lm4857_s *s = (struct lm4857_s *) opaque;
    int reg, value;
    if (s->i2c_dir)
        return 1;

    while (len --) {
        reg = *data >> 6;
        value = *(data ++) & 0x3f;

        if ((reg == 1 || reg == 2) && ((s->regs[reg] ^ value) & (1 << 5)))
            printf("%s: 3D enhance %s.\n", __FUNCTION__,
                            (value & (1 << 5)) ? "On" : "Off");
        s->regs[reg] = value;
    }
    return 0;
}

struct i2c_slave_s *lm4857_init()
{
    struct lm4857_s *s = qemu_mallocz(sizeof(struct lm4857_s));
    s->i2c.opaque = s;
    s->i2c.tx = lm_tx;
    s->i2c.start = lm_start;

    lm_reset(&s->i2c);

    return &s->i2c;
}

/* I2C bus */
#define NEO_PMU_ADDR	0x08
#define NEO_WM_ADDR	0x1a
#define NEO_AMP_ADDR	0x7c	/* ADR wired to low */

static struct i2c_slave_s *neo_i2c_setup(struct s3c_state_s *cpu)
{
    struct i2c_bus_s *bus = (struct i2c_bus_s *)
            qemu_mallocz(sizeof(struct i2c_bus_s));
    struct i2c_slave_s *pmu;
#ifdef HAS_AUDIO
    struct i2c_slave_s *wm;
    AudioState *audio;
#endif

    pmu = pcf5060x_init(cpu->pic, GTA01_IRQ_PCF50606, 0);

    /* Attach the CPU on one end of our I2C bus.  */
    i2c_master_attach(bus, s3c_i2c_master(cpu->i2c));

    /* Attach a PCF50606 to the bus */
    i2c_slave_attach(bus, NEO_PMU_ADDR, pmu);

    /* Attach a LM4857 to the bus */
    i2c_slave_attach(bus, NEO_AMP_ADDR, lm4857_init());

#ifdef HAS_AUDIO
    audio = AUD_init();
    if (!audio)
        goto done;
    wm = wm8753_init(audio);

    /* Attach a WM8750 to the bus */
    i2c_slave_attach(bus, NEO_WM_ADDR, wm);
    /* .. and to the sound interface.  */
    cpu->i2s->opaque = wm->opaque;
    cpu->i2s->codec_out = wm8753_dac_dat;
    cpu->i2s->codec_in = wm8753_adc_dat;
    wm8753_data_req_set(wm, cpu->i2s->data_req, cpu->i2s);
done:
#endif
    return pmu;
}

static void neo_spi_setup(struct s3c_state_s *cpu)
{
    void *jbt6k74 = jbt6k74_init();

    s3c_spi_attach(cpu->spi, 0, 0, 0, 0);
    s3c_spi_attach(cpu->spi, 1, jbt6k74_txrx, jbt6k74_btxrx, jbt6k74);
}

static void neo_reset(void *opaque)
{
    struct s3c_state_s *cpu = (struct s3c_state_s *) opaque;
    s3c2410_reset(cpu);
    cpu->env->regs[15] = S3C_RAM_BASE;
}

/* Board init.  */
static void neo_init(int ram_size, int vga_ram_size, int boot_device,
                DisplayState *ds, const char **fd_filename, int snapshot,
                const char *kernel_filename, const char *kernel_cmdline,
                const char *initrd_filename)
{
    uint32_t neo_ram = 0x08000000;
    struct s3c_state_s *cpu;
    struct i2c_slave_s *pmu;

    cpu = s3c2410_init(ds);

    /* Setup memory */
    if (ram_size < neo_ram + cpu->free_ram_start) {
        fprintf(stderr, "This platform requires %i bytes of memory\n",
                        neo_ram + cpu->free_ram_start);
        exit(1);
    }
    cpu_register_physical_memory(S3C_RAM_BASE, neo_ram,
                    cpu->free_ram_start | IO_MEM_RAM);
    cpu->free_ram_start += neo_ram;

    s3c_nand_register(cpu, nand_init(NAND_MFR_SAMSUNG, 0x76));

    /* Setup peripherals */
    neo_gpio_setup(cpu);

    pmu = neo_i2c_setup(cpu);

    neo_kbd_init(cpu, pmu);

    neo_spi_setup(cpu);

    qemu_register_reset(neo_reset, cpu);

    /* Setup initial (reset) machine state */
#if 0
    cpu->env->regs[15] = S3C_SRAM_BASE;

    arm_load_kernel(ram_size, kernel_filename, kernel_cmdline,
                    initrd_filename, 0x49e, S3C_RAM_BASE);
#else
    load_image("u-boot.bin",
                    phys_ram_base + S3C_SRAM_SIZE + 0x03f80000);
    load_image(kernel_filename,
                    phys_ram_base + S3C_SRAM_SIZE + 0x02000000);
    cpu->env->regs[15] = S3C_RAM_BASE | 0x03f80000;
#endif

    /* Imitate ONKEY wakeup */
    pcf_onkey_set(pmu, 0);
    pcf_onkey_set(pmu, 1);
    /* Connect the charger */
    pcf_exton_set(pmu, 1);
}

QEMUMachine neo1973_machine = {
    "neo",
    "Neo1973 phone aka FIC GTA01 aka OpenMoko (S3C2410A)",
    neo_init,
};
